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

#define WINDBG_KD_PATH "\"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\kd.exe\""

int main(int argc, char *argv[]) {
	unsigned pid;
	if (argc<2) {
		fprintf(stderr,"Usage: %s <pid> [output-file]\n",argv[0]);
	}
	pid=strtoul(argv[1],NULL,10);
	ofstream fout;
	if (argc>=3) {
		if (argv[2][0]!='-' || argv[2][1]!='\0') {
			fout=ofstream(argv[2],ios::binary);
			if (!fout.good() || !fout.is_open()) {
				ErrorExit("Unable to open output file");
			}
		}
	}
	if (!fout.is_open()) {
		fout=ofstream(_fdopen(1,"wb")); //cout is "text mode". seriously, windows...
	}
	cerr << "Launching debugger\n";
	ChildProcess child(WINDBG_PATH " -kl",string(".detach"));
	child.ReadUntil("lkd>");
	Dumper dumper(child,pid,fout);
	ptdump(&dumper,3);
	return 0;
}

void ErrorExit(PCSTR lpszFunction)
// Format a readable error message, display a message box,
// and exit from the application.
{
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &lpMsgBuf,
        0, NULL );

    printf(
        "%s failed with error %d: %s",
        lpszFunction, dw, lpMsgBuf);

    LocalFree(lpMsgBuf);
    ExitProcess(1);
}
