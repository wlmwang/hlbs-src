
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _SVR_H_
#define _SVR_H_

#include "wCore.h"
#include "wMisc.h"

const char kSvrLog[] = "../log/svr.log";

// 每次请求svr、agnt最多个数
const int32_t	kMaxNum		= 64;

const int8_t	kMaxHost	= 16;
const int8_t	kMaxName	= 64;

const int32_t	kInitWeight = 100;
const int32_t	kMaxWeight	= 1000;
const int64_t	kDelayMax	= 100000000;	// 最大延时值 100s
const int64_t	kOkLoadMax	= 1000000;		// 最大成功率负载

enum SVR_RTN {
	SVR_RTN_OK	   		= 0x00,
	SVR_RTN_ACCEPT,					// 接收
	SVR_RTN_OVERLOAD	= -10000,	// 过载
	SVR_RTN_TIMEOUT     = -9999,    // 超时
	SVR_RTN_SYSERR      = -9998,    // 系统错误
	SVR_RTN_GET_ROUTE   = -9997,
	SVR_RTN_GET_SNAME   = -9996,
	SVR_RTN_OFFSIDE   = -9995    
};

using namespace hnet;

#pragma pack(1)

// 节点信息（基础信息）
struct SvrNet_t {
public:
	int32_t		mGid;			// 一级id
	int32_t		mXid;			// 二级id
	char		mHost[kMaxHost];// 主机地址
	uint16_t	mPort;			// 端口
	int32_t 	mWeight;		// 静态权重，0为禁用此路由
	int32_t		mVersion;		// 版本号
	char		mName[kMaxName];// 服务名
	int8_t		mIdc;			// IDC

    //int32_t 	mPre;			// 预取数
    //int32_t 	mExpired;		// 过期时间
    SvrNet_t(): mGid(0), mXid(0), mPort(0), mWeight(kInitWeight), mVersion(soft::TimeUnix()), mIdc(0) /*,mPre(1), mExpired(0)*/ {
    	memset(mHost, 0, kMaxHost);
    	memset(mName, 0, kMaxName);
    }

	SvrNet_t(const SvrNet_t& other): mGid(other.mGid), mXid(other.mXid), mPort(other.mPort), mWeight(other.mWeight), 
	mVersion(other.mVersion), mIdc(other.mIdc) /*,mPre(other.mPre), mExpired(other.mExpired)*/ {
		memcpy(mHost, other.mHost, kMaxHost);
		memcpy(mName, other.mName, kMaxName);
	}

	SvrNet_t& operator=(const SvrNet_t& other) {
		mGid = other.mGid;
		mXid = other.mXid;
		mPort = other.mPort;
		mWeight = other.mWeight;
		mVersion = other.mVersion;
		mIdc = other.mIdc;
		//mPre = other.mPre;
		//mExpired = other.mExpired;
		memcpy(mHost, other.mHost, kMaxHost);
		memcpy(mName, other.mName, kMaxName);
		return *this;
	}

	// 忽略weight version排序、find
	bool operator<(const SvrNet_t &other) const {
        if (mGid < other.mGid ) {
            return true;
        } else if (mGid > other.mGid) {
        	return false;
        } else if (mXid < other.mXid) {
            return true;
        } else if (mXid > other.mXid) {
            return false;
        } else if (mPort < other.mPort) {
            return true;
        } else if (mPort > other.mPort) {
        	return false;
        }
        return strcmp(mHost, other.mHost) < 0 ? true : false;
	}

	// 忽略 weight version ==比较
    bool operator==(const SvrNet_t &other) const {
        if (mGid != other.mGid) {
            return false;
        } else if (mXid != other.mXid) {
            return false;
        } else if (mPort != other.mPort) {
            return false;
        }
        return !strcmp(mHost, other.mHost);
    }
};

// 上报调用信息
struct SvrCaller_t {
public:
	int32_t mCallerGid;		// 主调一级id
	int32_t	mCallerXid;		// 主调二级id

