
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

struct Svr_t
{
	int		mId;
	int		mGid;
	int		mXid;
	int		mDelay;		//最小延时 单位：s
	float	mOkRate;	//最大成功率 0-1
	float	mWeight;	//权重
	short	mDisabled;	//是否可用
	short	mDirty;		//是否要更新
	int		mPort;
	char	mIp[MAX_SVR_IP_LEN];
	char	mName[MAX_SVR_NAME_LEN];
	

	Svr_t()
	{
		mId = 0;
		mGid = 0;
		mXid = 0;
		mPort = 0;
		mDisabled = 0;
		mDirty = 0;
		mDelay = 0;
		mOkRate = 0.0;
		mWeight = 1.0;
		memset(mName, 0, MAX_SVR_NAME_LEN);
		memset(mIp, 0, MAX_SVR_IP_LEN);
	}

	Svr_t(const Svr_t &svr)
	{
		mId = svr.mId;
		mGid = svr.mGid;
		mXid = svr.mXid;
		mPort = svr.mPort;
		mWeight = svr.mWeight;
		mDisabled = svr.mDisabled;
		mDirty = svr.mDirty;
		mDelay = svr.mDelay;
		mOkRate = svr.mOkRate;
		memcpy(mName, svr.mName, MAX_SVR_NAME_LEN);
		memcpy(mIp, svr.mIp, MAX_SVR_IP_LEN);
	}

	Svr_t & operator=(const Svr_t &svr)
	{
		mId = svr.mId;
		mGid = svr.mGid;
		mXid = svr.mXid;
		mPort = svr.mPort;
		mDisabled = svr.mDisabled;
		mDirty = svr.mDirty;
		mWeight = svr.mWeight;
		mDelay = svr.mDelay;
		mOkRate = svr.mOkRate;
		memcpy(mName, svr.mName, MAX_SVR_NAME_LEN);
		memcpy(mIp, svr.mIp, MAX_SVR_IP_LEN);
		return *this;
	}
	
	bool operator>(const Svr_t& svr)  const	//降序
	{
		return 1/mWeight > 1/svr.mWeight; 
	}
	
	bool operator<(const Svr_t& svr)  const	//升序
	{
		return 1/mWeight < 1/svr.mWeight;
	}
	
	bool operator==(const Svr_t &svr) const
	{
		return this->mId == svr.mId;
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
