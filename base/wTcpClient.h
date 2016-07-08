
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_TCP_CLIENT_H_
#define _W_TCP_CLIENT_H_

#include <typeinfo>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "wCore.h"
#include "wTimer.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSocket.h"
#include "wIO.h"
#include "wTask.h"
#include "wTcpTask.h"
#include "wNoncopyable.h"

template <typename T>
class wTcpClient : private wNoncopyable
{
	public:
        wTcpClient(int iType, string sClientName);
		wTcpClient(const wTcpClient&);
		virtual ~wTcpClient();
		void Final();

		virtual void PrepareStart();
		virtual void Start(bool bDaemon = true);

		int ConnectToServer(const char *vIPAddress, unsigned short vPort);
		int ReConnectToServer();
		
		void CheckTimer();
		virtual void CheckTimeout();
		virtual void CheckReconnect();

		bool &IsCheckTimer() { return mIsCheckTimer;}
		string &ClientName() { return mClientName; }
		bool IsRunning() { return mStatus = CLIENT_RUNNING; }
		CLIENT_STATUS &Status() { return mStatus; }
		int Type() { return mType; }

		virtual wTcpTask* NewTcpTask(wIO *pIO);
		wTcpTask* TcpTask() { return mTcpTask; }
		
	protected:
		int mErr;
		string mClientName;
		int mType {0};	//服务器类型(CLIENT_TYPE)
		wTcpTask* mTcpTask {NULL};	//每个客户端只对应一个task
		CLIENT_STATUS mStatus {CLIENT_INIT};

		unsigned long long mLastTicker {0};
		wTimer mCheckTimer;
		wTimer mReconnectTimer;
		int mReconnectTimes {0};
		bool mIsCheckTimer {false};
		bool mIsReconnect {true};
};

#include "wTcpClient.inl"

#endif
