
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_PING_H_
#define _W_PING_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

//#include <netinet/tcp.h>
//#include <netinet/udp.h>

#include "wCore.h"
#include "wMisc.h"
#include "wNoncopyable.h"

#define PACKET_SIZE	4096
#define PDATA_SIZE	56
#define NOT_PRI		-100
#define RECV_RETRY_TIMES	4
#define ICMP_DATA   5

class wPing : private wNoncopyable
{
	public:
		wPing(const char *ip, float timeout = 0.1);
		void Initialize();
		virtual ~wPing();

		int Open();
		int Close();

		int Ping();
		void SendPacket();
		void RecvPacket();

		int Pack();
		int Unpack(char *buf, int len);
		
		unsigned short CalChksum(unsigned short *addr, int len);
		
	protected:
		pid_t mPid;
		int mFD;
		string mStrIp;
		float mTimeout;	//超时时间	
		int mSeqNum;

		struct sockaddr_in mDestAddr;	//目的地址
		struct sockaddr_in mFromAddr;	//返回地址

		char mSendpacket[PACKET_SIZE];
		char mRecvpacket[PACKET_SIZE];
		char mCtlpacket[PACKET_SIZE];
		
		//struct timeval mRecvtv;		//接受到时间
};

#endif