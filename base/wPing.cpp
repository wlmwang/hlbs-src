
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wPing.h"

wPing::wPing()
{
	Initialize();
}

wPing::wPing(const char *ip, float timeout)
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
	//生成使用ICMP的原始套接字,这种套接字只有root才能生成
	if ((mFD = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		return NOT_PRI;
	}
	
	//回收root权限,设置当前用户权限
	//setuid(getuid());
	 
	//扩大套接字接收缓冲区到50K。主要为了减小接收缓冲区溢出的的可能性：若无意中ping一个广播地址或多播地址，将会引来大量应答
	int size = 50*1024;
	if (setsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) != 0)
	{
		LOG_ERROR(ELOG_KEY, "[ping] set socket receive buf failed:%d", size);
		return FD_UNKNOWN;
	}

	struct timeval tv;
	tv.tv_sec = (int)mTimeout;
	if(tv.tv_sec < 0 || tv.tv_sec >= 3) tv.tv_sec = 0;
	tv.tv_usec = (int)((mTimeout - (int)mTimeout) * 1000000);
	if(tv.tv_usec < 0 || tv.tv_usec >= 1000000 || (tv.tv_sec == 0 && tv.tv_usec == 0)) tv.tv_usec = 700000;

    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, tv, sizeof(struct timeval));	//发送超时
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, tv, sizeof(struct timeval));	//接受超时

	bzero(&mDestAddr, sizeof(mDestAddr));
	mDestAddr.sin_family = AF_INET;
	
	struct hostent *host;
	unsigned long inaddr = 0l;
	if (inaddr = inet_addr(mStrIp.c_str()) == INADDR_NONE)
	{
		//是主机名
		if ((host = gethostbyname(mStrIp.c_str())) == NULL)
		{
			return FD_UNKNOWN;
		}
		memcpy((char *) &mDestAddr.sin_addr, host->h_addr, host->h_length);
	}
	else
	{
		//是ip地址
		memcpy((char *) &mDestAddr, (char *) &inaddr, host->h_length);
	}
	
	return mFD;
}

int wPing::Ping()
{
	if (mFD == FD_UNKNOWN)
	{
		return -1;
	}
	
	//发送所有ICMP报文
    if (SendPacket() == -1)
    {
        Close();
        return -1;
    }

    int iRet = RecvPacket();
    Close();
    return iRet;
}

/** 发送num个ICMP报文 */
void wPing::SendPacket()
{
	//设置ICMP报头
	int iLen = Pack();
	int iRet = sendto(mFD, mSendpacket, iLen, 0, (struct sockaddr *)&mDestAddr, sizeof(mDestAddr));
	if (iRet < iLen)
	{
		LOG_ERROR(ELOG_KEY, "[ping] ip=%s,send ping error=%d,%s", mStrIp.c_str(), errno, strerror(errno));
		return -1;
	}
	return 0;
}

