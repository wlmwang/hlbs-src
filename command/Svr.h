
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _SVR_H_
#define _SVR_H_

#include "wCore.h"

#define MAX_SVR_IP_LEN 255
#define MAX_SVR_NUM 255
#define REPORT_TIME_TICK 3000		//3s 重建时间svr

#define ZOOM_WEIGHT 100
#define ERR_RATE_THRESHOLD 0.5

#define INIT_WEIGHT     100      //默认权重值 
#define MAX_WEIGHT      1000     //最大权重值

#define DELAY_MAX 100000000	//最大延时值 100s

enum SVR_STATUS
{
	SVR_UNKNOWN = -1,
	SVR_ERR,
	SVR_SUC,	
};

enum STAT_ID
{
	STAT_REQ_ALL,		//总的请求量
	STAT_REQ_SUC,		//成功数
	STAT_REQ_REJ,		//过载数
	STAT_REQ_ERR,		//失败数
	STAT_REQ_ERRTM,		//超时失败数
	STAT_OVER
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

	//int 		mPreTime;          	//2(不能大于route重建时间的一半)
	//int 		mBatchTime;        	//批量获取被调时，每次取多长时间所需的被调
	
	SvrCfg_t()
	{
		mReqLimit = 0;
		mReqMax = 0;
		mReqMin = 0;
		mReqCount = 0;
		mReqErrMin = 0.0;
		mReqExtendRate = 0.0;
		RebuildTm = 3;
		//mPreTime=0;
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

/**
 * 定义Svr_t
 */
#pragma pack(1)

/*Svr 基础属性 && 通信结构*/

/**
 * mSWeight ：静态权重，类似动态权重比例的系数。
 * 不要配太高(0~10)，否则会弱化Svr动态负载均衡效果，能表示各个Svr处理能力即可。0为禁用此Svr。
 * 建议设为1，让动态权重感知各Svr性能
 */
struct SvrNet_t
{
	int		mId;
	int		mGid;
	int		mXid;
	int		mSWeight;	//静态权重
	int 	mWeight;	//权重
	int		mPort;
	char	mIp[MAX_SVR_IP_LEN];
	short	mVersion;
	int 	mDelay;		//延时信息
	//int 	mPre;
	//int 	mExpired;
	
	SvrNet_t()
	{
		mId = 0;
		mGid = 0;
		mXid = 0;
		mSWeight = 1;
		mWeight = INIT_WEIGHT;
		mPort = 0;
		mVersion = 0;
		memset(mIp, 0, MAX_SVR_IP_LEN);
	}

	SvrNet_t(const SvrNet_t &svr)
	{
		mId = svr.mId;
		mGid = svr.mGid;
		mXid = svr.mXid;
		mSWeight = svr.mSWeight;
		mWeight = svr.mWeight;
		mPort = svr.mPort;
		mVersion = svr.mVersion;
		memcpy(mIp, svr.mIp, MAX_SVR_IP_LEN);
	}

	SvrNet_t& operator=(const SvrNet_t &svr)
	{
		mId = svr.mId;
		mGid = svr.mGid;
		mXid = svr.mXid;
		mSWeight = svr.mSWeight;
		mWeight = svr.mWeight;
		mPort = svr.mPort;
		mVersion = svr.mVersion;
		memcpy(mIp, svr.mIp, MAX_SVR_IP_LEN);
		return *this;
	}
};

#pragma pack()

/** svr 统计数据结构 */
struct Svr_t : public SvrNet_t
{
	/*
	long	mUsedNum;	//已被分配数量（返回给某CGI次数），0未知
	int		mWeight;	//动态权重值(经放大因子 动态计算 时延率、成功率得出)
	short	mPreNum;	//预取数，默认为1
	short 	mShutdown;	//非压力故障造成当机，需人工介入修复

	//当前(5min)周期数据统计
	short	mDirty;		//是否要更新本svr（本周期有上报数据）
	short	mOverLoad;	//是否过载
	long	mOkNum;		//成功数量，0未知
	long	mErrNum;	//失败数量，
	long	mRfuNum;	//拒绝数
	int		mDelay;		//延时时间。0未知，>0具体延时（只保留周期内最小延时）

	//统计结果：每周期结束(5min)，由上周期统计数据计算得出
	float	mOkRate;	//成功率 0-1，0未知
	float	mErrRate;	//失败率 0-1
	float	mRfuRate;	//拒绝率率 0-1
	void Initialize()
	{
		mOkRate = 0.0;
		mErrRate = 0.0;
		mRfuRate = 0.0;
		mDirty = 0;
		mOkNum = 0;
		mErrNum = 0;
		mRfuNum = 0;
		mDelay = 0;
		mOverLoad = 0;
		mUsedNum = 0;
		mShutdown = 0;
		mPreNum = 1;
		mWeight = ZOOM_WEIGHT * mSWeight;	//静态权重 * 放大系数
	}
	*/

	//gettimeofday(&mBuildTm,NULL);
	struct timeval mBuildTm;		//统计信息开始时间, 每个节点 rebuild 时刻的绝对时间
	int			mReqAll;			//总的请求数
	int			mReqRej;			//被拒绝的请求数
	int			mReqSuc;			//成功的请求数
	int			mReqErrRet;			//失败的请求数
	int			mReqErrTm;			//超时的请求数

	float       mLoadX;				//负载总乘数，权重的倒数
	float       mOkLoad;			//成功率乘数
	float       mDelayLoad;			//时延乘数
    float       mDelayLoadAmplify;	//时延放大

	float       mOkRate;			//上一周期成功率
	unsigned int mAvgTm;			//上一周期成功请求平均时延，微秒  mTotalUsec/mReqSuc

	long long   mTotalUsec; 		//请求总微秒数
	float 		mAvgErrRate;

	int 		mLastReqAll;
	int 		mLastReqRej;
	int 		mLastReqErrRet;
	int 		mLastReqErrTm;
	int  		mLastReqSuc;
	bool  		mLastErr;
	int 		mLastAlarmReq;
	int 		mLastAlarmSucReq;
	int 		mPreAll;

	int mContinuousErrCount; //连续失败次数累积

	int			mSReqAll;			//总的请求数(统计用)
	int			mSReqRej;			//被拒绝的请求数(统计用)
	int			mSReqSuc;			//成功的请求数(统计用)
	int			mSReqErrRet;		//失败的请求数(统计用)
	int			mSReqRrrTm;			//超时的请求数(统计用)

    int         mSPreAll;

	/*
	void ClearStatistics()
	{
		mDirty = 0;
		mOkNum = 0;
		mErrNum = 0;
		mRfuNum = 0;
		mOverLoad = 0;
	}
	*/

	Svr_t()
	{
		Initialize();
	}

	//由SvrNet_t 复制构造 Svr_t
	Svr_t(const SvrNet_t &svr) : SvrNet_t(svr) 
	{
		Initialize();
	}
	
	Svr_t(const Svr_t &svr) : SvrNet_t(svr)
	{
		/*
		mDirty = svr.mDirty;
		mOkNum = svr.mOkNum;
		mErrNum = svr.mErrNum;
		mRfuNum = svr.mRfuNum;
		mUsedNum = svr.mUsedNum;
		mDelay = svr.mDelay;
		mOkRate = svr.mOkRate;
		mErrRate = svr.mErrRate;
		mRfuRate = svr.mRfuRate;
		mPreNum = svr.mPreNum;
		mOverLoad = svr.mOverLoad;
		mShutdown = svr.mShutdown;
		mWeight = svr.mWeight;
		*/
	}

	void Initialize()
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

	bool operator==(const Svr_t &svr) const
	{
		return this->mId == svr.mId;
	}

	bool operator>(const Svr_t& svr)  const	//降序
	{
		return mWeight > svr.mWeight; 
	}
	
	bool operator<(const Svr_t& svr)  const	//升序
	{
		return mWeight < svr.mWeight;
	}
};

inline bool GreaterSvr(const Svr_t* pR1, const Svr_t* pR2)
{
	return   pR1->mWeight > pR2->mWeight;
}

inline bool LessSvr(const Svr_t* pR1, const Svr_t* pR2)
{
	return   pR1->mWeight < pR2->mWeight;
}

#endif
