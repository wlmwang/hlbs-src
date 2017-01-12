
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _SVR_CMD_H_
#define _SVR_CMD_H_

#include "wCore.h"
#include "wCommand.h"
#include "Svr.h"

using namespace hnet;

#pragma pack(1)

/** 服务端消息通信 router、agent */
//++++++++++++请求数据结构
const uint8_t CMD_SVR_REQ = 60;
struct SvrReqCmd_s : public wCommand {
public:
	SvrReqCmd_s(uint8_t para) : wCommand(CMD_SVR_REQ, para) { }
};

// agent主动初始化请求
const uint8_t SVR_REQ_INIT = 0;
struct SvrReqInit_t : public SvrReqCmd_s  {
public:
	SvrReqInit_t() : SvrReqCmd_s(SVR_REQ_INIT) { }
};

// router重新读取配置（agent重读配置由初始化请求替代）
const uint8_t SVR_REQ_RELOAD = 1;
struct SvrReqReload_t : public SvrReqCmd_s {
public:
	SvrReqReload_t() : SvrReqCmd_s(SVR_REQ_RELOAD) { }
};

// router主动下发同步svr
const uint8_t SVR_REQ_SYNC = 2;
struct SvrReqSync_t : public SvrReqCmd_s {
public:
	SvrReqSync_t() : SvrReqCmd_s(SVR_REQ_SYNC) { }
};

// 获取全部svr
const uint8_t SVR_REQ_ALL = 3;
struct SvrReqAll_t : public SvrReqCmd_s {
public:
	SvrReqAll_t() : SvrReqCmd_s(SVR_REQ_ALL) { }
};

// 根据gid、xid获取svr
const uint8_t SVR_REQ_GXID = 4;
struct SvrReqGXid_t : public SvrReqCmd_s {
public:
	SvrReqGXid_t() : SvrReqCmd_s(SVR_REQ_GXID), mGid(0), mXid(0), mPort(0) {
		memset(mHost, 0, kMaxHost);
	}

	int32_t mGid;
	int32_t mXid;
	int32_t mPort;
	char	mHost[kMaxHost];
};

/** 上报数据结构 */
const uint8_t SVR_REQ_REPORT = 5;
struct SvrReqReport_t : public SvrReqCmd_s {
public:
	SvrReqReport_t() : SvrReqCmd_s(SVR_REQ_REPORT) { }
	struct SvrCaller_t mCaller;
};

// ++++++++++++返回数据结构
const uint8_t CMD_SVR_RES = 61;
struct SvrResCmd_s : public wCommand {
public:
	SvrResCmd_s(uint8_t para) : wCommand(CMD_SVR_RES, para), mCode(0) { }

	int8_t mCode;
};

// 初始化返回
const uint8_t SVR_RES_INIT = 0;
struct SvrResInit_t : public SvrResCmd_s {
public:
	SvrResInit_t() : SvrResCmd_s(SVR_RES_INIT), mNum(0) { }

	int32_t mNum;
	struct SvrNet_t mSvr[kMaxNum];
};

//重载返回
const uint8_t SVR_RES_RELOAD = 1;
struct SvrResReload_t : public SvrResCmd_s {
public:
	SvrResReload_t() : SvrResCmd_s(SVR_RES_RELOAD), mNum(0) { }

	int32_t mNum;
	struct SvrNet_t mSvr[kMaxNum];
};

//同步返回
const uint8_t SVR_RES_SYNC = 2;
struct SvrResSync_t : public SvrResCmd_s {
public:
	SvrResSync_t() : SvrResCmd_s(SVR_RES_SYNC), mNum(0) { }

	int32_t mNum;
	struct SvrNet_t mSvr[kMaxNum];
};

//获取svr请求返回数据，包括all|id|name|gid/xid
const uint8_t SVR_RES_DATA = 3;
struct SvrResData_t : public SvrResCmd_s {
public:
	SvrResData_t() : SvrResCmd_s(SVR_RES_DATA), mReqId(0), mNum(0) { }

	// 包括all|id|name|gid/xid的wCommand::mId  区分请求cmd
	int8_t mReqId;
	int32_t mNum;
	struct SvrNet_t mSvr[kMaxNum];
};

//获取单个svr
const uint8_t SVR_ONE_RES = 4;
struct SvrOneRes_t : public SvrResCmd_s {
public:
	SvrOneRes_t() : SvrResCmd_s(SVR_ONE_RES),mNum(0) { }

	int32_t mNum;
	struct SvrNet_t mSvr;
};

//上报返回
const uint8_t SVR_RES_REPORT = 5;
struct SvrResReport_t : public SvrResCmd_s {
public:
	SvrResReport_t() : SvrResCmd_s(SVR_RES_REPORT) { }
};

#pragma pack()

#endif
