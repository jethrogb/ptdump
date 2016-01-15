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

void ErrorExit(PCSTR lpszFunction);

class ChildProcess {
	HANDLE m_hChildStd_IN_Rd;
	HANDLE m_hChildStd_IN_Wr;
	ifstream m_sChildStd_OUT_Rd;
	HANDLE m_hChildStd_OUT_Wr;
	HANDLE m_hProcess;
	HANDLE m_hThread;
	string m_exitcmd;
public:
	ChildProcess(const char* cmdline, string exitcmd);
	~ChildProcess();

	bool WriteLine(string& str);
	bool Write(const void* data, size_t len);
	string ReadUntil(const char* delim);
};

class Dumper {
	ChildProcess& m_child;
	ostream& m_out;
	unsigned m_pid;
	unordered_set<ullong> m_pages_seen;

	void output(void* data, ullong len);
	ullong get_base();
	ullong* read_page(ullong address);
	void free_page(ullong* page);

	friend void output(void* p, void* data, ullong len);
	friend ullong get_base(void* p);
	friend ullong* read_page(void* p, ullong address);
	friend void free_page(void* p, ullong* page);
public:
	Dumper(ChildProcess& child, unsigned pid, ostream& out);
};
