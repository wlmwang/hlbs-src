
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MISC_H_
#define _W_MISC_H_

#include <sstream>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>	//int flock(int fd,int operation);

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#include "wCore.h"
#include "wCommand.h"
#include "wSignal.h"
#include "wLog.h"

inline const char* IP2Text(long ip)
{
	in_addr in;
	in.s_addr = ip;

	return inet_ntoa(in);
}

inline long Text2IP(const char* ipstr)
{
	return inet_addr(ipstr);
}

inline unsigned GetIpByIF(const char* pIfname)
{
    int iFD, iIntrface;
    struct ifreq buf[64];
    struct ifconf ifc = {0, {0}};
    unsigned ip = 0; 

    memset(buf, 0, sizeof(buf));
    if ((iFD = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(iFD, SIOCGIFCONF, (char*)&ifc))
        {
            iIntrface = ifc.ifc_len / sizeof(struct ifreq); 
            while(iIntrface-- > 0)
            {
                if(strcmp(buf[iIntrface].ifr_name, pIfname) == 0)
                {
                    if(!(ioctl(iFD, SIOCGIFADDR, (char *)&buf[iIntrface])))
                    {
                        ip = (unsigned)((struct sockaddr_in *)(&buf[iIntrface].ifr_addr))->sin_addr.s_addr;
                    }
                    break;  
                }       
            }       
        }
        close(iFD);
    }
    return ip;
}

inline unsigned int HashString(const char* s)
{
	unsigned int hash = 5381;
	while (*s)
	{
		hash += (hash << 5) + (*s ++);
	}
	return hash & 0x7FFFFFFF;
}

inline unsigned long long GetTickCount()    //clock_t
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000;
}

inline int64_t GetTimeofday()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec;
}

inline void GetTimeofday(struct timeval* pVal, void*)
{
    if (pVal == NULL)
    {
        return;
    }
    gettimeofday(pVal, NULL);
}

//long -> string
inline string Itos(const long &i)
{
    string sTmp;
    stringstream sRet(sTmp);
    sRet << i;
    return sRet.str();
}

inline void Strlow(u_char *dst, u_char *src, size_t n)
{
    while (n) 
    {
        *dst = tolower(*src);
        dst++;
        src++;
        n--;
    }
}

inline int GetCwd(char *path, size_t size)
{
    if(getcwd(path, size) == NULL)
    {
        return -1;
    }
    return 0;
}

//long -> char*
void itoa(unsigned long val, char *buf, unsigned radix);

u_char *Cpystrn(u_char *dst, u_char *src, size_t n);

vector<string> Split(string sStr, string sPattern, bool bRepeat = true);

int Gcd(int a, int b);
int Ngcd(int *arr, int n);

int InitDaemon(const char *filename);

#endif
