
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_API_H
#define _AGENT_API_H

#include "wCore.h"
#include "Common.h"
#include "Svr.h"
#include "SvrCmd.h"

#include "wShm.h"
#include "wMsgQueue.h"

struct post_handle_t
{
	wShm *shm;
	wMsgQueue *queue;
};
//extern struct post_handle_t g_handle;

/** 申请提交agent地址或共享内存 */
int BeforePost(struct post_handle_t *handle);

/** 单次获取路由 */
int QueryNode(int iGid, int iXid, char* vHost, int* pPort, int *pWeight);

/** 调用结果上报 */
int NotifyCallerRes(int iGid, int iXid, const char* vHost, int iPort, int iReqRet, int iReqCount);

/** 调用数上报 */
int NotifyCaller(int iGid, int iXid, const char* vHost, int iPort, int iReqCount);

/** 提交到agentsvr */
int PostAgentSvr(SvrReqReport_t *pReport);

#endif
