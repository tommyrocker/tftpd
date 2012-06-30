/*
 * CTftp.cpp
 *
 *  Created on: 2012-5-13
 *      Author: tommy
 */

#include "CTftp.h"

void addTftpProcToVector(CTftp *p);
void delTftpProcFromVector(CTftp *pd);

CRITICAL_SECTION CTftp::cs_files;
vector <CFile*> CTftp::file_list;

extern vector <CTftp *> g_tftpproc;
extern CRITICAL_SECTION g_cs;

bool matchTftpClient(sockaddr_in peer)
{
	bool ret = FALSE;
	EnterCriticalSection(&g_cs);

	unsigned int i;
	for(i=0; i<g_tftpproc.size(); i++)
	{
		CTftp * p = g_tftpproc.at(i);
		sockaddr_in sin = p->pcsock->getPeer();
		if(!memcmp(&sin, &peer, sizeof(sockaddr_in)))
		{
			ret = TRUE;
			break;
		}
	}

	LeaveCriticalSection(&g_cs);

	return ret;
}


CTftp::CTftp()
{
	pcfile = NULL;
	pcsock = NULL;
	filename = "";
	mode = "";
	blockno = 0;
	state = EM_IDLE;
}

int CTftp::recvmsgproc(const char *pbuf, const int len)
{
	int ret = TFTP_ERR;
	short int opcode = ntohs(*(short int*)pbuf);
	switch(opcode)
	{
		case OP_RRQ:
			ret = rrqMsgProc(pbuf+2, len-2);
			break;
		case OP_WRQ:
			ret = wrqMsgProc(pbuf+2, len-2);
			break;
		case OP_DAT:
			ret = dataMsgProc(pbuf+2, len-2);
			break;
		case OP_ACK:
			ackMsgProc(pbuf+2, len-2);
			break;
		default:
			break;
	}
	return ret;
}

int CTftp::rrqMsgProc(const char *pbuf, const int len)
{

	int ret = TFTP_ERR;

	if(matchTftpClient(pcsock->getPeer()))
		return ret;

	CTftp * ptftp= new CTftp();

	if(ptftp)
	{
		char *p = (char *)pbuf;
		int flen = strlen(p);
		ptftp->filename.assign(p, flen);
		p += flen+1;
		ptftp->mode.assign(p, strlen(p));

		ptftp->pcfile = new CFile(ptftp->filename);
		if(ptftp->pcfile && ptftp->pcfile->open("r"))
		{
			ptftp->pcfile->read();
			ptftp->blockno = 1;
			ptftp->direct =  "get";

			CSock * lsock = new CSock();
			if(lsock)
			{
				lsock->setPeer(pcsock->getPeer());

				ptftp->pcsock = lsock;
				ptftp->sendDataMsg(ptftp->blockno);
				ptftp->state = EM_RUNNING;
				ret = TFTP_OK;
				addTftpProcToVector(ptftp);

				DWORD dwThread = 0;
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)workThread, ptftp, 0, &dwThread);

			}
			else
			{
				delete ptftp;
			}
		}
		else
			delete ptftp;
	}

	return ret;

}

int CTftp::sendDataMsg(unsigned short int blockno)
{
	int ret = TFTP_ERR;

	if(blockno <= pcfile->strlist.size() && blockno >=1)
	{
		short int idx = blockno-1;
		char *buf = new char[pcfile->lenlist.at(idx)+4];
		if (buf)
		{
			*(short int *) buf = htons(OP_DAT);
			*(short int *) (buf + 2) = htons(blockno);
			memcpy(buf + 4, pcfile->strlist.at(idx),
					pcfile->lenlist.at(idx));
			int nsend = pcsock->send(buf, pcfile->lenlist.at(idx) + 4);
			ret = TFTP_OK;
			delete buf;
		}
	}

	return ret;
}

int CTftp::sendAckMsg(short int blockno)
{
	char buf[4]="";
	*(short int*)buf = htons(OP_ACK);
	*(short int *)(buf+2) = htons(blockno);
	pcsock->send(buf, 4);
	return TFTP_OK;
}

int CTftp::sendErrorMsg(short int errcode)
{
	char buf[4]="";
	*(short int*)buf = htons(OP_ERR);
	*(short int *)(buf+2) = htons(errcode);
	pcsock->send(buf, 4);
	return TFTP_OK;
}

