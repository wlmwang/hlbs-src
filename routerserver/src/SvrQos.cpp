
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "SvrQos.h"

SvrQos::SvrQos()
{
	Initialize();
}

SvrQos::~SvrQos() {}

void SvrQos::Initialize() {}

/** 添加节点&路由 | 修改节点&路由权重 */
int SvrQos::SaveNode(struct SvrNet_t& stSvr)
{
	//访问量加1
	//...
	
	map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
	if (mapReqIt == mMapReqSvr.end())
	{
		if(AllocNode(stSvr) < 0)
		{
			//log
			return -1;
		}
	}
	else
	{
		//修改weight
		struct SvrNet_t &stKey = const_cast<struct SvrNet_t&> (mapReqIt->first);
		if (stSvr.mWeight != stKey.mWeight)
		{
			//log
            stKey.mWeight = stSvr.mWeight;
            ModRouteNode(stSvr);
		}
	}
	return 0;
}

/** 删除 节点&路由 */
int SvrQos::DelNode(struct SvrNet_t& stSvr)
{
	//访问量加1
	//...
	
    map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
    if (mapReqIt == mMapReqSvr.end())
	{
		//log
        return -1;
    }
    SvrStat_t* pSvrStat = mapReqIt->second;
    mMapReqSvr.erase(mapReqIt);
    
    DelRouteNode(stSvr);
    SAFE_DELETE(pSvrStat);
    return 0;
}

/** 修改 节点&路由 权重（权重为0删除节点） */
int SvrQos::ModNode(struct SvrNet_t& stSvr)
{
	//访问量加1
	//...
	
	if (stSvr.mWeight < 0)
	{
		//log
		return -1;
	}

	int iRet = 0;
	if (stSvr.mWeight == 0)		//权重为0删除节点
	{
		//log
		iRet = DelNode(stSvr);
	}
	else
	{
		//log
		if(stSvr.mWeight > MAX_WEIGHT)
		{
			//log
			stSvr.mWeight = MAX_WEIGHT;
		}
		iRet = SaveNode(stSvr);
	}
	return iRet;
}

