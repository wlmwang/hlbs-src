
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wMisc.h"
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
		ret = NotifyCallerRes(svr, 0, 2000, s);
		std::cout << "ret:" << ret << std::endl;
		return ret;
	}
	return ret;
}

int main(int argc, char *argv[]) {
	int64_t start_usec = misc::GetTimeofday();

	int number = 10000, errNum = 0;
	for (int i = 0; i < number; i++) {
		if (GetReportSvr() != kOk) {
			errNum++;
		}
	}
	int64_t total_usec = (misc::GetTimeofday() - start_usec)/1000000;

	std::cout << "error:" << errNum << std::endl;
	std::cout << "second:" << total_usec << std::endl;
	std::cout << "TPS:" << number/total_usec << std::endl;
	return 0;
}