/** 接收所有ICMP报文 */
void wPing::RecvPacket()  
{
	if (mFD == FD_UNKNOWN)
	{
		return;
	}

    struct msghdr msg;
    struct iovec iov;
    memset(&msg, 0, sizeof(msg));
    memset(&iov, 0, sizeof(iov));

	int iLen = 0, i = 0;
    int fromlen = sizeof(mFrom);

	memset(mRecvpacket, 0, sizeof(mRecvpacket));
	memset(mCtlpacket, 0, sizeof(mCtlpacket));
    for (i = 0; i < RECV_RETRY_TIMES; i++)
    {
        iov.iov_base = mRecvpacket;
        iov.iov_len = sizeof(mRecvpacket);
        msg.msg_name = (struct sockaddr *)&mFromAddr;	//fixed
        
        msg.msg_namelen = sizeof(struct sockaddr_in);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = mCtlpacket;
        msg.msg_controllen = sizeof(mCtlpacket);

    	//iLen = recvfrom(mFD, mRecvpacket, sizeof(mRecvpacket), 0, (struct sockaddr *)&mFrom, (struct socklen_t *)&fromlen);
        iLen = recvmsg(mFD, &msg, 0);
        if (iLen < 0)
        {
            if (errno == EINTR)
			{
				continue;
			}
			else if(errno == EAGAIN)
			{
				LOG_ERROR(ELOG_KEY, "[ping] ping recv timeout,ip=%s,err=EAGAIN", mStrIp.c_str());
				//continue;
				return -1;
			}
			else
			{
				LOG_ERROR(ELOG_KEY, "[ping] ping recv error,ip=%s,err=%d,%s", mStrIp.c_str(), errno, strerror(errno));
				return -1;
			}
        }
        else if (iLen == 0)
        {
        	LOG_ERROR(ELOG_KEY, "[ping] ping recv return 0,ip=%s", mStrIp.c_str());
        	return -1;
        }
		else
		{
            struct ip *iphdr = (struct ip *)mRecvpacket;
            if(iphdr->ip_p == IPPROTO_ICMP && iphdr->ip_src.s_addr == mDestAddr->sin_addr.s_addr)
            {
                break;
            }
		}
    }

	if (i >= RECV_RETRY_TIMES)
	{
		LOG_ERROR(ELOG_KEY, "[ping] ping recv retry times=%d,ip=%s", i, mStrIp.c_str());
        return -2;
	}

    //gettimeofday(&mRecvtv, NULL);
    int iRet = Unpack(mRecvpacket, iLen);
    if (iRet < 0)
	{
		LOG_ERROR(ELOG_KEY, "[ping] ping parse error=%d,ip=%s,retry=%u", iRet, mStrIp.c_str(), i);
		return iRet;
	}
	else
	{
        LOG_DEBUG(ELOG_KEY, "[ping] ping succ,ip=%s,retry=%u", mStrIp.c_str(), i);
        return i;
	}
}

/** 设置ICMP请求报头 */
int wPing::Pack()
{
	struct icmp *icmp;
	memset(mSendpacket, 0, sizeof(mSendpacket));
	icmp = (struct icmp*) mSendpacket;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_seq = mSeqNum++;
	icmp->icmp_id = mPid;
	memset(icmp->icmp_data, 0xa5, PDATA_SIZE);
	icmp->icmp_data[0] = 5;
	//ICMP数据字段为当前时间截
	//gettimeofday((struct timeval *)icmp->icmp_data, NULL);
	
	int iLen = 8 + PDATA_SIZE;
	
	//校验算法
	icmp->icmp_cksum = CalChksum((unsigned short *)icmp, iLen);
	return iLen;
}

/** 剥去ICMP报头 */
int wPing::Unpack(char *pBuffer, int iLen)
{
	if (iLen == 0)
	{
		return -7;
	}
	if (pBuffer == NULL)
	{
		return -8;
	}

	struct ip *ip = (struct ip *)pBuffer;
    if(ip->ip_p != IPPROTO_ICMP)
    {
        return -2;
    }

    int iphdrlen = ip->ip_hl << 2;
	
	//越过ip报头,指向ICMP报头
	struct icmp *icmp = (struct icmp *) (pBuffer + iphdrlen); 
	
	//ICMP报头及ICMP数据报的总长度
	iLen -= iphdrlen;
	
	//小于ICMP报头长度则不合理
	if (iLen < 8)
	{
		return -3;  
	}
	
	//确保所接收的是我所发的的ICMP的回应
	if (icmp->icmp_type == ICMP_ECHOREPLY)
	{
		if (icmp->icmp_id != mPid)
		{
			return -4;
		}
        if(iLen < 16)
        {
            return -5;     
        }
        if (icmp->icmp_data[0] != ICMP_DATA)
        {
            return -6;
        }
	}
	else
	{
		return -1;
	}
	return 0;
}

/** 校验和 */
unsigned short wPing::CalChksum(unsigned short *addr, int len)
{
	int sum = 0;
	int nleft = len;
	unsigned short *w = addr;
	unsigned short answer = 0;
			
	//把ICMP报头二进制数据以2字节为单位累加起来
	while(nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}
	
	//若ICMP报头为奇数个字节，会剩下最后一字节。把最后一个字节视为一个2字节数据的高字节，这个2字节数据的低字节为0，继续累加
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
