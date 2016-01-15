
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _SVR_H_
#define _SVR_H_

#include <string.h>
#include "wType.h"

#define MAX_SVR_IP_LEN 16
#define MAX_SVR_NUM 255
#define ZOOM_WEIGHT 100
#define ERR_RATE_THRESHOLD 0.5
#define REPORT_TIME_TICK 300000

/**
 * 定义Svr_t
 */
#pragma pack(1)


//不要配太高(0~10)，否则会弱化Svr动态负载均衡效果。
//能表示各个Svr处理能力即可。0为禁用此Svr
//建议设为1，让动态权重感知各Svr性能

/*Svr 基础属性 && 通信结构*/
struct SvrNet_t
{
	int		mId;
	int		mGid;
	int		mXid;
	int		mSWeight;	//静态权重，类似动态权重比例的系数
	int		mPort;
	char	mIp[MAX_SVR_IP_LEN];
	short	mVersion;

	SvrNet_t()
	{
		mId = 0;
		mGid = 0;
		mXid = 0;
		mSWeight = 1;
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
		mPort = svr.mPort;
		mVersion = svr.mVersion;
		memcpy(mIp, svr.mIp, MAX_SVR_IP_LEN);
		return *this;
	}
};

struct Svr_t : public SvrNet_t
{
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

	Svr_t()
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

	//由SvrNet_t 复制构造 Svr_t
	Svr_t(const SvrNet_t &svr) : SvrNet_t(svr) {}
	
	Svr_t(const Svr_t &svr) : SvrNet_t(svr)
	{
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
	}

	void ClearStatistics()
	{
		mDirty = 0;
		mOkNum = 0;
		mErrNum = 0;
		mRfuNum = 0;
		mOverLoad = 0;
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

#pragma pack()

#endif
