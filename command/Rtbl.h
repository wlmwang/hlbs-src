
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _RTBL_H_
#define _RTBL_H_

#include <string.h>
#include "wType.h"

/**
 * ¶¨ÒåRtbl_t
 */
#pragma pack(1)

struct Rtbl_t
{
	int		mId;
	int		mGid;
	int		mXid;
	char	mName[MAX_NAME_LEN];
	char	mIp[MAX_IP_LEN];
	int		mPort;
	short	mWeight;
	short	mDisabled;
	
	Rtbl_t()
	{
		this->mId = 0;
		this->mGid = 0;
		this->mXid = 0;
		this->mPort = 0;
		this->mWeight = 0;
		this->mDisabled = 0;
		memset(this->mName, 0, MAX_NAME_LEN);
		memset(this->mIp, 0, MAX_IP_LEN);
	}

	Rtbl_t(const Rtbl_t &rtbl)
	{
		this->mId = rtbl.mId;
		this->mGid = rtbl.mGid;
		this->mXid = rtbl.mXid;
		this->mPort = rtbl.mPort;
		this->mWeight = rtbl.mWeight;
		this->mDisabled = rtbl.mDisabled;
		memset(this->mName, 0, MAX_NAME_LEN);
		memset(this->mIp, 0, MAX_IP_LEN);
	}

	Rtbl_t & operator= (const Rtbl_t &rtbl)
	{
		this->mId = rtbl.mId;
		this->mGid = rtbl.mGid;
		this->mXid = rtbl.mXid;
		this->mPort = rtbl.mPort;
		this->mWeight = rtbl.mWeight;
		this->mDisabled = rtbl.mDisabled;
		memcpy(this->mName, rtbl.mName, MAX_NAME_LEN);
		memcpy(this->mIp, rtbl.mIp, MAX_IP_LEN);
		return *this;
	}
	
	bool operator== (const Rtbl_t &rtbl) const
	{
		return this->mId == rtbl.mId;
	}
};

#pragma pack()

#endif
