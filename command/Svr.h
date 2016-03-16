
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _SVR_H_
#define _SVR_H_

#include "wCore.h"

#define REPORT_TIME_TICK 3000	//3s重建时间svr
#define MAX_SVR_HOST 255
#define MAX_SVR_NUM 255			//每次请求svr最多个数

#define INIT_WEIGHT     100      //默认权重值 
#define MAX_WEIGHT      1000     //最大权重值
#define DELAY_MAX 100000000		//最大延时值 100s

enum RET_STATUS
{
	SVR_UNKNOWN = -1,
	SVR_ERR,
	SVR_SUC,	
};

//访问量的配置信息
struct SvrReqCfg_t
{
	int			mReqLimit;			//访问量控制的阀值
	int			mReqMax;			//访问量控制的最大值
	int			mReqMin;			//访问量控制的最小值
	int			mReqCount;			//访问量控制的实际值
	float		mReqErrMin;			//错误的最小阀值[小于这个值认为是无错]    <1
	float		mReqExtendRate;		//无错误的时候的阀值扩张率    (0.001,101)
	int         RebuildTm;        	//配置的 rebuild 的时间间隔
	SvrCfg_t()
	{
		mReqLimit = 0;
		mReqMax = 0;
		mReqMin = 0;
		mReqCount = 0;
		mReqErrMin = 0.0;
		mReqExtendRate = 0.0;
		RebuildTm = 3;
	};
};

//并发量的配置信息
struct SvrListCfg_t
{
	int			mListLimit;		//并发量控制的阀值
	int			mListMax;		//并发量控制的最大值
	int			mListMin;		//并发量控制的最小值
	int			mListCount;		//并发量控制的实际值
	float		mListErrMin;	//并发的最小阀值[小于这个值认为是无错]
	float		mListExtendRate;//并发无错误的时候的阀值扩张率
	SvrListCfg_t()
	{
		mListLimit = 0;
		mListMax = 0;
		mListMin = 0;
		mListCount = 0;
		mListErrMin = 0.0;
		mListExtendRate = 0.0;
	};
};

//当机配置
struct SvrDownCfg_t
{
	int mReqCountTrigerProbe;   //100000
	int mDownTimeTrigerProbe;   //600
	int mProbeTimes;            //(3,~)
	int mPossibleDownErrReq;    //10
	float mPossbileDownErrRate; //0.5   (0.01,1)

    //mProbeBegin >0 才打开自探测
    int mProbeBegin;    //0
    int mProbeInterval; //3~10
    int mProbeNodeExpireTime;//3~600

	SvrDownCfg_t()
    {
        mReqCountTrigerProbe = 0;
        mDownTimeTrigerProbe = 0;
        mProbeTimes = 0;
        mPossibleDownErrReq = 0;
        mPossbileDownErrRate = 0.0f;

        mProbeBegin = 0;
        mProbeInterval = 0;
        mProbeNodeExpireTime = 0;
    }
};


struct SvrInfo_t
{
	struct timeval mBuildTm;		//统计信息开始时间, 每个节点 rebuild 时刻的绝对时间
	int			mReqAll;			//总的请求数
	int			mReqRej;			//被拒绝的请求数
	int			mReqSuc;			//成功的请求数
	int			mReqErrRet;			//失败的请求数
	int			mReqErrTm;			//超时的请求数

	float       mLoadX;				//负载总乘数 (mOkLoad*mDelayLoad)*(max<mWeight>)/mWeight  系数
	float       mOkLoad;			//成功率乘数
	float       mDelayLoad;			//时延乘数
    float       mDelayLoadAmplify;	//时延放大
	float       mOkRate;			//上一周期成功率 0-1
	unsigned int mAvgTm;			//上一周期成功请求平均时延，微秒  mTotalUsec/mReqSuc
	long long   mTotalUsec; 		//请求总微秒数
	float 		mAvgErrRate;		//平均错误率

	/** 上一周期统计数据 */
	int 		mLastReqAll;
	int 		mLastReqRej;
	int 		mLastReqErrRet;
	int 		mLastReqErrTm;
	int  		mLastReqSuc;
	bool  		mLastErr;
	int 		mLastAlarmReq;
	int 		mLastAlarmSucReq;
	int 		mPreAll;

