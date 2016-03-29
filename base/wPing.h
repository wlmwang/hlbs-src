
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_PING_H_
#define _W_PING_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#include "wCore.h"
#include "wMisc.h"
#include "wNoncopyable.h"

#define PACKET_SIZE     4096
#define SEND_DATA_LEN   56
#define MAX_WAIT_TIME  20
#define MAX_NO_PACKETS  4
//#define ERROR           -1
//#define SUCCESS         1

class wPing : private wNoncopyable
{
	public:
		wPing();
		wPing(const char *ip, int timeout = 2000);
		void Initialize();
		virtual ~wPing();
		
		int Ping(int times);
		int Pack(int pack_no);
		int Unpack(char *buf, int len);
		void SendPacket(void);  
		void RecvPacket(void);  
		void TvSub(struct timeval *out, struct timeval *in);  
		
		int Open();
		int Close();
		unsigned short CalChksum(unsigned short *addr, int len);
		
	protected:
		pid_t mPid;
		int mFD;
		string mStrIp;
		int mTimeout;
		
		int mSend;	//已发送ICMP包个数
		int mRecv;	//已接受ICMP包个数
		
		double mFasterResponseTime;	//最快响应时间
		double mLowerResponseTime;	//最慢响应时间
		double mTotalResponseTimes;	//总共收到响应时间总和 毫秒
		
		struct sockaddr_in mDestAddr;	//目的地址
		struct sockaddr_in mFrom;		//返回地址

		char mSendpacket[PACKET_SIZE];
		char mRecvpacket[PACKET_SIZE];
		struct timeval mRecvtv;		//接受到时间
};

#endif