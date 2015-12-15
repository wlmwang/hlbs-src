
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_ROUTER_COMMAND_H_
#define _W_ROUTER_COMMAND_H_

#include <string.h>
#include "wCommand.h"
#include "Rtbl.h"

#pragma pack(1)

//请求数据结构
const BYTE CMD_RTBL_REQ = 10;
struct RtblReqCmd_s : public wCommand
{
	RtblReqCmd_s(BYTE para)
	{
		mCmd = CMD_RTBL_REQ;
		mPara = para;
	}
};

const BYTE RTBL_REQ_ALL = 0;
struct RtblReqAll_t : RtblReqCmd_s 
{
	RtblReqAll_t() : RtblReqCmd_s(RTBL_REQ_ALL) {}
};

const BYTE RTBL_REQ_ID = 1;
struct RtblReqId_t : RtblReqCmd_s 
{
	RtblReqId_t() : RtblReqCmd_s(RTBL_REQ_ID), mId(0) {}
	
	WORD mId;
};

const BYTE RTBL_REQ_GID = 2;
struct RtblReqGid_t : RtblReqCmd_s 
{
	RtblReqGid_t() : RtblReqCmd_s(RTBL_REQ_GID), mGid(0) {}
	
	WORD mGid;
};

const BYTE RTBL_REQ_NAME = 3;
struct RtblReqName_t : RtblReqCmd_s 
{
	RtblReqName_t() : RtblReqCmd_s(RTBL_REQ_NAME) 
	{
		memset(mName, 0, 64);
	}
	
	char mName[64];
};

const BYTE RTBL_REQ_GXID = 4;
struct RtblReqGXid_t : RtblReqCmd_s 
{
	RtblReqGXid_t() : RtblReqCmd_s(RTBL_REQ_GXID), mGid(0),mXid(0) {}
	
	WORD mGid;
	WORD mXid;
};

//返回数据结构
const BYTE CMD_RTBL_RES = 11;
struct RtblResCmd_s : public wCommand
{
	RtblResCmd_s(BYTE para)
	{
		mCmd = CMD_RTBL_RES;
		mPara = para;
	}
};

const BYTE RTBL_RES_DATA = 0;
struct RtblResData_t : RtblResCmd_s 
{
	RtblResData_t() : RtblResCmd_s(RTBL_RES_DATA) 
	{
		mNum = 0;
		memset(mRtbl, 0, 256);
	}

	Rtbl_t mRtbl[256];
	int mNum;
};

/** 客户端设置rtbl数据结构 */
//请求数据结构
const BYTE CMD_RTBL_SET_REQ = 12;
struct RtblSetReqCmd_s : public wCommand
{
	RtblSetReqCmd_s(BYTE para)
	{
		mCmd = CMD_RTBL_SET_REQ;
		mPara = para;
	}
};

const BYTE RTBL_SET_REQ_ID = 1;
struct RtblSetReqId_t : RtblSetReqCmd_s 
{
	RtblSetReqId_t() : RtblSetReqCmd_s(RTBL_SET_REQ_ID)
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

//返回数据结构
const BYTE CMD_RTBL_SET_RES = 13;
struct RtblSetResCmd_s : public wCommand
{
	RtblSetResCmd_s(BYTE para)
	{
		mCmd = CMD_RTBL_SET_RES;
		mPara = para;
	}
};

const BYTE RTBL_SET_RES_DATA = 1;
struct RtblSetResData_t : RtblSetResCmd_s 
{
	RtblSetResData_t() : RtblSetResCmd_s(RTBL_SET_REQ_ID)
	{
		mId = 0;
		mRes = 0;
	}
	
	WORD mId;
	BYTE mRes;
};

#pragma pack()

#endif