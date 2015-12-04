
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_SERVER_H_
#define _ROUTER_SERVER_H_

#include <arpa/inet.h>

#include "wType.h"
#include "wTimer.h"
#include "wSingleton.h"

#include "wMsgQueue.h"
#include "wTcpServer.h"
#include "wTcpTask.h"

class RouterServer: public wTcpServer<RouterServer>
{
	public:
		RouterServer();

		//初始化
		virtual void Initialize();
		
		//准备工作
		virtual void PrepareRun();
		
		//主循环
		virtual void Run();
		
		virtual wTcpTask* NewTcpTask(int iNewSocket, struct sockaddr_in* stSockAddr);
		
		virtual ~RouterServer();
		
		/**
		 * 初始化共享内存
		 * 队列多进程通信
		 */		
		void InitailizeBuffer();
	    
        void ProcessRecvMessage();
		
		/**
		 *  输入/输出的消息队列的缓冲区位置
		 */		
		char *mInBufferPtr;
		char *mOutBufferPtr;
		
		wMsgQueue mInMsgQueue;	//输入的消息队列
		wMsgQueue mOutMsgQueue;	//输出的消息队列
		
		void CheckTimer();
		void CheckClientTimeout();
		
	private:
		//服务器重连计时器
		wTimer mServerReconnectTimer;

		//客户端检测计时器
		wTimer mClientCheckTimer;

	//protected:
		
		//发送到客户端(网络层)
		//int SendToClient(int vSocketFD, char *vArray, int vLen);
		
		//发送到客户端(逻辑层)
		//bool Send2Client(int vSocketFD, char *vArray, int vLen);
		
		//检查是否有客户端消息
		//void CheckClientMessage();
		
		//接收并处理服务器消息
		//void CheckServerMessage();
		
		
		//从客户端接收一个消息
		//int RecvFromClient(int vSocketFD, char *vArray, int vLen);
		
		//发送给指定类型的服务器
		//bool Send2Server(char *vArray, int vLen);
};

#endif
