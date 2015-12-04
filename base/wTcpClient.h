
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

//--------------------------------------------------
// 本文件的类适合于两种实体
// 1. 连接到某个监听端口的连接
//    使用ConnectToServer初始化
//-------------------------------------------------- 

#ifndef _W_TCP_CLIENT_H_
#define _W_TCP_CLIENT_H_

#include <sys/epoll.h>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <string.h>

#include <netinet/in.h>
#include <pthread.h>

#include "wSingleton.h"
#include "wTcpTask.h"
#include "wSocket.h"
#include "wLog.h"

enum ETCPClientError
{
	ETC_IPADDR_NOT_VALID		= -1,		// IP地址非法
	ETC_SOCKET_CREATE_FAILED	= -2,		// socket创建失败
	ETC_SET_BUFF_SIZE_FAILED	= -3,		// 设置缓冲区大小失败
	ETC_CONNECT_FAILED			= -4,		// 连接失败
	ETC_ARRAY_IS_NULL			= -5,		// 接收数组为null
	ETC_BUFFER_POINTER_ERROR	= -6,		// 缓冲区指针错误
	ETC_MSG_LENGTH_ERROR		= -7,		// 消息长度出错
	ETC_MSG_IS_TOO_LONG			= -8,		// 消息过长
	ETC_SOCKET_NOT_CONNECT		= -9,		// 该socket未连接
	ETC_BUFFER_IS_FULL			= -10,		// 缓冲区已满
	ETC_REMOTE_CLOSE			= -11,		// 远端关闭
	ETC_OTHER_ERROR				= -12,		// 其他错误
	ETC_NO_BUFFER_TO_SAVE		= -13,		// 发送失败，并且没有缓冲区来存储
	ETC_RECV_NO_MSG				= -14,		// 没有接收到消息
};

template <typename T, unsigned int nRecvBufLen, unsigned int nSendBufLen>
class wTcpClient : public wSocket , public  wSingleton<T>
{
	public:
		unsigned int mTimeout;
		unsigned int mEpollFD;
		wTcpTask *mTcpTask;
		
		//epoll_event
		struct epoll_event mEpollEvent;
		vector<struct epoll_event> mEpollEventPool;
		int mTaskCount;

		wTcpClient()
		{
			Initialize();
		}

		~wTcpClient() {}
		
		//初始化
		void Initialize()
		{
			unsigned int mTimeout = 10;
			mEpollFD = -1;
			mTaskCount = 1;
			mEpollEventPool.resize(1);	//只有一个客户端，就是自己

			mRecvFirst = 0;
			mRecvLast = 0;
			memset(mRecvBuffer, 0, nRecvBufLen);

			mSendFirst = 0;
			mSendLast = 0;
			memset(mSendBuffer, 0, nRecvBufLen);
		}

		//关闭连接
		virtual void Close()
		{
			wSocket::Close();
			mRecvFirst = 0;
			mRecvLast = 0;
			mSendFirst = 0;
			mSendLast = 0;
		}

		virtual void PrepareRun() {}

		/**
		 * 准备|启动服务
		 */
		void PrepareStart(string sIpAddr ,unsigned int nPort)
		{
			LOG_INFO("default", "Client Prepare start succeed");

			wSocket::Close();

			mIPAddr = sIpAddr;
			mPort = nPort;
			
			//初始化epoll
			InitEpoll();
			
			//初始化连接到Server
			if (ConnectToServer() < 0)
			{
				LOG_INFO("default", "Client ConnectToServer faild.\n");
			}

			//运行前工作
			PrepareRun();
		}
		
		void InitEpoll()
		{
			mEpollFD = epoll_create(512); //512
			if(mEpollFD < 0)
			{
				LOG_ERROR("default", "epoll_create failed: %s", strerror(errno));
				exit(1);
			}
		}

		void Start()
		{
			LOG_INFO("default", "Client start succeed");
			//进入客户端主循环
			while(mSocketFD != -1)
			{	
				//接受服务端请求
				Recv();

				//定制服务
				Run();
			}
		}
		
