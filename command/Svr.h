
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _SVR_H_
#define _SVR_H_

#include <string.h>
#include "wType.h"

//名字的最大长度
#define MAX_SVR_NAME_LEN 64
#define MIN_SVR_NAME_LEN 3
#define MAX_SVR_IP_LEN 16
#define MIN_SVR_IP_LEN 3

#define REPORT_TIME_TICK 300000

/**
 * 定义Svr_t
 */
#pragma pack(1)

/*Svr 基础属性 && 通信结构*/
struct SvrNet_t
{
	int		mId;
	int		mGid;
	int		mXid;
	int		mSWeight;	//静态权重
	short	mDisabled;	//是否可用
	int		mPort;		//端口
	char	mIp[MAX_SVR_IP_LEN];	//ip
	char	mName[MAX_SVR_NAME_LEN];//名称
	
	SvrNet_t()
	{
		mId = 0;
		mGid = 0;
		mXid = 0;
		mDisabled = 0;
		mPort = 0;
		mSWeight = 100;
		memset(mName, 0, MAX_SVR_NAME_LEN);
		memset(mIp, 0, MAX_SVR_IP_LEN);
	}

	SvrNet_t(const SvrNet_t &svr)
	{
		mId = svr.mId;
		mGid = svr.mGid;
		mXid = svr.mXid;
		mPort = svr.mPort;
		mSWeight = svr.mSWeight;
		mDisabled = svr.mDisabled;
		memcpy(mName, svr.mName, MAX_SVR_NAME_LEN);
		memcpy(mIp, svr.mIp, MAX_SVR_IP_LEN);
	}

	SvrNet_t& operator=(const SvrNet_t &svr)
	{
		mId = svr.mId;
		mGid = svr.mGid;
		mXid = svr.mXid;
		mPort = svr.mPort;
		mSWeight = svr.mSWeight;
		mDisabled = svr.mDisabled;
		memcpy(mName, svr.mName, MAX_SVR_NAME_LEN);
		memcpy(mIp, svr.mIp, MAX_SVR_IP_LEN);
		return *this;
	}
};

struct Svr_t : public SvrNet_t
{
	float	mWeight;	//综合权重值（动态计算, 默认为1.0*mSWeight）
	short	mPreNum;	//预取数，默认为1
	short	mOverLoad;	//是否过载
	long	mUsedNum;	//已被分配数量（返回给某CGI次数），0未知
	long	mTotalNum;	//冗余字段：总请求数量（同一Gid，Xid下所有请求数量），0未知  = SUM(mUsedNum(1 2 ...n))

	//当前(5min)周期数据统计
	short	mDirty;		//是否要更新本svr（本周期有上报数据）
	long	mOkNum;		//成功数量，0未知
	long	mErrNum;	//失败数量，
	int		mDelay;		//延时时间。0未知，>0具体延时（只保留周期内最小延时）
	long	mRfuNum;	//拒绝数

	//统计结果：每周期结束(5min)，由上周期统计数据计算得出
	float	mOkRate;	//成功率 0-1，0未知
	float	mErrRate;	//失败率 0-1
	float	mRfuRate;	//拒绝率率 0-1

	Svr_t()
	{
		mDirty = 0;
		mOkNum = 0;
		mErrNum = 0;
		mRfuNum = 0;
		mUsedNum = 0;
		mTotalNum = 0;
		mDelay = 0;
		mOkRate = 0.0;
		mErrRate = 0.0;
		mRfuRate = 0.0;
		mOverLoad = 0;
		mPreNum = 1;
		mWeight = mSWeight;
	}

	//由SvrNet_t 复制构造 Svr_t
	Svr_t(const SvrNet_t &svr) : SvrNet_t(svr) 
	{
		mWeight *= svr.mSWeight;
	}
	
	Svr_t(const Svr_t &svr) : SvrNet_t(svr)
	{
		mDirty = svr.mDirty;
		mOkNum = svr.mOkNum;
		mErrNum = svr.mErrNum;
		mRfuNum = svr.mRfuNum;
		mUsedNum = svr.mUsedNum;
		mTotalNum = svr.mTotalNum;
		mDelay = svr.mDelay;
		mOkRate = svr.mOkRate;
		mErrRate = svr.mErrRate;
		mRfuRate = svr.mRfuRate;
		mPreNum = svr.mPreNum;
		mOverLoad = svr.mOverLoad;
		mWeight = svr.mWeight;
	}

	void ClearStatistics()
	{
		mDirty = 0;
		mOkNum = 0;
		mErrNum = 0;
		mRfuNum = 0;
		mDelay = 0;
		mOkRate = 0.0;
		mErrRate = 0.0;
		mRfuRate = 0.0;
	}

	bool operator==(const Svr_t &svr) const
	{
		return this->mId == svr.mId;
	}

	bool operator>(const Svr_t& svr)  const	//降序
	{
		return 1/mWeight > 1/svr.mWeight; 
	}
	
	bool operator<(const Svr_t& svr)  const	//升序
	{
		return 1/mWeight < 1/svr.mWeight;
	}
};

inline bool GreaterSvr(const Svr_t* pR1, const Svr_t* pR2)
{
	return   1/pR1->mWeight > 1/pR2->mWeight;
}

inline bool LessSvr(const Svr_t* pR1, const Svr_t* pR2)
{
	return   1/pR1->mWeight < 1/pR2->mWeight;
}

#pragma pack()

#endif