int CTftp::ackMsgProc(const char *pbuf, const int len)
{
	int ret = TFTP_OK;
	short int rbno = ntohs(*(short int*)pbuf);
	short int maxno = pcfile->strlist.size();

	if(rbno < maxno)
	{
		if(rbno == blockno)
			sendDataMsg(++blockno);
		else if(rbno < blockno)
		{
			sendDataMsg(++rbno);
			blockno = rbno;
		}
		else
			ret = TFTP_ERR;
	}

	if(ret == TFTP_OK && maxno == blockno)
		state = EM_IDLE;

	return ret;
}

DWORD WINAPI workThread(void *pdata)
{
	CTftp * ptftp = (CTftp*)pdata;

	if(!ptftp)
		return (0xffffffff);

	while(1)
	{
		CSock *psock = ptftp->pcsock;
		char buf[2000];
		int len = 2000;
		int nread = psock->recieve(buf, len);
		if(nread > 0)
		{
			ptftp->recvmsgproc(buf, nread);
		}

		if(!ptftp->running())
			break;
	}

	delTftpProcFromVector(ptftp);

	delete ptftp;

	return 0;
}

int CTftp::wrqMsgProc(const char *pbuf, const int len)
{

	int ret = TFTP_ERR;

	if(matchTftpClient(pcsock->getPeer()))
		return ret;

	CTftp * ptftp= new CTftp();

	if(ptftp)
	{
		char *p = (char *)pbuf;
		int flen = strlen(p);
		ptftp->filename.assign(p, flen);
		p += flen+1;
		ptftp->mode.assign(p, strlen(p));

		ptftp->pcfile = new CFile(ptftp->filename);
		if(ptftp->pcfile && ptftp->pcfile->open("w"))
		{
			ptftp->blockno = 0;
			ptftp->direct =  "pet";

			CSock * lsock = new CSock();
			if(lsock)
			{
				lsock->setPeer(pcsock->getPeer());

				ptftp->pcsock = lsock;
				ptftp->state = EM_RUNNING;
				ret = TFTP_OK;

				ptftp->sendAckMsg(ptftp->blockno);

				addTftpProcToVector(ptftp);

				DWORD dwThread = 0;
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)workThread, ptftp, 0, &dwThread);

			}
			else
			{
				delete ptftp;
			}
		}
		else
			delete ptftp;
	}

	return ret;

}

int CTftp::dataMsgProc(const char *pbuf, const int len)
{
	int ret = TFTP_ERR;

	short int rbno = ntohs(*(unsigned short*)pbuf);

	char * p = new char[len-2];

	if(p)
	{
		memcpy(p, pbuf+2, len-2);

		if(rbno == blockno+1)
		{
			pcfile->write(pbuf+2, len-2);
			sendAckMsg(blockno+1);
			blockno++;
			ret = TFTP_OK;

			if(len-2 != 512)
			{
				state = EM_IDLE;
				pcfile->close();
			}
		}
		else if(rbno < blockno)
		{
			sendAckMsg(blockno);
			delete p;
			ret = TFTP_OK;
		}
		else
			delete p;
	}

	return ret;
}

CFile * CTftp::addFileToList(string fname)
{
	CFile *pf = NULL;
	EnterCriticalSection(&CTftp::cs_files);
	int n = CTftp::file_list.size();
	int i = 0;
	for(i=0; i<n; i++)
	{
		pf = (CFile*)CTftp::file_list.at(i);
		if(pf->getName() == fname)
			break;
	}

	if(i >= n)
	{
		pf = new CFile(fname);
		if(pf->open("r"))
		{
			pf->read();
			CTftp::file_list.push_back(pf);
		}
	}

	if(pf)
		pf->get();

	LeaveCriticalSection(&CTftp::cs_files);

	return pf;
}

int CTftp::delFileFromList(string fname)
{
	EnterCriticalSection(&CTftp::cs_files);

	vector <CFile*>::iterator iter;

	for(iter = CTftp::file_list.begin(); iter != CTftp::file_list.end(); iter++)
	{
		if((*iter)->getName() == fname)
		{
			CTftp::file_list.erase(iter);
		}
	}
	LeaveCriticalSection(&CTftp::cs_files);

	return 0;
}

CTftp::~CTftp()
{
	if(pcfile)
		delete pcfile;

	if(pcsock)
		delete pcsock;
}

