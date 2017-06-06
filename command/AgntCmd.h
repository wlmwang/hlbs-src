
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGNT_CMD_H_
#define _AGNT_CMD_H_

#include "wCore.h"
#include "wCommand.h"
#include "Agnt.h"

using namespace hnet;

#pragma pack(1)

const uint8_t CMD_AGNT_REQ = 80;
struct AgntReqCmd_s : public wCommand {
public:
	AgntReqCmd_s(uint8_t para) : wCommand(CMD_AGNT_REQ, para) { }
};

const uint8_t AGNT_REQ_INIT = 0;
struct AgntReqInit_t : public AgntReqCmd_s  {
public:
	AgntReqInit_t() : AgntReqCmd_s(AGNT_REQ_INIT) { }
};

const uint8_t AGNT_REQ_RELOAD = 1;
struct AgntReqReload_t : public AgntReqCmd_s {
public:
	AgntReqReload_t() : AgntReqCmd_s(AGNT_REQ_RELOAD) { }
};

const uint8_t AGNT_REQ_SYNC = 2;
struct AgntReqSync_t : public AgntReqCmd_s {
public:
	AgntReqSync_t() : AgntReqCmd_s(AGNT_REQ_SYNC) { }
};

const uint8_t AGNT_REQ_ALL = 3;
struct AgntReqAll_t : public AgntReqCmd_s {
public:
	AgntReqAll_t() : AgntReqCmd_s(AGNT_REQ_ALL) { }
};

// ++++++++++++返回数据结构
const uint8_t CMD_AGNT_RES = 81;
struct AgntResCmd_s : public wCommand {
public:
	AgntResCmd_s(uint8_t para) : wCommand(CMD_AGNT_RES, para), mCode(0) { }

	int8_t mCode;
};

const uint8_t AGNT_RES_INIT = 0;
struct AgntResInit_t : public AgntResCmd_s {
public:
	AgntResInit_t() : AgntResCmd_s(AGNT_RES_INIT), mNum(0) { }

	int32_t mNum;
	struct Agnt_t mAgnt[kMaxNum];
};

const uint8_t AGNT_RES_RELOAD = 1;
struct AgntResReload_t : public AgntResCmd_s {
public:
	AgntResReload_t() : AgntResCmd_s(AGNT_RES_RELOAD), mNum(0) { }

	int32_t mNum;
	struct Agnt_t mAgnt[kMaxNum];
};

//同步返回
const uint8_t AGNT_RES_SYNC = 2;
struct AgntResSync_t : public AgntResCmd_s {
public:
	AgntResSync_t() : AgntResCmd_s(AGNT_RES_SYNC), mNum(0) { }

	int32_t mNum;
	struct Agnt_t mAgnt[kMaxNum];
};

const uint8_t AGNT_RES_DATA = 3;
struct AgntResData_t : public AgntResCmd_s {
public:
	AgntResData_t() : AgntResCmd_s(AGNT_RES_DATA), mReqId(0), mNum(0) { }

	int8_t mReqId;
	int32_t mNum;
	struct Agnt_t mAgnt[kMaxNum];
};

#pragma pack()

#endif
