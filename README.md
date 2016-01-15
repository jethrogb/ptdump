# ptdump

Dump page tables on various OSes and analyze them.

## Dumping

Currently only dumping of x86-64 page tables is supported, and only on Linux
and Windows.

### Linux

There is a Linux kernel module that creates special procfs files which will
dump the page tables of the accessing process. The file is in
`/proc/page_table_N`, where `N` is the depth to dump (3=all levels).

To build and load:

```sh
make
sudo insmod ptdump.ko
```

The process that you want to dump the page tables for needs to read the file.

### Windows

The current Windows page table dumper is built on top of WinDbg. This requires
you to boot in debug mode (`bcdedit /debug`), but it does not require the
installation of an untrusted driver. Unfortunately, this setup is quite slow.
It does allow dumping the page tables of any running process.

To build and use:

- Set the correct path to `kd.exe` on line 3 of main.cpp
- Build the Visual C++ project
- Run `ptdump.exe <pid> [output-file]`.

## Dump format

The dump format is very simplistic, consisting of 4104 byte records. The first
8 bytes of each record contain the physical address of a page. The next 4096
bytes represent that page, that is, 512 8-byte page table entries. The records
are in no particular order.

## Analyzing

The rust program `ptanalyze` will take a page table dump and resolve all pages
in an address range, printing the virtual and physical addresses as well as the
applicable access flags.

To build and load:

```sh
cargo build
cargo run -- <address> <length> <pt-dump-file>
```

The `address` and `length` need to be supplied in hexadecimal with the `0x`
prefix.
