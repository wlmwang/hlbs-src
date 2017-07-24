
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wLogger.h"
#include "RouterTcpTask.h"
#include "RouterConfig.h"
#include "SvrCmd.h"
#include "AgntCmd.h"
#include "Misc.h"

RouterTcpTask::RouterTcpTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_INIT, &RouterTcpTask::InitSvrReq, this);
	On(CMD_SVR_REQ, SVR_REQ_RELOAD, &RouterTcpTask::ReloadSvrReq, this);

	On(CMD_AGNT_REQ, AGNT_REQ_INIT, &RouterTcpTask::InitAgntReq, this);
	On(CMD_AGNT_REQ, AGNT_REQ_RELOAD, &RouterTcpTask::ReloadAgntReq, this);
	On(CMD_AGNT_RES, AGNT_RES_SYNC, &RouterTcpTask::SyncAgntRes, this);		// agent注册
}

int RouterTcpTask::Connect() {
	RouterConfig* config = Config<RouterConfig*>();
	struct Agnt_t agnt, old;
	std::string ip = FilterLocalIp(Socket()->Host());	// 客户端地址
	uint16_t port = 10005;

	memcpy(agnt.mHost, ip.c_str(), kMaxHost);
	agnt.mPort = port;

	// 更新本进程agent队列
	agnt.mStatus = kAgntUreg;
	if (config->IsExistAgnt(agnt, &old)) {
		agnt.mConfig = old.mConfig;
		agnt.mWeight = old.mWeight;
		if (old.mConfig == 0) {
			agnt.mStatus = kAgntOk;
		}
	}
	config->SaveAgnt(agnt);
	config->WriteFileAgnt();

	// 同步更新其他进程agent队列
	//AgntResSync_t vRRt;
	//vRRt.mCode = 0;
	//vRRt.mNum = 1;
	//vRRt.mAgnt[0] = agnt;
	//AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	return 0;
}

int RouterTcpTask::DisConnect() {
	RouterConfig* config = Config<RouterConfig*>();
	struct Agnt_t agnt, old;
	std::string ip = FilterLocalIp(Socket()->Host());
	uint16_t port = 10005;

	memcpy(agnt.mHost, ip.c_str(), kMaxHost);
	agnt.mPort = port;

	// 更新本进程agent队列
	agnt.mStatus = kAgntDisc;
	if (config->IsExistAgnt(agnt, &old)) {
		memcpy(agnt.mName, old.mName, kMaxName);
		agnt.mConfig = old.mConfig;
		agnt.mWeight = old.mWeight;
		if (agnt.mVersion < old.mVersion + 3) {	// 3秒之内认定agent抖动（agent重启也是抖动）
			return 0;
		}
	}
	config->SaveAgnt(agnt);
	config->WriteFileAgnt();

	// 同步更新其他进程agent队列
	//AgntResSync_t vRRt;
	//vRRt.mCode = 0;
	//vRRt.mNum = 1;
	//vRRt.mAgnt[0] = agnt;
	//AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	return 0;
}

// 向单个agent发送init回应（agent启动、重连时请求）
int RouterTcpTask::InitSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	
	std::string host = FilterLocalIp(Socket()->Host());	// 客户端agent地址
	if (!config->IsExistRlt(host)) {
		return 0;
	}

	// 获取路由节点
	struct SvrResInit_t vRRt;
	int32_t start = 0, i = 0;
	while (true) {
		vRRt.mNum += config->Qos()->GetNodeAll(vRRt.mSvr, i, start, kMaxNum - i);
		if (vRRt.mNum <= 0) {
			break;
		}
		start += kMaxNum;
		i = vRRt.mNum;

		// 过滤svr节点
		i = config->Qos()->FilterSvrBySid(vRRt.mSvr, vRRt.mNum, config->Rlts(host));
		
		if (i < vRRt.mNum && vRRt.mNum < kMaxNum) {	// 至少过滤了一个svr节点
			vRRt.mNum = i;
			continue;
		}
		vRRt.mNum = i;

		if (vRRt.mNum >= kMaxNum) {
			AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));

			vRRt.mCode++;
			vRRt.mNum = i = 0;
			continue;
		}
		break;
	}

	// 扫尾
	if (vRRt.mNum > 0) {
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	}
	return 0;
}

// 向单个agent发送reload响应（讲道理agent不会有此权限）
int RouterTcpTask::ReloadSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	
	// 重新加载配置文件
	config->Qos()->CleanNode();
	config->ParseSvrConf();
	
	std::string host = FilterLocalIp(Socket()->Host());	// 客户端agent地址
	if (!config->IsExistRlt(host)) {
		return 0;
	}

	// 获取路由节点
	struct SvrResReload_t vRRt;
	int32_t start = 0, i = 0;
	while (true) {
		vRRt.mNum += config->Qos()->GetNodeAll(vRRt.mSvr, i, start, kMaxNum - i);
		if (vRRt.mNum <= 0) {
			break;
		}
		start += kMaxNum;
		i = vRRt.mNum;

		// 过滤svr节点
		i = config->Qos()->FilterSvrBySid(vRRt.mSvr, vRRt.mNum, config->Rlts(host));
		
		if (i < vRRt.mNum && vRRt.mNum < kMaxNum) {	// 至少过滤了一个svr节点
			vRRt.mNum = i;
			continue;
		}
		vRRt.mNum = i;
		
		if (vRRt.mNum >= kMaxNum) {
			AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));

			vRRt.mCode++;
			vRRt.mNum = i = 0;
			continue;
		}
		break;
	}

	// 扫尾
	if (vRRt.mNum > 0) {
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	}
	return 0;
}

// agent增量更新时请求（agent注册）
int RouterTcpTask::SyncAgntRes(struct Request_t *request) {
	struct AgntResSync_t* cmd = reinterpret_cast<struct AgntResSync_t*>(request->mBuf);
	RouterConfig* config = Config<RouterConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		for (int32_t i = 0; i < cmd->mNum; i++) {
			struct Agnt_t old;
			if (cmd->mAgnt[i].mStatus == kAgntUreg && config->IsExistAgnt(cmd->mAgnt[i], &old) && old.mConfig == 0) {
				cmd->mAgnt[i].mStatus = kAgntOk;
				cmd->mAgnt[i].mConfig = old.mConfig;
			}
			config->SaveAgnt(cmd->mAgnt[i]);
			config->WriteFileAgnt();
		}
		//AsyncWorker(request->mBuf, request->mLen);	// 同步其他进程
	}
	return 0;
}

// ignore
// agent v3.0.8以前版本，启动、重连时请求
// 向单个agent发送init响应
int RouterTcpTask::InitAgntReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	int32_t start = 0;
	struct AgntResInit_t vRRt;
	do {
		vRRt.mNum = config->GetAgntAll(vRRt.mAgnt, start, kMaxNum);
		if (vRRt.mNum <= 0) {
			break;
		}
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
		vRRt.mCode++;
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);
	return 0;
}

// ignore
// 向单个agent发送reload响应（讲道理agent不会有此权限）
int RouterTcpTask::ReloadAgntReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	// 重新加载配置文件
	config->CleanAgnt();
	config->ParseAgntConf();
	
	int32_t start = 0;
	struct AgntResReload_t vRRt;
	do {
		vRRt.mNum = config->GetAgntAll(vRRt.mAgnt, start, kMaxNum);
		if (vRRt.mNum <= 0) {
			break;
		}
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
		vRRt.mCode++;
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);
	return 0;
}
