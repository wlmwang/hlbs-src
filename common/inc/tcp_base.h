//--------------------------------------------------
// 该文件描述了网络TCP套接字的基础类
//-------------------------------------------------- 
#ifndef _TCP_BASE_H_
#define _TCP_BASE_H_

#include <unistd.h>
#include <fcntl.h>

// 套接字状态
enum ESocketStatus
{
	SOCKET_STATUS_UNCONNECT	= 0,	// 关闭状态
	SOCKET_STATUS_CONNECTED,		// 已经建立连接
	SOCKET_STATUS_LISTEN			// 已经处于监听状态
};

class CTCPBase
{
	public:
		CTCPBase()
		{
			Initialize();
		}

		virtual ~CTCPBase()
		{
			Close();
		}

		int GetSocketFD()
		{
			return mSocketFD;
		}

		// 初始化
		virtual void Initialize()
		{
			mSocketFD = -1;
			mIPAddress = 0;
			mPort = 0;
		}

		bool IsConnected()
		{
			return mSocketFD != -1;
		}

		// 关闭连接
		virtual void Close()
		{
			if( mSocketFD >= 0 ) 
			{
				close(mSocketFD);
			}

			mSocketFD = -1;
		}

		unsigned int GetIPAddress()
		{
			return mIPAddress;
		}

		unsigned short GetPort()
		{
			return mPort;
		}

	protected:
		// 将套接字设置为非阻塞
		void SetNoBlock()
		{
			if( mSocketFD < 0 ) 
			{
				return;
			}

			int iFlags = fcntl(mSocketFD, F_GETFL, 0);
			if( iFlags == -1 ) 
			{
				return;
			}

			fcntl(mSocketFD, F_SETFL, iFlags|O_NONBLOCK);
		}

		int mSocketFD;				// 网络套接字描述符
		unsigned int mIPAddress;	// 需要连接或者需要绑定的IP地址
		unsigned short mPort;		// 需要连接或者需要绑定的端口
};

#endif