		virtual void Run() {}

		void Recv()
		{
			int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], 1, mTimeout);
			if(iRet < 0)
			{
				LOG_ERROR("default", "epoll_wait failed: %s", strerror(errno));
				return;
			}
			socklen_t iSockAddrSize = sizeof(struct sockaddr_in);
			
			for(int i = 0 ; i < iRet ; i++)
			{
				wTcpTask *vTask = (wTcpTask *)mEpollEventPool[i].data.ptr;	//TcpTask对象
				if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))
				{
					LOG_ERROR("default", "epoll event recv error from fd(%d): %s, close it", vTask->mSocketFD, strerror(errno));
					if (RemoveEpoll(vTask))
					{
						RemoveTaskPool(vTask);
						//vTask->Terminate(CLOSE_REASON vReason);
					}
				}
				else
				{
					if (mEpollEventPool[i].events & EPOLLIN)
					{
						//套接口准备好了读取操作
						if (vTask->ListeningRecv() < 0)
						{	
							LOG_ERROR("default", "EPOLLIN(read) failed: %s", strerror(errno));
							if (RemoveEpoll(vTask))
							{
								RemoveTaskPool(vTask);
								//vTask->Terminate(CLOSE_REASON vReason);
							}
						}
					}
					if (mEpollEventPool[i].events & EPOLLOUT)
					{
						//套接口准备好了写入操作
						if (vTask->ListeningSend() < 0)
						{
							LOG_ERROR("default", "EPOLLOUT(write) failed: %s", strerror(errno));
							if (RemoveEpoll(vTask))
							{
								RemoveTaskPool(vTask);
								//vTask->Terminate(CLOSE_REASON vReason);
							}
						}
					}
				}
			}
		}

		int RemoveEpoll(wTcpTask* stTcpTask)
		{
			int iSocketFD = stTcpTask->mSocketFD;
			mEpollEvent.data.fd = iSocketFD;
			if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iSocketFD, &mEpollEvent) < 0)
			{
				LOG_ERROR("default", "epoll remove socket fd(%d) error : %s", iSocketFD, strerror(errno));
				return -1;
			}
			stTcpTask->Close();
			return 0;
		}

		void RemoveTaskPool(wTcpTask* stTcpTask)
		{
			/*
		    std::vector<wTcpTask*>::iterator itPosition = std::find(mTcpTaskPool.begin(), mTcpTaskPool.end(), stTcpTask);
		    if(itPosition != mTcpTaskPool.end())
		    {
		    	(*itPosition)->Close();
		        mTcpTaskPool.erase(itPosition);
		    }
		    mTaskCount = mTcpTaskPool.size();
		    */
		}

		//通过内部存储的IP端口进行连接
		int ConnectToServer()
		{
			//创建socket
			mSocketFD = socket(AF_INET, SOCK_STREAM, 0);
			if( mSocketFD < 0 )
			{
				LOG_ERROR("default", "create socket failed: %s", strerror(errno));
				return ETC_SOCKET_CREATE_FAILED;
			}

			sockaddr_in stSockAddr;
			memset(&stSockAddr, 0, sizeof(sockaddr_in));
			stSockAddr.sin_family = AF_INET;
			stSockAddr.sin_port = htons(mPort);
			stSockAddr.sin_addr.s_addr = inet_addr(mIPAddr.c_str());

			socklen_t iOptVal = nSendBufLen;
			socklen_t iOptLen = sizeof(int);
			// 设置发送缓冲区大小
			if(setsockopt(mSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) != 0)
			{
				LOG_ERROR("default", "set send buffer size to %d failed: %s", iOptVal, strerror(errno));
				Close();
				return ETC_SET_BUFF_SIZE_FAILED;
			}

			//获取设置完的值
			if(getsockopt(mSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, &iOptLen) == 0)
			{
				LOG_INFO("default", "set send buffer of socket is %d", iOptVal);
			}

			//连接服务端
			int iRet = connect(mSocketFD, (const struct sockaddr *)&stSockAddr, sizeof(stSockAddr));
			if( iRet < 0 )
			{
				LOG_ERROR("default", "connect to server port(%d) failed: %s", mPort, strerror(errno));
				Close();
				return ETC_CONNECT_FAILED;
			}

			LOG_INFO("default", "connect to %s:%d successfully", inet_ntoa(stSockAddr.sin_addr), mPort);

			//new tcp task
			mTcpTask = NewTcpTask(mSocketFD, &stSockAddr);
			if(NULL != mTcpTask)
			{
				if(mTcpTask->SetNonBlock() < 0) 
				{
					LOG_ERROR("default", "set non block failed: %d, close it", mSocketFD);
					mTcpTask->Close();
				}
				return AddToEpoll(mTcpTask);
			}
			return -1;
		}

		int AddToEpoll(wTcpTask* stTcpTask)
		{
			mEpollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;	//客户端事件
			mEpollEvent.data.fd = stTcpTask->mSocketFD;
			mEpollEvent.data.ptr = stTcpTask;
			int iRet = epoll_ctl(mEpollFD, EPOLL_CTL_ADD, stTcpTask->mSocketFD, &mEpollEvent);
			if(iRet < 0)
			{
				LOG_ERROR("default", "fd(%d) add into epoll failed: %s", stTcpTask->mSocketFD, strerror(errno));
				return -1;
			}
			return 0;
		}

		virtual wTcpTask* NewTcpTask(int iNewSocket, struct sockaddr_in *stSockAddr)
		{
			wTcpTask *pTcpTask = new wTcpTask(iNewSocket, stSockAddr);
			return pTcpTask;
		}

		// 发送一个消息
		// 返回值
		// < 0 出错
		// = 0 socket缓冲区已满，消息存入发送缓冲区
		// > 0 发送成功
		int SendOneMsg(char *vArray, int vLength)
		{
			//判断接受数组是否为空
			if(vArray == NULL) 
			{
				LOG_ERROR("default", "SendOneMsg: vArray is NULL");
				return ETC_ARRAY_IS_NULL;
			}

#ifdef _USE_THREAD_
			pthread_mutex_lock( &stMutex );
#endif

			//判断是否连接
			if(mSocketFD < 0) 
			{
#ifdef _USE_THREAD_
				pthread_mutex_unlock( &stMutex );
#endif

				LOG_ERROR("default", "SendOneMsg: socket not ready");
				return ETC_SOCKET_NOT_CONNECT;
			}

			//是否还有上一次的剩余数据
			int iBytesLeft = mSendLast - mSendFirst;
			char *pSendBuff = &(mSendBuffer[mSendFirst]);
			while (iBytesLeft > 0) 
			{
				 int iBytesSent = send(mSocketFD, (const void *)pSendBuff, iBytesLeft, 0);
				 if(iBytesSent > 0) 
				 {
					 pSendBuff += iBytesSent;
					 iBytesLeft -= iBytesSent;
					 mSendFirst += iBytesSent;
				 }
				 //如果出错，需要关闭连接
				 else if(iBytesSent < 0 && errno != EAGAIN && errno != EINTR) 
				 {
					 Close();
#ifdef _USE_THREAD_
					 pthread_mutex_unlock( &stMutex );
#endif

					 LOG_ERROR("default", "SendOneMsg failed: %s, fd(%d) close connect", strerror(errno), mSocketFD);
					 return ETC_OTHER_ERROR;
				 }
				 //如果是缓冲区满
				 else if(iBytesSent < 0 && errno == EAGAIN) 
				 {
					 if(mSendLast + vLength <= (int)nSendBufLen - 1) 
					 {
						 memcpy((void *)&(mSendBuffer[mSendLast+1]), (const void *)vArray, vLength);
						 mSendLast += vLength;
#ifdef _USE_THREAD_
						 pthread_mutex_unlock( &stMutex );
#endif

						 LOG_DEBUG("default", "SendOneMsg failed: %s, fd(%d) save to buffer" , strerror(errno), mSocketFD);
						 return 0;
					 }
					 else
					 {
						 //这种情况缓冲区已经无法存入，只能丢失
#ifdef _USE_THREAD_
						 pthread_mutex_unlock(&stMutex);
#endif

						 LOG_ERROR("default", "SendOneMsg failed: %s, fd(%d) no more buffer to save" , strerror(errno), mSocketFD);
						 return ETC_NO_BUFFER_TO_SAVE;
					 }
				 }
				 //如果是被信号中断，则再次尝试
				 else if(iBytesSent < 0) 
				 {
					 continue;
				 }
			}

			//只要能走到这里
			//必然已经把所有的滞留数据全部发完了
			mSendFirst = 0;
			mSendLast = 0;

			iBytesLeft = vLength;
			pSendBuff = vArray;

			//继续发送当前需要发送的数据
			while (iBytesLeft > 0) 
			{
				int iBytesSent = send(mSocketFD, (const void *)pSendBuff, iBytesLeft, 0);
				if(iBytesSent > 0) 
				{
					pSendBuff += iBytesSent;
					iBytesLeft -= iBytesSent;
				}
				//如果出错则关闭连接
				else if(iBytesSent < 0 && errno != EAGAIN && errno != EINTR) 
				{
					 Close();
#ifdef _USE_THREAD_
					 pthread_mutex_unlock( &stMutex );
#endif

					 LOG_ERROR("default", "SendOneMsg 2st failed: %s, fd(%d) close connect", strerror(errno), mSocketFD);
					 return ETC_OTHER_ERROR;
				}
				//如果被信号中断则继续
				else if(iBytesSent < 0 && errno == EINTR) 
				{
					continue;
				}
				//如果是缓冲区满
				else if(iBytesSent < 0) 
				{
					memcpy((void *)&(mSendBuffer[mSendLast+1]), (const void *)pSendBuff, iBytesLeft);
					mSendLast += iBytesLeft;
#ifdef _USE_THREAD_
					pthread_mutex_unlock( &stMutex );
#endif

					LOG_DEBUG("default", "SendOneMsg failed: %s, fd(%d) save to buffer" , strerror(errno), mSocketFD);
					return 0;
				}
			}

#ifdef _USE_THREAD_
			pthread_mutex_unlock( &stMutex );
#endif
			return 1;
		}

		// 获取一个消息
		// 返回值
		// < 0 出错
		// = 0 没有消息可取
		// > 0 取到一个消息
		int GetOneMsg(char *vArray, int &vLength)
		{
			//判断接受数组是否为空
			if(vArray == NULL)
			{
				LOG_ERROR("default", "GetOneMsg: the given array is NULL");
				return ETC_ARRAY_IS_NULL;
			}

			int iMaxRecvLen = vLength;
			int iDataLen = mRecvLast - mRecvFirst;
			//如果消息长度为负值
			if(iDataLen < 0)
			{
				LOG_ERROR("default", "GetOneMsg: buffer pointer is error: first(%d), last(%d)", mRecvFirst, mRecvLast);
				return ETC_BUFFER_POINTER_ERROR;
			}
			//现在暂无信息
			else if(iDataLen == 0)
			{
				return 0;
			}

			//如果长度不够获取消息长度所需
			if(iDataLen < (int)sizeof(int))
			{
				//如果mRecvLast已经到达缓冲区结尾
				//则移动该消息到缓冲区头部
				if(mRecvLast == nRecvBufLen - 1)
				{
					memmove((void *)&mRecvBuffer[0], (const void *)&mRecvBuffer[mRecvFirst], iDataLen);
					mRecvFirst = 0;
					mRecvLast = iDataLen;
					return 0;
				}

				// 很可能还有一部分还没有接过来
				LOG_INFO("default", "GetOneMsg: iDataLen(%d) < sizeof(int)", iDataLen);
				return 0;
			}

			//获取第一个消息的长度
			unsigned int iMsgLen = *((unsigned int *)&(mRecvBuffer[mRecvFirst]));
			//如果消息长度出错，则直接断开连接
			if(iMsgLen == 0 || iMsgLen + sizeof(unsigned int) > sizeof(mRecvBuffer))
			{
				Close();
				LOG_ERROR("default", "GetOneMsg: iMsgLen(%d), mRecvBuffer length(%u)", iMsgLen, sizeof(mRecvBuffer));
				return ETC_MSG_LENGTH_ERROR;
			}

			//如果消息还没有接完整
			if( (int)iMsgLen > iDataLen)
			{
				//如果已经到了缓冲区末尾
				if(mRecvLast == nRecvBufLen - 1)
				{
					memmove((void *)&mRecvBuffer[0], (const void *)&mRecvBuffer[mRecvFirst], iDataLen);
					mRecvFirst = 0;
					mRecvLast = iDataLen;
				}

				return 0;
			}

			vLength = iMsgLen;

			//如果消息长度大于接收消息的缓冲区
			//则断开连接
			if(vLength >= iMaxRecvLen)
			{
				Close();
				LOG_ERROR("default", "GetOneMsg: msg len(%d), get msg buffer len(%d)", vLength, iMaxRecvLen);
				return ETC_MSG_IS_TOO_LONG;
			}

			memcpy((void *)vArray, (const void *)&mRecvBuffer[mRecvFirst+sizeof(int)], vLength);

			// 处理一下接收指针
			mRecvFirst += vLength+sizeof(int);

			if( mRecvFirst == mRecvLast)
			{
				mRecvFirst = 0;
				mRecvLast = 0;
			}

			return 1;
		}

		//从网络上获取消息
		//为什么接收消息不需要线程锁？
		//谁都不会搞几个线程同时接收消息的
		//不然他肯定疯了
		int ReceiveMsg()
		{
			//检查socket是否已经连接
			if(mSocketFD < 0) 
			{
				LOG_ERROR("default", "ReceiveMsg: socket not ready");
				return ETC_SOCKET_NOT_CONNECT;
			}

			int iRecvedBytes = 0;
			bool bRecvFlag = false;

			do {
				//如果缓冲区已满
				if(mRecvLast == nRecvBufLen - 1) 
				{
					LOG_ERROR("default", "the recv buffer is full now fd(%d), first(%d), last(%d)", mSocketFD, mRecvFirst, mRecvLast);
					return ETC_BUFFER_IS_FULL;
				}

				iRecvedBytes = recv(mSocketFD, &mRecvBuffer[mRecvLast], nRecvBufLen - 1 - mRecvLast, 0);
				if(iRecvedBytes > 0) 
				{
					mRecvLast += iRecvedBytes;
					bRecvFlag = true;
				}
				// 远端关闭
				else if(iRecvedBytes == 0) 
				{
					LOG_ERROR("default", "connect close by remote from fd(%d): %s", mSocketFD, strerror(errno));
					Close();
					return ETC_REMOTE_CLOSE;
				}
				//其他错误
				else if(errno != EAGAIN && errno != EINTR) 
				{
					LOG_ERROR("default", "socket receive error from fd(%d): %s, close connect", mSocketFD, strerror(errno));
					Close();
					return ETC_OTHER_ERROR;
				}
				// 没有消息
				else if(errno == EAGAIN)
				{
					break;
				}
			} while (1);

			if(bRecvFlag)
			{
				return 0;
			}
			return ETC_RECV_NO_MSG;
		}

	private:
		int mRecvFirst;		// 接收消息的首指针(数组地址)
		int mRecvLast;		// 接收消息的末指针(数组地址)
		char mRecvBuffer[nRecvBufLen];	// 接收消息的缓冲区

		int mSendFirst;		// 发送消息的首指针(数组地址)
		int mSendLast;		// 发送消息的末指针(数组地址)
		char mSendBuffer[nSendBufLen];	// 发送消息的缓冲区

#ifdef _USE_THREAD_
		pthread_mutex_t stMutex;	// 如果用到线程，需要的线程锁
#endif
};

#endif
