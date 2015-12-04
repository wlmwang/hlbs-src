//--------------------------------------------------
// 本文用于管理包括拥有最多iMaxListenNum个监听端口
// 最多iMaxClientNum个主动连接到监听端口的连接
// 最多iMaxAcceptNum个由监听端口accept的连接
//
// 对于mListenConnect和mClientConnect
// 数量是固定不会改变的，断连以后再次尝试连接
//
// 对于mAcceptConnect
// mAcceptCtrlArray的前mAcceptConnectNum元素
// 对应着mAcceptConnect中相应的数组位置
//
// 由于所有的客户端消息都通过gateserver
// 只要连接到gateserver的消息都由gateserver去listen
// 那么只需要在mClientConnect中有SrcFlag就可以了
//
// 最后再罗嗦一句，这个模块有点设计过度了
//-------------------------------------------------- 
#ifndef _TCP_CTRL_H_
#define _TCP_CTRL_H_

#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "tcp_client.h"
#include "tcp_listen.h"
#include "server_ctrl.h"
#include "my_assert.h"
#include "core_type.h"

template <unsigned int nRecvBufLen, unsigned int nSendBufLen,
		 int iMaxListenNum, int iMaxClientNum, int iMaxAcceptNum>
class CTCPCtrl
{
	public:
		typedef void (*RemoveAcceptFD)(int vPos);
		CTCPCtrl()
		{
			if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT )
			{
				Initialize();
			}

