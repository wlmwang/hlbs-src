
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
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
		int Type() { return mType; }
		
		wTcpTask* TcpTask() { return mTcpTask; }
		virtual wTcpTask* NewTcpTask(wIO *pIO);
		
	protected:
		int mType;
		string mClientName;		
		wTcpTask* mTcpTask;
		CLIENT_STATUS mStatus;

		unsigned long long mLastTicker;
		wTimer mReconnectTimer;
		int mReconnectTimes;
		bool mIsCheckTimer;
};

#include "wTcpClient.inl"

#endif
