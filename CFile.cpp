/*
 * CFile.cpp
 *
 *  Created on: 2012-5-13
 *      Author: tommy
 */

#include "CFile.h"

CFile::CFile()
{
	fp = NULL;
	strlist.clear();
	lenlist.clear();
}

BOOL CFile::open(string mode)
{

	if(name.empty() || mode.empty())
		return FALSE;

	fp = fopen(name.data(), mode.data());
	if(fp)
		return TRUE;
	else
		return FALSE;

}

void CFile::close()
{
	if(fp)
	{
		if(fp->_flag == _IOWRT || fp->_flag == _IORW)
		{
			fseek(fp, 0, SEEK_SET);
			unsigned int i;

			for(i=0; i<strlist.size(); i++)
			{
				fwrite(strlist.at(i), 1, lenlist.at(i), fp);
			}
		}
		fclose(fp);
		fp = NULL;
	}
}

CFile::CFile(string s)
{
	name = s;
	strlist.clear();
	lenlist.clear();
}

string CFile::getName() const
{
    return name;
}

void CFile::setName(string name)
{
    this->name = name;
}

void CFile::read()
{
	if(fp)
	{
		int nread = 0;
		char *pread = NULL;
		fseek(fp, 0, SEEK_SET);

		do{
			pread = new char[512];
			if(pread)
			{
				memset(pread, 0 , 512);
				nread = fread(pread, 1, 512, fp);
				if(nread > 0)
				{
					strlist.push_back(pread);
					lenlist.push_back(nread);
				}
				else
				{
					delete pread;
					break;
				}
			}
			else
				break;
		}while(1);

	}
}

void CFile::display()
{
	unsigned int pos = 0;
	while(pos < strlist.size()){
		string s;
		s = strlist.at(pos);
		cout << s << "    " << lenlist.at(pos) << endl;
		pos++;
	}
}

void CFile::write(const char *buf, const int len)
{
	char *p = new char[512];
	if(p)
	{
		memcpy(p, buf, len);
		strlist.push_back(p);
		lenlist.push_back(len);
	}
}

CFile::~CFile()
{
	if(!strlist.empty())
	{
		unsigned int  i;
		for(i=0; i<strlist.size(); i++)
			delete(strlist.at(i));
		strlist.clear();
	}

	if(!lenlist.empty())
	{
		lenlist.clear();
	}

	close();


}

