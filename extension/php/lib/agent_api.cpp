
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "agent_api.h"

struct postHandle_t g_handle = {NULL, false};

int QueryNode(struct SvrNet_t &svr, double timeout, std::string &err) {
	int ret = ConnectAgent();
	if (ret == kOk && g_handle.mConnecting && g_handle.mTask) {
		struct SvrReqGXid_t cmd;
		cmd.mSvr = svr;
		ssize_t size;
		// 查询请求
		if (g_handle.mTask->SyncSend(reinterpret_cast<char*>(&cmd), sizeof(cmd), &size) == 0) {
			// 接受返回
			struct SvrOneRes_t res;
			if (g_handle.mTask->SyncRecv(reinterpret_cast<char*>(&res), &size, sizeof(res), timeout>0? timeout: kAgentTimeout) == 0) {
				if (res.mCode == 0 && res.mNum == 1 && res.mSvr.mPort > 0) {
					svr.mPort = res.mSvr.mPort;
					memcpy(svr.mHost, res.mSvr.mHost, kMaxHost);
					ret = kOk;
				} else {
					ret = kDataError;
				}
			} else {
				ret = kRecvError;
			}
		} else {
			ret = kSendError;
		}
	} else {
		ret = kConnError;
	}

	CloseAgent();
	return ret;
}

int NotifyCallerRes(const struct SvrNet_t &svr, int stat, uint64_t usec, std::string &err) {
	int ret = ConnectAgent();
	if (ret == kOk && g_handle.mConnecting && g_handle.mTask) {
		// 上报数据
		struct SvrReqReport_t cmd;
		cmd.mCaller = svr;
		cmd.mCaller.mReqRet = stat;
		cmd.mCaller.mReqUsetimeUsec = usec;
		ssize_t size;
		// 上报请求
		if (g_handle.mTask->SyncSend(reinterpret_cast<char*>(&cmd), sizeof(cmd), &size) == 0) {
			// 接受返回
			struct SvrResReport_t res;
			if (g_handle.mTask->SyncRecv(reinterpret_cast<char*>(&res), &size, sizeof(res), kAgentTimeout) == 0) {
				if (res.mCode == 0) {
					ret = kOk;
				} else {
					ret = kDataError;
				}
			} else {
				ret = kRecvError;
			}
		} else {
			ret = kSendError;
		}
	} else {
		ret = kConnError;
	}

	CloseAgent();
	return ret;
}

int ConnectAgent() {
	// 已连接
	if (g_handle.mConnecting && g_handle.mTask) {
		return kOk;
	}
	CloseAgent();
	
	wSocket *sock = NULL;
	if (kAgentSocket == 0) {
		HNET_NEW(wUnixSocket(kStConnect), sock);
	} else if (kAgentSocket == 1) {
		HNET_NEW(wTcpSocket(kStConnect), sock);
	} else if (kAgentSocket == 2) {
		HNET_NEW(wUdpSocket(kStConnect), sock);
	}
	
	if (sock && sock->Open() == 0) {
		HNET_NEW(wTask(sock), g_handle.mTask);
		if (sock->Connect(kAgentHost, kAgentPort, kAgentTimeout) == 0) {
			g_handle.mConnecting = true;
			return kOk;
		}
	}
	CloseAgent();
	return kConnError;
}

int CloseAgent() {
	HNET_DELETE(g_handle.mTask);
	g_handle.mConnecting = false;
	return kOk;
}
