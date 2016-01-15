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

ChildProcess::ChildProcess(const char* cmdline,string exitcmd) :
	m_hChildStd_IN_Rd(NULL),
	m_hChildStd_IN_Wr(NULL),
	m_sChildStd_OUT_Rd(),
	m_hChildStd_OUT_Wr(NULL),
	m_exitcmd(exitcmd)
{
	SECURITY_ATTRIBUTES saAttr={};

	// Set the bInheritHandle flag so pipe handles are inherited.
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;

	HANDLE hChildStd_OUT_Rd;
	// Create a pipe for the child process's STDOUT.
	if ( ! CreatePipe(&hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0) )
		ErrorExit("StdoutRd CreatePipe");

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if ( ! SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
		ErrorExit("Stdout SetHandleInformation");

	int fd=_open_osfhandle((intptr_t)hChildStd_OUT_Rd,_O_RDONLY);
	if (fd==-1)
		ErrorExit("_open_osfhandle");

	FILE* fp=_fdopen(fd,"rb");
	if (!fp)
		ErrorExit("_fdopen");

	m_sChildStd_OUT_Rd=ifstream(fp);

	// Create a pipe for the child process's STDIN.
	if (! CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0))
		ErrorExit("Stdin CreatePipe");

	// Ensure the write handle to the pipe for STDIN is not inherited.
	if ( ! SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
		ErrorExit("Stdin SetHandleInformation");

	PROCESS_INFORMATION piProcInfo={};
	STARTUPINFO siStartInfo={};
	BOOL bSuccess = FALSE;

	// Set up members of the STARTUPINFO structure.
	// This structure specifies the STDIN and STDOUT handles for redirection.
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	siStartInfo.hStdOutput = m_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = m_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process.
	bSuccess = CreateProcess(
		NULL,
		(LPSTR)cmdline, // command line
		NULL,           // process security attributes
		NULL,           // primary thread security attributes
		TRUE,           // handles are inherited
		0,              // creation flags
		NULL,           // use parent's environment
		NULL,           // use parent's current directory
		&siStartInfo,   // STARTUPINFO pointer
		&piProcInfo     // receives PROCESS_INFORMATION
	);

	// If an error occurs, exit the application.
	if ( ! bSuccess ) {
		ErrorExit("CreateProcess");
	} else {
		m_hProcess=piProcInfo.hProcess;
		m_hThread=piProcInfo.hThread;
	}
}

ChildProcess::~ChildProcess() {
	WriteLine(m_exitcmd);
}

bool ChildProcess::WriteLine(string& str) {
	if (str.back()!='\n')
		str+='\n';
	return Write(str.data(),str.length());
}

bool ChildProcess::Write(const void* data, size_t len) {
	return WriteFile(m_hChildStd_IN_Wr, data, (DWORD)len, NULL, NULL) ? true : false;
}

string ChildProcess::ReadUntil(const char* delim) {
	string _delim(delim);
	string out,tmp;
	while (true) {
		getline(m_sChildStd_OUT_Rd,tmp,_delim.back());
		out+=tmp;
		if (!m_sChildStd_OUT_Rd.good()) break;
		out+=_delim.back();
		size_t len=out.length();
		if (len<_delim.length()) continue;
		if (out.substr(len-_delim.length())==_delim) break;
	}

	return out;
}
