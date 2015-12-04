
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_TASK_H_
#define _W_TCP_TASK_H_

#include <string>
#include <arpa/inet.h>

#include "wType.h"
#include "wHeadCmd.h"
#include "wSocket.h"

//关闭客户端连接的原因
enum CLOSE_REASON
{
	CLOSE_BY_ROUTER = 1,
	CLOSE_BY_PEER,
	CLOSE_BY_SERVER,
};

class wTcpTask : public wSocket
{
	public:
		wTcpTask(int iNewSocket, struct sockaddr_in *stSockAddr);
		virtual ~wTcpTask();
			
		/**
		 * 初始化
		 */
		virtual void Initialize();
		
		virtual void CloseTcpTask(CLOSE_REASON vReason = CLOSE_BY_ROUTER);
		
		/**
		 *  接受消息
		 */
		int ListeningRecv();
		
		/**
		 *  发送消息
		 */
		int ListeningSend();
		
		int RecvBytes(char *vArray, int vLen);
		int SendBytes(char *vArray, int vLen);
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen) {}
		virtual void ProcessSendMessage() {}

		virtual int HandleSendMessage(char * pBuffer, int nLen) {}
		virtual void ProcessRecvMessage() {}
		virtual void ParseRecvMessage(struct wHeadCmd* pHeadCmd ,char *pBuffer,int iLen) {}
    public:
		int mSocketType;					//socket类型：监听socket、连接socket
		int mSocketFlag;					//socket标志：是否收包		
		time_t  mCreateTime;				//socket的创建时间
		time_t mStamp;						//接收到数据包的时间戳		
		
		char mToken[MAX_NAME_LEN];			//标识一个客户端
		
		//接收消息的缓冲区 100k
		int mRecvBytes;						//接收的字节数
		char mRecvMsgBuff[MAX_RECV_BUFFER_LEN];	
		
		//发送消息时的临时缓冲区 1M
		int mSendBytes;						//发送字节数
		char mSendMsgBuff[MAX_SEND_BUFFER_LEN];

};

#endif
