
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterConfig.h"

RouterConfig::RouterConfig()
{
	mSvrQos = SvrQos::Instance();
	mDoc = new TiXmlDocument();
	mMemPool = new wMemPool();
	mMemPool->Create(MEM_POOL_MAX);
}

RouterConfig::~RouterConfig()
{
	SAFE_DELETE(mDoc);
	SAFE_DELETE(mMemPool);
}

void RouterConfig::GetBaseConf()
{
	if (!mDoc->LoadFile(mBaseConfFile.c_str()))
	{
		LOG_ERROR(ELOG_KEY, "[config] Load config file(conf.xml) failed");
		exit(2);
	}

	TiXmlElement *pRoot = mDoc->FirstChildElement();
	if (NULL == pRoot)
	{
		LOG_ERROR(ELOG_KEY, "[config] Read root from config file(conf.xml) failed");
		exit(2);
	}
	
	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;

	//读取日志配置
	pElement = pRoot->FirstChildElement("LOG");
	if(NULL != pElement)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL; pChildElm = pChildElm->NextSiblingElement())
		{
			const char *szKey = pChildElm->Attribute("KEY");
			const char *szFile = pChildElm->Attribute("FILE");
			const char *szLevel = pChildElm->Attribute("LEVEL");
			if (szKey != NULL && szFile != NULL && szLevel != NULL)
			{
				const char *szMaxFileSize = pChildElm->Attribute("MAX_FILE_SIZE");
				long long nMaxFileSize = szMaxFileSize == NULL ? atoi(szMaxFileSize) : 10*1024*1024;
				INIT_ROLLINGFILE_LOG(szKey, szFile, (LogLevel)atoi(szLevel), nMaxFileSize, 20);
			}
			else
			{
				LOG_ERROR(ELOG_KEY, "[config] Get log config from conf.xml error");
			}
		}
	}
	else
	{
		LOG_ERROR(ELOG_KEY, "[config] Get log config from conf.xml failed");
		exit(2);
	}

	pElement = pRoot->FirstChildElement("SERVER");
	if(NULL != pElement)
	{
		const char *szIPAddr = pElement->Attribute("IPADDRESS");
		const char *szPort = pElement->Attribute("PORT");
		const char *szBacklog = pElement->Attribute("BACKLOG");
		const char *szWorkers = pElement->Attribute("WORKERS");
		if (szIPAddr != NULL && szPort != NULL)
		{
			mPort = atoi(szPort);
			memcpy(mIPAddr, szIPAddr, MAX_IP_LEN);
		}
		else
		{
			LOG_ERROR(ELOG_KEY, "[config] Get SERVER ip or port from conf.xml failed");
		}
		mBacklog = szBacklog != NULL ? atoi(szBacklog): mBacklog;
		mWorkers = szWorkers != NULL ? atoi(szWorkers): mWorkers;
	}
	else
	{
		LOG_ERROR(ELOG_KEY, "[config] Get SERVER node from conf.xml failed");
		exit(2);
	}
}

void RouterConfig::GetSvrConf()
{
	if (!mDoc->LoadFile(mSvrConfFile.c_str()))
	{
		LOG_ERROR(ELOG_KEY, "[svr] Load config file(svr.xml) failed");
		exit(2);
	}

	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	
	pElement = pRoot->FirstChildElement("SVRS");
	if(pElement != NULL)
	{
		int i = 0;
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			i++;
			const char *szGid = pChildElm->Attribute("GID");
			const char *szXid = pChildElm->Attribute("XID");
			const char *szHost = pChildElm->Attribute("HOST");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szWeight = pChildElm->Attribute("WEIGHT");
			const char *szVersion = pChildElm->Attribute("VERSION");
			if (szGid != NULL && szXid != NULL && szHost != NULL && szPort != NULL)
			{
				struct SvrNet_t stSvr;

				stSvr.mGid = atoi(szGid);
				stSvr.mXid = atoi(szXid);
				stSvr.mPort = atoi(szPort);
				memcpy(stSvr.mHost, szHost, MAX_SVR_HOST);
				stSvr.mWeight = szWeight != NULL ? atoi(szWeight): stSvr.mWeight;
				stSvr.mVersion = szVersion != NULL ? atoi(szVersion): stSvr.mVersion;
				
				if (stSvr.mWeight == 0)
				{
					continue;
				}
				mSvrQos->SaveNode(stSvr);
			}
			else
			{
				LOG_ERROR(ELOG_KEY, "[svr] Parse svr config from svr.xml occur error: line(%d)!", i);
			}
		}
	}
	else
	{
		LOG_ERROR(ELOG_KEY, "[svr] Get SVRS node from svr.xml failed");
		exit(2);
	}

	SetModTime();
}