/** 调用数上报 */
int NotifyNode(struct SvrNet_t& stSvr)
{
	//访问量加1
	//...
	
	//log
	
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
int CallerNode(struct SvrNet_t& stSvr, struct SvrCaller_t& stCaller)
{
	//访问量加1
	//...
	
	//log
	
	if (stCaller.mReqUsetimeUsec <= 0)
	{
		stCaller.mReqUsetimeUsec = 1;
	}

    map<struct SvrNet_t, struct SvrStat_t*>::iterator mapReqIt = mMapReqSvr.find(stSvr);
    if (mapReqIt == mMapReqSvr.end())
	{
		//log
        return -1;
    }
   
    SvrStat_t* pSvrStat = mapReqIt->second;
    if (stCaller.mReqRet >= 0)
    {
    	pSvrStat->mInfo.mReqSuc += stCaller.mReqCount;
    	pSvrStat->mInfo.mSReqSuc += stCaller.mReqCount;
    	pSvrStat->mInfo.mTotalUsec += stCaller.mReqUsetimeUsec;
    	pSvrStat->mInfo.mContErrCount = 0;
    }
    else
    {
    	pSvrStat->mInfo.mReqErrRet += stCaller.mReqCount;
    	pSvrStat->mInfo.mSReqErrRet += stCaller.mReqCount;
    	pSvrStat->mInfo.mContErrCount += stCaller.mReqCount;
    }

    SvrKind_t stNode(stSvr);
    RebuildRoute(stNode);
    return 0;
}

/** 添加 节点&路由 */
int AllocNode(struct SvrNet_t& stSvr)
{
	struct SvrStat_t* pSvrStat = new SvrStat_t();
	if (pSvrStat == NULL)
	{
		//log
		SAFE_DELETE(pSvrStat);
		return -1;
	}

	gettimeofday(&pSvrStat->mInfo.mBuildTm, NULL);	//重建时间
	if(LoadStatCfg(stSvr, pSvrStat) < 0)
	{
		//log
		SAFE_DELETE(pSvrStat);
		return -1;
	}

	mMapReqSvr[stSvr] = pSvrStat;
	pSvrStat->mRebuildTm = mRebuildTm;	//设置节点重建时间间隔
	AddRouteNode(stSvr, pSvrStat);		//将新Req节点加入到节点路由map
	//log
	return 0;
}

/** 单次获取路由 */
int QueryNode(struct SvrNet_t& stSvr)
{
	//统计信息
	//
	int iAck = GetRouteNode(stSvr);
	//log
	return iAck;
}

/** 加载阈值配置 */
int LoadStatCfg(struct SvrNet_t& stSvr, struct SvrStat_t* pSvrStat)
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
int AddRouteNode(struct SvrNet_t& stSvr, struct SvrStat_t* pSvrStat)
{
    SvrKind_t stNode(stSvr);
    stNode.mRebuildTm = pSvrStat->mReqCfg.mRebuildTm;

    multimap<float, struct SvrNode_t>* &pTable = mRouteTable[stSvr];
    if(pTable == NULL)
	{
        pTable = new multimap<float, SvrNode_t>;
    }
    if(pTable == NULL)
    {
    	//log
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

        //正在运行时，突然新加一个服务器，如果mPreAll=0。GetRoute函数分配服务器时，会连续新加的服务器
        //导致：被调扩容时，新加的服务器流量会瞬间爆增，然后在一个周期内恢复正常
        pSvrStat->mInfo = stNode.mStat->mInfo;	//统计信息
        break;
    }

    struct SvrNode_t stRouteInfo(stSvr, pSvrStat);
    pTable->insert(make_pair(stRouteInfo.mKey, stRouteInfo));
    return 0;
}

/** 单次获取路由 */
int GetRouteNode(struct SvrNet_t& stSvr)
{
	//log
	if(stSvr.mGid <= 0 || stSvr.mXid <= 0)
	{
		return -1;
	}
	SvrKind_t stNode(stSvr);
	
	/** 重建路由 */
	RebuildRoute(stNode);

	map<struct SvrKind_t,  multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.find(stNode);
	//找不到相关路由
	if(rtIt == mRouteTable.end())
	{
		//log
		if (stSvr.mGid > 0 && stSvr.mXid > 0)
		{
			/* code */
		}
        return -1;
    }

    struct SvrKind_t &stKind = (struct SvrKind_t &) (rtIt->first);
    stKind.mAccess64tm = GetTimeofday();	//访问时间

    multimap<float, struct SvrNode_t>* pTable = rtIt->second;
	if(pTable == NULL || pTable->empty())
	{
		//log
		return -1;
	}

	int iIndex = stKind.mPindex;	//已分配到第几个路由
	if (iIndex < 0 || iIndex >= pTable->size())
	{
		iIndex = 0;
		stKind.mPindex = 0;	//防止死循环
	}
	
	multimap<float, struct SvrNode_t>::iterator it = pTable->begin() , it1 = pTable->begin();
	
	int iReq = it1->second.mStat->mInfo.mPreAll;	//负载因子
	double dFirstReq = it1->second.mKey * iReq;		//最小权重 调整值
	bool bFirstRt = false;
	it += iIndex;

	if (iIndex != 0)
	{
		int iReq = it->second.mStat->mInfo.mPreAll;
		double dCurAdjReq = it->second.mKey * iReq;

		//此处采用round_robin的经典算法,获得目标路由,需要进行循环
        //如果不是第一个路由，获得比第一个路由的负载更低的路由
		while(dCurAdjReq >= dFirstReq)
		{
            if(iIndex == 0) break;

			int iReq = it->second.mStat->mInfo.mPreAll;
			dCurAdjReq = it->second.mKey * iReq;
			stKind.mPindex = iIndex;
		}
	}

	if(iIndex == 0)
	{
		bFirstRt = true;
	} 
	
	//已经获得了预选路由即ip,port
	memcpy(stSvr.mHost, it->second.mNet.mHost, sizeof(stSvr.mHost));
	stSvr.mPort = it->second.mNet.mPort;
    stSvr.mWeight = it->second.mNet.mWeight; //fixed

    int iRet;
	while((iRet = RouteCheck(it->second.mStat, stSvr, dFirstReq, bFirstRt)) != 0)
	{
		iIndex++;
		bFirstRt = false;
		if(iIndex >= (int)pTable->size())
		{
			iIndex = 0;
			bFirstRt = true;
		}

		if(iIndex == stKind.mPindex)	//一个轮回
		{
			it = it1;
			it += iIndex;
            if (iRet == QOS_RTN_OFFSIDE)	//todo.
			{
				memcpy(stSvr.mHost, it->second.mNet.mHost, sizeof(stSvr.mHost));
				stSvr.mPort = it->second.mNet.mPort;
                //log
                break;
            }
			else
			{
                it->second->mStat->mInfo->mReqRej++;
                it->second->mStat->mInfo->mSReqRej++;
                //log
                return -1;
            }
		}

		it = it1;
		it += iIndex;

		memcpy(stSvr.mHost, it->second.mNet.mHost, sizeof(stSvr.mHost));
		stSvr.mPort = it->second.mNet.mPort;
	    stSvr.mWeight = it->second.mNet.mWeight; //fixed
	}

	time_t nowTm = time(NULL);

	stSvr.mExpired = nowTm + stKind.mRebuildTm - (nowTm - stKind.mPtm);
	if(it1 == it)
	{
		it++;
		if(it == pTable->end())
		{
			return 0;
		}

        //如果是第一个路由, 获取负载更低的路由
		if(it->second.mStat->mInfo.mPreAll * it->second.mKey < dFirstReq)
		{
			if((iIndex + 1) < (int)pTable->size())
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
int RouteCheck(struct SvrStat_t* pSvrStat, struct SvrNet& stNode, double iFirstReq, bool bFirstRt)
{
    pSvrStat->mInfo.mReqAll++; 
    pSvrStat->mInfo.mSReqAll++;

    stNode.mPre = 1;
    if (pSvrStat->mReqCfg.mReqLimit <= 0)
	{
	    pSvrStat->mInfo.mReqAll++; 
	    pSvrStat->mInfo.mSReqAll++;
        return 0;
    }

    int iReqCount = pSvrStat->mInfo.mReqAll;
    int iErrCount = pSvrStat->mInfo.mReqErrRet + pSvrStat->mInfo.mReqErrTm;
    float fErrRate = (float) iErrCount / (float)iReqCount;


    if (iReqCount >= pSvrStat->mReqCfg.mReqLimit)
	{
		//当访问量超过阀值，且当前周期的错误率超过配置的错误率阀值时才判断为过载！
		if (fErrRate > pSvrStat->mReqCfg.mReqErrMin)
		{
			//pSvrStat->mInfo.mReqRej++; //当真正发生过载的时候才对选定的路由的拒绝数加1
			//log
			
		    pSvrStat->mInfo.mReqAll--; 
		    pSvrStat->mInfo.mSReqAll--;
		    
		    pSvrStat->mInfo.mPreAll++; //bug fix 
            return -1;
        }
		pSvrStat->mInfo.mPreAll++;
        pSvrStat->mInfo.mSPreAll++;
        return 0;
    }
    
    return 0;
}

/*路由节点重建*/
int RouteNodeRebuild(struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat)
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

/** 访问量控制重建 */
int ReqRebuild(SvrNet_t &stSvr, SvrStat_t* pSvrStat)
{
	//总错误个数
	int iErrCount = pSvrStat->mInfo->mReqErrRet + pSvrStat->mInfo->mReqErrTm;
	int iSucCount = pSvrStat->mInfo->mReqSuc;

    if(pSvrStat->mInfo.mReqAll < iErrCount + iSucCount)
    {
        pSvrStat->mInfo.mReqAll = iErrCount + iSucCount;
    }
    int iReqAll = pSvrStat->mInfo.mReqAll;

    //save the old state 
    pSvrStat->mInfo.mLastReqAll = iReqAll;
    pSvrStat->mInfo.mLastReqRej = pSvrStat->mInfo.mReqRej;
    pSvrStat->mInfo.mLastReqErrRet = pSvrStat->mInfo.mReqErrRet;
    pSvrStat->mInfo.mLastReqErrTm = pSvrStat->mInfo.mReqErrTm;
    pSvrStat->mInfo.mLastReqSuc = pSvrStat->mInfo.mReqSuc;
    if(pSvrStat->mInfo.mLastReqSuc < 0) 
	{
		pSvrStat->mInfo.mLastReqSuc = 0;
	}

    if(iReqAll <= 0 && iErrCount + iSucCount <= 0)
	{
        pSvrStat->mInfo.mOkRate = 1;
        pSvrStat->Reset(); //avg_tm not change
        return 0;
    }

    //失败率
    float fErrRate = iReqAll>0 ? (float)iErrCount/iReqAll : 0;
    if (isnan(fErrRate))
    {
        //log. fErrRate is Nan!!!
        
        fErrRate = 0;
    }
    iSucCount > 0 ? iSucCount : 0;
    fErrRate < 1 ? fErrRate : 1;
    pSvrStat->mInfo.mOkRate = 1 - fErrRate;

	//统计节点延时值
    if(pSvrStat->mInfo.mReqSuc > 0)
	{
        pSvrStat->mInfo.mAvgTm = pSvrStat->mInfo.mTotalUsec / pSvrStat->mInfo.mReqSuc;
    }
	else
	{
        if(iReqAll > pSvrStat->mReqCfg.mReqMin)
		{
            pSvrStat->mInfo.mAvgTm = DELAY_MAX; 	//超过阈值，延时为最大值
        }
		else
		{
            if(pSvrStat->mInfo.mAvgTm && pSvrStat->mInfo.mAvgTm < DELAY_MAX)
			{
                pSvrStat->mInfo.mAvgTm = 2* pSvrStat->mInfo.mAvgTm; 	//随着时间推移越来越大
            }
			else
			{
                pSvrStat->mInfo.mAvgTm = 1000000;
            }
		}
	}

	//调整门限值limit
	//路由的过载判断逻辑修改为：路由的当前周期错误率超过配置的错误率阀值时才判断为过载
	//不在需要_req_limit_default的配置。
	if(pSvrStat->mReqCfg->mReqLimit <= 0)
	{
		if(iReqAll > pSvrStat->mReqCfg.mReqMin)
		{
			pSvrStat->mReqCfg.mReqLimit = iReqAll;
		}
		else
		{
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg._req_min + 1;
		}
	}
	pSvrStat->mReqCfg.mReqCount = iReqAll;

    if(fErrRate <= pSvrStat->mReqCfg.mReqErrMin)
	{
        pSvrStat->mInfo.mAvgErrRate = 0;	//平均失败率
    }

    if (fErrRate <= pSvrStat->mReqCfg.mReqErrMin + pSvrStat->mInfo.mAvgErrRate)
	{	
        //错误率小于一定的比例,认同为成功的情况
        //如果成功数大于上次过度扩张时的门限，则门限不再有意义
        if(pSvrStat->mInfo.mLastErr && iSucCount > pSvrStat->mInfo.mLastAlarmReq)
        {
            pSvrStat->mInfo.mLastErr = false;
        }

        //目标调整值： iReqAll * (1 + mReqExtendRate)
        int iDest = iReqAll + (int)(iReqAll * mReqCfg.mReqExtendRate);
        if(pSvrStat->mReqCfg.mReqLimit < iDest)
		{
            pSvrStat->mReqCfg.mReqLimit += GetAddCount(pSvrStat, iReqAll);
            if(pSvrStat->mReqCfg.mReqLimit > iDest)
            {
                pSvrStat->mReqCfg.mReqLimit = iDest;
            }
        }
    }
	else
	{ 
		//错误率大于一定的比例,失败的情况,需要按失败率收缩门限
		if(pSvrStat->mInfo.mOkRate > 0)
		{
			fErrRate -= pSvrStat->mInfo.mAvgErrRate;
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

		pSvrStat->mInfo.mLastErr = true;
		pSvrStat->mInfo.mLastAlarmReq = iReqAll;
		pSvrStat->mInfo.mLastAlarmSucReq = iSucCount;

		if (iReqAll <= pSvrStat->mReqCfg.mReqMin)
		{
			pSvrStat->mInfo.mLastErr = false;
		}
	}

    //1)_req_limit 大于最小值+1
	//2)如果最后记录的连续失败次数累加达到设定值，则该机器有宕机嫌疑
	//3)错误率大于一定的比例,判定为有宕机嫌疑
	if (pSvrStat->mReqCfg.mReqLimit > (pSvrStat->mReqCfg.mReqMin + 1) 
		&& ((pSvrStat->mInfo.mContErrCount >= mDownCfg.mPossibleDownErrReq)
		|| (fErrRate > mDownCfg.mPossbileDownErrRate)))
	{
		pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin + 1;
		//log
	}

    //硬的限制：不可超出[req_min, req_max]的范围
    if(pSvrStat->mReqCfg.mReqLimit > pSvrStat->mReqCfg.mReqMax)
    {
        pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMax;
    }
    if(pSvrStat->mReqCfg.mReqLimit < pSvrStat->mReqCfg.mReqMin)
    {
        pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin;
    }

    pSvrStat->Reset();
    return 0;
}

/** 并发量控制重建 */
int ListRebuild(SvrNet_t &stSvr, SvrStat_t* pSvrStat)
{
	int iReqCount = pSvrStat->mInfo->mReqAll - pSvrStat->mInfo->mReqRej;	//请求数
	if (iReqCount <= 0)
	{
		return 0;
	}

	//总错误个数
	int iErrCount = pSvrStat->mInfo->mReqErrRet + pSvrStat->mInfo->mReqErrTm;
	//错误率
	float fErrRate = (float)iErrCount / (float)iReqCount;
	//拒绝率
	float fRejRate = pSvrStat->mInfo->mReqRej / pSvrStat->mInfo->mReqAll;

	fErrRate = fErrRate>1 ? 1: fErrRate;
	pSvrStat->mInfo->mOkRate = 1 - fErrRate;	//重置成功率

	if (fErrRate <= pSvrStat->mListCfg->mListMin)	//错误率小于阈值
	{
		pSvrStat->mInfo->mAvgErrRate = 0;
	}

	//错误数超过阈值
	if (iErrCount < iReqCount)
	{
		pSvrStat->mInfo->mAvgTm = pSvrStat->mInfo->mTotalUsec / pSvrStat->mInfo->mReqSuc;
	}
	else
	{
		pSvrStat->mInfo->mAvgTm = DELAY_MAX;	//最大延时
		pSvrStat->mInfo->mOkRate = 0;
	}

	//并发队列
    //pSvrStat->mList.QosRebuild(fErrRate, fRejRate, pSvrStat->mInfo);
    //pSvrStat->mListCfg.mListLimit = pSvrStat->mList.Limit();
    pSvrStat->Reset();
    return 0;
}

/** 重建路由 */
int RebuildRoute(struct SvrKind_t& stItem, int bForce)
{
	map<struct SvrKind_t, multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.find(stItem);
	if (rtIt == mRouteTable.end())
	{
		mAllReqMin = true;
        multimap<float, struct SvrNode_t>* pNewTable = new multimap<float, struct SvrNode_t>;
        RebuildErrRoute(stItem, pNewTable);

        if(!pNewTable->empty())
		{
            stItem.inner_change++;
            stItem.pindex = 0;
            mRouteTable.insert(make_pair(stItem, pNewTable));
        }
        else
        {
            SAFE_DELETE(pNewTable);
        }
		return -1;
	}
	
	struct SvrKind_t &stKind = (struct SvrKind_t &) (rtIt->first);
	multimap<float, struct SvrNode_t>* pTable = rtIt->second;
	if(pTable == NULL || pTable->empty())
	{
		return -1;
	}

	int iCurTm = time(NULL);
	//如果还没到路由rebuild时间直接返回，默认一分钟rebuild一次
	if((iCurTm - stKind.mPtm) > 2 * stKind.mRebuildTm || (stKind.mPtm - iCurTm) > 2 * stKind.mRebuildTm) 
	{
		//防时间跳变
		stKind.mPtm = iCurTm;
	}
    if(!bForce && iCurTm - stKind.mPtm < stKind.mRebuildTm)
	{
        return -1;
    }
    //rebuild时间
    stKind.mPtm = iCurTm;
    stItem.mPtm = iCurTm;

    //当前时间
    struct timeval nowTm;
    gettimeofday(&nowTm, NULL);

    unsigned int iLowDelay = DELAY_MAX;
    bool bErrRateBig = true;  //所有被调都过载标志

    int iReqAll = 0, iReqSuc = 0, iReqRej = 0, iReqErrRet = 0 , iReqErrTm = 0;
    float fAvgErrRate = 0;

    int iRouteTotalReq = 0;
    float fCfgErrRate = 0, fNodeErrRate = 0, fTotalErrRate = 0, fHeightSucRate = 0,fHighWeight = 0,  fBestLowPri = 1;/*, avg_err_rate = 0, total_err_rate = 0, high_suc_rage = 0, node_err_rate = 0;*/

    //统计数量
    multimap<float, struct SvrNode_t>::iterator it = pTable->begin();
    for(; it != pTable.end(); it++)
    {
    	struct SvrInfo_t &si = it->second.mStat->mInfo;

    	iReqAll = si.mReqAll;
    	iReqSuc = si.mReqSuc;
    	iReqRej = si.mReqRej;
    	iReqErrRet = si.mReqErrRet;
    	iReqErrTm = si.mReqErrTm;
    	fAvgErrRate = si.mAvgErrRate;

    	iRouteTotalReq += iReqAll - iReqRej;	//总数
    	if(iReqAll < iReqSuc + mReqErrRet + iReqErrTm) 
    	{
    		iReqAll = iReqSuc + mReqErrRet + iReqErrTm;	//总请求数
    	}

    	//caculate loadbalance dependent data items of one subroute by current statistics
    	RouteNodeRebuild(it->second.mNet, it->second.mStat);

    	fCfgErrRate = it->second.mStat->mReqCfg.mReqErrMin;	//访问量控制的最小值
    	fNodeErrRate = 1 - si.mOkRate;	//路由失败率

    	//只要有一个路由失败率未超过阈值，设置此标志
    	if(fNodeErrRate < fCfgErrRate + fAvgErrRate)
    	{
    		bErrRateBig = false;
    	}

    	//过载逻辑
    	if (fNodeErrRate < fCfgErrRate)
    	{
    		//清除连续过载标志
    		stKind.mPsubCycCount = 0;
    		stKind.mPtotalErrRate= 0;
    	}
    	else
    	{
    		//过载
    		//log
    		//通知
    	}

    	//重置权重值
    	if (it.second->mNet.mWeight == 0)
    	{
    		it.second->mNet.mWeight = INIT_WEIGHT;

    		//log
    	}
		
		si.mBuildTm = nowTm;
    	fTotalErrRate += fNodeErrRate;						//路由失败率总和
    	fHeightSucRate = max(fHeightSucRate, si.mOkRate);	//路由最高成功率
    	iLowDelay = min(iLowDelay, si.mAvgTm);				//路由最低延时时间
    	fHighWeight = max(fHighWeight, (float)it.second->mNet.mWeight);		//路由最高权重值
    }

    if (iLowDelay <= 0)
    {
    	iLowDelay = 1;
    }

    //全部节点过载
    if (bErrRateBig)
    {
    	stKind.mPtotalErrRate += fTotalErrRate / pTable->size();	//失败率平均值
    	stKind.mPsubCycCount++;

    	mAvgErrRate = stKind.mPtotalErrRate / stKind.mPsubCycCount;
    	//log
    	//通知
    }

    //更新宕机请求数
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stItem);
	if (reIt != mErrTable.end())
	{
		list<struct SvrNode_t>::iterator it = reIt->second->begin();
		for (; it != reIt->second->end(); it++)
		{
			//累积宕机的服务自宕机开始，该sid的所有请求数
			//此请求数用来作为判断是否进行宕机恢复探测的依据
			it->mReqAllAfterDown += iRouteTotalReq;
		}
	}

	multimap<float, struct SvrNode_t>* pNewTable = new multimap<float, struct SvrNode_t>;

	float fWeightLoad = 1;
	int iReqLimit = 0;
	//计算负载
	for(; it != pTable.end(); it++)
	{
		struct SvrStat_t *pStat = it->second.mStat;
		struct SvrInfo_t &si = pStat->mInfo;

		fCfgErrRate = pStat->mReqCfg.mReqErrMin;	//访问量控制的最小值
        
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

        //si.next_cyc_ready();

        struct SvrNode_t stNode(it->second.mNet, pStat); //set mIsDetecting = false
        iReqLimit = pStat->mReqCfg.mReqLimit;
        iReqLimit = iReqLimit <= 0 ? pStat->mReqCfg.mReqMin + 1 : iReqLimit;	//扩张的限制值
        if(iReqLimit > pStat->mReqCfg.mReqMin)
		{
			pNewTable->insert(make_pair(stNode.mKey, stNode));
			mAllReqMin = false;
        }
		else
		{
            AddErrRoute(stItem, stNode);	//加入宕机列表
        }
	}

	if (mAllReqMin)
	{
        if(stKind.mPsubCycCount <= 0)
        {
        	stKind.mPsubCycCount = 1;
        }

        mAvgErrRate = stKind.mPtotalErrRate / stKind.mPsubCycCount;
        if(mAvgErrRate > 0.99999)
        {
        	mAvgErrRate = 0;
        }

        if((float) (1 - fCfgErrRate) < mAvgErrRate)
		{
            mAvgErrRate = 1 - fCfgErrRate - 0.1;
            if(mAvgErrRate < 0) 
            {
            	mAvgErrRate = 0;
            }
        }
        //log
        //通知
	}

    RebuildErrRoute(stItem, pNewTable);

    SvrKind_t stNewKind(stKind);
    //stNewKind.pindex = 0;
    mRouteTable.erase(rtIt);
    SAFE_DELETE(pTable);
    mRouteTable.insert(make_pair(stNewKind, pNewTable));
    return 0;
}

int AddErrRoute(struct SvrKind_t& stItem, struct SvrNode_t& stNode)
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
	//log
	
	//record down server
	stNode.mStopTime = time(NULL);
	stNode.mReqAllAfterDown = 0;
	//stNode.mHasDumpStatistic = 0;
    pErrRoute->push_back(stNode); 
    return 0;
}

/** TODO 重建宕机路由（故障恢复） */
int RebuildErrRoute(struct SvrKind_t& stItem, multimap<float, struct SvrNode_t>* pSvrNode)
{
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stItem);
    if(reIt == mErrTable.end())
	{
        return -1;
    }
    list<struct SvrNode_t>* pErrRoute = reIt->second;
    if(pErrRoute->empty())
	{
        SAFE_DELETE(pErrRoute);
		mErrTable.erase(reIt);
        return -1;
	}
	list<struct SvrNode_t>::iterator it = pErrRoute->begin();


	int iCurTm = time(NULL);

	//宕机检测、故障恢复
	int iRet = 0;
	bool bDelFlag = false;
	while(it != pErrRoute->end())
	{
		bDelFlag = false;

		//故障探测：
		//1)先进行网络层探测ping connect udp_icmp，网络探测成功后再用业务请求进行探测
		//2)机器宕机后该sid已经收到超过一定量的请求
		//3)机器宕机已经超过一定时间，需要进行一次探测
		
		//mProbeBegin>0才打开网络层探测
		if (mDownCfg.mProbeBegin > 0 && iCurTm > it->mStopTime + mDownCfg.mProbeBegin)
		{
			/* code */
		}
	}

    if(pErrRoute->empty())
	{
        SAFE_DELETE(pErrRoute);
		mErrTable.erase(reIt);
	}

	return 0;
}

/** 修改路由权重 */
int ModRouteNode(struct SvrNet_t& stSvr)
{
	SvrKind_t stNode(stSvr);
    map<struct SvrKind_t,  multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.find(stNode);
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stNode);

	if(rtIt == mRouteTable.end() && reIt == mErrTable.end())
	{
		//log
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
                if(it->mNet.mWeight != stSvr.mWeight)
				{
                    it->mNet.mWeight = stSvr.mWeight;
                }
				return 0;
			}
		}
	}

	//log
	return 0;
}

