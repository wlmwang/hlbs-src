
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_HTTP_TASK_H_
#define _ROUTER_HTTP_TASK_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wHttpTask.h"
#include "SvrCmd.h"
#include "AgntCmd.h"

using namespace hnet;

//+++
const uint8_t CMD_SVR_HTTP = 70;

const uint8_t SVR_HTTP_RELOAD = 0;	// 重载SVR记录(重新读取svr.xml配置文件信息)
const uint8_t SVR_HTTP_SAVE = 1;	// 更新SVR记录（添加、删除、修改SVR记录，并发布变化SVR）	# 增量下发
const uint8_t SVR_HTTP_COVER = 2;	// 重写SVR记录（清除所有已有SVR，保留设置为本请求SVR记录）	# 全量下发
const uint8_t SVR_HTTP_LIST = 3;	// 查找所有SVR记录

//+++
const uint8_t CMD_AGNT_HTTP = 71;

const uint8_t AGNT_HTTP_RELOAD = 0;	// agent重载
const uint8_t AGNT_HTTP_SAVE = 1;	// agent更新
const uint8_t AGNT_HTTP_COVER = 2;	// agent重写
const uint8_t AGNT_HTTP_LIST = 3;	// agent列表

//+++
const uint8_t CMD_MASTER_HTTP = 72;
const uint8_t MASTER_HTTP_RELOAD = 0;
const uint8_t MASTER_HTTP_RESTART = 1;
const uint8_t MASTER_HTTP_STOP = 2;
const uint8_t MASTER_HTTP_INFO = 3;

class RouterHttpTask : public wHttpTask {
public:
	RouterHttpTask(wSocket *socket, int32_t type);

	int ReloadSvrReq(struct Request_t *request);
	int SaveSvrReq(struct Request_t *request);
	int CoverSvrReq(struct Request_t *request);
	int ListSvrReq(struct Request_t *request);
	
	int ReloadAgntReq(struct Request_t *request);
	int SaveAgntReq(struct Request_t *request);
	int CoverAgntReq(struct Request_t *request);
	int ListAgntReq(struct Request_t *request);
	
	int MasterStartReq(struct Request_t *request);
	int MasterRestartReq(struct Request_t *request);
	int MasterReloadReq(struct Request_t *request);
	int MasterStopReq(struct Request_t *request);
	int MasterInfoReq(struct Request_t *request);

protected:
	int32_t ParseJsonSvr(const std::string& svrs, struct SvrNet_t** svr);
	int32_t ParseJsonAgnt(const std::string& agnts, struct Agnt_t** agnt);
};

#endif
