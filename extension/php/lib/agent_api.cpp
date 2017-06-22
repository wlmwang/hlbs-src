
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "agent_api.h"

struct postHandle_t g_handle = {NULL, false};

int QueryNode(struct SvrNet_t &svr, double timeout, std::string &err) {
	int ret = ConnectAgent();
	if (ret == kOk && g_handle.mConnecting == true && g_handle.mTask != NULL) {
		struct SvrReqGXid_t cmd;
		cmd.mSvr = svr;
		ssize_t size;
		// 查询请求
		if (g_handle.mTask->SyncSend(reinterpret_cast<char*>(&cmd), sizeof(cmd), &size).Ok()) {
			// 接受返回
			struct SvrOneRes_t res;
			if (g_handle.mTask->SyncRecv(reinterpret_cast<char*>(&res), &size, sizeof(res), timeout>0? timeout: kAgentTimeout).Ok()) {
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
	if (ret == kOk && g_handle.mConnecting == true && g_handle.mTask != NULL) {
		// 上报数据
		struct SvrReqReport_t cmd;
		cmd.mCaller = svr;
		cmd.mCaller.mReqRet = stat;
		cmd.mCaller.mReqUsetimeUsec = usec;

		ssize_t size;
		// 上报请求
		if (g_handle.mTask->SyncSend(reinterpret_cast<char*>(&cmd), sizeof(cmd), &size).Ok()) {
			// 接受返回
			struct SvrResReport_t res;
			if (g_handle.mTask->SyncRecv(reinterpret_cast<char*>(&res), &size, sizeof(res), kAgentTimeout).Ok()) {
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
	if (g_handle.mConnecting == true && g_handle.mTask != NULL) {
		return kOk;
	}
	CloseAgent();
	
	wSocket *sock;
	if (kAgentSocket == 0) {
		SAFE_NEW(wUnixSocket(kStConnect), sock);
	} else if (kAgentSocket == 1) {
		SAFE_NEW(wTcpSocket(kStConnect), sock);
	} else if (kAgentSocket == 2) {
		SAFE_NEW(wUdpSocket(kStConnect), sock);
	} else {
		sock = NULL;
	}
	
	if (sock && sock->Open().Ok()) {
		int64_t ret;
		SAFE_NEW(wTask(sock), g_handle.mTask);
		if (sock->Connect(&ret, kAgentHost, kAgentPort, kAgentTimeout).Ok()) {
			g_handle.mConnecting = true;
			return kOk;
		}
	}
	CloseAgent();
	return kConnError;
}

int CloseAgent() {
	SAFE_DELETE(g_handle.mTask);
	g_handle.mConnecting = false;
	return kOk;
}
