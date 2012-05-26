/*
 * CSock.h
 *
 *  Created on: 2012-5-11
 *      Author: tommy
 */

#ifndef CSOCK_H_
#define CSOCK_H_

#include <winsock.h>

class CSock
{
private:
	SOCKET m_hsock;
	sockaddr_in m_peer;
public:
	CSock();
	virtual ~CSock();
	CSock(int nport);
    SOCKET getHsock() const;
    void setHsock(SOCKET hsock);
    int recieve(char *pbuf, const int buflen);
    int send(const char *pbuf, const int buflen);
    sockaddr_in getPeer() const;
    void setPeer(sockaddr_in peer);
};

#endif /* CSOCK_H_ */
