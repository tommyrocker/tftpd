/*
 * tftpmain.cpp
 *
 *  Created on: 2012-5-11
 *      Author: tommy
 */

#include <iostream>

#include "CTftp.h"

using  namespace std;

vector <CTftp *> g_tftpproc;
CRITICAL_SECTION g_cs;


void addTftpProcToVector(CTftp *p)
{
	EnterCriticalSection(&g_cs);
	g_tftpproc.push_back(p);
	LeaveCriticalSection(&g_cs);
}

void delTftpProcFromVector(CTftp *pd)
{
	EnterCriticalSection(&g_cs);

	uint32_t i;
	for(i=0; i<g_tftpproc.size(); i++)
	{
		CTftp * p = g_tftpproc.at(i);
		if(p == pd)
			break;
	}

	if(i < g_tftpproc.size())
		g_tftpproc.erase(g_tftpproc.begin()+i);

	LeaveCriticalSection(&g_cs);
}

int main(int argc, char **argv)
{

	WSAData data;
	WSAStartup(MAKEWORD(2,2), &data);

	InitializeCriticalSection(&g_cs);

	CTftp ltftp;
	ltftp.pcsock = new CSock(69);

	while(1)
	{
		CSock *psock = ltftp.pcsock;
		char buf[2000];
		int len = 2000;
		int nread = psock->recieve(buf, len);
		if(nread > 0)
		{
			ltftp.recvmsgproc(buf, nread);
		}
	}

	WSACleanup();

	return 0;
}



