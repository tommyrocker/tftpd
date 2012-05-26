/*
 * CSock.cpp
 *
 *  Created on: 2012-5-11
 *      Author: tommy
 */

#include "CSock.h"

CSock::CSock()
{
	m_hsock = INVALID_SOCKET;
	m_hsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if(m_hsock)
	{
		SOCKADDR_IN inaddr;
		inaddr.sin_family = AF_INET;
		inaddr.sin_port =  0;
		inaddr.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(m_hsock, (struct sockaddr*)&inaddr, sizeof(sockaddr));
	}
}

CSock::CSock(int nport)
{
	m_hsock = INVALID_SOCKET;
	m_hsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if(m_hsock)
	{
		SOCKADDR_IN inaddr;
		inaddr.sin_family = AF_INET;
		inaddr.sin_port = ntohs(nport);
		inaddr.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(m_hsock, (struct sockaddr*)&inaddr, sizeof(sockaddr));
	}
}

SOCKET CSock::getHsock() const
{
    return m_hsock;
}

void CSock::setHsock(SOCKET hsock)
{
    m_hsock = hsock;
}

CSock::~CSock()
{
	if(m_hsock != INVALID_SOCKET)
	{
		closesocket(m_hsock);
		m_hsock = INVALID_SOCKET;
	}
}

int CSock::recieve(char *pbuf, const int buflen)
{

	if(!pbuf)
		return 0;

	fd_set fds;
	SOCKET fd = m_hsock;

	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	int n = select(fd + 1, &fds, NULL, NULL, &tv);

	if (n > 0)
	{
		sockaddr_in sin;
		int peerlen = sizeof(sockaddr);
		char buf[2000] = "";
		int len = 2000;
		//getpeername(fd, (sockaddr*) &sin, &peerlen);

		int nlen = recvfrom(fd, buf, len, 0, (sockaddr*) &sin, &peerlen);

		if(nlen > 0)
		{
			int length = min(nlen, buflen);
			memcpy(pbuf, buf, length);
			setPeer(sin);
			return length;
		}
		else
			 return 0;
	}
	else
		return 0;
}

sockaddr_in CSock::getPeer() const
{
    return m_peer;
}

void CSock::setPeer(sockaddr_in peer)
{
    m_peer = peer;
}

int CSock::send(const char *pbuf, const int buflen)
{
	int sin_len = sizeof(sockaddr);
	sockaddr_in peer = getPeer();


	return sendto(m_hsock, pbuf, buflen, 0, (sockaddr*)&peer, sin_len);
}