	int 		mCityId;	//被调所属城市id
	int 		mOffSide;	//被调节点与主调异地标志，默认为0， 1标为异地
	int mContinuousErrCount;//连续失败次数累积

	int			mSReqAll;			//总的请求数(统计用)
	int			mSReqRej;			//被拒绝的请求数(统计用)
	int			mSReqSuc;			//成功的请求数(统计用)
	int			mSReqErrRet;		//失败的请求数(统计用)
	int			mSReqRrrTm;			//超时的请求数(统计用)
    int         mSPreAll;

    SvrInfo_t()
    {
		mBuildTm.tv_sec = mBuildTm.tv_usec = 0;
		mReqAll = 0;
		mReqRej = 0;
		mReqSuc = 0;
		mReqErrRet = 0;
		mReqErrTm	 = 0;
		mOkRate = 1;
		mOkLoad = 1;
		mDelayLoad = 1;
		mDelayLoadAmplify = 0;
		mLoadX = 1;
		mAvgTm = 1;
		mTotalUsec = 1;
		mLastReqAll = 0;
		mLastReqRej = 0;
		mLastReqErrRet = 0;
		mLastReqErrTm = 0;
		mLastReqSuc = 0;

		mLastErr = false;
		mLastAlarmReq = 0;
		mLastAlarmSucReq = 0;
		mPreAll = 0;
		mContinuousErrCount = 0;

		mSReqAll = 0;
		mSReqRej = 0;
		mSReqSuc = 0;
		mSReqErrRet = 0;
		mSReqRrrTm  = 0;
        mSPreAll = 0;
    }

    void InitInfo(struct SvrNet_t& stSvr)
    {

    }
};

//svr统计结构
struct SvrStat_t
{
	SvrReqCfg_t		mReqCfg;	//访问量配置
	SvrListCfg_t	mListCfg;	//并发量配置
	SvrInfo_t		mInfo;		//统计信息

	SvrStat_t() {}
	
	void Reset()
	{
		mInfo.mReqAll = 0;
		mInfo.mReqSuc = 0;
		mInfo.mReqRej = 0;
		mInfo.mReqErrRet = 0;
		mInfo.mReqErrTm = 0;
		mInfo.mTotalUsec = 0;
		mInfo.mPreAll = 0;
		mInfo.mContinuousErrCount = 0;
	}

	void AddStatistic()
	{
		mInfo.mSReqAll += mInfo.mReqAll;
		mInfo.mSReqSuc += mInfo.mReqSuc;
		mInfo.mSReqRej += mInfo.mReqRej;
		mInfo.mSReqErrRet += mInfo.mReqErrRet;
		mInfo.mLastReqErrTm += mInfo.mReqErrTm;
		mInfo.mSPreAll += mInfo.mPreAll;
	}

	void ResetStatistic()
	{
		mInfo.mSReqAll = 0;
		mInfo.mSReqSuc = 0;
		mInfo.mSReqRej = 0;
		mInfo.mSReqErrRet = 0;
		mInfo.mLastReqErrTm = 0;
		mInfo.mSPreAll = 0;
	}
};

//由mGid、mXid组成的一类svr
struct SvrKind_t
{
	int		mGid;
	int		mXid;
	int 	mOverload;
    float 	mPtotalErrRate;  //错误率总和
    int 	mPsubCycCount;
    int 	mPtm; 			//rebuild 时刻的绝对时间
    int 	mRebuildTm; 	//rebuild 的时间间隔
	float 	mWeightSum;

	int mIdx;	//当前分配到索引号
	int mCur;	//当前weight。初始化为最大值
	int mGcd;	//weight最大公约数
	
	SvrKind_t()
	{
		mGid = 0;
		mXid = 0;
        mPtotalErrRate = 0;
        mPsubCycCount = 0;
        mRebuildTm = 3;
        mWeightSum = 0.0f;
        mOverload = 0;
        mPtm = time(NULL);

		mIdx = -1;
		mCur = 0;
		mGcd = 0;
		mSumConn = 0;
	}

