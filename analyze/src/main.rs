extern crate byteorder;
#[macro_use]
extern crate bitflags;

use std::path::Path;
use std::fs::File;
use std::ffi::OsString;
use std::io::{BufReader,Result as IoResult,Error as IoError,ErrorKind as IoErrorKind};

use std::collections::HashMap;

#[derive(Debug)]
enum TranslateError {
	/// The virt address was not canonical
	NotCanonical,
	/// There was no mapping at the given table level (0-3)
	NotPresent(u8),
	/// This page containing a page table was missing from the page table dump
	Missing(u64),
}

mod peflags {
	bitflags! {
		flags PageEntryFlags: u64 {
			const P       = 1 <<  0,
			const RW      = 1 <<  1,
			const US      = 1 <<  2,
			const PWT     = 1 <<  3,
			const PCD     = 1 <<  4,
			const A       = 1 <<  5,
			const D       = 1 <<  6,
			const PS_PAT  = 1 <<  7,
			const G       = 1 <<  8,
			const PAT     = 1 << 12,
			const XD      = 1 << 63,
		}
	}
}

struct PageTables {
	base: u64,
	ptmem: HashMap<u64,Vec<u64>>,
}

struct Translation {
	phys: u64,
	flags: [peflags::PageEntryFlags;4],
}

macro_rules! try_break {
	($expr:expr) => (
		match $expr {
			Err(byteorder::Error::UnexpectedEOF) => break,
			other => try!(other),
		}
	)
}

impl PageTables {
	fn load<P: AsRef<Path>>(path: P) -> IoResult<PageTables> {
		use byteorder::{LittleEndian,ReadBytesExt};

		let mut file=BufReader::new(try!(File::open(path)));
		let mut mem=HashMap::new();
		let mut base=None;

		loop {
			let address=try_break!(file.read_u64::<LittleEndian>());
			if base.is_none() { base=Some(address) };
			let mut pte=Vec::with_capacity(512);
			for _ in 0..512 {
				pte.push(try_break!(file.read_u64::<LittleEndian>()));
			}
			mem.insert(address,pte);
		}

		match base {
			Some(base) => Ok(PageTables{base:base,ptmem:mem}),
			None => Err(IoError::new(IoErrorKind::InvalidData, "Truncated file")),
		}
	}

	fn get_shift(level: u8) -> u8 {
		if level>3 { panic!("Level must be 0, 1, 2 or 3!") }
		12+9*(3-level)
	}

	fn get_index(virt: u64, level: u8) -> usize {
		(virt>>Self::get_shift(level)&0x1ff) as usize
	}

	fn get_offset(virt: u64, level: u8) -> u64 {
		virt&((!0u64)>>(64-Self::get_shift(level)))
	}

	fn translate(&self, virt: u64) -> Result<Translation,TranslateError> {
		if virt>=0x0000_8000_0000_0000 && virt<0xffff_0000_0000_0000 {
			return Err(TranslateError::NotCanonical);
		}

		let mut base=self.base;
		let mut ret_flags=[peflags::PageEntryFlags::from_bits_truncate(0);4];
		for &level in &[0,1,2,3] {
			let pt=match self.ptmem.get(&base) {
				None => return Err(TranslateError::Missing(base)),
				Some(pt) => pt,
			};
			let pte=pt[Self::get_index(virt,level)];
			let flags=peflags::PageEntryFlags::from_bits_truncate(pte);
			ret_flags[level as usize]=flags&!peflags::PAT;
			base=pte&0x000f_ffff_ffff_f000;
			if !flags.contains(peflags::P) {
				return Err(TranslateError::NotPresent(level));
			} else if flags.contains(peflags::PS_PAT) || level==3 {
				if level!=3 { ret_flags[level as usize]=ret_flags[level as usize]|(flags&peflags::PAT) }
				return Ok(Translation{
					phys: base+Self::get_offset(virt,level),
					flags: ret_flags,
				});
			}
		}
		unreachable!()
	}
}

const USAGE: &'static str = "Usage: ptanalyze <hex-address> <hex-size> <pt-dump-file>\n";

fn hex_arg(opt: Option<OsString>,name: &str) -> u64 {
	u64::from_str_radix(
		&opt.expect(&format!("{}\nMust supply {} on command line!",USAGE,name))
		.into_string().expect(&format!("{}\nMust supply valid numeric {}!",USAGE,name)),
	16).expect(&format!("{}\nMust supply valid hexadecimal {} (no prefix)!",USAGE,name))
}

fn reduce_flags(flags: &[peflags::PageEntryFlags;4]) -> peflags::PageEntryFlags {
	let rw=flags.iter().fold(peflags::RW,|m,&f|m&f&peflags::RW);
	let us=flags.iter().fold(peflags::US,|m,&f|m&f&peflags::US);
	let xd=flags.iter().fold(peflags::PageEntryFlags::empty(),|m,&f|m|(f&peflags::XD));
	rw|us|xd
}

fn main() {
	let mut args=std::env::args_os();
	args.next();
	let addr=hex_arg(args.next(),"address");
	let size=hex_arg(args.next(),"size");
	let file=args.next().expect(&format!("{}\nMust supply filename on command line!",USAGE));

	let pt=PageTables::load(file).expect("Failed to load input file!");

	let mut cur=addr;
	let end=addr+size;
	while cur<end {
		match pt.translate(cur) {
			Ok(trans) =>	println!("{:x} {:x} {:?}",cur,trans.phys,reduce_flags(&trans.flags)),
			Err(err) =>	println!("{:x} {:?}",cur,err),
		};
		cur+=0x1000;
	}
}