	int32_t	mCalledGid;		// 被调一级id
	int32_t	mCalledXid;		// 被调二级id
	int32_t mPort;			// 被调主机PORT
	char	mHost[kMaxHost];// 被调主机IP

	int32_t	mReqRet;		// 请求结果。 >=0 成功
	int32_t mReqCount;		// 请求次数
	int64_t mReqUsetimeUsec;// 微妙
	int32_t mTid;			// 进程id（为实现）
	
	SvrCaller_t(): mCallerGid(0), mCallerXid(0), mCalledGid(0), mCalledXid(0), mPort(0),
			mReqRet(0), mReqCount(0), mReqUsetimeUsec(0), mTid(0) {
		memset(mHost, 0, kMaxHost);
	}
	
	SvrCaller_t& operator=(const SvrNet_t& svr) {
		mCalledGid = svr.mGid;
		mCalledXid = svr.mXid;
		mPort = svr.mPort;
		memcpy(mHost, svr.mHost, kMaxHost);
		mReqCount = 1;
		
		mCallerGid = 0;
		mCallerXid = 0;
		mReqRet = 0;
		mReqUsetimeUsec = 0;
		mTid = 0;
		return *this;
	}

	bool operator==(const SvrCaller_t &other) const {
        if (mCallerGid != other.mCallerGid) {
            return false;
        } else if (mCallerXid != other.mCallerXid) {
            return false;
        } else if (mCalledGid != other.mCalledGid) {
            return false;
        } else if(mCalledXid != other.mCalledXid) {
            return false;
        } else if(strcmp(mHost, other.mHost)) {
            return false;
        } else if(mPort != other.mPort) {
            return false;
        }
        return true;
    }
};

#pragma pack()

// 宕机检测和探测的相关配置
struct SvrDownCfg_t {
public:
	int32_t mReqCountTrigerProbe;
	int32_t mDownTimeTrigerProbe;
	int32_t mProbeTimes;
	int32_t mPossibleDownErrReq;    // 连续错误阈值
	float mPossbileDownErrRate;		// 宕机错误率阈值

    // mProbeBegin >0 才打开自探测
	int32_t mProbeBegin;
	int32_t mProbeInterval;
	int32_t mProbeNodeExpireTime;

	SvrDownCfg_t(): mReqCountTrigerProbe(100000), mDownTimeTrigerProbe(600), mProbeTimes(3), mPossibleDownErrReq(10), mPossbileDownErrRate(0.5),
			mProbeBegin(3), mProbeInterval(10), mProbeNodeExpireTime(600) { }
};

// 节点访问量的配置信息（含门限）
struct SvrReqCfg_t {
public:
	int32_t mReqLimit;		// 访问量控制的阀值(门限)
	int32_t	mReqMax;		// 访问量控制的最大值
	int32_t	mReqMin;		// 访问量控制的最小值
	int32_t	mReqCount;		// 访问量控制的实际值（请求数）
	int32_t mRebuildTm;		// 统计的周期 60s
	int32_t mPreTime;		// 4(不能大于route重建时间的一半) 可以设定预取时间长度N秒内的路由计数，N小于当前周期的1/2
	float	mReqErrMin;		// 错误的最小阀值 0-1 [小于则服务无错，应增大访问量。大于则服务过载，应减少访问量。]
	float	mReqExtendRate;	// 无错误的时候的访问量阀值扩张率 0.001-101

	SvrReqCfg_t(): mReqLimit(0), mReqMax(10000), mReqMin(10), mReqCount(0), mRebuildTm(60), mPreTime(4),
			mReqErrMin(0.5), mReqExtendRate(0.2) { }
};

struct PidInfo_t {
public:
	int32_t mReq;
	int32_t mRej;
	int32_t mSuc;
	int32_t mErr;
	int32_t mTo;
	int32_t mLastReq;
	int32_t mLastRej;
	int32_t mLastSuc;
	int32_t mLastErr;
	int32_t mLastTo;

	int32_t mCyc;
	time_t mCycTm;
	int32_t mTid;
	int32_t mIdle;
	int32_t mAdd;
	int32_t mTotalUsec;
	int32_t mAvgDelay;

