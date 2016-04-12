
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

template <typename T>
class wTcpClient
{
	public:
        wTcpClient(int iType, string sClientName);
		wTcpClient(const wTcpClient&);
		virtual ~wTcpClient();
		void Initialize();
		void Final();

		virtual void PrepareStart();
		virtual void Start(bool bDaemon = true);

		int ConnectToServer(const char *vIPAddress, unsigned short vPort);
		int ReConnectToServer();
		
		void CheckTimer();
		void CheckReconnect();
		
		string &ClientName() { return mClientName; }
		bool IsRunning() { return mStatus = CLIENT_RUNNING; }
		CLIENT_STATUS &Status() { return mStatus; }
		bool &IsCheckTimer() { return mIsCheckTimer;}
		int Type() { return mType; }

		virtual wTcpTask* NewTcpTask(wIO *pIO);
		wTcpTask* TcpTask() { return mTcpTask; }
		
	protected:
		int mType;	//服务器类型(CLIENT_TYPE)
		string mClientName;		
		wTcpTask* mTcpTask;	//每个客户端只对应一个task
		CLIENT_STATUS mStatus;

		unsigned long long mLastTicker;
		wTimer mReconnectTimer;
		int mReconnectTimes;
		bool mIsCheckTimer;

		int mErr;
};

#include "wTcpClient.inl"

#endif