/** 删除路由节点 */
int DelRouteNode(struct SvrNet_t& stSvr)
{
    SvrKind_t stNode(stSvr);
    map<struct SvrKind_t,  multimap<float, struct SvrNode_t>* >::iterator rtIt = mRouteTable.find(stNode);
    map<struct SvrKind_t, list<struct SvrNode_t>* >::iterator reIt = mErrTable.find(stNode);

	if(rtIt == mRouteTable.end() && reIt == mErrTable.end())
	{
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
        	//log
        }
	}
	if (reIt != mErrTable.end())
	{
		list<SvrNode_t>* pNode = reIt->second;
		if (pNode != NULL)
		{
			list<SvrNode_t>::iterator it = pNode->begin();
			while(it != pNode.end())
			{
				if (it->mNet == stSvr)
				{
					pNode->erase(it++);
					if (pNode->empty())
					{
                        mErrTable.erase(reIt);
                        SAFE_DELETE(it);
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
			//log
		}
	}

	if(iDelCount > 0)
	{
		return 0;
	}

	//log
	return -1;
}

int GetAddCount(SvrStat_t* pSvrStat, int iReqCount)
{
    if(pSvrStat->mInfo->mLastErr)
    {
        return max((int)((pSvrStat->mInfo.mLastAlarmReq-iReqCount) * pSvrStat->mReqCfg.mReqExtendRate), pSvrStat->mReqCfg.mReqMin);
    }
    else
    {
        return max((int)(pSvrStat->mReqCfg.mReqLimit * pSvrStat->mReqCfg.mReqExtendRate/pSvrStat->mInfo.mDelayLoad), pSvrStat->mReqCfg.mReqMin);
    }
}











int SvrQos::InitSvr(SvrNet_t* pSvr, int iNum)
{
	if (iNum <= 0)
	{
		return -1;
	}
	Final();

	Svr_t *pTmpSvr;
	for(int i = 0; i < iNum ; i++)
	{
		if (pSvr[i] == NULL)
		{
			continue;
		}

		pTmpSvr = new Svr_t(pSvr[i]);	//SvrNet -> Svr
		if (pTmpSvr == NULL)
		{
			LOG_ERROR(ELOG_KEY, "[runtime] init id(%d) svr failed", pSvr[i]->mId);
			continue;
		}
		mSvr.push_back(pTmpSvr);
	}

	SvrRebuild();
	return 0;
}

int SvrQos::ReloadSvr(SvrNet_t *pSvr, int iNum)
{
	return InitSvr(pSvr, iNum);
}

//更新不同配置 & 添加新配置
int SvrQos::SyncSvr(SvrNet_t *pSvr, int iNum)
{
	if (iNum <= 0)
	{
		return -1;
	}

	int j = 0;
	Svr_t *pTmpSvr;
	for(int i = 0; i < iNum ; i++)
	{
		vector<Svr_t*>::iterator it = GetItById((pSvr+i)->mId);
		if (it != mSvr.end())
		{
			if(IsChangeSvr(*it, pSvr+i))
			{
				//更新配置
				SAFE_DELETE(*it);
				mSvr.erase(it);
				pTmpSvr = new Svr_t(pSvr[i]);	//SvrNet -> Svr
				if (pTmpSvr == NULL)
				{
					LOG_ERROR(ELOG_KEY, "[runtime] init id(%d) svr failed", pSvr[i]->mId);
					continue;
				}
				mSvr.push_back(pTmpSvr);
				j++;
			}
		}
		else
		{
			//添加新配置
			pTmpSvr = new Svr_t(pSvr[i]);
			if (pTmpSvr == NULL)
			{
				LOG_ERROR(ELOG_KEY, "[runtime] init id(%d) svr failed", pSvr[i]->mId);
				continue;
			}
			mSvr.push_back(pTmpSvr);
			j++;
		}
	}
	SvrRebuild();	
	return j;
}

int SvrQos::GetSvrAll(SvrNet_t* pBuffer)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	if (mSvr.size() < 1)
	{
		return 0;
	}

	for(int i = 0; it != mSvr.end(); i++, it++)
	{
		pBuffer[i] = ((struct SvrNet_t)**it);   //Svr_t => SvrNet_t
	}
	return mSvr.size();
}

int SvrQos::GetAllSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid)
{
	string sGid = Itos(iGid);
	string sXid = Itos(iXid);
	string sGXid = sGid + "-" + sXid;
	
	vector<Svr_t*> vSvr;
	int iNum = 0;
	map<string, vector<Svr_t*> >::iterator mn = mSvrByGXid.find(sGXid);
	if(mn != mSvrByGXid.end())
	{
		vSvr = mn->second;	//已排序
		iNum = vSvr.size();

		vector<Svr_t*>::iterator it = vSvr.begin();
		for(int i = 0; i < iNum && it != vSvr.end(); i++, it++)
		{
			pBuffer[i] = ((struct SvrNet_t)**it);   //Svr_t => SvrNet_t
		}
	}
	else
	{
		iNum = 0;
	}
	return iNum;
}

