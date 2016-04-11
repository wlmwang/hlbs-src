
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _DETECT_H_
#define _DETECT_H_

#include "wCore.h"

#define DETECT_INIT_VALUE   -999

enum DETECT_TYPE
{
    DETECT_UNKNOWN = -1,
    DETECT_PING,
    DETECT_TCP,
    DETECT_UDP,    
};

struct DetectNode_t
{
    string mIp;
    unsigned short mPort;
    time_t mCreateTime;
    time_t mExpireTime;     //探测节点过期时间

    void Touch(time_t tm, time_t expire) 
    {
        mCreateTime = tm; 
        mExpireTime = expire;
    }

    DetectNode_t(time_t tm, time_t expire)
    {
        mIp.clear();
        mPort = 0;
        mCreateTime = tm;
        mExpireTime = expire;
    }

    DetectNode_t(const string& ip, unsigned short port, time_t tm, time_t expire)
    {
        mIp = ip;
        mPort = port;
        mCreateTime = tm;
        mExpireTime = expire;
    }

    bool operator <(DetectNode_t const &o) const
    {
        int iCmp = strcmp(mIp.c_str(), o.mIp.c_str());
        if (iCmp < 0)
        {
            return true;
        }
        else if (iCmp > 0)
        {
            return false;
        }
        else if (mPort < o.mPort)
        {
            return true;
        }
        else if (mPort > o.mPort)
        {
            return false;
        }
        return false;
    }

    bool operator ==(DetectNode_t const &o) const
    {
        return (mIp == o.mIp && mPort == o.mPort);
    }
};

struct DetectResult_t
{
	int mRc;
    int mDetectType;

	int mElapse;
    int mPingElapse;
    int mConnElapse;
    int mUdpElapse;

    time_t mLastDetectTime;
    time_t mNextDetectTime;     //下次检测时间

    DetectResult_t()
    {
        mRc = DETECT_INIT_VALUE;
        mDetectType = -1;

        mElapse = 0;
        mPingElapse = 0;
        mConnElapse = 0;
        mUdpElapse = 0;

        mLastDetectTime = 0;
        mNextDetectTime = 0;
    }

    void HasDetect(time_t now, unsigned int interval)
    {
        mLastDetectTime = now;
        mNextDetectTime = now + interval;
    }
};

#endif