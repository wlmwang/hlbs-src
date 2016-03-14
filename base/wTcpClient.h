
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
#include "wTcpTask.h"

template <typename T>
class wTcpClient
{
	public:
		virtual ~wTcpClient();
		
        wTcpClient(int iType, string sClientName);
	    	
		wTcpClient(const wTcpClient&);
		
		void Initialize();
		
		int ConnectToServer(const char *vIPAddress, unsigned short vPort);

		int ReConnectToServer();
		
		int GetType() { return mType; }
		
		void Final();
		
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);

		virtual void PrepareStart();
		virtual void Start(bool bDaemon = true);

		void CheckTimer();
		void CheckReconnect();

		bool IsRunning() { return mStatus = CLIENT_RUNNING; }
		void SetStatus(CLIENT_STATUS eStatus = CLIENT_QUIT) { mStatus = eStatus; }
		CLIENT_STATUS GetStatus() { return mStatus; }
		string &GetClientName() { return mClientName; }
		
		wTcpTask* TcpTask() { return mTcpTask; }
		
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