//获取gid、xid下的某一svr
int SvrQos::GetSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid)
{
	string sGid = Itos(iGid);
	string sXid = Itos(iXid);
	string sGXid = sGid + "-" + sXid;
	
	int iId = 0;
	map<string, vector<Svr_t*> >::iterator mn = mSvrByGXid.find(sGXid);
	if(mn != mSvrByGXid.end())
	{
		iId = GXidWRRSvr(pBuffer, sGXid, mn->second);
	}
	return iId;
}

//加权轮询
int SvrQos::GXidWRRSvr(SvrNet_t* pBuffer, string sKey, vector<Svr_t*> vSvr)
{
	map<string, StatcsGXid_t*>::iterator mn = mStatcsGXid.find(sKey);
	int iLen = vSvr.size();
	if (iLen <= 0 || mn == mStatcsGXid.end())
	{
		return -1;	//讲道理的话，这里不会被执行
	}
	int iLoop = iLen;
	StatcsGXid_t *pStatcs = mn->second;

	LOG_DEBUG(ELOG_KEY, "[runtime] WRR init id,%d", pStatcs->mIdx);
	LOG_DEBUG(ELOG_KEY, "[runtime] WRR init cur,%d", pStatcs->mCur);
	LOG_DEBUG(ELOG_KEY, "[runtime] WRR init gcd,%d", pStatcs->mGcd);

	while(iLoop-- >= 0)
	{
		pStatcs->mIdx = (pStatcs->mIdx + 1) % iLen;
		if (pStatcs->mIdx == 0)
		{
			pStatcs->mCur = pStatcs->mCur - pStatcs->mGcd;
			if (pStatcs->mCur <= 0)
			{
				pStatcs->mCur = vSvr[0]->mWeight; //最大值
				if (pStatcs->mCur <= 0)
				{
					return -1;
				}
			}
		}
		//权重为0，禁用
		if (vSvr[pStatcs->mIdx]->mWeight == 0)
		{
			continue;
		}
		else if (vSvr[pStatcs->mIdx]->mShutdown == 1)	//是否故障
		{
			vSvr[pStatcs->mIdx]->mRfuNum++;
		}
		else if (vSvr[pStatcs->mIdx]->mWeight >= pStatcs->mCur)
		{
			if (vSvr[pStatcs->mIdx]->mPreNum <= 0 && (vSvr[pStatcs->mIdx]->mOverLoad == 1 || CalcOverLoad(vSvr[pStatcs->mIdx]) == 1))	//过载
			{
				vSvr[pStatcs->mIdx]->mRfuNum++;
			} 
			else
			{
				if(vSvr[pStatcs->mIdx]->mOverLoad == 1 || CalcOverLoad(vSvr[pStatcs->mIdx]) == 1)
				{
					vSvr[pStatcs->mIdx]->mPreNum--;
				}
				pStatcs->mSumConn++;
				vSvr[pStatcs->mIdx]->mUsedNum++;
				
				*pBuffer = (struct SvrNet_t) *vSvr[pStatcs->mIdx];	//Svr_t => SvrNet_t
				return vSvr[pStatcs->mIdx]->mId;
			}
		}
	}
	return -1;
}

