/*
 * CTftp.h
 *
 *  Created on: 2012-5-13
 *      Author: tommy
 */

#ifndef CTFTP_H_
#define CTFTP_H_

#include "CSock.h"
#include "CFile.h"

const string MODASCII = "netascii";
const string MODOCTET = "octet";

const int TFTP_OK = 0;
const int TFTP_ERR = -1;

class CTftp
{
public:
	CFile *pcfile;
	CSock * pcsock;
	string filename;
	string mode;
	string direct;
	unsigned short int  blockno;
	int state;
	enum {OP_RRQ=1, OP_WRQ, OP_DAT,OP_ACK,OP_ERR};
	enum {ERR_UNKNOWN, ERR_NOFILE, ERR_ACCESSIL, ERR_DISKFULL, ERR_ILFTP, ERR_UNKNOWNTID, ERR_FILEEXIST, ERR_NOUSER};
	enum {EM_IDLE, EM_RUNNING};
public:
	int recvmsgproc(const char *pbuf, const int len);
	int rrqMsgProc(const char * pbuf, const int len);
	int ackMsgProc(const char *pbuf, const int len);
	int wrqMsgProc(const char *pbuf, const int len);
	int dataMsgProc(const char *pbuf, const int len);
	int sendDataMsg(unsigned short int blockno);
	int sendAckMsg(short int blockno);
	int sendErrorMsg(short int errcode);

	BOOL running( ){return state == EM_RUNNING;};

	CTftp();
	virtual ~CTftp();
};

 DWORD workThread(void *pdata);

#endif /* CTFTP_H_ */
