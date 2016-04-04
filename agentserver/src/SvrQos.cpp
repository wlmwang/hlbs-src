
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "SvrQos.h"

SvrQos::SvrQos()
{
	Initialize();
}

SvrQos::~SvrQos() 
{
	CleanNode();
}

void SvrQos::Initialize() 
{
	mPreRoute = 1;	//开启预取缓存功能
	mDetectThread = DetectThread::Instance();
}

/** 查找某一节点 */
map<struct SvrNet_t, struct SvrStat_t*>::iterator SvrQos::SearchNode(struct SvrNet_t& stSvr)
{
	map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
	return mapReqIt;
}

/** 添加节点&路由 | 修改节点&路由权重 */
int SvrQos::SaveNode(struct SvrNet_t& stSvr)
{
	map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
	if (mapReqIt == mMapReqSvr.end())
	{
		LOG_DEBUG(ELOG_KEY, "[svr] SaveNode new Svr gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
		if(AllocNode(stSvr) < 0)
		{
			LOG_ERROR(ELOG_KEY, "[svr] SaveNode new Svr failed");
			return -1;
		}
	}
	else
	{
		//修改weight
		struct SvrNet_t &stKey = const_cast<struct SvrNet_t&> (mapReqIt->first);
		if (stSvr.mWeight != stKey.mWeight)
		{
            stKey.mWeight = stSvr.mWeight;
            ModRouteNode(stSvr);
		}
		LOG_DEBUG(ELOG_KEY, "[svr] SaveNode modify Svr weight gid(%d),xid(%d),host(%s),port(%d),weight(%d),old_key(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight, stKey.mWeight);
	}
	return 0;
}

/** 修改 节点&路由 权重（权重为0删除节点） */
int SvrQos::ModNode(struct SvrNet_t& stSvr)
{
	if (stSvr.mWeight < 0)
	{
		LOG_ERROR(ELOG_KEY, "[svr] ModNode invalid gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
		return -1;
	}

	int iRet = 0;
	if (stSvr.mWeight == 0)		//权重为0删除节点
	{
		LOG_DEBUG(ELOG_KEY, "[svr] DelNode start gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
		iRet = DelNode(stSvr);
	}
	else
	{
		LOG_DEBUG(ELOG_KEY, "[svr] ModNode start gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
		if(stSvr.mWeight > MAX_WEIGHT)
		{
			LOG_ERROR(ELOG_KEY, "[svr] ModNode but weight is setting default(invalid) gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
			stSvr.mWeight = MAX_WEIGHT;
		}
		iRet = SaveNode(stSvr);
	}
	return iRet;
}

/** 清除所有节点 */
int SvrQos::CleanNode()
{
    map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.begin();
    struct SvrNet_t stSvr;
    struct SvrStat_t* pSvrStat = NULL;
    if (mMapReqSvr.size() > 0)
    {
    	for(mapReqIt; mapReqIt != mMapReqSvr.end(); mapReqIt++)
    	{
    		stSvr = mapReqIt->first;
    		pSvrStat = mapReqIt->second;

    		DelRouteNode(stSvr);
    		SAFE_DELETE(pSvrStat);
    	}
    	mMapReqSvr.clear();
    }
    return 0;
}

/** 删除 节点&路由 */
int SvrQos::DelNode(struct SvrNet_t& stSvr)
{
    map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
    if (mapReqIt == mMapReqSvr.end())
	{
		LOG_ERROR(ELOG_KEY, "[svr] del node failed(cannot find svr) gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
        return -1;
    }
    SvrStat_t* pSvrStat = mapReqIt->second;
    mMapReqSvr.erase(mapReqIt);
    
    DelRouteNode(stSvr);
    SAFE_DELETE(pSvrStat);
    return 0;
}

/** 调用数上报 */
int SvrQos::NotifyNode(struct SvrNet_t& stSvr)
{
	LOG_DEBUG(ELOG_KEY, "[svr] notify node start gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
    
    map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
    if (mapReqIt == mMapReqSvr.end())
	{
		//log
        return -1;
    }

	SvrStat_t* pSvrStat = mapReqIt->second;
	pSvrStat->mInfo.mReqAll++;
	pSvrStat->mInfo.mSReqAll++;
	
	//log
	
	return 0;
}

/** 调用结果上报 */
int SvrQos::CallerNode(struct SvrCaller_t& stCaller)
{
	LOG_DEBUG(ELOG_KEY, "[svr] CallerNode start gid(%d),xid(%d),host(%s),port(%d), ret(%d),usec(%d)",
		stCaller.mCalledGid,stCaller.mCalledXid,stCaller.mHost,stCaller.mPort,stCaller.mReqRet,stCaller.mReqUsetimeUsec);

	if (stCaller.mCalledGid <= 0 || stCaller.mCalledXid <= 0 || stCaller.mPort <= 0 || stCaller.mHost[0] == 0)
	{
		LOG_ERROR(ELOG_KEY, "[svr] CallerNode failed gid(%d),xid(%d),host(%s),port(%d), ret(%d),usec(%d)",
			stCaller.mCalledGid,stCaller.mCalledXid,stCaller.mHost,stCaller.mPort,stCaller.mReqRet,stCaller.mReqUsetimeUsec);

		return -1;
	}
	
	struct SvrNet_t stSvr;
	stSvr.mGid = stCaller.mCalledGid;
	stSvr.mXid = stCaller.mCalledXid;
	stSvr.mPort = stCaller.mPort;
	memcpy(stSvr.mHost, stCaller.mHost, strlen(stCaller.mHost)+1);

	if (stCaller.mReqUsetimeUsec <= 0)
	{
		stCaller.mReqUsetimeUsec = 1;
	}

    map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
    if (mapReqIt == mMapReqSvr.end())
	{
		LOG_ERROR(ELOG_KEY, "[svr] CallerNode failed(cannot find req-svr) gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
        return -1;
    }
   
    struct SvrStat_t* pSvrStat = mapReqIt->second;
    if (stCaller.mReqRet >= 0)
    {
    	//成功
    	pSvrStat->mInfo.mReqSuc += stCaller.mReqCount;
    	pSvrStat->mInfo.mSReqSuc += stCaller.mReqCount;
    	pSvrStat->mInfo.mTotalUsec += stCaller.mReqUsetimeUsec;
    	pSvrStat->mInfo.mContErrCount = 0;
    }
    else
    {
    	//失败
    	pSvrStat->mInfo.mReqErrRet += stCaller.mReqCount;
    	pSvrStat->mInfo.mSReqErrRet += stCaller.mReqCount;
    	pSvrStat->mInfo.mContErrCount += stCaller.mReqCount;
    }

    //重建route
    struct SvrKind_t stKind(stSvr);
    RebuildRoute(stKind);
    return 0;
}

/** 添加 节点&路由 */
int SvrQos::AllocNode(struct SvrNet_t& stSvr)
{
	struct SvrStat_t* pSvrStat = new SvrStat_t();
	if (pSvrStat == NULL)
	{
		LOG_ERROR(ELOG_KEY, "[svr] AllocNode new SvrStat failed! gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
		SAFE_DELETE(pSvrStat);
		return -1;
	}
	GetTimeofday(&pSvrStat->mInfo.mBuildTm, NULL);	//重建时间
	if(LoadStatCfg(stSvr, pSvrStat) < 0)
	{
		LOG_ERROR(ELOG_KEY, "[svr] AllocNode LoadStatCfg failed! gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
		SAFE_DELETE(pSvrStat);
		return -1;
	}

	mMapReqSvr[stSvr] = pSvrStat;
	pSvrStat->mReqCfg.mRebuildTm = mRebuildTm;	//设置节点重建时间间隔
	
	AddRouteNode(stSvr, pSvrStat);		//将新Req节点加入到节点路由map
	return 0;
}

/** 单次获取路由 */
int SvrQos::QueryNode(struct SvrNet_t& stSvr)
{	
	int iAck = GetRouteNode(stSvr);
	
	LOG_DEBUG(ELOG_KEY, "[svr] QueryNode(%d) Svr:gid(%d),xid(%d),host(%s),port(%d),weight(%d)",iAck,stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
	
	/** for test */
	LogAllNode();
	return iAck;
}

/** 加载阈值配置 */
int SvrQos::LoadStatCfg(struct SvrNet_t& stSvr, struct SvrStat_t* pSvrStat)
{
	pSvrStat->mInfo.InitInfo(stSvr);

	{
		pSvrStat->mReqCfg = mReqCfg;
	}
	{
		pSvrStat->mListCfg = mListCfg;
	}
	return 0;
}

/** 添加新路由 */
int SvrQos::AddRouteNode(struct SvrNet_t& stSvr, struct SvrStat_t* pSvrStat)
{
    struct SvrKind_t stKind(stSvr);
    stKind.mRebuildTm = pSvrStat->mReqCfg.mRebuildTm;	//重建时间

    multimap<float, struct SvrNode_t>* &pTable = mRouteTable[stKind];
    if(pTable == NULL)
	{
        pTable = new multimap<float, SvrNode_t>;
    }
    if(pTable == NULL)
    {
		LOG_ERROR(ELOG_KEY, "[svr] AddRouteNode failed(new multimap<float, SvrNode_t>)! gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
    	return -1;
    }

    //路由表中已有相关节点，取优先级最低的那个作为新节点的默认值
    multimap<float, struct SvrNode_t>::reverse_iterator it = pTable->rbegin();
    for(; it != pTable->rend(); ++it)
	{
        struct SvrNode_t& stNode = it->second;
        pSvrStat->mReqCfg.mReqLimit = stNode.mStat->mReqCfg.mReqLimit;
        pSvrStat->mReqCfg.mReqMin = stNode.mStat->mReqCfg.mReqMin;
        pSvrStat->mReqCfg.mReqMax = stNode.mStat->mReqCfg.mReqMax;
        pSvrStat->mReqCfg.mReqErrMin = stNode.mStat->mReqCfg.mReqErrMin;
        pSvrStat->mReqCfg.mReqExtendRate = stNode.mStat->mReqCfg.mReqExtendRate;

        //正在运行时，突然新加一个服务器，如果mPreAll=0，GetRoute函数分配服务器时，会连续分配该新加的服务器
        //导致：被调扩容时，新加的服务器流量会瞬间爆增，然后在一个周期内恢复正常
        pSvrStat->mInfo = stNode.mStat->mInfo;
        break;
    }
	
	LOG_DEBUG(ELOG_KEY, "[svr] AddRouteNode success! gid(%d),xid(%d),host(%s),port(%d),weight(%d). req: limit(%d),min(%d),max(%d),errmin(%f),extend(%f)",
		stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight,pSvrStat->mReqCfg.mReqLimit,pSvrStat->mReqCfg.mReqMin,pSvrStat->mReqCfg.mReqMax,
		pSvrStat->mReqCfg.mReqErrMin,pSvrStat->mReqCfg.mReqExtendRate);

    struct SvrNode_t stRouteInfo(stSvr, pSvrStat);
    pTable->insert(make_pair(stRouteInfo.mKey, stRouteInfo));
    return 0;
}

/** 单次获取路由 */
int SvrQos::GetRouteNode(struct SvrNet_t& stSvr)
{
	if (stSvr.mGid <= 0 || stSvr.mXid <= 0)
	{
		LOG_ERROR(ELOG_KEY, "[svr] GetRouteNode failed! gid or xid invalid");
		return -1;
	}

	struct SvrKind_t stNode(stSvr);

	RebuildRoute(stNode);	//重建路由
	map<struct SvrKind_t,  multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.find(stNode);

	//反向注册路由
	if(rtIt == mRouteTable.end())
	{
		LOG_ERROR(ELOG_KEY, "[svr] GetRouteNode invalid gid=%d xid=%d",stSvr.mGid,stSvr.mXid);
		if (stSvr.mGid > 0 && stSvr.mXid > 0)
		{
			//反向注册路由 TODO（讲道理，这里一定会被执行）
			
			return -1;
		}
		else
		{
			LOG_ERROR(ELOG_KEY, "[svr] GetRouteNode failed! gid or xid invalid");
			return -1;
		}
    }
    
    multimap<float, struct SvrNode_t>* pTable = rtIt->second;
	if(pTable == NULL || pTable->empty())
	{
		LOG_ERROR(ELOG_KEY, "[svr] GetRouteNode failed. gix(%d) xid(%d) svr is empty", stSvr.mGid,stSvr.mXid);
		return -1;
	}

    struct SvrKind_t &stKind = (struct SvrKind_t &) (rtIt->first);
    stKind.mAccess64tm = GetTimeofday();

	int iIndex = stKind.mPindex;	//已分配到第几个路由
	if (iIndex < 0 || iIndex >= (int) pTable->size())
	{
		iIndex = 0;
		stKind.mPindex = 0;
	}
	
	multimap<float, struct SvrNode_t>::iterator it = pTable->begin() , it1 = pTable->begin();
	
	int iReq = it1->second.mStat->mInfo.mPreAll;	//负载因子（与分配数密切相关？？）
	double dFirstReq = it1->second.mKey * iReq;		//此次WRR轮转的负载分配限度（首节点的 mKey = mOkLoad*mDelayLoad 是最小值）
	bool bFirstRt = false;
	
	//it += iIndex;
	advance(it, iIndex);
	
    //如果不是第一个路由，获得比第一个路由的负载更低的路由 round_robin
	if (iIndex != 0)
	{
		int iReq = it->second.mStat->mInfo.mPreAll;
		double dCurAdjReq = it->second.mKey * iReq;

		//此处采用round_robin的经典算法，获得目标路由
		while (dCurAdjReq >= dFirstReq)
		{
			iIndex++;
            if (iIndex >= (int) pTable->size())
			{
				it = it1;
                iIndex = 0;
                stKind.mPindex = 0;
                break;
            }
            it++;
			iReq = it->second.mStat->mInfo.mPreAll;
			dCurAdjReq = it->second.mKey * iReq;
			stKind.mPindex = iIndex;
		}
	}

	//未找到合适节点 或 第一个节点即为合适节点
	if (iIndex == 0)
	{
		bFirstRt = true;
	}
	
	//已经获得了预选路由即host,port
	memcpy(stSvr.mHost, it->second.mNet.mHost, sizeof(stSvr.mHost));
	stSvr.mPort = it->second.mNet.mPort;
    stSvr.mWeight = it->second.mNet.mWeight; //fixed

    int iRet;
    //检测分配路由，如果有路由分配产生，则更新相关统计计数
	while ((iRet = RouteCheck(it->second.mStat, stSvr, dFirstReq, bFirstRt)) != 0)
	{
		iIndex++;
		bFirstRt = false;
		if (iIndex >= (int)pTable->size())
		{
			iIndex = 0;
			bFirstRt = true;
		}

		if (iIndex == stKind.mPindex)	//一个轮回
		{
			it = it1;
			//it += iIndex;
			advance(it, iIndex);

            if (iRet == -9995)		//节点无错误（非本地节点而已~）
			{
				memcpy(stSvr.mHost, it->second.mNet.mHost, sizeof(stSvr.mHost));
				stSvr.mPort = it->second.mNet.mPort;

                LOG_ERROR(ELOG_KEY, "[svr] GetRouteNode off_side_node gid=%d xid=%d host=%s port=%hu index=%d",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,iIndex);
                break;
            }
			else	//整体过载
			{
                it->second.mStat->mInfo.mReqRej++;
                it->second.mStat->mInfo.mSReqRej++;

                LOG_ERROR(ELOG_KEY, "[svr] GetRouteNode overload gid=%d xid=%d host=%s port=%hu rej=%d",
                	stSvr.mGid,stSvr.mXid,it->second.mNet.mHost,it->second.mNet.mPort,it->second.mStat->mInfo.mSReqRej);
                return -1;
            }
		}

		it = it1;
		//it += iIndex;
		advance(it, iIndex);
		memcpy(stSvr.mHost, it->second.mNet.mHost, sizeof(stSvr.mHost));
		stSvr.mPort = it->second.mNet.mPort;
	    stSvr.mWeight = it->second.mNet.mWeight; //fixed
	}

	time_t nowTm = time(NULL);
	stSvr.mExpired = nowTm + stKind.mRebuildTm - (nowTm - stKind.mPtm);	//过期时间
	
	//如果是第一个路由, 获取负载更低的路由（探测下一个路由）
	if (it1 == it)
	{
		it++;
		if (it != pTable->end() && (it->second.mStat->mInfo.mPreAll * it->second.mKey < dFirstReq))
		{
			if ((iIndex + 1) < (int)pTable->size())
			{
				iIndex++;
			}
			else
			{
				iIndex = 0;
			}
			stKind.mPindex = iIndex;
		}
		return 0;
	}

    //如果不是第一个路由,  若负载比第一个路由大，滚动到下一个路由
    dFirstReq = it1->second.mKey * it1->second.mStat->mInfo.mPreAll; 
	int iPriCurReqSuc = it->second.mStat->mInfo.mPreAll;
	if(iPriCurReqSuc * it->second.mKey >= dFirstReq)
	{
		if((iIndex + 1) < (int)pTable->size())
		{
			stKind.mPindex = iIndex + 1;
		}
		else
		{
			stKind.mPindex = 0;
		}
	}
	return 0;
}

/** 单次分配路由检查，如果有路由分配产生，则更新相关统计计数 */
int SvrQos::RouteCheck(struct SvrStat_t* pSvrStat, struct SvrNet_t& stNode, double dFirstReq/*此次WRR轮转的分配限度*/, bool bFirstRt)
{
    pSvrStat->mInfo.mReqAll++;
    pSvrStat->mInfo.mSReqAll++;

    stNode.mPre = 1;	//路由预取数
    
    //新增获取路由对应的时延信息
    /*
    if (pSvrStat->mInfo.mReqSuc > 0)
    {
    	stNode.mDelay = pSvrStat->mInfo.mTotalUsec / pSvrStat->mInfo.mReqSuc;
    }
    else
    {
    	stNode.mDelay = pSvrStat->mInfo.mAvgTm;
    }
    */
   
    if (pSvrStat->mReqCfg.mReqLimit <= 0)	//请求数门限收缩到0值(讲道理，不可能执行这里：mReqLimit至少等于mReqMin)
	{
	    pSvrStat->mInfo.mPreAll++; 
	    pSvrStat->mInfo.mSPreAll++;

	    LOG_DEBUG(ELOG_KEY, "[svr] RouteCheck gid(%d), xid(%d) mReqLimit<=0,preall++", stNode.mGid,stNode.mXid);
        return 0;
    }

    int iReqCount = pSvrStat->mInfo.mReqAll;
    int iErrCount = pSvrStat->mInfo.mReqErrRet + pSvrStat->mInfo.mReqErrTm;
    float fErrRate = (float) iErrCount / (float)iReqCount;

    //请求超过门限
    if (iReqCount >= pSvrStat->mReqCfg.mReqLimit)
	{
		//当访问量超过阀值，且当前周期的错误率超过配置的错误率阀值时才判断为过载
		if (fErrRate > pSvrStat->mReqCfg.mReqErrMin)
		{
			//pSvrStat->mInfo.mReqRej++; //当真正发生过载的时候才对选定的路由的拒绝数加1
			LOG_ERROR(ELOG_KEY, "[svr] RouteCheck return overload gid=%d xid=%d host:%s port:%hu", stNode.mGid,stNode.mXid,stNode.mHost,stNode.mPort);

		    pSvrStat->mInfo.mReqAll--; 
		    pSvrStat->mInfo.mSReqAll--;
		    
		    pSvrStat->mInfo.mPreAll++; //bug fix 
            return -1;	//过载
        }
		pSvrStat->mInfo.mPreAll++;
        pSvrStat->mInfo.mSPreAll++;
        return 0;
    }

    //路由被分配次数。mPreRoute：路由未加载在内存中
    if (mPreRoute == 0 || true)	
    {
		pSvrStat->mInfo.mPreAll++;
        pSvrStat->mInfo.mSPreAll++;
        return 0;
    }
    
    if (!bFirstRt)
	{
        double dCurAllReq = pSvrStat->mInfo.mPreAll * pSvrStat->mInfo.mLoadX;
        if(dCurAllReq >= dFirstReq)		//当前负载高于第一个负载，被分配数翻倍
		{
			pSvrStat->mInfo.mPreAll++;
	        pSvrStat->mInfo.mSPreAll++;
            return 0;
        }
	}
	
	//若节点的有效错误率高于预设的最低错误率，被分配数翻倍
	if (fErrRate > (pSvrStat->mReqCfg.mReqErrMin + pSvrStat->mInfo.mAvgErrRate))
	{
		pSvrStat->mInfo.mPreAll++;
        pSvrStat->mInfo.mSPreAll++;
        return 0;
	}

	time_t nowTm = time(NULL);
    float fErrate = 1 - pSvrStat->mInfo.mOkRate - pSvrStat->mInfo.mAvgErrRate;	//节点总体错误率
    
    //初始值为预测的此进程本周期的成功请求数
    //上个周期的成功请求数加上 上个周期相对于 上上个周期成功请求数的差值，在一个预取时间段（qos.xml中pre_time的值，默认为4秒）内的部分
    int iLastSuc = pSvrStat->mInfo.mLastReqSuc + 1;
    int iLastInc = pSvrStat->mInfo.mAddSuc;
    int iSuc = mReqCfg.mPreTime * ((iLastSuc + iLastInc) / pSvrStat->mReqCfg.mRebuildTm);
    //int iSuc = mReqCfg.mPreTime * 100;

    //预取数收缩
    if (fErrate > 0 && fErrate < 1)
    {
    	iSuc -= (int) (iSuc * fErrate);
    	if(pSvrStat->mInfo.mLoadX > 1)
    	{
    		iSuc = (int)(iSuc / pSvrStat->mInfo.mLoadX);
    	}
    }
    
    //构建时间间隔多余120s
    if (nowTm - pSvrStat->mInfo.mBuildTm.tv_sec > 120)
    {
    	iSuc = 1;
    }
   
    //已分配数+预取数 > mReqLimit
    if((pSvrStat->mInfo.mPreAll + iSuc) > pSvrStat->mReqCfg.mReqLimit)
	{
        int iLimV = pSvrStat->mInfo.mPreAll  + iSuc  - pSvrStat->mReqCfg.mReqLimit;
        iSuc -= iLimV;
    }

    if(iSuc < 1) iSuc = 1;

    //检查是否满足WRR算法的要求: 如果 mPreAll + iSuc 超出了此次轮转的分配限度，则扣除超出部分
    if(!bFirstRt)
	{
        double dPreAllreq = (pSvrStat->mInfo.mPreAll + iSuc) * pSvrStat->mInfo.mLoadX; // fixme
        double dDstReq = dFirstReq - dPreAllreq;
        if(dDstReq < 0)	//扣除超出部分
		{
            dDstReq = -dDstReq;
            iSuc = iSuc - (int) ((float) dDstReq / pSvrStat->mInfo.mLoadX) + 1;
            dPreAllreq = (pSvrStat->mInfo.mPreAll + iSuc) * pSvrStat->mInfo.mLoadX;
            dDstReq = dFirstReq - dPreAllreq;
        }
    }

    if(iSuc < 1) iSuc = 1;

    pSvrStat->mInfo.mPreAll += iSuc;
    pSvrStat->mInfo.mSPreAll += iSuc;
    stNode.mPre = iSuc;

    LOG_DEBUG(ELOG_KEY, "[svr] RouteCheck gid=%d xid=%d host:%s port:%hu at end return 0 pre(%d)", stNode.mGid,stNode.mXid,stNode.mHost,stNode.mPort,stNode.mPre);
    return 0;
}

/*路由节点重建*/
int SvrQos::RouteNodeRebuild(struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat)
{
	/*
    if(pSvrStat->mType == QOS_TYPE_REQ)
	{
        return ReqRebuild(stSvr, pSvrStat);
    }

    if(pSvrStat->mType == QOS_TYPE_LIST)
	{
        return ListRebuild(stSvr, pSvrStat);
    }
    */
	
	ReqRebuild(stSvr, pSvrStat);
    ListRebuild(stSvr, pSvrStat);
    return 0;
}

/** 节点访问量控制重建 */
int SvrQos::ReqRebuild(struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat)
{
	int iErrCount = pSvrStat->mInfo.mReqErrRet + pSvrStat->mInfo.mReqErrTm;
	int iSucCount = pSvrStat->mInfo.mReqSuc;

    if (pSvrStat->mInfo.mReqAll < iErrCount + iSucCount)
    {
        pSvrStat->mInfo.mReqAll = iErrCount + iSucCount;	//防止核算错误
    }
    int iReqAll = pSvrStat->mInfo.mReqAll;
	
	//上个周期与上上个周期成功请求数差值
	if (pSvrStat->mInfo.mIdle > 3)
	{
		pSvrStat->mInfo.mAddSuc = pSvrStat->mInfo.mReqSuc;
	}
	else if (pSvrStat->mInfo.mReqSuc > pSvrStat->mInfo.mLastReqSuc)
	{
		pSvrStat->mInfo.mAddSuc = pSvrStat->mInfo.mReqSuc - pSvrStat->mInfo.mLastReqSuc;
	}
	else
	{
		pSvrStat->mInfo.mAddSuc = 0;
	}
	if (pSvrStat->mInfo.mReqSuc > 0)
	{
		pSvrStat->mInfo.mIdle = 0;
	}
	else
	{
		pSvrStat->mInfo.mIdle++;
	}

    //保存上一周期数据
    pSvrStat->mInfo.mLastReqAll = iReqAll;
    pSvrStat->mInfo.mLastReqRej = pSvrStat->mInfo.mReqRej;
    pSvrStat->mInfo.mLastReqErrRet = pSvrStat->mInfo.mReqErrRet;
    pSvrStat->mInfo.mLastReqErrTm = pSvrStat->mInfo.mReqErrTm;
    pSvrStat->mInfo.mLastReqSuc = pSvrStat->mInfo.mReqSuc;
    if (pSvrStat->mInfo.mLastReqSuc < 0) 
	{
		pSvrStat->mInfo.mLastReqSuc = 0;
	}

	//当前周期无请求，重置成功率为1
    if (iReqAll <= 0 && iErrCount + iSucCount <= 0)
	{
        pSvrStat->mInfo.mOkRate = 1;
        pSvrStat->Reset(); //mAvgTm not change！
        return 0;
    }

    //失败率
    float fErrRate = iReqAll > 0 ? (float)iErrCount/iReqAll : 0;
    if (isnan(fErrRate))
    {
        LOG_ERROR(ELOG_KEY, "[svr] ReqRebuild fErrRate is NaN");
        fErrRate = 0;
    }

    iSucCount = iSucCount > 0 ? iSucCount : 0;
    fErrRate = fErrRate < 1 ? fErrRate : 1;
    pSvrStat->mInfo.mOkRate = 1 - fErrRate;

	//统计节点延时值
    if (pSvrStat->mInfo.mReqSuc > 0)		//有成功的请求
	{
        pSvrStat->mInfo.mAvgTm = pSvrStat->mInfo.mTotalUsec / pSvrStat->mInfo.mReqSuc;
    }
	else	//只有失败的请求
	{
        if (iReqAll > pSvrStat->mReqCfg.mReqMin)
		{
			//失败请求数超过请求最小阈值
            pSvrStat->mInfo.mAvgTm = DELAY_MAX;
        }
		else
		{
            if(pSvrStat->mInfo.mAvgTm && pSvrStat->mInfo.mAvgTm < DELAY_MAX)
			{
                pSvrStat->mInfo.mAvgTm = 2 * pSvrStat->mInfo.mAvgTm; 	//延时增加一倍
            }
			else
			{
                pSvrStat->mInfo.mAvgTm = DELAY_MAX/100; //1000000;
            }
		}
	}

	//调整请求门限值mReqLimit
	//路由的过载判断逻辑修改为：路由的当前周期错误率超过配置的错误率阀值时才判断为过载
	if(pSvrStat->mReqCfg.mReqLimit <= 0)
	{
		if(iReqAll > pSvrStat->mReqCfg.mReqMin)
		{
			pSvrStat->mReqCfg.mReqLimit = iReqAll;	//门限扩张为请求数
		}
		else
		{
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin + 1;	//门限最低值
		}
	}
	pSvrStat->mReqCfg.mReqCount = iReqAll;

	//节点错误率未超门限，重置节点失败率为0
    if(fErrRate <= pSvrStat->mReqCfg.mReqErrMin)
	{
        pSvrStat->mInfo.mAvgErrRate = 0;
    }

    //有效错误率 = 实际错误率 - 平均错误率
    if (fErrRate <= pSvrStat->mReqCfg.mReqErrMin + pSvrStat->mInfo.mAvgErrRate)	//门限扩张
	{
        //如果成功数大于上次过度扩张时的门限，则门限不再有意义
        if (pSvrStat->mInfo.mLastErr && iSucCount > pSvrStat->mInfo.mLastAlarmReq)
        {
            pSvrStat->mInfo.mLastErr = false;
        }

        //目标调整值： iReqAll * (1 + mReqExtendRate)
        int iDest = iReqAll + (int)(iReqAll * mReqCfg.mReqExtendRate);

        LOG_DEBUG(ELOG_KEY, "[svr] ReqRebuild :expand: lasterr=%d succount=%d alarmreq=%d, dest=%d reqlimit=%d errrate=%f avgerrate=%f reqerrmin=%f",
        	pSvrStat->mInfo.mLastErr,iSucCount, pSvrStat->mInfo.mLastAlarmReq,
        	iDest, pSvrStat->mReqCfg.mReqLimit, fErrRate,pSvrStat->mInfo.mAvgErrRate,pSvrStat->mReqCfg.mReqErrMin);

        if(pSvrStat->mReqCfg.mReqLimit < iDest)	//扩张
		{
            pSvrStat->mReqCfg.mReqLimit += GetAddCount(pSvrStat, iReqAll);
            if(pSvrStat->mReqCfg.mReqLimit > iDest)
            {
                pSvrStat->mReqCfg.mReqLimit = iDest;
            }
        }
    }
	else	//门限收缩(可能原因为：上周期存在过度扩张)，需要按有效错误率收缩门限
	{
		if(pSvrStat->mInfo.mOkRate > 0)
		{
			fErrRate -= pSvrStat->mInfo.mAvgErrRate;	//有效错误率
		}

		//直接收缩至成功数
		//因为宕机时直接缩到成功数，可能会使得下个周期的limit比其他机器的大
		//所以门限值按照 req_limit 和req_count中较小的一个进行收缩
		if (iReqAll > pSvrStat->mReqCfg.mReqLimit)
		{
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqLimit - (int)(pSvrStat->mReqCfg.mReqLimit * fErrRate);
		}
		else
		{
			pSvrStat->mReqCfg.mReqLimit = iReqAll - (int)(iReqAll * fErrRate);
		}

        LOG_DEBUG(ELOG_KEY, "[svr] ReqRebuild :reduce: okrate=%f succount=%d reqall=%d reqlimit=%d errrate=%f reqmin=%f",
        	pSvrStat->mInfo.mOkRate,iSucCount,iReqAll,pSvrStat->mReqCfg.mReqLimit, fErrRate,pSvrStat->mReqCfg.mReqMin);

		pSvrStat->mInfo.mLastErr = true;
		pSvrStat->mInfo.mLastAlarmReq = iReqAll;
		pSvrStat->mInfo.mLastAlarmSucReq = iSucCount;

		if (iReqAll <= pSvrStat->mReqCfg.mReqMin)
		{
			pSvrStat->mInfo.mLastErr = false;
		}
	}

    //宕机嫌疑判定：
    //1)mReqLimit 大于最小值+1
	//2)如果最后记录的连续失败次数累加达到设定值，则该机器有宕机嫌疑
	//3)错误率大于一定的比例,判定为有宕机嫌疑
	if (pSvrStat->mReqCfg.mReqLimit > (pSvrStat->mReqCfg.mReqMin + 1) 
		&& ((pSvrStat->mInfo.mContErrCount >= mDownCfg.mPossibleDownErrReq) || (fErrRate > mDownCfg.mPossbileDownErrRate)))
	{
		pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin + 1;	//宕机门限设置最小值
        
        LOG_ERROR(ELOG_KEY, "[svr] gid=%d xid=%d host=%s port=%hu weight=%d continuous errcount[%d]>=cfg[%d] or _err_rate[%f]>cfg_rate[%f], req_limit set to[%d]", 
        	stSvr.mGid, stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight,
        	pSvrStat->mInfo.mContErrCount,mDownCfg.mPossibleDownErrReq,
        	fErrRate,mDownCfg.mPossbileDownErrRate,pSvrStat->mReqCfg.mReqLimit);		
	}

    //硬的限制：不可超出[req_min, req_max]的范围
    if (pSvrStat->mReqCfg.mReqLimit > pSvrStat->mReqCfg.mReqMax)
    {
        pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMax;
    }
    if (pSvrStat->mReqCfg.mReqLimit < pSvrStat->mReqCfg.mReqMin)
    {
        pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin;	//设置为最小值（注意：不是最小值+1），会被加入宕机列表
    }

    pSvrStat->Reset();	//清除当前周期统计数据
    return 0;
}

/** 节点并发量控制重建 */
int SvrQos::ListRebuild(struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat)
{
	int iReqCount = pSvrStat->mInfo.mReqAll - pSvrStat->mInfo.mReqRej;	//请求数
	if (iReqCount <= 0)
	{
        LOG_ERROR(ELOG_KEY, "[svr] ListRebuild mReqAll(%d) <= mReqRej(%d)", pSvrStat->mInfo.mReqAll, pSvrStat->mInfo.mReqRej);
		return 0;
	}

	//总错误个数
	int iErrCount = pSvrStat->mInfo.mReqErrRet + pSvrStat->mInfo.mReqErrTm;
	//错误率
	float fErrRate = (float)iErrCount / (float)iReqCount;
	//拒绝率
	float fRejRate = pSvrStat->mInfo.mReqRej / pSvrStat->mInfo.mReqAll;

	fErrRate = fErrRate>1 ? 1: fErrRate;
	pSvrStat->mInfo.mOkRate = 1 - fErrRate;	//重置成功率

	if (fErrRate <= pSvrStat->mListCfg.mListMin)	//错误率小于阈值
	{
		pSvrStat->mInfo.mAvgErrRate = 0;
	}

	if (iErrCount < iReqCount)	//拒绝 < 成功
	{
		pSvrStat->mInfo.mAvgTm = pSvrStat->mInfo.mTotalUsec / pSvrStat->mInfo.mReqSuc;
	}
	else
	{
		LOG_DEBUG(ELOG_KEY, "[svr] ListRebuild mReqRej(%d) > mReqSuc(%d), mAvgTm is setting DELAY_MAX",iErrCount,iReqCount);
		pSvrStat->mInfo.mAvgTm = DELAY_MAX;	//最大延时
		pSvrStat->mInfo.mOkRate = 0;
	}

	//并发队列
    //pSvrStat->mList.QosRebuild(fErrRate, fRejRate, pSvrStat->mInfo);
    //pSvrStat->mListCfg.mListLimit = pSvrStat->mList.Limit();	//并发数门限
    pSvrStat->Reset();
    return 0;
}

/** 重建路由 */
int SvrQos::RebuildRoute(struct SvrKind_t& stItem, int bForce)
{
	map<struct SvrKind_t, multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.find(stItem);
	if (rtIt == mRouteTable.end())
	{
		//全部过载
		mAllReqMin = true;
        multimap<float, struct SvrNode_t>* pNewTable = new multimap<float, struct SvrNode_t>;
        
        RebuildErrRoute(stItem, pNewTable);
        //有ping测试通过路由
        if (!pNewTable->empty())
		{
			LOG_DEBUG(ELOG_KEY, "[svr] RebuildRoute recovery errorroute restore! gid(%d),xid(%d)",stItem.mGid,stItem.mXid);

            stItem.mPindex = 0;
            mRouteTable.insert(make_pair(stItem, pNewTable));
        }
        else
        {
            SAFE_DELETE(pNewTable);
        }

        LOG_ERROR(ELOG_KEY, "[svr] RebuildRoute recovery errorroute failed! gid(%d),xid(%d)",stItem.mGid,stItem.mXid);
		return -1;
	}
	
	struct SvrKind_t &stKind = (struct SvrKind_t &) (rtIt->first);
	multimap<float, struct SvrNode_t>* pTable = rtIt->second;
	if (pTable == NULL || pTable->empty())
	{
        LOG_ERROR(ELOG_KEY, "[svr] RebuildRoute failed(cannot find routenode)! gid(%d),xid(%d)",stItem.mGid,stItem.mXid);
		return -1;
	}

	int iCurTm = time(NULL);
	if ((iCurTm - stKind.mPtm) > 2 * stKind.mRebuildTm || (stKind.mPtm - iCurTm) > 2 * stKind.mRebuildTm) 
	{
		stKind.mPtm = iCurTm;	//防时间跳变
	}
	//如果还没到路由rebuild时间直接返回，默认一分钟rebuild一次
    if (!bForce && (iCurTm-stKind.mPtm < stKind.mRebuildTm))
	{
        LOG_DEBUG(ELOG_KEY, "[svr] RebuildRoute nothing to do(rebuild not over) bForce(%d) iCurTm-stKind.mPtm(%d) < stKind.mRebuildTm(%d)! gid(%d),xid(%d)",(int) bForce,iCurTm-stKind.mPtm,stKind.mRebuildTm, stItem.mGid,stItem.mXid);
        return -1;
    }
    //rebuild时间
    stItem.mPtm = stKind.mPtm = iCurTm;

    //当前时间
    struct timeval nowTm;
    GetTimeofday(&nowTm, NULL);

    unsigned int iLowDelay = DELAY_MAX;
    bool bErrRateBig = true;  //所有被调都过载标志
    int iReqAll = 0, iReqSuc = 0, iReqRej = 0, iReqErrRet = 0 , iReqErrTm = 0;
    float fAvgErrRate = 0;
    int iRouteTotalReq = 0;
    float fCfgErrRate = 0, fNodeErrRate = 0, fTotalErrRate = 0, fHeightSucRate = 0,fHighWeight = 0,  fBestLowPri = 1;

    //统计数量
    multimap<float, struct SvrNode_t>::iterator it;
    for (it = pTable->begin(); it != pTable->end(); it++)
    {
    	struct SvrInfo_t &si = it->second.mStat->mInfo;

    	iReqAll = si.mReqAll;
    	iReqSuc = si.mReqSuc;
    	iReqRej = si.mReqRej;			//拒绝数
    	iReqErrRet = si.mReqErrRet;
    	iReqErrTm = si.mReqErrTm;
    	fAvgErrRate = si.mAvgErrRate;	//平均错误率

    	iRouteTotalReq += iReqAll - iReqRej;
    	if(iReqAll < iReqSuc + iReqErrRet + iReqErrTm) 
    	{
    		iReqAll = iReqSuc + iReqErrRet + iReqErrTm;	//重置总请求数
    	}

    	//计算节点总请求数、延时值、门限收缩
    	RouteNodeRebuild(it->second.mNet, it->second.mStat);

    	fCfgErrRate = it->second.mStat->mReqCfg.mReqErrMin;	//访问量控制的错误最小值
    	fNodeErrRate = 1 - si.mOkRate;	//路由错误率

    	//只要有一个路由有效错误率未超过阈值，设置此标志
    	if (fNodeErrRate < fCfgErrRate + fAvgErrRate)
    	{
    		bErrRateBig = false;
    	}

    	//过载逻辑
    	if (fNodeErrRate < fCfgErrRate)	//节点正常 fNodeErrRate < mReqErrMin(0.2)
    	{
    		//清除连续过载标志
    		stKind.mPsubCycCount = 0;
    		stKind.mPtotalErrRate= 0;
    	}
    	else
    	{
    		//节点过载
	        LOG_ERROR(ELOG_KEY, "[svr] RebuildRoute one node ready to overload, gid(%d),xid(%d),host(%s),port(%d),limit(%d),reqall(%d),reqsuc(%d),reqrej(%d),err(%d),errtm(%d), service err rate[%f][%f]>config err_rate[%f]",
	        	stItem.mGid,stItem.mXid,it->second.mNet.mHost,it->second.mNet.mPort,
	        	it->second.mStat->mReqCfg.mReqLimit,iReqAll,iReqSuc,iReqRej,iReqErrRet,iReqErrTm,fNodeErrRate,fAvgErrRate,fCfgErrRate);
    		
    		//通知 TODO
    	}

    	//重置权重值
    	if (it->second.mNet.mWeight == 0)
    	{
    		it->second.mNet.mWeight = INIT_WEIGHT;
	        LOG_ERROR(ELOG_KEY, "[svr] RebuildRoute weight(0) invalid gid(%d),xid(%d),host(%s),port(%d)",stItem.mGid,stItem.mXid,it->second.mNet.mHost,it->second.mNet.mPort);
    	}
		
		si.mBuildTm = nowTm;
    	fTotalErrRate += fNodeErrRate;						//路由失败率总和
    	fHeightSucRate = max(fHeightSucRate, si.mOkRate);	//路由最高成功率
    	iLowDelay = min(iLowDelay, si.mAvgTm);				//路由最低延时时间
    	fHighWeight = max(fHighWeight, (float)it->second.mNet.mWeight);		//路由最高权重值
    }

    if (iLowDelay <= 0)
    {
    	iLowDelay = 1;
    }

    //全部节点都过载
    if (bErrRateBig)
    {
    	fAvgErrRate = fTotalErrRate / pTable->size();
    	stKind.mPtotalErrRate += fTotalErrRate / pTable->size();	//所有节点过载平均错误率
    	stKind.mPsubCycCount++;
    	mAvgErrRate = stKind.mPtotalErrRate / stKind.mPsubCycCount;

	    LOG_ERROR(ELOG_KEY, "[svr] RebuildRoute all node ready to overload,gid(%d),xid(%d),avg err rate=%f, all service err rate>config err_rate[%f]",
	    	stItem.mGid,stItem.mXid,fAvgErrRate,fCfgErrRate);
    	
    	//通知 TODO
    }

    //更新宕机请求数
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stItem);
	if (reIt != mErrTable.end())
	{
		list<struct SvrNode_t>::iterator eit = reIt->second->begin();
		for (; eit != reIt->second->end(); eit++)
		{
			//累积宕机的服务自宕机开始，该Svr的所有请求数
			//此请求数用来作为判断是否进行宕机恢复探测的依据
			eit->mReqAllAfterDown += iRouteTotalReq;
		}
	}

	multimap<float, struct SvrNode_t>* pNewTable = new multimap<float, struct SvrNode_t>;
    
    mAllReqMin = true;
    float fBestLowSucRateValidate = 1;
    unsigned int iBestBigDelayValidate = 1;
	float fWeightLoad = 1;
	int iReqLimit = 0;
	
	//计算负载
	for(it = pTable->begin(); it != pTable->end(); it++)
	{
		struct SvrStat_t *pStat = it->second.mStat;
		struct SvrInfo_t &si = pStat->mInfo;

		fCfgErrRate = pStat->mReqCfg.mReqErrMin;	//访问量控制错误率最小值
        
        //成功率负载
        if(si.mOkRate > 0)
		{
            si.mOkLoad = (fHeightSucRate) / si.mOkRate;
        }
		else
		{
            si.mOkLoad = 1000000;
        }
        si.mOkLoad = si.mOkLoad < 1 ? 1 : si.mOkLoad;	//成功率最小值为1
        si.mDelayLoad = (float)si.mAvgTm / (float)iLowDelay;	//延时率负载
        fWeightLoad = (float)fHighWeight / (float)it->second.mNet.mWeight;	//权重因子
        
        //系统配置的负载因子
        if(mRateWeight == mDelayWeight)
		{
            si.mLoadX = si.mDelayLoad * si.mOkLoad;
        }
		else
		{
            si.mLoadX = si.mDelayLoad*mDelayWeight + si.mOkLoad*mRateWeight;
        }
        si.mLoadX *= fWeightLoad;	//权重因子

        fBestLowPri = max(fBestLowPri, si.mLoadX);	//最大负载

        struct SvrNode_t stNode(it->second.mNet, pStat); //set mIsDetecting = false
        iReqLimit = pStat->mReqCfg.mReqLimit;
        iReqLimit = iReqLimit <= 0 ? (pStat->mReqCfg.mReqMin + 1) : iReqLimit;
        
        //门限小于最低阈值，加入宕机列表
        if(iReqLimit > pStat->mReqCfg.mReqMin)
		{
            fBestLowSucRateValidate = min(fBestLowSucRateValidate, stNode.mStat->mInfo.mOkRate);
            iBestBigDelayValidate = max(iBestBigDelayValidate, stNode.mStat->mInfo.mAvgTm);

			pNewTable->insert(make_pair(stNode.mKey, stNode));
			mAllReqMin = false;	//fixed
        }
		else
		{
            AddErrRoute(stItem, stNode);	//加入宕机列表
        }
	    LOG_DEBUG(ELOG_KEY, "[svr] RebuildRoute reqlimit=%d reqmin=%d",iReqLimit,pStat->mReqCfg.mReqMin);
	}

	//所有节点都过载
	if (mAllReqMin)
	{
        if (stKind.mPsubCycCount <= 0)
        {
        	stKind.mPsubCycCount = 1;
        }
        mAvgErrRate = stKind.mPtotalErrRate / stKind.mPsubCycCount;	//所有节点过载平均错误率

        if (mAvgErrRate > 0.99999)
        {
        	mAvgErrRate = 0;
        }
        if ((float) (1 - fCfgErrRate) < mAvgErrRate)
		{
            mAvgErrRate = 1 - fCfgErrRate - 0.1;
            if(mAvgErrRate < 0) 
            {
            	mAvgErrRate = 0;
            }
        }
	    LOG_ERROR(ELOG_KEY, "[svr] RebuildRoute all overload, mod=%d cmd=%d avg err rate=%f, all req to min>config err_rate[%f]",
	    	stItem.mGid,stItem.mXid,mAvgErrRate,fCfgErrRate);
 	
        //通知 TODO
	}

	//宕机探测
    RebuildErrRoute(stItem, pNewTable, fBestLowPri, fBestLowSucRateValidate, iBestBigDelayValidate);
    
    mRouteTable.erase(rtIt);
    struct SvrKind_t stNewKind(stKind);
    stNewKind.mPindex = 0;
    if (pNewTable != NULL && !pNewTable->empty())
    {
    	mRouteTable.insert(make_pair(stNewKind, pNewTable));
    }
    SAFE_DELETE(pTable);
    return 0;
}

int SvrQos::AddErrRoute(struct SvrKind_t& stItem, struct SvrNode_t& stNode)
{
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stItem);
    list<struct SvrNode_t>* pErrRoute = NULL;
    int iErrCount = 0;
    if(reIt == mErrTable.end())
	{
        pErrRoute = new list<struct SvrNode_t>;
        stItem.mRebuildTm = stNode.mStat->mReqCfg.mRebuildTm;
        mErrTable.insert(make_pair(stItem, pErrRoute));
    }
	else
	{
        pErrRoute = reIt->second;
        iErrCount = pErrRoute == NULL ? 0 : pErrRoute->size();
	}

	time_t nowTm = time(NULL);
	
	LOG_DEBUG(ELOG_KEY, "[svr] AddErrRoute node overload, exist count=%d: gid=%d xid=%d host=%s port=%u stop_time=%d limit=%d",
		iErrCount+1,stNode.mNet.mGid,stNode.mNet.mXid,stNode.mNet.mHost,stNode.mNet.mPort,nowTm - stNode.mStopTime,stNode.mStat->mReqCfg.mReqLimit);
    	
	//record down server
	stNode.mStopTime = nowTm;
	stNode.mReqAllAfterDown = 0;
	//stNode.mHasDumpStatistic = 0;
    pErrRoute->push_back(stNode); 
    return 0;
}

/** TODO 重建宕机路由（故障恢复，恢复后放入pSvrNode指针中） */
int SvrQos::RebuildErrRoute(struct SvrKind_t& stItem, multimap<float, struct SvrNode_t>* pSvrNode, float iPri, float fLowOkRate, unsigned int iBigDelay)
{
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stItem);
    if(reIt == mErrTable.end())
	{
		LOG_ERROR(ELOG_KEY, "[svr] RebuildErrRoute failed(cannot find errroute) gid(%d),xid(%d)",stItem.mGid,stItem.mXid);
        return -1;
    }
    list<struct SvrNode_t>* pErrRoute = reIt->second;
    if(pErrRoute->empty())
	{
		LOG_ERROR(ELOG_KEY, "[svr] RebuildErrRoute failed(cannot find errroute routenode in list) gid(%d),xid(%d)",stItem.mGid,stItem.mXid);
        SAFE_DELETE(pErrRoute);
		mErrTable.erase(reIt);
        return -1;
	}

	list<struct SvrNode_t>::iterator it = pErrRoute->begin();
	if (mAllReqMin)
	{
		iPri = 1;
	}

	int iCurTm = time(NULL);

	vector<struct DetectNode_t> vDetectNodeadd, vDetectNodedel;
	
	//宕机检测、故障恢复
	int iRet = -1;
	bool bDelFlag = false;
	int iDetectStat = -1;
	while(it != pErrRoute->end())
	{
		struct DetectResult_t stRes;
		bDelFlag = false;
		iDetectStat = -1;  //-2: 没root权限探测  -1:网络探测失败 0:网络探测成功

		//故障探测：
		//1)先进行网络层探测ping connect udp_icmp，网络探测成功后再用业务请求进行探测
		//2)机器宕机后该sid已经收到超过一定量的请求
		//3)机器宕机已经超过一定时间，需要进行一次探测
		
		//mProbeBegin>0 才打开网络层探测
		if (mDownCfg.mProbeBegin > 0 && iCurTm > it->mStopTime + mDownCfg.mProbeBegin)
		{	
			struct DetectNode_t stDetectNode(it->mNet.mHost, it->mNet.mPort, iCurTm, iCurTm + mDownCfg.mProbeNodeExpireTime);
			iRet = mDetectThread->GetDetectResult(stDetectNode, stRes);
			
			LOG_ERROR(ELOG_KEY, "[svr] RebuildErrRoute detect ret=%d,code=%d,type=%d,gid=%d xid=%d host=%s port=%u after_stop_time=%d,reqall_after_down=%d,expire=%d,limit=%d",
				iRet, stRes.mRc, stRes.mDetectType, it->mNet.mGid,it->mNet.mXid,it->mNet.mHost, it->mNet.mPort,
				iCurTm - it->mStopTime, it->mReqAllAfterDown, stDetectNode.mExpireTime - stDetectNode.mCreateTime, it->mStat->mReqCfg.mReqLimit);
			
			if (iRet < 0)	//not found
			{
				vDetectNodeadd.push_back(stDetectNode);
			}
			else if (iRet == 0)
			{
				if (stRes.mRc < 0)
				{
					if(stRes.mRc == NOT_PRI)
					{
						//没有root权限，无法raw socket进行探测
						iDetectStat = -2;
					}
					else
					{
						iDetectStat = -1;
					}
				}
				else if(stRes.mDetectType == DETECT_TCP || stRes.mDetectType == DETECT_UDP)
				{
					//tcp or udp success, other fail
					vDetectNodedel.push_back(stDetectNode);
					bDelFlag = true;
					iDetectStat = 0;
				}
				else
				{
					//tcp or udp fail
					iDetectStat = -1;
				}
			}
		}

		//在没达到恢复条件之前，如果网络探测成功，提前恢复
		if (!mAllReqMin && (iCurTm - it->mStopTime < mDownCfg.mDownTimeTrigerProbe) && (it->mReqAllAfterDown < mDownCfg.mReqCountTrigerProbe))
		{
			if (iDetectStat != 0)	//探测不成功
			{
				it++;
				continue;
			}
		}

		//达到恢复条件，如果网络探测失败，推迟恢复
		if((mDownCfg.mProbeBegin > 0) && (-1 == iDetectStat))
		{
	        it++; 
	        continue;
		}

		it->mStat->mReqCfg.mReqLimit = mDownCfg.mProbeTimes;
		it->mIsDetecting = true;
		it->mKey = iPri;

		if (mAllReqMin)
		{
            it->mStat->mInfo.mAvgErrRate = mAvgErrRate;
            it->mStat->mInfo.mLoadX = 1;
            it->mStat->mInfo.mOkLoad = 1;
            it->mStat->mInfo.mDelayLoad = 1;
        }
		else
		{
            it->mStat->mInfo.mLoadX = iPri;
            it->mStat->mInfo.mOkRate = fLowOkRate;
            it->mStat->mInfo.mAvgTm = iBigDelay; 
        }
		//统计置0
		it->mStat->mInfo.mReqAll = 0;
		it->mStat->mInfo.mSReqAll = 0;
		it->mStat->mInfo.mReqRej = 0;
		it->mStat->mInfo.mSReqRej = 0;
		it->mStat->mInfo.mReqSuc = 0;
		it->mStat->mInfo.mSReqSuc = 0;
		it->mStat->mInfo.mReqErrRet = 0;
		it->mStat->mInfo.mSReqErrRet = 0;
		it->mStat->mInfo.mReqErrTm = 0;
		//it->mStat->mInfo.mSReqErrTm = 0;

		LOG_ERROR(ELOG_KEY, "[svr] RebuildErrRoute failed(cannot find errroute routenode in list) gid(%d),xid(%d)",stItem.mGid,stItem.mXid);

        pSvrNode->insert(make_pair(iPri, *it));
        pErrRoute->erase(it++);
	}

    if(pErrRoute->empty())
	{
        SAFE_DELETE(pErrRoute);
		mErrTable.erase(reIt);
	}

    if(!vDetectNodeadd.empty()) mDetectThread->AddDetect(vDetectNodeadd);
    if(!vDetectNodedel.empty()) mDetectThread->DelDetect(vDetectNodeadd);
	
	return 0;
}

/** 修改路由权重 */
int SvrQos::ModRouteNode(struct SvrNet_t& stSvr)
{
	SvrKind_t stNode(stSvr);
    map<struct SvrKind_t,  multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.find(stNode);
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stNode);

	if(rtIt == mRouteTable.end() && reIt == mErrTable.end())
	{
		LOG_ERROR(ELOG_KEY, "[svr] ModRouteNode route invalid Svr gid(%d),xid(%d)",stSvr.mGid,stSvr.mXid);
        return -1;
    }

	if(rtIt != mRouteTable.end())
	{
		multimap<float, struct SvrNode_t>* pTable = rtIt->second;
		multimap<float, struct SvrNode_t>::iterator it = pTable->begin();
		for(; it != pTable->end(); it++)
		{
			if(it->second.mNet == stSvr)	//请求节点
			{
				LOG_DEBUG(ELOG_KEY, "[svr] ModRouteNode route weight gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
                if (it->second.mNet.mWeight != stSvr.mWeight)
				{
                    it->second.mNet.mWeight = stSvr.mWeight;	//设置为请求权重
                }
				return 0;
			}
		}
	}

	if(reIt != mErrTable.end())
	{
		list<SvrNode_t>* pNode = reIt->second;
		list<SvrNode_t>::iterator it = pNode->begin();
		for(; it != pNode->end(); it++)
		{
			if(it->mNet == stSvr)
			{
				LOG_DEBUG(ELOG_KEY, "[svr] ModRouteNode error weight gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
                if(it->mNet.mWeight != stSvr.mWeight)
				{
                    it->mNet.mWeight = stSvr.mWeight;
                }
				return 0;
			}
		}
	}

	LOG_DEBUG(ELOG_KEY, "[svr] ModRouteNode nothing to do gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
	return 0;
}

/** 删除路由节点 */
int SvrQos::DelRouteNode(struct SvrNet_t& stSvr)
{
    SvrKind_t stNode(stSvr);
    map<struct SvrKind_t,  multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.find(stNode);
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stNode);

	if(rtIt == mRouteTable.end() && reIt == mErrTable.end())
	{
		LOG_ERROR(ELOG_KEY, "[svr] del route node failed(cannot find route) gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
        return -1;
    }

    unsigned int iDelCount = 0;
	if(rtIt != mRouteTable.end())
	{
		multimap<float, struct SvrNode_t>* pTable = rtIt->second;
        if (pTable != NULL)
		{
            multimap<float, struct SvrNode_t>::iterator it = pTable->begin();

            while(it != pTable->end())
			{
                if(it->second.mNet == stSvr)
				{
                    pTable->erase(it++);
                    if(pTable->empty())
					{
                        mRouteTable.erase(rtIt);
                        SAFE_DELETE(pTable);
                    }
                    iDelCount++;
                    break;
                }
				else
				{
                    it++;
                }
            }
        }
        else
        {
        	LOG_ERROR(ELOG_KEY, "[svr] del node info failed(cannot find node info) gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
        }
	}
	if (reIt != mErrTable.end())
	{
		list<SvrNode_t>* pNode = reIt->second;
		if (pNode != NULL)
		{
			list<SvrNode_t>::iterator it = pNode->begin();
			while(it != pNode->end())
			{
				if (it->mNet == stSvr)
				{
					pNode->erase(it++);
					if (pNode->empty())
					{
                        mErrTable.erase(reIt);
                        SAFE_DELETE(pNode);
					}
					iDelCount++;
					break;
				}
				else
				{
					it++;
				}
			}
		}
		else
		{
        	LOG_ERROR(ELOG_KEY, "[svr] del error node info failed(cannot find error node info) gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
		}
	}

	if(iDelCount > 0)
	{
		return 0;
	}

	LOG_ERROR(ELOG_KEY, "[svr] del node info failed(nothing to do) gid(%d),xid(%d),host(%s),port(%d),weight(%d)",stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight);
	return -1;
}

/** 请求节点是否存在 */
bool SvrQos::IsExistNode(struct SvrNet_t& stSvr)
{
	map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
	if(mapReqIt == mMapReqSvr.end())
	{
		return false;
	}
	return true;
}

/** 节点是否变化（新配置始终下发，旧配置检测到version变化才下发） */
bool SvrQos::IsVerChange(struct SvrNet_t& stSvr)
{
	map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
	if(mapReqIt != mMapReqSvr.end())
	{
		struct SvrNet_t &stKey = const_cast<struct SvrNet_t&> (mapReqIt->first);
		if(stKey.mVersion != stSvr.mVersion)
		{
			return true;
		}
	}
	return false;
}

/** 获取所有节点 */
int SvrQos::GetSvrAll(struct SvrNet_t* pBuffer)
{
	map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.begin();
	for(int i = 0; mapReqIt != mMapReqSvr.end(); i++, mapReqIt++)
	{
		pBuffer[i] = const_cast<struct SvrNet_t&> (mapReqIt->first);
	}
	return mMapReqSvr.size();
}

int SvrQos::GetAddCount(SvrStat_t* pSvrStat, int iReqCount)
{
    if(pSvrStat->mInfo.mLastErr)
    {
        return max((int)((pSvrStat->mInfo.mLastAlarmReq-iReqCount) * pSvrStat->mReqCfg.mReqExtendRate), pSvrStat->mReqCfg.mReqMin);
    }
    else
    {
        return max((int)(pSvrStat->mReqCfg.mReqLimit * pSvrStat->mReqCfg.mReqExtendRate/pSvrStat->mInfo.mDelayLoad), pSvrStat->mReqCfg.mReqMin);
    }
}

/** for test */
void SvrQos::LogAllNode()
{
	LOG_DEBUG(ELOG_KEY, "[svr] LogAllNode for test mRouteTable=%d mErrTable=%d", mRouteTable.size(),mErrTable.size());
	
	//路由map
	map<struct SvrKind_t,  multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.begin(), rtIt1 = mRouteTable.end();
	for(rtIt; rtIt != rtIt1; rtIt++)
	{
		multimap<float, struct SvrNode_t>* pTable = rtIt->second;
		multimap<float, struct SvrNode_t>::iterator it = pTable->begin() , it1 = pTable->end();
		LOG_DEBUG(ELOG_KEY, "[svr] LogAllNode mRouteTable kind(gid=%d xid=%d) have node size=%d", rtIt->first.mGid,rtIt->first.mXid,rtIt->second->size());

		for (it; it != it1; it++)
		{
            LOG_DEBUG(ELOG_KEY, "[svr] LogAllNode mRouteTable key=%f, gid=%d xid=%d host=%s port=%hu weight=%d pre=%d,reqall=%d reqrej=%d reqsuc=%d reqerrret=%d reqerrtm=%d loadx=%f okload=%f delayload=%f okrate=%f avgtm=%d preall=%d,reqlimit=%d reqmax=%d,reqmin=%d reqerrmin=%f reqrebuild=%d pretime=%d",
            	it->first,
            	it->second.mNet.mGid,it->second.mNet.mXid,it->second.mNet.mHost,it->second.mNet.mPort,it->second.mNet.mWeight,it->second.mNet.mPre,
            	it->second.mStat->mInfo.mReqAll,it->second.mStat->mInfo.mReqRej,it->second.mStat->mInfo.mReqSuc,it->second.mStat->mInfo.mReqErrRet,
            	it->second.mStat->mInfo.mReqErrTm,it->second.mStat->mInfo.mLoadX,it->second.mStat->mInfo.mOkLoad,it->second.mStat->mInfo.mDelayLoad,
            	it->second.mStat->mInfo.mOkRate,it->second.mStat->mInfo.mAvgTm,it->second.mStat->mInfo.mPreAll,
            	it->second.mStat->mReqCfg.mReqLimit,it->second.mStat->mReqCfg.mReqMax,it->second.mStat->mReqCfg.mReqMin,it->second.mStat->mReqCfg.mReqErrMin,it->second.mStat->mReqCfg.mRebuildTm,it->second.mStat->mReqCfg.mPreTime);
		}
	}

	//宕机map
	map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.begin(), reIt1 = mErrTable.end();
	LOG_DEBUG(ELOG_KEY,"[svr] LogAllNode mErrTable size=%d",mErrTable.size());
	for (reIt; reIt != reIt1; reIt++)
	{
		list<struct SvrNode_t>* pErrRoute = reIt->second;
		list<struct SvrNode_t>::iterator it = pErrRoute->begin(), it1 = pErrRoute->end();
		LOG_DEBUG(ELOG_KEY, "[svr] LogAllNode mErrTable kind(gid=%d xid=%d) have node size=%d", reIt->first.mGid,reIt->first.mXid,reIt->second->size());

		for (it; it != it1; it++)
		{
            LOG_DEBUG(ELOG_KEY, "[svr] LogAllNode mErrTable gid=%d xid=%d host=%s port=%hu weight=%d pre=%d,reqall=%d reqrej=%d reqsuc=%d reqerrret=%d reqerrtm=%d loadx=%f okload=%f delayload=%f okrate=%f avgtm=%d preall=%d,reqlimit=%d reqmax=%d,reqmin=%d reqerrmin=%f reqrebuild=%d pretime=%d",
            	it->mNet.mGid,it->mNet.mXid,it->mNet.mHost,it->mNet.mPort,it->mNet.mWeight,it->mNet.mPre,
            	it->mStat->mInfo.mReqAll,it->mStat->mInfo.mReqRej,it->mStat->mInfo.mReqSuc,it->mStat->mInfo.mReqErrRet,
            	it->mStat->mInfo.mReqErrTm,it->mStat->mInfo.mLoadX,it->mStat->mInfo.mOkLoad,it->mStat->mInfo.mDelayLoad,
            	it->mStat->mInfo.mOkRate,it->mStat->mInfo.mAvgTm,it->mStat->mInfo.mPreAll,
            	it->mStat->mReqCfg.mReqLimit,it->mStat->mReqCfg.mReqMax,it->mStat->mReqCfg.mReqMin,it->mStat->mReqCfg.mReqErrMin,it->mStat->mReqCfg.mRebuildTm,it->mStat->mReqCfg.mPreTime);
		}
	}
}