//接受上报数据
void SvrQos::ReportSvr(SvrReportReqId_t *pReportSvr)
{
	vector<Svr_t*>::iterator it = GetItById(pReportSvr->mId);
	if (it != mSvr.end() && pReportSvr->mDelay != 0 && pReportSvr->mStu != SVR_UNKNOWN)
	{
		SetGXDirty((*it), 1);	//设置同组所有Svr更新位(重新设置对应权值、错误率)
		(*it)->mDelay = pReportSvr->mDelay;
		if (pReportSvr->mStu == SVR_SUC)	//成功
		{
			//(*it)->mOkNum++;
		}
		else if(pReportSvr->mStu == SVR_ERR)	//失败
		{
			//(*it)->mErrNum++;
		}

		//释放连接
		ReleaseConn(*it);
		LOG_DEBUG(ELOG_KEY,"[runtime] recvive a report message(shm), Ok(%d),Err(%d)", (*it)->mOkNum, (*it)->mErrNum);
	}
}

void SvrQos::Statistics()
{
	if(mSvr.size() <= 0) return;

	int iSunConn = -1;
	vector<Svr_t*>::iterator it;
	for (it = mSvr.begin(); it != mSvr.end(); it++)
	{
		if ((*it)->mDirty == 1)
		{
			iSunConn = GetSumConn(*it);
			(*it)->mOkRate = (float)(*it)->mOkNum / iSunConn;
			(*it)->mErrRate = (float)(*it)->mErrNum / iSunConn;
			(*it)->mRfuRate = (float)(*it)->mRfuNum / iSunConn;

			(*it)->mPreNum = CalcPre(*it);	//每一周期预取数重置为1 （后期实现扩张、收缩）
			(*it)->mShutdown = CalcShutdown(*it);	//非压力故障
			(*it)->mWeight = CalcWeight(*it);
			
			(*it)->ClearStatistics();	//清除统计数据
		}
	}
	SvrRebuild();
	LOG_DEBUG(ELOG_KEY,"[runtime] statistics svr success");
}

