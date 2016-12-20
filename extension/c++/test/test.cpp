
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "agent_api.h"

int GetReportSvr() {
	struct SvrNet_t svr;
	svr.mGid = 1;
	svr.mXid = 2;

	std::string s;
	int ret = QueryNode(svr, 30, s);

	std::cout << "ret:" << ret << std::endl;
	std::cout << "host:" << svr.mHost << std::endl;
	std::cout << "port:" << svr.mPort << std::endl;

	if (ret == kOk)	{
		return NotifyCallerRes(svr, 0, 2000, s);
	}
	return -1;
}

int main(int argc, char *argv[]) {
	int errNum = 0;
	for (int i = 0; i < 1; i++) {
		if (GetReportSvr() < 0) {
			errNum++;
		}
	}
	std::cout << "errNum:" << errNum << std::endl;

	return 0;
}
