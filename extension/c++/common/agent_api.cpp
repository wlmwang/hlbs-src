
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
		cmd.mGid = svr.mGid;
		cmd.mXid = svr.mXid;
		ssize_t size;
		// 查询请求
		if (g_handle.mTask->SyncSend(reinterpret_cast<char*>(&cmd), sizeof(struct SvrReqGXid_t), &size).Ok()) {
			// 接受返回
			char buff[kPackageSize];
			if (g_handle.mTask->SyncRecv(buff, &size, timeout>0? timeout: kAgentTimeout).Ok() && size == sizeof(struct SvrOneRes_t)) {
				struct SvrOneRes_t *res = reinterpret_cast<struct SvrOneRes_t*>(buff);
				if (res->mCode == 0 && res->mNum == 1 && res->mSvr.mPort > 0) {
					svr.mPort = res->mSvr.mPort;
					memcpy(svr.mHost, res->mSvr.mHost, kMaxHost);
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

int NotifyCallerRes(const struct SvrNet_t &svr, int res, uint64_t usec, std::string &err) {
	int ret = ConnectAgent();
	if (ret == kOk && g_handle.mConnecting == true && g_handle.mTask != NULL) {
		// 上报数据
		struct SvrReqReport_t cmd;
		memcpy(cmd.mCaller.mHost, svr.mHost, kMaxHost);
		cmd.mCaller.mCalledGid = svr.mGid;
		cmd.mCaller.mCalledXid = svr.mXid;
		cmd.mCaller.mPort = svr.mPort;
		cmd.mCaller.mReqRet = res;
		cmd.mCaller.mReqUsetimeUsec = usec;
		cmd.mCaller.mReqCount = 1;

		ssize_t size;
		// 上报请求
		if (g_handle.mTask->SyncSend(reinterpret_cast<char*>(&cmd), sizeof(struct SvrReqReport_t), &size).Ok()) {
			// 接受返回
			char buff[kPackageSize];
			if (g_handle.mTask->SyncRecv(buff, &size, kAgentTimeout).Ok() && size == sizeof(struct SvrResReport_t)) {
				struct SvrResReport_t *res = reinterpret_cast<struct SvrResReport_t*>(buff);
				if (res->mCode == 0) {
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