short SvrQos::CalcPre(Svr_t* stSvr)
{
	return 1;
}

short SvrQos::CalcShutdown(Svr_t* stSvr)
{
	return 0;
}

//计算动态权重
int SvrQos::CalcWeight(Svr_t* stSvr)
{
	if (stSvr->mDelay == 0 && stSvr->mOkRate == 0)	//从未访问过
	{
		return stSvr->mWeight;
	}

	Svr_t pSvr[MAX_SVR_NUM];
	int iNum = GetAllSvrByGXid(pSvr, stSvr->mGid, stSvr->mXid);
	if (iNum <= 0)
	{
		return stSvr->mWeight;	//讲道理的话，这里不可能执行到（至少有其自身）
	}

	int iDelay = stSvr->mDelay;
	float fOkRate = stSvr->mOkRate;
	for (int i = 0; i < iNum; ++i)
	{
		if (iDelay > pSvr[i].mDelay && pSvr[i].mDelay != 0)
		{
			iDelay = pSvr[i].mDelay;	//最小延时
		}
		if (fOkRate < pSvr[i].mOkRate && pSvr[i].mOkRate != 0)
		{
			fOkRate = pSvr[i].mOkRate;	//最大成功率
		}
	}

	float fLoadWeight = 1/(((float)stSvr->mDelay/iDelay) * (fOkRate/stSvr->mOkRate));
	return (int)(fLoadWeight * ZOOM_WEIGHT * stSvr->mSWeight);	//放大
}

