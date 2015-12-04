//--------------------------------------------------
// 本文件定义了gateserver与其他服务器之间的消息头
//-------------------------------------------------- 
#ifndef _NET_HEAD_H_
#define _NET_HEAD_H_

#include <time.h>

#define NET_HEAD_LEN 20

// 客户端的消息类型
enum CLIENT_MSG_TYPE
{
	TYPE_LOGIC = 1,		// 逻辑控制消息
	TYPE_STREAM,		// 流消息
	TYPE_SOUND,			// 声音消息
};

struct CNetHead
{
	unsigned int mClientIP;		// 客户端的IP地址
	time_t mSockTime;			// socket的创建时间
	time_t mTimeStamp;			// 接收到此次消息的时间戳
	unsigned int mClientFD;		// gateserver中客户端对应的FD
	unsigned short mClientPort;	// 客户端的端口
	short mClientState;			// 客户端当前状态，=0表示一切正常，<0表示客户端已经断开连接，>0表示注册消息，这时mClientFD代表服务器的类型
	bool SerializeToArray(char *vBuffer, int vBufferLen)
	{
		if( vBuffer == 0 || vBufferLen < NET_HEAD_LEN )
		{
			return false;
		}

		char *pBuffer = vBuffer;
		*(unsigned int *)pBuffer = mClientIP;
		pBuffer += sizeof(int);
		*(unsigned int *)pBuffer = mSockTime;
		pBuffer += sizeof(int);
		*(unsigned int *)pBuffer = mTimeStamp;
		pBuffer += sizeof(int);
		*(unsigned int *)pBuffer = mClientFD;
		pBuffer += sizeof(int);
		*(unsigned short *)pBuffer = mClientPort;
		pBuffer += sizeof(short);
		*(unsigned short *)pBuffer = mClientState;

		return true;
	}

	bool ParseFromArray(char *vBuffer, int vBufferLen)
	{
		if( vBuffer == 0 || vBufferLen < NET_HEAD_LEN )
		{
			return false;
		}

		char *pBuffer = vBuffer;
		mClientIP = *(unsigned int *)pBuffer;
		pBuffer += sizeof(int);
		mSockTime = *(unsigned int *)pBuffer;
		pBuffer += sizeof(int);
		mTimeStamp = *(unsigned int *)pBuffer;
		pBuffer += sizeof(int);
		mClientFD = *(unsigned int *)pBuffer;
		pBuffer += sizeof(int);
		mClientPort = *(unsigned short *)pBuffer;
		pBuffer += sizeof(short);
		mClientState = *(unsigned short *)pBuffer;

		return true;
	}
};

typedef struct CNetHead CNetHead;

#endif