//不能删除节点（可修改WEIGHT=0属性，达到删除节点效果）
int RouterConfig::GetModSvr(SvrNet_t* pBuffer)
{
	if (!mDoc->LoadFile(mSvrConfFile.c_str()))
	{
		LOG_ERROR(ELOG_KEY, "[modify-svr] Load config file(svr.xml) failed");
		return -1;
	}

	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	
	pElement = pRoot->FirstChildElement("SVRS");	
	
	int i = 0 , j = 0;
	if(pElement != NULL)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			i++;
			const char *szGid = pChildElm->Attribute("GID");
			const char *szXid = pChildElm->Attribute("XID");
			const char *szHost = pChildElm->Attribute("HOST");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szWeight = pChildElm->Attribute("WEIGHT");
			const char *szVersion = pChildElm->Attribute("VERSION");
			if (szGid != NULL && szXid != NULL && szHost != NULL && szPort != NULL)
			{
				struct SvrNet_t stSvr;
				stSvr.mGid = atoi(szGid);
				stSvr.mXid = atoi(szXid);
				stSvr.mPort = atoi(szPort);
				memcpy(stSvr.mHost, szHost, MAX_SVR_HOST);
				stSvr.mWeight = szWeight != NULL ? atoi(szWeight): stSvr.mWeight;
				stSvr.mVersion = szVersion != NULL ? atoi(szVersion): stSvr.mVersion;
				
				/** 新配置始终下发，旧配置检测到version变化才下发*/
				if(mSvrQos->IsExistNode(stSvr))
				{
					if(mSvrQos->IsVerChange(stSvr))	//版本变化，确定下发配置
					{
						mSvrQos->SaveNode(stSvr);
						pBuffer[j++] = stSvr;
					}
				}
				else
				{
					mSvrQos->SaveNode(stSvr);	//添加新配置
					pBuffer[j++] = stSvr;
				}
			}
			else
			{
				LOG_ERROR(ELOG_KEY, "[modify-svr] Parse svr config from svr.xml occur error: line(%d)!", i);
			}
		}

		SetModTime();
	}
	else
	{
		LOG_ERROR(ELOG_KEY, "[modify-svr] Get SVRS node from svr.xml failed");
		return -1;
	}
	return j;
}

int RouterConfig::SetModTime()
{
	struct stat stBuf;
	int iRet = stat(mSvrConfFile.c_str(), &stBuf);
	if (iRet == 0)
	{
		mMtime = stBuf.st_mtime;
		return mMtime;
	}
	return iRet; //-1
}

bool RouterConfig::IsModTime()
{
	struct stat stBuf;
	int iRet = stat(mSvrConfFile.c_str(), &stBuf);
	if (iRet == 0 && stBuf.st_mtime > mMtime)
	{
		return true;
	}
	return false;
}