	PidInfo_t(): mReq(0),mRej(0),mSuc(0),mErr(0),mTo(0),mLastReq(0),mLastRej(0),mLastSuc(0),mLastErr(0),mLastTo(0),
		mCyc(0),mCycTm(soft::TimeUnix()),mTid(0),mIdle(0),mAdd(0),mTotalUsec(0),mAvgDelay(0) { }

	void NextCycReady() {
		mCyc++;

		if (mIdle > 3) {
		    mLastReq = mLastRej = mLastSuc = mLastErr = mLastTo = 0;
		}
		mLastReq = mReq;
		mLastRej  = mRej;
		mLastErr = mErr;
		mLastTo = mTo;
		if (mSuc > mLastSuc) {
			mAdd = mSuc - mLastSuc;
		} else {
			mAdd = 0;
		}

		if (mSuc > 0) {
		    mLastSuc = mSuc;
		    mIdle = 0;
		} else {
		    mIdle++;
		}
		if (mLastSuc > 0) {
			mAvgDelay = mTotalUsec/mLastSuc;
		} else {
			mAvgDelay = 1;
		}

		mReq = mRej = mSuc = mErr = mTo = 0;
		mTotalUsec = 0;
		mCycTm = soft::TimeUnix();
	}

	int32_t GetReq() {
		return mReq;
	}
	int32_t GetLastReq() {
	    return mLastReq;
	}
	int32_t GetIncremental() {
	    return mAdd;
	}
	int32_t GetLastSuc() {
	    return mLastSuc + 1;
	}
};

// 节点统计信息
struct SvrInfo_t {
public:
	struct timeval	mBuildTm;			// 统计信息开始时间，每个节点 rebuild 时刻的绝对时间
	int32_t			mReqAll;			// 总的请求数，理论上请求调用agent获取路由次数
	int32_t			mReqRej;			// 被拒绝的请求数
	int32_t			mReqSuc;			// 成功的请求数
	int32_t			mReqErrRet;			// 失败的请求数
	int32_t			mReqErrTm;			// 超时的请求数

	float       	mLoadX;				// 负载总乘数 (mOkLoad*mDelayLoad)*(max<mWeight>)/mWeight  系数
	float       	mOkLoad;			// 成功率乘数
	float       	mDelayLoad;			// 时延乘数
    float       	mDelayLoadAmplify;	// 时延放大
	float       	mOkRate;			// 上一周期成功率（除去失败率） 0-1
	uint32_t		mAvgTm;				// 上一周期成功请求平均时延，微秒  mTotalUsec/mReqSuc
	uint64_t		mTotalUsec; 		// 请求总微秒数
	float			mAvgErrRate;		// 平均错误率，统计多个周期的错误率并加以平均得到（未超过最低阈值(mReqCfg.mReqErrMin)，始终为0）

	/** 上一周期统计数据 */
	int32_t 		mLastReqAll;
	int32_t 		mLastReqRej;
	int32_t 		mLastReqErrRet;
	int32_t 		mLastReqErrTm;
	int32_t  		mLastReqSuc;
	bool  			mLastErr;			// 上周期请求门限是否过度扩张。 是：true  否：false
	int32_t 		mLastAlarmReq;		// 上周期门限扩张值
	int32_t 		mLastAlarmSucReq;	// 参考值。成功请求数扩张门限
	int32_t 		mPreAll;			// 路由被分配次数 + 预取数

	int32_t 		mCityId;			// 被调所属cityId = svrNet.idc
	int32_t 		mOffSide;			// 被调节点与主调异地标志，默认为0， 1标为异地

	int32_t 		mContErrCount;		// 连续失败次数累积

	int32_t			mSReqAll;			// 总的请求数(统计用)
	int32_t			mSReqRej;			// 被拒绝的请求数(统计用)
	int32_t			mSReqSuc;			// 成功的请求数(统计用)
	int32_t			mSReqErrRet;		// 失败的请求数(统计用)
	int32_t			mSReqErrTm;			// 超时的请求数(统计用)
	int32_t         mSPreAll;			// 路由被分配次数(统计)
    
