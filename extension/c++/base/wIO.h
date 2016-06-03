
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_IO_H_
#define _W_IO_H_

#include "wCore.h"
#include "wMisc.h"
#include "wNoncopyable.h"

enum IO_TYPE
{
	TYPE_UNKNOWN = -1,
	TYPE_SOCK,	//tcp|udp|http|unix
	TYPE_FILE,
	TYPE_BUF,
	TYPE_SHM
};

enum IO_FLAG
{
	FLAG_UNKNOWN = -1,
    FLAG_RECV,
    FLAG_SEND,
	FLAG_RVSD	//收发
};

/**	TASK_HTTP使用TASK_TCP传输协议*/
enum TASK_TYPE
{
	TASK_UNKNOWN = -1,
	TASK_UDP,
	TASK_TCP,
	TASK_UNIXS,
	TASK_UNIXD,
	TASK_HTTP
};

/**	未起实质性作用~*/
enum SOCK_STATUS
{
	STATUS_UNKNOWN = -1,
	STATUS_UNCONNECT,	//关闭状态
	STATUS_CONNECTED,	//已经建立连接
	STATUS_LISTEN		//已经处于监听状态
};

//套接字
enum SOCK_TYPE
{
	SOCK_UNKNOWN = -1,
	SOCK_LISTEN = 0,	//监听sock
	SOCK_CONNECT		//连接sock
};

class wIO : private wNoncopyable
{
	public:
		wIO();
		void Initialize();
		virtual ~wIO();
		
		int &FD() { return mFD; }
		int &Errno() { return mErr; }
		IO_TYPE &IOType() { return mIOType; }
		IO_FLAG &IOFlag() { return mIOFlag; }
		
		SOCK_TYPE &SockType() { return mSockType;}
		SOCK_STATUS &SockStatus() { return mSockStatus;}
		
		TASK_TYPE &TaskType() { return mTaskType;}
		
		unsigned long long &RecvTime() { return mRecvTime; }
		unsigned long long &SendTime() { return mSendTime; }
		unsigned long long &CreateTime() { return mCreateTime; }

		virtual string &Host() { return mHost; }
		virtual unsigned short &Port() { return mPort; }

		//30s
		virtual int SetTimeout(float fTimeout = 30) { return -1; }
		virtual int SetSendTimeout(float fTimeout = 30) { return -1; } 
		virtual int SetRecvTimeout(float fTimeout = 30) { return -1; }
		
		virtual int SetNonBlock(bool bNonblock = true);
		
		virtual int Open();
		virtual void Close();
		
		virtual ssize_t RecvBytes(char *vArray, size_t vLen) = 0;
		virtual ssize_t SendBytes(char *vArray, size_t vLen) = 0;
		
	protected:
		int mFD;
		int mErr;
		IO_TYPE mIOType;
		IO_FLAG mIOFlag;
		
		SOCK_TYPE mSockType;
		SOCK_STATUS mSockStatus;
		
		TASK_TYPE mTaskType;
		
		string mHost;
		unsigned short mPort;
		
		unsigned long long mRecvTime;	//最后接收到数据包的时间戳，毫秒
		unsigned long long mSendTime;	//最后发送数据包时间戳（主要用户心跳检测），毫秒
		unsigned long long mCreateTime;	//创建时间，毫秒
};

#endif
