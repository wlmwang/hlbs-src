
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  定义了与其他服务器之间的消息头
 *  负责服务器内部交换使用，和客户端交互的指令需要另外定义
 */
#ifndef _W_HEAD_CMD_H_
#define _W_HEAD_CMD_H_

#include <time.h>

#define HEAD_CMD_LEN 20

#pragma pack(1)

//客户端的消息类型
enum CLIENT_MSG_TYPE
{
	TYPE_LOGIC = 1,		// 逻辑控制消息
	TYPE_STREAM,		// 流消息
	TYPE_SOUND,			// 声音消息
};

struct wHeadCmd
{
	unsigned int mClientIP;		//客户端的IP地址
	unsigned short mClientPort;	//客户端的端口
	time_t mSockTime;			//socket的创建时间(发送时间)
	time_t mTimeStamp;			//接收到此次消息的时间戳
	short mClientState;			//客户端当前状态，=0表示一切正常，<0表示客户端已经断开连接，>0表示注册消息，这时mClientFD代表服务器的类型
	unsigned int mCmd;			//消息类型（以此字段定义服务端通信行为）
	
	//unsigned int mClientFD;		// gateserver中客户端对应的FD
	
	/*
	bool SerializeToArray(char *vBuffer, int vBufferLen)
	{
		if( vBuffer == 0 || vBufferLen < HEAD_CMD_LEN )
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
	*/
};

#pragma pack()

#endif