//检测是否过载
//TODO
short SvrQos::CalcOverLoad(Svr_t* stSvr)
{
	if (stSvr->mErrNum > 0 && stSvr->mPreNum <= 0)	//本周期
	{
		stSvr->mOverLoad = 1;	//过载
	}
	else if(stSvr->mErrRate > ERR_RATE_THRESHOLD)	//上一周期错误率
	{
		stSvr->mOverLoad = 1; //过载(存在故障可能性)
		//stSvr->mShutdown = 1;
	}
	else
	{
		stSvr->mOverLoad = 0;
	}
	return stSvr->mOverLoad;
}

//设置同组所有svr更新位
void SvrQos::SetGXDirty(Svr_t* stSvr, int iDirty)
{
	Svr_t pSvr[MAX_SVR_NUM];
	int iNum = GetAllSvrByGXid(pSvr, stSvr->mGid, stSvr->mXid);
	if (iNum <= 0)
	{
		return;	//讲道理的话，这里不可能执行到（至少有其自身）
	}
	for (int i = 0; i < iNum; ++i)
	{
		 pSvr[i].mDirty = iDirty;
	}
}

vector<Svr_t*>::iterator SvrQos::GetItFromV(Svr_t* pSvr)
{
	vector<Svr_t*>::iterator it;
	for (it = mSvr.begin(); it != mSvr.end(); it++)
	{
		if (**it == *pSvr)
		{
			break;
		}
	}
	return it;
}

