
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  消息头
 */
#ifndef _W_HEAD_CMD_H_
#define _W_HEAD_CMD_H_

#define HEAD_CMD_LEN 4

#pragma pack(1)

enum REQUEST_TYPE
{
	SERVER = 1,
	CLIENT,
};

const BYTE CMD_NULL = 0;		/** 空的指令 */
const BYTE PARA_NULL = 0;		/** 空的指令参数 */

struct Command
{
	BYTE cmd;					/** 指令代码 */
	BYTE para;					/** 指令代码子编号 */

	BYTE GetCmdType() const { return cmd; }
	BYTE GetParaType() const { return para; }
	
	Command(const BYTE cmd = CMD_NULL, const BYTE para = PARA_NULL) : cmd(cmd), para(para) {};
};

struct wHeadCmd
{
	//unsigned int mClientIP;		//客户端的IP地址
	//unsigned short mClientPort;	//客户端的端口
	//unsigned short mClientState;//客户端当前状态，=0表示一切正常，<0表示客户端已经断开连接
	
	unsigned short mRequestType;
	Command mCommand;			//消息类型
	
	wHeadCmd()
	{
		//mClientIP = 0;
		//mClientPort = 0;
		//mClientState = 0;
	}
	
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
