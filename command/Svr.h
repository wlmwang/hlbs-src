
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

#define SVR_TM_MAX 20

enum QOS_RTN
{
	QOS_OVERLOAD	= -10000,	//过载
	QOS_TIMEOUT     = -9999,    //超时
	QOS_SYSERR      = -9998    //系统错误
};

/*
enum ACCESS_STATUS
{
	SVR_UNKNOWN = -1,
	SVR_ERR,
	SVR_SUC,	
};
*/

#pragma pack(1)

/*Svr 基础属性 && 通信结构*/
struct SvrNet_t
{
	int		mGid;
	int		mXid;
	int 	mWeight;	//静态权重，0为禁用此Svr
	short	mVersion;
	int		mPort;
	char	mHost[MAX_SVR_HOST];

    int 	mPre;		//预取数
    int 	mExpired;	//过期时间

	SvrNet_t()
	{
		mGid = 0;
		mXid = 0;
		mWeight = INIT_WEIGHT;
		mPort = 0;
		mVersion = 0;
		mPre = 0;
		mExpired = 0;
		memset(mHost, 0, MAX_SVR_HOST);
	}

	SvrNet_t(const SvrNet_t &stSvr)
	{
		mGid = stSvr.mGid;
		mXid = stSvr.mXid;
		mWeight = stSvr.mWeight;
		mPort = stSvr.mPort;
		mVersion = stSvr.mVersion;
		mPre = stSvr.mPre;
		mExpired = stSvr.mExpired;
		memcpy(mHost, stSvr.mHost, MAX_SVR_HOST);
	}

	SvrNet_t& operator=(const SvrNet_t &stSvr)
	{
		mGid = stSvr.mGid;
		mXid = stSvr.mXid;
		mWeight = stSvr.mWeight;
		mPort = stSvr.mPort;
		mVersion = stSvr.mVersion;
		mPre = stSvr.mPre;
		mExpired = stSvr.mExpired;
		memcpy(mHost, stSvr.mHost, MAX_SVR_HOST);
		return *this;
	}

	/** 忽略 weight version 比较、排序*/
    bool operator==(SvrNet_t const &other) const
    {
        if (mGid != other.mGid)
        {
            return false;
        }
        else if(mXid != other.mXid)
        {
            return false;
        }
        else if(strcmp(mHost,other.mHost))
        {
            return false;
        }
        else if (mPort != other.mPort)
        {
            return false;
        }
        return true;
    }

