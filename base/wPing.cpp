
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wPing.h"

wPing::wPing()
{
	Initialize();
}

wPing::wPing(const char *ip, int timeout)
{
	mStrIp = ip;
	mTimeout = timeout;
}

wPing::~wPing()
{
	Close();
}

int wPing::Close()
{
	if(mFD != FD_UNKNOWN)
	{
		close(mFD);
		mFD = FD_UNKNOWN;
	}
	return 0;
}

void wPing::Initialize()
{
	mPid = getpid();
	mFD = FD_UNKNOWN;
}

int wPing::Open()
{
	struct protoent *protocol;
	if ((protocol = getprotobyname("icmp")) == NULL)
	{
		return FD_UNKNOWN;
	}
	/** 生成使用ICMP的原始套接字,这种套接字只有root才能生成 */
	if ((mFD = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)	//IPPROTO_ICMP
	{
		return FD_UNKNOWN;
	}
	
	/** 回收root权限,设置当前用户权限 */
	//setuid(getuid());
	 
	/** 扩大套接字接收缓冲区到50K。主要为了减小接收缓冲区溢出的的可能性：若无意中ping一个广播地址或多播地址，将会引来大量应答 */
	int size = 50*1024;
	if (setsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) != 0)
	{
		//set socket receive buf failed:%d
		return FD_UNKNOWN;
	}
	
	bzero(&mDestAddr, sizeof(mDestAddr));
	mDestAddr.sin_family = AF_INET;
	
	struct hostent *host;
	unsigned long inaddr = 0l;
	if (inaddr = inet_addr(mStrIp.c_str()) == INADDR_NONE)  
	{
		/** 是主机名 */
		if ((host = gethostbyname(mStrIp.c_str())) == NULL)
		{
			return false;
		}
		memcpy((char *) &mDestAddr.sin_addr, host->h_addr, host->h_length);
	}  
	else
	{
		/** 是ip地址 */
		memcpy((char *) &mDestAddr, (char *) &inaddr, host->h_length);
	}
	
	return mFD;
}

bool wPing::Ping(int times)
{
	if (mFD == FD_UNKNOWN)
	{
		return false;
	}
	
	while (i < times)
    {
        i++;
		
		/*发送所有ICMP报文*/
        SendPacket(1);
		
		/*接收所有ICMP报文*/
        RecvPacket();
    }
}

/** 发送num个ICMP报文 */
void wPing::SendPacket(int num)
{
	if (num > MAX_NO_PACKETS)
	{
		num = MAX_NO_PACKETS;
	}
	
	int packetsize;
	int i = 0;
	while (i < num)
	{
		i++;
		mSend++;
		
		/** 设置ICMP报头 */
		packetsize = Pack(mSend);
		if (sendto(mFD, mSendpacket, packetsize, 0, (struct sockaddr *)&mDestAddr, sizeof(mDestAddr)) < 0)
		{
			continue;
		}
		/** 每隔一秒发送一个ICMP报文 */
		sleep(1);
	}
}

/** 接收所有ICMP报文 */
void wPing::RecvPacket()  
{
	int n, fromlen;
    fromlen = sizeof(mFrom);
    while (mRecv < mSend)
    {
        alarm(MAX_WAIT_TIME);
        if ((n = recvfrom(mFD, mRecvpacket, sizeof(mRecvpacket), 0, (struct sockaddr *)&mFrom, (struct socklen_t *)&fromlen)) < 0)  
        {
            if (errno == EINTR)
			{
				continue;
			}
			//recvfrom error
            continue;  
        }
		
		/** 记录接收时间 */
        gettimeofday(&mRecvtv, NULL);
        if (Unpack(mRecvpacket, n) == -1)
		{
			continue;
		}
        mRecv++;
    }
}  

/** 校验和 */
unsigned short wPing::CalChksum(unsigned short *addr, int len)
{
	int sum = 0;
	int nleft = len;
	unsigned short *w = addr;
	unsigned short answer = 0;
			
	/** 把ICMP报头二进制数据以2字节为单位累加起来 */
	while(nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}
	
	/** 若ICMP报头为奇数个字节，会剩下最后一字节。把最后一个字节视为一个2字节数据的高字节，这个2字节数据的低字节为0，继续累加 */
	if(nleft == 1)
	{      
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return answer;
}

/** 设置ICMP请求报头 */
int wPing::Pack(int pack_no)
{
	struct icmp *icmp;
	icmp = (struct icmp*) mSendpacket;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_seq = pack_no;
	icmp->icmp_id = mPid;
	
	/** 记录发送时间 */
	struct timeval *tval;
	tval = (struct timeval *) icmp->icmp_data;
	gettimeofday(tval, NULL);
	
	int packsize = 8 + SEND_DATA_LEN;
	
	/** 校验算法 */
	icmp->icmp_cksum = CalChksum((unsigned short *)icmp, packsize);
	return packsize;
}

/** 剥去ICMP报头 */
int wPing::Unpack(char *buf, int len)
{
	int i,iphdrlen;
	struct ip *ip;
	struct icmp *icmp;
	struct timeval *tvsend;
	double rtt;
	ip = (struct ip *)buf;
	
	/** 求ip报头长度,即ip报头的长度标志乘4 */
	iphdrlen = ip->ip_hl << 2;
	
	/** 越过ip报头,指向ICMP报头 */
	icmp = (struct icmp *) (buf + iphdrlen); 
	
	/** ICMP报头及ICMP数据报的总长度 */
	len -= iphdrlen;
	
	/** 小于ICMP报头长度则不合理 */
	if (len < 8)
	{
		//ICMP packets/'s length is less than 8/n
		return -1;  
	}
	
	/** 确保所接收的是我所发的的ICMP的回应 */
	if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == mPid))  
	{  
		tvsend = (struct timeval *) icmp->icmp_data;  
		
		/** 接收和发送的时间差 */
		TvSub(&mRecvtv, tvsend);
		
		/** 以毫秒为单位计算rtt */
		rtt = mRecvtv.tv_sec * 1000 + mRecvtv.tv_usec / 1000;
		mTotalResponseTimes += rtt;
		if (mFasterResponseTime == -1)
		{
			mFasterResponseTime = rtt;
		}
		else if(mFasterResponseTime > rtt)
		{
			mFasterResponseTime = rtt;  
		}
		
		if(mLowerResponseTime == -1)  
		{  
			mLowerResponseTime = rtt;  
		}  
		else if(mLowerResponseTime < rtt)  
		{
			mLowerResponseTime = rtt;  
		}
		/** 显示相关信息 */  
		//printf("%d/tbyte from %s/t: icmp_seq=%u/tttl=%d/trtt=%.3f/tms/n", len, inet_ntoa(m_from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);  
		
		return 0;
	}
	return -1;
}

void wPing::TvSub(struct timeval *out,struct timeval *in)  
{         
    if((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}
