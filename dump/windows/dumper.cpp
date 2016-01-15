/*
 * Page table dumper for Windows
 *
 * (C) Copyright 2016 Jethro G. Beekman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#include "stdafx.h"

#include "ptdump-common.h"

static void trim(string& str) {
	size_t s=str.find_first_not_of(" \t\r\n");
	if (s==string::npos) {
		str.clear();
		return;
	}
	size_t e=str.find_last_not_of(" \t\r\n");
	if (e!=string::npos) {
		e-=s;
		e++;
	}
	str=str.substr(s,e);
}

static string int2hex(ullong i) {
	char id[32];
	sprintf_s(id,"0x%llx",i);
	return string(id);
}

static ullong hex2int_at(const string& str, size_t startpos, size_t* endpos=NULL) {
	startpos=str.find_first_not_of("` ",startpos);
	size_t e=str.find_first_not_of("0123456789abcdefABCDEF`xX",startpos);
	if (endpos) *endpos=e;
	string val=str.substr(startpos,e-startpos);

	for (size_t i=0;i<val.length();i++) {
		if (val[i]=='`' || val[i]=='x' || val[i]=='X') {
			val.erase(i,1);
			i--;
			continue;
		} else if (val[i]==' ') {
			val.erase(i);
			break;
		}
	}

	return _STRTO_ULL(val.c_str(),NULL,16);
}

Dumper::Dumper(ChildProcess& child, unsigned pid, ostream& out) :
	m_child(child), m_out(out), m_pid(pid)
{
}

extern "C" void output(void* p, void* data, ullong len) { ((Dumper*)p)->output(data,len); }
void Dumper::output(void* data, ullong len) {
	m_out.write((const char*)data,len);
}

extern "C" ullong get_base(void* p) { return ((Dumper*)p)->get_base(); }
ullong Dumper::get_base() {
	cerr << "Obtaining base address\n";
	if (m_child.WriteLine(string("!process ")+int2hex(m_pid))) {
		string out=m_child.ReadUntil("lkd>");
		size_t s=out.find("DirBase: ");
		if (s==string::npos) return 0;
		return hex2int_at(out,s+9);
	}

	return 0;
}

extern "C" ullong* read_page(void* p, ullong address) { return ((Dumper*)p)->read_page(address); }
ullong* Dumper::read_page(ullong address) {
	if (m_pages_seen.count(address)) return NULL;
	fprintf(stderr,"Reading page %llx\n",address);
	ullong* page=new ullong[512];

	if (m_child.WriteLine(string("!dq ")+int2hex(address)+" L 0x200")) {
		string out=m_child.ReadUntil("lkd>");
		istringstream dump(out);

		string line;
		size_t curp=0;
		for (getline(dump,line);dump.good();getline(dump,line)) {
			trim(line);
			if (line.length()==0) continue;
			if (line[0]!='#') continue;
			if (line.substr(0,4)=="lkd>") break;
			if (curp>=512) {
				cerr << "Too much data:"<<line;
				break;
			}
			size_t p;
			hex2int_at(line,1,&p);
			page[curp++]=hex2int_at(line,p,&p);
			page[curp++]=hex2int_at(line,p,NULL);
		}
	}

	m_pages_seen.insert(address);

	return page;
}

extern "C" void free_page(void* p, ullong* page) { ((Dumper*)p)->free_page(page); }
void Dumper::free_page(ullong* page) {
	delete [] page;
}
