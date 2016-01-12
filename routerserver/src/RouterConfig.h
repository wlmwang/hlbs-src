
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <unistd.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <sys/stat.h>

#include "wType.h"
#include "wConfig.h"
#include "wLog.h"
#include "Svr.h"

/**
 * 配置文件读取的数据结构
 */
class RouterConfig: public wConfig<RouterConfig>
{
	public:
		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;
		unsigned int mWorkers;

		void Final();
		virtual ~RouterConfig() 
		{
			Final();
			SAFE_DELETE(mDoc);
		}

		//初始化
		void Initialize()
		{
			memset(mIPAddr, 0, sizeof(mIPAddr));
			mPort = 0;
			mBacklog = 1024;
			mWorkers = 1;
			mDoc = new TiXmlDocument();
		}

		RouterConfig()
		{
            Initialize();
		}

		bool IsModTime();
		int SetModTime();
		int GetModSvr(Svr_t* pBuffer);

		/**
		 * 解析配置
		 */
		void ParseBaseConfig();
		
		/**
		 *  解析Svr配置
		 */
		void ParseSvrConfig();
		int GetSvrAll(Svr_t* pBuffer);
		int ReloadSvr(Svr_t* pBuffer);
		int SyncSvr(Svr_t* pBuffer);
		
		int GetSvrById(Svr_t* pBuffer, int iId);
		int GetSvrByName(Svr_t* pBuffer, string sName, int iNum = 0);
		int GetSvrByGid(Svr_t* pBuffer, int iGid, int iNum = 0);
		int GetSvrByGXid(Svr_t* pBuffer, int iGid, int iXid, int iNum = 0);
	protected:
		void FixContainer();
		void DelContainer();
		vector<Svr_t*>::iterator GetItById(int iId);
		
		vector<Svr_t*> mSvr;
		map<int, Svr_t*> mSvrById;
		map<int, vector<Svr_t*> > mSvrByGid;
		map<string, vector<Svr_t*> > mSvrByName;
		map<string, vector<Svr_t*> > mSvrByGXid;

		time_t mMtime;	//svr.xml 修改时间
		TiXmlDocument* mDoc;
};

#endif