void RouterConfig::GetQosConf()
{
	if (!mDoc->LoadFile(mQosConfFile.c_str()))
	{
		LOG_ERROR(ELOG_KEY, "[qos] Load config file(qos.xml) failed");
		exit(2);
	}
	
	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	TiXmlElement *pRoot = mDoc->FirstChildElement();

	/** 成功率、时延比例配置 */
	pElement = pRoot->FirstChildElement("FACTOR");
	const char *szRate = NULL, *szDelay = NULL;
	if(NULL != pElement)
	{
		szRate = pElement->Attribute("RATE_WEIGHT");
		szDelay = pElement->Attribute("DELAY_WEIGHT");
	}
	mSvrQos->mRateWeight = szRate != NULL ? (atoi(szRate)>0 ? atoi(szRate):7) : 7;
	mSvrQos->mDelayWeight = szDelay != NULL ? (atoi(szDelay)>0 ? atoi(szDelay):1) : 1;

	/** rate 需在 0.01-100 之间*/
    float rate = (float) mSvrQos->mRateWeight / (float) mSvrQos->mDelayWeight;
    if(rate > 100000)
    {
        mSvrQos->mRateWeight = 100000;
        mSvrQos->mDelayWeight = 1;
    }
    if(rate < 0.00001)
    {
        mSvrQos->mDelayWeight = 100000;
        mSvrQos->mRateWeight = 1;
    }
	
	/** 路由重建时间 */
	pElement = pRoot->FirstChildElement("CFG");
	const char *szType = NULL, *szRebuild = NULL;
	if(NULL != pElement)
	{
		szType = pElement->Attribute("TYPE");
		szRebuild = pElement->Attribute("REBUILD");
	}
	mSvrQos->mRebuildTm = szRebuild != NULL ? (atoi(szRebuild)>0 ? atoi(szRebuild):60) : 60;

	/** 访问量配置 */
	pElement = pRoot->FirstChildElement("REQ");
	const char *szReqMax = NULL, *szReqMin = NULL, *szReqErrMin = NULL, *szReqExtendRate = NULL, *szPreTime = NULL;
	if(NULL != pElement)
	{
		szReqMax = pElement->Attribute("REQ_MAX");
		szReqMin = pElement->Attribute("REQ_MIN");
		szReqErrMin = pElement->Attribute("REQ_ERR_MIN");
		szReqExtendRate = pElement->Attribute("REQ_EXTEND_RATE");
		szPreTime = pElement->Attribute("PRE_TIME");
	}
	mSvrQos->mReqCfg.mReqMax = szReqMax != NULL ? (atoi(szReqMax)>0 ? atoi(szReqMax):10000) : 10000;
	mSvrQos->mReqCfg.mReqMin = szReqMin != NULL ? (atoi(szReqMin)>0 ? atoi(szReqMin):10) : 10;
	mSvrQos->mReqCfg.mReqErrMin = szReqErrMin != NULL ? (atof(szReqErrMin)>0 ? atof(szReqErrMin):0.5) : 0.5;
	mSvrQos->mReqCfg.mReqExtendRate = szReqExtendRate != NULL ? (atof(szReqExtendRate)>0 ? atof(szReqExtendRate):0.2) : 0.2;
	mSvrQos->mReqCfg.mPreTime = szPreTime != NULL ? (atoi(szPreTime)>0 ? atoi(szPreTime):4) : 4;

	mSvrQos->mReqCfg.mRebuildTm = mSvrQos->mRebuildTm;

	/** 并发量配置 */
	pElement = pRoot->FirstChildElement("LIST");
	const char *szListMax = NULL, *szListMin = NULL, *szListErrMin = NULL, *szListExtendRate = NULL, *szTimeout = NULL;
	if(NULL != pElement)
	{
		szListMax = pElement->Attribute("LIST_MAX");
		szListMin = pElement->Attribute("LIST_MIN");
		szListErrMin = pElement->Attribute("LIST_ERR_MIN");
		szListExtendRate = pElement->Attribute("LIST_EXTEND_RATE");
		szTimeout = pElement->Attribute("LIST_TIMEOUT");
	}
	mSvrQos->mListCfg.mListMax = szListMax != NULL ? (atoi(szListMax)>0 ? atoi(szListMax):400) : 400;
	mSvrQos->mListCfg.mListMin = szListMin != NULL ? (atoi(szListMin)>0 ? atoi(szListMin):10) : 10;
	mSvrQos->mListCfg.mListErrMin = szListErrMin != NULL ? (atof(szListErrMin)>0 ? atof(szListErrMin):0.001) : 0.001;
	mSvrQos->mListCfg.mListExtendRate = szListExtendRate != NULL ? (atof(szListExtendRate)>0 ? atof(szListExtendRate):0.2) : 0.2;

	/** 宕机配置 */
	pElement = pRoot->FirstChildElement("DOWN");
	const char *szBegin = NULL, *szInterval = NULL, *szExpire = NULL, *szTimes = NULL, *szReqCount = NULL, *szDownTime = NULL, *szDownErrReq = NULL, *szDownErrRate = NULL;
	if(NULL != pElement)
	{
		szBegin = pElement->Attribute("BEGIN");
		szInterval = pElement->Attribute("INTERVAL");
		szExpire = pElement->Attribute("EXPIRE");
		szTimes = pElement->Attribute("TIMES");
		szReqCount = pElement->Attribute("REQ_COUNT");
		szDownTime = pElement->Attribute("DOWN_TIME");
		szDownErrReq = pElement->Attribute("DOWN_ERR_REQ");
		szDownErrRate = pElement->Attribute("DOWN_ERR_RATE");
	}
	mSvrQos->mDownCfg.mProbeBegin = szBegin != NULL ? (atoi(szBegin)>0 ? atoi(szBegin):3) : 3;
	mSvrQos->mDownCfg.mProbeInterval = szInterval != NULL ? (atoi(szInterval)>0 ? atoi(szInterval):10) : 10;
	mSvrQos->mDownCfg.mProbeNodeExpireTime = szExpire != NULL ? (atoi(szExpire)>0 ? atoi(szExpire):600) : 600;
	mSvrQos->mDownCfg.mProbeTimes = szTimes != NULL ? (atoi(szTimes)>0 ? atoi(szTimes):3) : 3;
	mSvrQos->mDownCfg.mReqCountTrigerProbe = szReqCount != NULL ? (atoi(szReqCount)>0 ? atoi(szReqCount):100000) : 100000;
	mSvrQos->mDownCfg.mDownTimeTrigerProbe = szDownTime != NULL ? (atoi(szDownTime)>0 ? atoi(szDownTime):600) : 600;
	mSvrQos->mDownCfg.mPossibleDownErrReq = szDownErrReq != NULL ? (atoi(szDownErrReq)>0 ? atoi(szDownErrReq):10) : 10;
	mSvrQos->mDownCfg.mPossbileDownErrRate = szDownErrRate != NULL ? (atof(szDownErrRate)>0 ? atof(szDownErrRate):0.5) : 0.5;

	if(!(mSvrQos->mReqCfg.mReqExtendRate > 0.001 && mSvrQos->mReqCfg.mReqExtendRate < 101))
	{
		LOG_ERROR(ELOG_KEY, "[qos] Init invalid req_extend_rate[%f]  !((ext > 0.001) && (ext < 101))", mSvrQos->mReqCfg.mReqExtendRate);
		exit(2);
	}

	if (mSvrQos->mReqCfg.mReqErrMin >= 1)
	{
		LOG_ERROR(ELOG_KEY, "[qos] Init invalid _req_err_min[%f]  _qos_req_cfg._req_err_min > 1", mSvrQos->mReqCfg.mReqErrMin);
		exit(2);
	}

	if (mSvrQos->mDownCfg.mPossbileDownErrRate > 1 || mSvrQos->mDownCfg.mPossbileDownErrRate < 0.01)
	{
		LOG_ERROR(ELOG_KEY, "[qos] Init invalid err_rate_to_def_possible_down[%f] > 1 or < _req_min[%f]", mSvrQos->mReqCfg.mReqErrMin, mSvrQos->mDownCfg.mPossbileDownErrRate);
		exit(2);
	}

	if (mSvrQos->mDownCfg.mProbeTimes < 3)
	{
		LOG_ERROR(ELOG_KEY, "[qos] Init invalid continuous_err_req_count_to_def_possible_down[%d] <3", mSvrQos->mDownCfg.mProbeTimes);
		exit(2);
	}

	if (mSvrQos->mReqCfg.mRebuildTm < 3)
	{
		LOG_ERROR(ELOG_KEY, "[qos] Init invalid rebuildtm[%d] < 3", mSvrQos->mReqCfg.mRebuildTm);
		exit(2);
	}
	
	if (mSvrQos->mReqCfg.mPreTime <= 0 || mSvrQos->mReqCfg.mPreTime > (mSvrQos->mRebuildTm / 2))
	{
		mSvrQos->mReqCfg.mPreTime = 2;
	}
}