	bool operator<(const SvrNet_t &stSvr) const
	{
        if (mGid < stSvr.mGid )
        {
            return true;
        }
        else if(mGid > stSvr.mGid)
        {
        	return false;
        }
        else if(mXid < stSvr.mXid)
        {
            return true;
        }
        else if(mXid > stSvr.mXid)
        {
            return false;
        }
        else if(mPort < stSvr.mPort)
        {
            return true;
        }
        else if(mPort > stSvr.mPort)
        {
            return false;
        }
        else
        {
            int cmp = strcmp(mHost, stSvr.mHost);
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

struct SvrCaller_t
{
	int 	mCallerGid;
	int     mCallerXid;
	int		mCalledGid;			//被调模块编码
	int		mCalledXid;			//被调接口编码
	char	mHost[MAX_SVR_HOST];//被调主机IP
	unsigned short mPort;		//被调主机PORT
	
	int mReqRet;				//请求结果。 >=0 成功
	int mReqCount;				//请求次数
	long long mReqUsetimeUsec;	//微妙
	int mTid;					//进程id（为实现）

	SvrCaller_t()
	{
		mCallerGid = 0;
		mCallerXid = 0;
		mCalledGid = 0;
		mCalledXid = 0;
		mPort = 0;
		memset(mHost, 0, MAX_SVR_HOST);
	}

    bool operator==(SvrCaller_t const &other) const
    {
        if(mCallerGid != other.mCallerGid)
        {
            return false;
        }
        else if(mCallerXid != other.mCallerXid)
        {
            return false;
        }
        if (mCalledGid != other.mCalledGid)
        {
            return false;
        }
        else if(mCalledXid != other.mCalledXid)
        {
            return false;
        }
        else if(strcmp(mHost, other.mHost))
        {
            return false;
        }
        else if(mPort != other.mPort)
        {
            return false;
        }
        return true;
    }

};

#pragma pack()

//访问量的配置信息
struct SvrReqCfg_t
{
	int			mReqLimit;			//访问量控制的阀值
	int			mReqMax;			//访问量控制的最大值
	int			mReqMin;			//访问量控制的最小值
	int			mReqCount;			//访问量控制的实际值（请求数）
	float		mReqErrMin;			//错误的最小阀值 0-1 [小于则服务无错，应增大访问量。大于则服务过载，应减少访问量。]
	float		mReqExtendRate;		//无错误的时候的访问量阀值扩张率 0.001-101
	int         mRebuildTm;        	//统计的周期 60s
	int 		mPreTime;			//4(不能大于route重建时间的一半) 可以设定预取时间长度N秒内的路由计数，N小于当前周期的1/2
	SvrReqCfg_t()
	{
		mReqLimit = 0;
		mReqMax = 0;
		mReqMin = 0;
		mReqCount = 0;
		mReqErrMin = 0.0;
		mReqExtendRate = 0.0;
		mRebuildTm = 3;
		mPreTime = 0;
	};
};

//并发量的配置信息
struct SvrListCfg_t
{
	int			mListLimit;		//并发量控制的阀值
	int			mListMax;		//并发量控制的最大值 400
	int			mListMin;		//并发量控制的最小值 10
	int			mListCount;		//并发量控制的实际值
	float		mListErrMin;	//并发的最小阀值[小于这个值认为是无错] 0.5
	float		mListExtendRate;//并发无错误的时候的阀值扩张率 0.2
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

//宕机检测和探测的相关配置
struct SvrDownCfg_t
{
	int mReqCountTrigerProbe;   //100000
	int mDownTimeTrigerProbe;   //600
	int mProbeTimes;            //(3,~)
	int mPossibleDownErrReq;    //10	连续错误阈值
	float mPossbileDownErrRate; //0.5   (0.01,1)	宕机错误率阈值

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

/** 时间段配置 */
struct SvrTM
{
	int mCfgCount;					//配置的个数
	int mBeginUsec[SVR_TM_MAX];		//返回时间段开始时间
	int mEndUsec[SVR_TM_MAX];		//返回时间段结束时间
	int mRet[SVR_TM_MAX];			//返回值[0:成功 -1:失败]

	SvrTM(const SvrTM & stm);
};

/** 访问统计 */
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
	float 		mAvgErrRate;		//平均错误率，统计多个周期的错误率并加以平均得到（未超过最低阈值(mReqCfg.mReqErrMin)，始终为0）

	/** 上一周期统计数据 */
	int 		mLastReqAll;
	int 		mLastReqRej;
	int 		mLastReqErrRet;
	int 		mLastReqErrTm;
	int  		mLastReqSuc;
	bool  		mLastErr;			//门限扩张标识 true：收缩  false：扩张
	int 		mLastAlarmReq;		//参考值。请求数扩张门限（上一周期数量），判断扩展是否有效
	int 		mLastAlarmSucReq;	//参考值。成功请求数扩张门限
	int 		mPreAll;			//mPreAll*mLoadX，作为WRR的标准  路由已分配数

	int 		mCityId;	//被调所属城市id
	int 		mOffSide;	//被调节点与主调异地标志，默认为0， 1标为异地

	int 		mContErrCount;		//连续失败次数累积

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
		mContErrCount = 0;

		mSReqAll = 0;
		mSReqRej = 0;
		mSReqSuc = 0;
		mSReqErrRet = 0;
		mSReqRrrTm  = 0;
        mSPreAll = 0;
    }

    void InitInfo(struct SvrNet_t& stSvr)
    {
    	//...
    }
};

/**
 * svr阈值（便于自行自行扩张收缩）、统计结构
 */
struct SvrStat_t
{
	int 			mType;
	SvrReqCfg_t		mReqCfg;	//访问量配置
	SvrListCfg_t	mListCfg;	//并发量配置
	SvrInfo_t		mInfo;		//统计信息

	//mlist并发量
	//mreq按各个时间段统计信息
	
	void Reset()
	{
		mInfo.mReqAll = 0;
		mInfo.mReqSuc = 0;
		mInfo.mReqRej = 0;
		mInfo.mReqErrRet = 0;
		mInfo.mReqErrTm = 0;
		mInfo.mTotalUsec = 0;
		mInfo.mPreAll = 0;
		mInfo.mContErrCount = 0;
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

/**
 * 路由分类
 * 由mGid、mXid组成的一类svr（便于分类路由）
 */
struct SvrKind_t
{
	int		mGid;
	int		mXid;
	int 	mOverload;
    float 	mPtotalErrRate;		//累计连续过载，所有路由错误率平均值总和
    int 	mPsubCycCount;		//累计连续过载次数

    int 	mPtm; 			//rebuild 时刻的绝对时间 time_t
    int 	mRebuildTm; 	//rebuild 的时间间隔
	float 	mWeightSum;
	int64_t mAccess64tm;	//最近访问时间 微妙

	int mPindex;
	
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
        mAccess64tm = GetTimeofday();
        mPindex = 0;
	}
	
	SvrKind_t(const SvrKind_t& stKind)
	{
		mGid = stKind.mGid;
		mXid = stKind.mXid;
        mPtotalErrRate = stKind.mPtotalErrRate;
        mPsubCycCount = stKind.mPsubCycCount;
        mRebuildTm = stKind.mRebuildTm;
        mWeightSum = stKind.mWeightSum;
        mOverload = stKind.mOverload;
        mPtm = stKind.mPtm;
        mAccess64tm = stKind.mAccess64tm;
        mPindex = stKind.mPindex;
	}

	SvrKind_t(const SvrNet_t& stNode)
	{
		mGid = stNode.mGid;
		mXid = stNode.mXid;
        mPtotalErrRate = 0;
        mPsubCycCount = 0;
        mRebuildTm = 3;
        mWeightSum = 0.0f;
        mOverload = 0;
        mPtm = time(NULL);
        mAccess64tm = GetTimeofday();
        mPindex = 0;
	}

	SvrKind_t& operator=(const SvrKind_t& stKind)
	{
		mGid = stKind.mGid;
		mXid = stKind.mXid;
        mPtotalErrRate = stKind.mPtotalErrRate;
        mPsubCycCount = stKind.mPsubCycCount;
        mRebuildTm = stKind.mRebuildTm;
        mWeightSum = stKind.mWeightSum;
        mOverload = stKind.mOverload;
        mPtm = stKind.mPtm;
        mAccess64tm = stKind.mAccess64tm;
        mPindex = stKind.mPindex;
        return *this;
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

    bool operator==(SvrKind_t const &other) const
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

/**
 * Svr节点信息
 * 路由信息、各种阈值及统计信息
 */
struct SvrNode_t
{
	SvrNet_t mNet;
	SvrStat_t *mStat;

	float mKey;				//关键值，初始化为 mInfo.mLoadX = 1

	int mStopTime;			//宕机记录信息
	int mReqAllAfterDown;	//宕机以来所有请求数量
    
    /** 宕机相关的额外恢复条件 */
    int mDownTimeTrigerProbeEx;		//时间
    int mReqCountTrigerProbeEx;		//请求数量
    
    bool mIsDetecting; 		//是否处在 "探测宕机是否恢复" 的状态  
    //int mHasDumpStatistic;//是否备份

	SvrNode_t(const struct SvrNet_t& nt, struct SvrStat_t* pStat)
	{
        if (pStat == NULL)
        {
            mKey = 1;
        }
        else
        {
            mKey = pStat->mInfo.mLoadX;
        }
        mNet = nt;
        mStat = pStat;
		mStopTime = 0;
		mReqAllAfterDown = 0;
		mIsDetecting = false;
		mDownTimeTrigerProbeEx = 0;
		mReqCountTrigerProbeEx = 0;
		//mHasDumpStatistic = 0;
	}
};

/**
 * sort用到（降序）
 */
inline bool GreaterSvr(const struct SvrNet_t* pR1, const struct SvrNet_t* pR2)
{
	return   pR1->mWeight > pR2->mWeight;
}

#endif
