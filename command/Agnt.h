
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGNT_H_
#define _AGNT_H_

#include "Svr.h"

const char kAgntLog[] = "../log/agent.log";

const int8_t	kAgntOk	  = 0;		// 有效agent客户端
const int8_t 	kAgntInit = -1;		// 初始化
const int8_t 	kAgntUreg = -2;		// agent已连接，router无此配置
const int8_t 	kAgntDisc = -3;		// agent与router断开

using namespace hnet;

#pragma pack(1)

// agent节点信息
struct Agnt_t {
public:
	char	mHost[kMaxHost];
	int32_t	mPort;		// 使用过程中，参考意义不大
	int32_t	mVersion;
	int8_t	mIdc;
	int8_t 	mStatus;
	int8_t 	mConfig;		// 是否是配置文件中记录

	Agnt_t(): mPort(0), mVersion(misc::GetTimeofday()/1000000), mIdc(0), mStatus(kAgntInit), mConfig(-1) {
		memset(mHost, 0, kMaxHost);
	}
	
	Agnt_t(const Agnt_t& other): mPort(other.mPort), mVersion(other.mVersion), mIdc(other.mIdc), mStatus(other.mStatus), mConfig(other.mConfig) {
		memcpy(mHost, other.mHost, kMaxHost);
	}

	Agnt_t& operator=(const Agnt_t &other) {
		memcpy(mHost, other.mHost, kMaxHost);
		mPort = other.mPort;
		mVersion = other.mVersion;
		mIdc = other.mIdc;
		mStatus = other.mStatus;
		mConfig = other.mConfig;
		return *this;
	}

	// 忽略port
	bool operator<(const Agnt_t &other) const {
		return strcmp(mHost, other.mHost) < 0 ? true : false;
	}

	// 忽略port
	bool operator==(const Agnt_t &other) const {
		return !strcmp(mHost, other.mHost);
	}
};

#pragma pack()

#endif