	SvrKind_t(const SvrNet_t& node)
	{
		mGid = node.mGid;
		mXid = node.mXid;
        mPtotalErrRate = 0;
        mPsubCycCount = 0;
        mRebuildTm = 3;
        mWeightSum = 0.0f;
        mOverload = 0;
        mPtm = time(NULL);

		mIdx = -1;
		mCur = 0;
		mGcd = 0;
		mSumConn = 0;
	}

    bool operator<(SvrKind_t const &other) const
    {
        if (mGid < other.mGid)
        {
            return true;
        }
        else if(mGid > other.mGid)
        {
            return false;
        }
        else if(mXid < other.mXid)
        {
            return true;
        }
        else if (mXid > other.mXid)
        {
            return false;
        }
        return false;
    }

    bool operator==(route_kind const &other) const
    {
        if (mGid != other.mGid)
        {
            return false;
        }
        else if(mXid != other.mXid)
        {
            return false;
        }
        return true;
    }
};

//svr节点信息
struct SvrNode_t
{
	SvrNet_t mNet;
	SvrStat_t *mStat;

	//关键值，初始化为 mInfo.mLoadX=1
	float mKey;
	
	//当机记录信息
	int mStopTime;
	int mReqAllAfterDown;	//当机以来所有请求数量

	SvrNode_t(const SvrNet_t& nt, const SvrStat_t* stat)
	{
        if (!stat)
        {
            mKey = 1;
        }
        else
        {
            mKey = stat->mInfo.mLoadX;
        }
        mNet = nt;
        mStat = stat;
		mStopTime = 0;
		mReqAllAfterDown = 0;
	}
};


#pragma pack(1)

/**
 * mWeight ：静态权重，动态权重比例的系数
 * 不要配太高(0~10)，否则会弱化Svr动态负载均衡效果，能表示各个Svr处理能力即可。0为禁用此Svr。
 */
/*Svr 基础属性 && 通信结构*/
struct SvrNet_t
{
	int		mGid;
	int		mXid;
	int 	mWeight;
	short	mVersion;
	int		mPort;
	char	mHost[MAX_SVR_HOST];

	SvrNet_t()
	{
		mGid = 0;
		mXid = 0;
		mWeight = INIT_WEIGHT;
		mPort = 0;
		mVersion = 0;
		memset(mHost, 0, MAX_SVR_HOST);
	}

	SvrNet_t(const SvrNet_t &svr)
	{
		mGid = svr.mGid;
		mXid = svr.mXid;
		mWeight = svr.mWeight;
		mPort = svr.mPort;
		mVersion = svr.mVersion;
		memcpy(mHost, svr.mHost, MAX_SVR_HOST);
	}

	SvrNet_t& operator=(const SvrNet_t &svr)
	{
		mGid = svr.mGid;
		mXid = svr.mXid;
		mWeight = svr.mWeight;
		mPort = svr.mPort;
		mVersion = svr.mVersion;
		memcpy(mHost, svr.mHost, MAX_SVR_HOST);
		return *this;
	}
	
	bool operator<(const SvrNet_t &other) const
	{
        if (mGid < other.mGid )
        {
            return true;
        }
        else if(mGid > other.mGid)
        {
        	return false;
        }
        else if(mXid < other.mXid)
        {
            return true;
        }
        else if(mXid > other.mXid)
        {
            return false;
        }
        else if(mPort < other.mPort)
        {
            return true;
        }
        else if(mPort > other.mPort)
        {
            return false;
        }
        else
        {
            int cmp = strcmp(mHost, other.mHost);
            if(cmp < 0)
        	{
				return true;
        	}
            else if(cmp > 0)
            {
            	return false;
            }
        }
        return false;
	}
};

#pragma pack()

/*
inline bool GreaterSvr(const Svr_t* pR1, const Svr_t* pR2)
{
	return   pR1->mWeight > pR2->mWeight;
}

inline bool LessSvr(const Svr_t* pR1, const Svr_t* pR2)
{
	return   pR1->mWeight < pR2->mWeight;
}
*/

#endif
