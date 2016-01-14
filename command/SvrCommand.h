
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_ROUTER_COMMAND_H_
#define _W_ROUTER_COMMAND_H_

#include <string.h>

#include "BaseCommand.h"
#include "Svr.h"

#pragma pack(1)

/** 服务端消息通信 router、agent */
//++++++++++++请求数据结构
const BYTE CMD_SVR_REQ = 10;
struct SvrReqCmd_s : public wCommand
{
	SvrReqCmd_s(BYTE para)
	{
		mCmd = CMD_SVR_REQ;
		mPara = para;
	}
};

//agent主动初始化请求
const BYTE SVR_REQ_INIT = 0;
struct SvrReqInit_t : SvrReqCmd_s 
{
	SvrReqInit_t() : SvrReqCmd_s(SVR_REQ_INIT) {}
};

//router重新读取配置（agent重读配置由初始化请求替代）
const BYTE SVR_REQ_RELOAD = 1;
struct SvrReqReload_t : SvrReqCmd_s 
{
	SvrReqReload_t() : SvrReqCmd_s(SVR_REQ_RELOAD) {}
};

//router主动下发同步svr
const BYTE SVR_REQ_SYNC = 2;
struct SvrReqSync_t : SvrReqCmd_s 
{
	SvrReqSync_t() : SvrReqCmd_s(SVR_REQ_SYNC) {}
};

//获取全部svr
const BYTE SVR_REQ_ALL = 3;
struct SvrReqAll_t : SvrReqCmd_s 
{
	SvrReqAll_t() : SvrReqCmd_s(SVR_REQ_ALL) {}
};

//根据gid获取svr
const BYTE SVR_REQ_GID = 5;
struct SvrReqGid_t : SvrReqCmd_s 
{
	SvrReqGid_t() : SvrReqCmd_s(SVR_REQ_GID), mGid(0) {}
	
	WORD mGid;
};

//根据gid、xid获取svr
const BYTE SVR_REQ_GXID = 7;
struct SvrReqGXid_t : SvrReqCmd_s 
{
	SvrReqGXid_t() : SvrReqCmd_s(SVR_REQ_GXID), mGid(0),mXid(0) {}
	
	WORD mGid;
	WORD mXid;
};

//++++++++++++返回数据结构
const BYTE CMD_SVR_RES = 11;
struct SvrResCmd_s : public wCommand
{
	SvrResCmd_s(BYTE para)
	{
		mCmd = CMD_SVR_RES;
		mPara = para;
		mCode = 0;
	}
	short mCode;
};

//初始化返回
const BYTE SVR_RES_INIT = 0;
struct SvrResInit_t : SvrResCmd_s 
{
	SvrResInit_t() : SvrResCmd_s(SVR_RES_INIT) 
	{
		mNum = 0;
		memset(mSvr, 0, sizeof(mSvr));
	}
	int mNum;
	SvrNet_t mSvr[MAX_SVR_NUM];
};

//重载返回
const BYTE SVR_RES_RELOAD = 1;
struct SvrResReload_t : SvrResCmd_s 
{
	SvrResReload_t() : SvrResCmd_s(SVR_RES_RELOAD) 
	{
		mNum = 0;
		memset(mSvr, 0, sizeof(mSvr));
	}
	int mNum;
	SvrNet_t mSvr[MAX_SVR_NUM];
};

//同步返回
const BYTE SVR_RES_SYNC = 2;
struct SvrResSync_t : SvrResCmd_s 
{
	SvrResSync_t() : SvrResCmd_s(SVR_RES_SYNC) 
	{
		mNum = 0;
		memset(mSvr, 0, sizeof(mSvr));
	}
	int mNum;
	SvrNet_t mSvr[MAX_SVR_NUM];
};

//获取svr请求返回数据，包括all|id|name|gid/xid
const BYTE SVR_RES_DATA = 3;
struct SvrResData_t : SvrResCmd_s 
{
	SvrResData_t() : SvrResCmd_s(SVR_RES_DATA) 
	{
		mReqId = 0;
		mNum = 0;
		memset(mSvr, 0, sizeof(mSvr));
	}
	short mReqId;	//包括all|id|name|gid/xid的wCommand::mId
	int mNum;
	SvrNet_t mSvr[MAX_SVR_NUM];
};


/** 客户端(agentcmd) <=> 服务端消息通信 */
//++++++++++++请求数据结构
const BYTE CLI_SVR_REQ = 20;
struct SvrReqCli_s : public wCommand
{
	SvrReqCli_s(BYTE para)
	{
		mCmd = CLI_SVR_REQ;
		mPara = para;
	}
};

const BYTE SVR_SET_REQ_ID = 0;
struct SvrSetReqId_t : SvrReqCli_s 
{
	SvrSetReqId_t() : SvrReqCli_s(SVR_SET_REQ_ID)
	{
		mId = 0;
		mDisabled = 0;
		mWeight = 0;
		mTimeline = 0;
		mConnTime = 0;
		mTasks = 0;
		mSuggest = 0;
	}
	WORD mId;
	BYTE mDisabled;
	WORD mWeight;	//直接设置权重，1-100
	WORD mTimeline;	//本次处理暂用时间,单位秒
	WORD mConnTime; //连接时间,单位秒
	WORD mTasks;	//系统进程总数负载
	WORD mSuggest;	//建议参考值 1-100
};

const BYTE SVR_REPORT_REQ_ID = 1;
struct SvrReportReqId_t : SvrReqCli_s 
{
	SvrReportReqId_t() : SvrReqCli_s(SVR_REPORT_REQ_ID)
	{
		mId = 0;
		mDelay = 0;
		mStu = 0;
	}
	WORD mId;
	WORD mDelay;
	BYTE mStu;
};

//++++++++++++返回数据结构
const BYTE CLI_SVR_RES = 21;
struct SvrResCli_s : public wCommand
{
	SvrResCli_s(BYTE para)
	{
		mCmd = CLI_SVR_RES;
		mPara = para;
		mCode = 0;
	}
	short mCode;
};

const BYTE SVR_SET_RES_DATA = 0;
struct SvrSetResData_t : SvrResCli_s 
{
	SvrSetResData_t() : SvrResCli_s(SVR_SET_RES_DATA) {}
};

#pragma pack()

#endif