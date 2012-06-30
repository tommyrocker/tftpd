/*
 * CFile.h
 *
 *  Created on: 2012-5-13
 *      Author: tommy
 */

#ifndef CFILE_H_
#define CFILE_H_

#include <windef.h>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

enum{
	TFTP_FILE_MODE_READ,
	TFTP_FILE_MODE_WRITE
};

class CFile
{
private:
	string name;
	int refer;
public:
	FILE * fp;
	vector <char *> strlist;
	vector <int> lenlist;
    string getName() const;
    void setName(string name);
public:
	BOOL open(string mode);
	void close();
	void read();
	void write(const char *buf, const int len);
	void display();
	void get(){refer++;};
	int put();
	BOOL isWrite();

	CFile();
	CFile( string s);
	virtual ~CFile();
};

#endif /* CFILE_H_ */
