
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

	if (ret == kOk)	{
		ret = NotifyCallerRes(svr, 0, 2000, s);
		return ret;
	}
	return ret;
}

void Handle(int i, int request) {
	int64_t start_usec = misc::GetTimeofday();

	int errNum = 0;
	for (int i = 0; i < request; i++) {
		if (GetReportSvr() != kOk) {
			errNum++;
		}
	}
	int64_t total_usec = (misc::GetTimeofday() - start_usec)/1000000;
	std::cout << i << ":error:" << errNum << std::endl;
	std::cout << i << ":second:" << total_usec << std::endl;
	exit(errNum);
}

pid_t SpawnProcess(int i, int request) {
	pid_t pid = fork();

	switch (pid) {
	case -1:
		assert("error child");
		break;

	case 0:
		Handle(i, request);
		break;
	}
	return pid;
}

int main(int argc, char *argv[]) {
	int64_t start_usec = misc::GetTimeofday();

	const int request = 10000;
	const int worker = 10;
	pid_t process[worker];

	// 创建进程
	for (int i = 0; i < worker; i++) {
		pid_t pid = SpawnProcess(i, request);
		std::cout << "fork children:" << i << "|" << pid << std::endl;
		if (pid > 0) {
			process[i] = pid;
		}
	}

	// 回收进程
	int status, ret;
	for (int i = 0; i < worker; i++) {
		std::cout << "wait children:" << i << "|" << process[i] << std::endl;
		if (process[i] > 0) {
			ret = waitpid(process[i], &status, 0);
		}
		process[i] = -1;
	}

	int64_t total_usec = (misc::GetTimeofday() - start_usec)/1000000;
	std::cout << "[second]:" << total_usec << std::endl;
	std::cout << "[quest per second]:" << request*worker/total_usec << std::endl;
	return 0;
}