			// resume模式这个函数指针必须再次指定，防止指针位置的变化
			mRemoveAcceptFDProc = NULL;
		}

		~CTCPCtrl() {}

		// 初始化
		void Initialize()
		{
			mAcceptConnectNum = 0;
			for( int i = 0 ; i < iMaxAcceptNum ; i++ )
			{
				mAcceptCtrlArray[i] = i;
			}

			memset(mClientMsgSrcFlag, 0, iMaxClientNum);
		}

		// 将某个监听数组元素设置监听，默认0为接收消息来自服务器，1为客户端的消息
		int ListenToAddress(int vConnectNum, char *vIPAddress, unsigned short vPort)
		{
			MY_ASSERT(vConnectNum >= 0 && vConnectNum < iMaxListenNum, return -99);

			return mListenConnect[vConnectNum].ListenToAddress(vIPAddress, vPort);
		}

		// 将某个client数组元素连接到监听端，默认0为接收消息来自服务器，1为客户端的消息
		int ConnectToServer(int vConnectNum, char *vIPAddress, unsigned short vPort, int vSrcFlag = 0)
		{
			MY_ASSERT(vConnectNum >= 0 && vConnectNum < iMaxClientNum, return -99);

			int iRet = mClientConnect[vConnectNum].ConnectToServer(vIPAddress, vPort);
			if( iRet == 0 )
			{
				mClientMsgSrcFlag[vConnectNum] = vSrcFlag;
			}
			return iRet;
		}

		// 检查网络连接，如果有断连的则再次建立连接
		void CheckConnectStatus()
		{
			for( int i = 0; i < mAcceptConnectNum; i++ ) 
			{
				int iAcceptPos = mAcceptCtrlArray[i];
				CTCPClient<nRecvBufLen, nSendBufLen> *pAccept = &mAcceptConnect[iAcceptPos];
				if( !pAccept->IsConnected() )
				{
					DeleteOneAcceptConn(i);
					// 如果有需要回调处理accept的处理
					if( mRemoveAcceptFDProc != NULL )
					{
						mRemoveAcceptFDProc(iAcceptPos);
					}

					if( i != mAcceptConnectNum )
					{
						i--;
					}
				}
			}

			for( int i = 0 ; i < iMaxListenNum ; i++ )
			{
				CTCPListen *pListen = &mListenConnect[i];
				if( !pListen->IsConnected() )
				{
					pListen->ListenToAddress();
				}
			}
			
			for( int i = 0 ; i < iMaxClientNum ; i++ )
			{
				CTCPClient<nRecvBufLen, nSendBufLen> *pClient = &mClientConnect[i];
				if( !pClient->IsConnected() )
				{
					pClient->ConnectToServer();
				}
			}
		}

		// 接收消息并处理消息，顺便接受连接请求
		// 回调函数的四个参数：
		// 1.缓冲区的指针
		// 2.缓冲区中的消息的长度
		// 3.发消息过来的FD对应的结构体指针
		// 4.是否客户端发过来的消息
		// 5.该FD在mAcceptConnect数组中的位置，如果在mClientConnect中则设置为-1
		// 如果accept连接来注册，则可以在外部处理消息时注册相应的连接类型
		void ReceiveAndProcessMessage(int (*ProcessMsg)(char *, int, CTCPClient<nRecvBufLen, nSendBufLen> &, int, int))
		{
			fd_set fds_read;
			struct timeval stTimeVal;
			stTimeVal.tv_sec = 0;
			//stTimeVal.tv_usec = 500;
			stTimeVal.tv_usec = 10000;
			int iMaxFD = 0;

			FD_ZERO( &fds_read );

			for ( int i = 0; i < iMaxListenNum; i++ ) 
			{
				if( mListenConnect[i].IsConnected() ) 
				{
					int iSocketFD = mListenConnect[i].GetSocketFD();
					FD_SET(iSocketFD, &fds_read);
					if( iSocketFD > iMaxFD ) 
					{
						iMaxFD = iSocketFD;
					}
				}
			}

			for ( int i = 0; i < iMaxClientNum; i++ ) 
			{
				if( mClientConnect[i].IsConnected() ) 
				{
					int iSocketFD = mClientConnect[i].GetSocketFD();
					FD_SET(iSocketFD, &fds_read);
					if( iSocketFD > iMaxFD ) 
					{
						iMaxFD = iSocketFD;
					}
				}
			}

			for ( int i = 0; i < mAcceptConnectNum; i++ ) 
			{
				int iAcceptPos = mAcceptCtrlArray[i];
				if( mAcceptConnect[iAcceptPos].IsConnected() ) 
				{
					int iSocketFD = mAcceptConnect[iAcceptPos].GetSocketFD();
					FD_SET(iSocketFD, &fds_read);
					if( iSocketFD > iMaxFD ) 
					{
						iMaxFD = iSocketFD;
					}
				}
			}

			int iRet = select(iMaxFD + 1, &fds_read, NULL, NULL, &stTimeVal);
			if( iRet == 0 ) 
			{
				// 没有任何消息，睡一微秒
				usleep(1);
				return;
			}
			else if( iRet < 0 )
			{
				LOG_ERROR("default", "select failed: %s", strerror(errno));
				return;
			}

			//int flag = false;
			for ( int i = 0; i < iMaxListenNum; i++ ) 
			{
				if( mListenConnect[i].IsConnected() ) 
				{
					int iSocketFD = mListenConnect[i].GetSocketFD();
					if( FD_ISSET(iSocketFD, &fds_read) ) 
					{
						struct sockaddr_in stAcceptAddr;
						memset(&stAcceptAddr, 0, sizeof(stAcceptAddr));
						socklen_t iAddrLen = sizeof(stAcceptAddr);
						int iNewFD = accept(iSocketFD, (struct sockaddr *)&stAcceptAddr, &iAddrLen);
						if( iNewFD < 0 ) 
						{
							continue;
						}

						//flag = true;
						unsigned short nPort = ntohs(stAcceptAddr.sin_port);
						if( AddOneAcceptConn(iNewFD, i, stAcceptAddr.sin_addr.s_addr, nPort) < 0 )
						{
							LOG_ERROR("get a new connect from %s:%d, fd(%d), but too many connect, close it", inet_ntoa(stAcceptAddr.sin_addr), nPort, iNewFD);
							close(iNewFD);
						}

						LOG_INFO("get a new connect from %s:%d, fd(%d)", inet_ntoa(stAcceptAddr.sin_addr), nPort, iNewFD);
					}
				}
			}

			static char szRecvBuff[MAX_PACKAGE_LEN];
			int iBuffLen;
			for ( int i = 0; i < iMaxClientNum; i++ ) 
			{
				if( mClientConnect[i].IsConnected() ) 
				{
					int iSocketFD = mClientConnect[i].GetSocketFD();
					if( FD_ISSET(iSocketFD, &fds_read) ) 
					{
						if( mClientConnect[i].ReceiveMsg() == 0 )
						{
							int iRet;
							while( 1 )
							{
								iBuffLen = sizeof(szRecvBuff);
								iRet = mClientConnect[i].GetOneMsg(szRecvBuff, iBuffLen);
								if( iRet <= 0 ) 
								{
									// 跳出内层循环
									break;
								}
								//flag = true;

								ProcessMsg(szRecvBuff, iBuffLen, mClientConnect[i], mClientMsgSrcFlag[i], -1);
							}
						}
					}
				}
			}

			for ( int i = 0; i < mAcceptConnectNum; i++ ) 
			{
				int iAcceptPos = mAcceptCtrlArray[i];
				if( mAcceptConnect[iAcceptPos].IsConnected() ) 
				{
					int iSocketFD = mAcceptConnect[iAcceptPos].GetSocketFD();
					if( FD_ISSET(iSocketFD, &fds_read) ) 
					{
						if( mAcceptConnect[iAcceptPos].ReceiveMsg() == 0 ) 
						{
							int iRet;
							while ( 1 ) 
							{
								iBuffLen = sizeof(szRecvBuff);
								iRet = mAcceptConnect[iAcceptPos].GetOneMsg(szRecvBuff, iBuffLen);
								if( iRet <= 0 ) 
								{
									// 跳出内层循环
									break;
								}
								//flag = true;

								ProcessMsg(szRecvBuff, iBuffLen, mAcceptConnect[iAcceptPos], 0, iAcceptPos);
							}
						}
					}
				}
			}
			
			//--------------------------------------------------
			// if( flag == false )
			// {
			// 	usleep(1);
			// }
			//-------------------------------------------------- 
		}

		CTCPClient<nRecvBufLen, nSendBufLen> *GetClientConnect(int vClientNum)
		{
			MY_ASSERT(vClientNum >= 0 && vClientNum < iMaxClientNum, return NULL);
			return &(mClientConnect[vClientNum]);
		}

		CTCPClient<nRecvBufLen, nSendBufLen> *GetAcceptConnect(int vAcceptNum)
		{
			MY_ASSERT(vAcceptNum >= 0 && vAcceptNum < mAcceptConnectNum, return NULL);
			return &(mAcceptConnect[vAcceptNum]);
		}

		CTCPListen *GetListenConnect(int vListenNum)
		{
			MY_ASSERT(vListenNum >= 0 && vListenNum < iMaxListenNum, return NULL);
			return &(mListenConnect[vListenNum]);
		}

		RemoveAcceptFD mRemoveAcceptFDProc;			// 删除已经accept的连接的外部回调函数，因为是accept的连接是什么样连接需要在分析消息体时才能确定，而删除accept则必须在网络层确定

	private:
		// 添加一个accept的连接
		// 第一个参数:accept之后的socket fd
		// 第二个参数:监听端口的位置
		int AddOneAcceptConn(int vNewSocketFD, int vListenPos, unsigned int vIPAddress, unsigned short vPort)
		{
			MY_ASSERT(mAcceptConnectNum < iMaxAcceptNum && "too many connect now", return -1);

			int iAcceptNum = mAcceptCtrlArray[mAcceptConnectNum];
			mAcceptConnect[iAcceptNum].GetAcceptFD(vNewSocketFD, vIPAddress, vPort);

			mAcceptConnectNum++;

			return 0;
		}

		// 删除一个accept的连接
		// 参数:在该FD在mAcceptCtrlArray中的位置
		int DeleteOneAcceptConn(int vAcceptCtrlPos)
		{
			MY_ASSERT(mAcceptConnectNum > 0 && vAcceptCtrlPos < mAcceptConnectNum, return -1);

			int iDeleteNum = mAcceptCtrlArray[vAcceptCtrlPos];
			mAcceptConnect[iDeleteNum].Close();

			if( mAcceptConnectNum - 1 != vAcceptCtrlPos )
			{
				// 交换mAcceptCtrlArray中的指定元素和最后一个元素
				int iTemp = mAcceptCtrlArray[mAcceptConnectNum-1];
				mAcceptCtrlArray[mAcceptConnectNum-1] = mAcceptCtrlArray[vAcceptCtrlPos];
				mAcceptCtrlArray[vAcceptCtrlPos] = iTemp;
			}

			mAcceptConnectNum--;
			return 0;
		}

		CTCPListen mListenConnect[iMaxListenNum];	// 所有的监听端口
		CTCPClient<nRecvBufLen, nSendBufLen> mClientConnect[iMaxClientNum];	// 所有的主动连接监听端口的连接
		CTCPClient<nRecvBufLen, nSendBufLen> mAcceptConnect[iMaxAcceptNum];	// 所有被listen端口监听所接受的端口
		int mClientMsgSrcFlag[iMaxClientNum];		// 标识client的接收消息的来源(0表示服务器之间，1表示客户端的消息)
		int mAcceptCtrlArray[iMaxAcceptNum];		// 用于控制accept的连接的数组
		int mAcceptConnectNum;						// accept的连接的数量
};

#endif