	int32_t 		mAddSuc;			// 上个周期与上上个周期成功请求数差值
	int32_t 		mIdle;				// 空闲周期统计
    
    std::map<int, struct PidInfo_t> mClientInfo;

    SvrInfo_t(): mReqAll(0), mReqRej(0), mReqSuc(0),mReqErrRet(0), mReqErrTm(0),mLoadX(1.0),mOkLoad(1.0),mDelayLoad(1.0),mDelayLoadAmplify(0.0),mOkRate(1.0f),
    		mAvgTm(1),mTotalUsec(1), mAvgErrRate(0.0),mLastReqAll(0),mLastReqRej(0),mLastReqErrRet(0),mLastReqErrTm(0),mLastReqSuc(0),mLastErr(false),mLastAlarmReq(0),
			mLastAlarmSucReq(0), mPreAll(0), mCityId(0), mOffSide(0),mContErrCount(0), mSReqAll(0),mSReqRej(0),mSReqSuc(0),mSReqErrRet(0),mSReqErrTm(0),mSPreAll(0),
			mAddSuc(0),mIdle(0) {
		mBuildTm.tv_sec = mBuildTm.tv_usec = 0;
    }

    void InitInfo(const struct SvrNet_t& svr) { }

    void NextCycReady() {
		std::map<int, struct PidInfo_t>::iterator it = mClientInfo.begin();
		while (it != mClientInfo.end()) {
			it->second.NextCycReady();
            if (it->second.mLastReq <= 0) {
				mClientInfo.erase(it++); // Fixme: optimize
            } else {
                it++;
            }
		}
    }
};

// 节点阈值（含门限） && 统计 信息
struct SvrStat_t {
public:
	struct SvrReqCfg_t		mReqCfg;	// 访问量配置 && 门限
	struct SvrInfo_t		mInfo;		// 统计信息
	
	void Reset() {
		mInfo.mReqAll = 0;
		mInfo.mReqSuc = 0;
		mInfo.mReqRej = 0;
		mInfo.mReqErrRet = 0;
		mInfo.mReqErrTm = 0;
		mInfo.mTotalUsec = 0;
		mInfo.mPreAll = 0;
		mInfo.mContErrCount = 0;
	}

	void AddStatistic() {
		mInfo.mSReqAll += mInfo.mReqAll;
		mInfo.mSReqSuc += mInfo.mReqSuc;
		mInfo.mSReqRej += mInfo.mReqRej;
		mInfo.mSReqErrRet += mInfo.mReqErrRet;
		mInfo.mLastReqErrTm += mInfo.mReqErrTm;
		mInfo.mSPreAll += mInfo.mPreAll;
	}

	void ResetStatistic() {
		mInfo.mSReqAll = 0;
		mInfo.mSReqSuc = 0;
		mInfo.mSReqRej = 0;
		mInfo.mSReqErrRet = 0;
		mInfo.mLastReqErrTm = 0;
		mInfo.mSPreAll = 0;
	}
};

// 分类节点信息（由mGid、mXid组成的一类节点）
struct SvrKind_t {
public:
	int32_t		mGid;
	int32_t		mXid;
	int32_t		mInnerChange;
	int32_t 	mOverload;
	int32_t 	mPindex;			// 分类路由轮转索引
    float 		mPtotalErrRate;		// 累计连续过载的所有路由错误率平均值总和
    int32_t 	mPsubCycCount;		// 累计连续过载次数

    int32_t 	mPtm; 				// rebuild 时刻的绝对时间 time_t
    int64_t 	mAccess64tm;		// 最近访问时间 微妙
    int32_t 	mRebuildTm; 		// rebuild 的时间间隔
	int8_t		mNearestAccessFlag;	// 就近接入标志 1:就近接入 0:不
	float 		mWeightSum;
	
	SvrKind_t(): mGid(0), mXid(0), mInnerChange(1), mOverload(0), mPindex(0), mPtotalErrRate(0.0f), mPsubCycCount(0),
		mPtm(soft::TimeUnix()), mAccess64tm(soft::TimeUsec()), mRebuildTm(3), mNearestAccessFlag(0), mWeightSum(0.0f) { }