vector<Svr_t*>::iterator SvrQos::GetItById(int iId)
{
	vector<Svr_t*>::iterator it;
	for (it = mSvr.begin(); it != mSvr.end(); it++)
	{
		if ((*it)->mId == iId)
		{
			break;
		}
	}
	return it;
}

bool SvrQos::IsChangeSvr(const SvrNet_t* pR1, const SvrNet_t* pR2)
{
	if (pR1->mVersion!=pR2->mVersion || pR1->mGid!=pR2->mGid || pR1->mXid!=pR2->mXid || pR1->mSWeight!=pR2->mSWeight || pR1->mPort!=pR2->mPort || strncmp(pR1->mIp,pR2->mIp,MAX_SVR_IP_LEN)!=0)
	{
		return true;
	}
	return false;
}

void SvrQos::DelContainer()
{
	mSvrByGXid.clear();
	mStatcsGXid.clear();
}

void SvrQos::Final()
{
	DelContainer();
	for(vector<Svr_t*>::iterator it = mSvr.begin(); it != mSvr.end(); it++)
	{
		SAFE_DELETE(*it);
	}
	for(map<string, StatcsGXid_t*>::iterator it = mStatcsGXid.begin(); it != mStatcsGXid.end(); it++)
	{
		SAFE_DELETE(it->second);
	}
	mSvr.clear();
}

//整理容器 & 同步其他容器
void SvrQos::SvrRebuild()
{
	if(mSvr.size() <= 0) return;
	sort(mSvr.begin(), mSvr.end(), GreaterSvr);	//降序排序

	DelContainer();
	string sGid, sXid, sGXid;
	vector<Svr_t*> vSvr;
	for(vector<Svr_t*>::iterator it = mSvr.begin(); it != mSvr.end(); it++)
	{
		sGid = Itos((*it)->mGid); 
		sXid = Itos((*it)->mXid);
		sGXid = sGid + "-" + sXid;

		//mSvrByGXid
		map<string, vector<Svr_t*> >::iterator mg = mSvrByGXid.find(sGXid);
		vSvr.clear();
		if(mg != mSvrByGXid.end())
		{
			vSvr = mg->second;
			mSvrByGXid.erase(mg);
		}
		vSvr.push_back(*it);
		mSvrByGXid.insert(pair<string, vector<Svr_t*> >(sGXid, vSvr));		
	}

	StatcsGXid_t *pStatcsGXid = 0;
	map<string, vector<Svr_t*> >::iterator it = mSvrByGXid.begin();
	for(; it != mSvrByGXid.end(); it++)
	{
		map<string, StatcsGXid_t*>::iterator mgx = mStatcsGXid.find(it->first);
		if(mgx == mStatcsGXid.end())
		{
			pStatcsGXid = new StatcsGXid_t();
		}
		else
		{
			pStatcsGXid = mgx->second;
			//mStatcsGXid.erase(mgx);
		}
		pStatcsGXid->mGcd = GcdWeight(it->second, it->second.size());
		mStatcsGXid.insert(pair<string, StatcsGXid_t*>(it->first, pStatcsGXid));
	}
}

void SvrQos::ReleaseConn(Svr_t* stSvr)
{
	string sGid, sXid, sGXid;
	sGid = Itos(stSvr->mGid); 
	sXid = Itos(stSvr->mXid);
	sGXid = sGid + "-" + sXid;
	map<string, StatcsGXid_t*>::iterator mgx = mStatcsGXid.find(sGXid);
	StatcsGXid_t *pStatcsGXid = 0;
	if(mgx == mStatcsGXid.end())
	{
		pStatcsGXid = mgx->second;
		pStatcsGXid->mSumConn--;
	}
}

int SvrQos::GetSumConn(Svr_t* stSvr)
{
	string sGid, sXid, sGXid;
	sGid = Itos(stSvr->mGid); 
	sXid = Itos(stSvr->mXid);
	sGXid = sGid + "-" + sXid;

	map<string, StatcsGXid_t*>::iterator mgx = mStatcsGXid.find(sGXid);
	StatcsGXid_t *pStatcsGXid = 0;
	if(mgx == mStatcsGXid.end())
	{
		pStatcsGXid = mgx->second;
		return pStatcsGXid->mSumConn;
	}
	return -1;
}

int SvrQos::GcdWeight(vector<Svr_t*> vSvr, int n)
{
	if (n == 1) 
	{
		return vSvr[0]->mWeight;
	}
	return Gcd(vSvr[n-1]->mWeight, GcdWeight(vSvr, n - 1));
}