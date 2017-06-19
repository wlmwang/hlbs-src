
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wMisc.h"
#include "agent_api.h"

int GetReportSvr() {
	struct SvrNet_t svr;
	svr.mGid = 1;
	svr.mXid = 1;

	std::string s;
	int ret = QueryNode(svr, 10, s);

	std::cout << "ret:" << ret << std::endl;
	std::cout << "host:" << svr.mHost << std::endl;
	std::cout << "port:" << svr.mPort << std::endl;

	if (ret == kOk)	{
		ret = NotifyCallerRes(svr, 0, 5000, s);
		std::cout << "ret:" << ret << std::endl;
		return ret;
	}
	return ret;
}

int main(int argc, char *argv[]) {
	int64_t start_usec = misc::GetTimeofday();
	int request = 1, err = 0;
	for (int i = 0; i < request; i++) {
		if (GetReportSvr() != kOk) {
			err++;
		}
	}
	int64_t total_usec = misc::GetTimeofday() - start_usec;

	std::cout << "[error]	:	" << err << std::endl;
	std::cout << "[success]	:	" << request - err << std::endl;
	std::cout << "[second]	:	" << total_usec << std::endl;
	return 0;
}