	SvrKind_t(const SvrKind_t& other): mGid(other.mGid), mXid(other.mXid), mInnerChange(other.mInnerChange), mOverload(other.mOverload), 
	mPindex(other.mPindex), mPtotalErrRate(other.mPtotalErrRate), mPsubCycCount(other.mPsubCycCount), mPtm(other.mPtm), mAccess64tm(other.mAccess64tm),
	mRebuildTm(other.mRebuildTm), mNearestAccessFlag(other.mNearestAccessFlag), mWeightSum(other.mWeightSum) { }

	SvrKind_t(const SvrNet_t& svr): mGid(svr.mGid), mXid(svr.mXid), mInnerChange(1), mOverload(0), mPindex(0), mPtotalErrRate(0.0f), mPsubCycCount(0),
	mPtm(soft::TimeUnix()), mAccess64tm(soft::TimeUsec()), mRebuildTm(3), mNearestAccessFlag(0), mWeightSum(0.0f) { }

	SvrKind_t& operator=(const SvrKind_t& other) {
		mGid = other.mGid;
		mXid = other.mXid;
		mInnerChange = other.mInnerChange;
		mOverload = other.mOverload;
		mPindex = other.mPindex;
        mPtotalErrRate = other.mPtotalErrRate;
        mPsubCycCount = other.mPsubCycCount;
        mPtm = other.mPtm;
        mAccess64tm = other.mAccess64tm;
        mRebuildTm = other.mRebuildTm;
        mNearestAccessFlag = other.mNearestAccessFlag;
        mWeightSum = other.mWeightSum;
        return *this;
	}

    bool operator<(const SvrKind_t &other) const {
        if (mGid < other.mGid) {
            return true;
        } else if(mGid > other.mGid) {
            return false;
        } else if(mXid < other.mXid) {
            return true;
        } else if (mXid > other.mXid) {
            return false;
        }
        return false;
    }

    bool operator==(const SvrKind_t &other) const {
        if (mGid != other.mGid) {
            return false;
        } else if(mXid != other.mXid) {
            return false;
        }
        return true;
    }
};

// 节点信息
struct SvrNode_t {
public:
	struct SvrNet_t  mNet;	// 节点信息
	struct SvrStat_t *mStat;// 节点统计 && 阈值、门限

	float mKey;					// 关键值，初始化为 mInfo.mLoadX = 1

	int32_t mStopTime;			// 宕机记录信息
	int32_t mReqAllAfterDown;	// 宕机以来所有请求数量
    
    // 宕机相关的额外恢复条件
	int32_t mDownTimeTrigerProbeEx;		// 时间
	int32_t mReqCountTrigerProbeEx;		// 请求数量
    
    bool mIsDetecting; 		// 是否处在 "探测宕机是否恢复" 的状态

    SvrNode_t(): mStat(NULL), mKey(0.0), mStopTime(0), mReqAllAfterDown(0), mDownTimeTrigerProbeEx(0),
    		mReqCountTrigerProbeEx(0), mIsDetecting(false) { }
    
    SvrNode_t(const struct SvrNode_t& other): mNet(other.mNet), mStat(other.mStat), mKey(other.mKey), mStopTime(other.mStopTime),
    		mReqAllAfterDown(other.mReqAllAfterDown), mDownTimeTrigerProbeEx(other.mDownTimeTrigerProbeEx), 
    		mReqCountTrigerProbeEx(other.mReqCountTrigerProbeEx), mIsDetecting(other.mIsDetecting) {}
	
	SvrNode_t(const struct SvrNet_t& svr, struct SvrStat_t* stat): mNet(svr), mStat(stat) {
		if (!mStat) {
            mKey = 1;
        } else {
            mKey = mStat->mInfo.mLoadX;
        }
        mStopTime = 0;
        mReqAllAfterDown = 0;
        mIsDetecting = false;
        mDownTimeTrigerProbeEx = 0;
        mReqCountTrigerProbeEx = 0;
	}
};

#endif
