
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <vector>
#include <algorithm>
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

	int num = 0;
	for (int i = 0; i < request; i++) {
		if (GetReportSvr() != kOk) {
			num++;
		}
	}

	int64_t total_usec = misc::GetTimeofday() - start_usec;
	std::cout << getpid() << "|" << "[error]	: " << num << std::endl;
	std::cout << getpid() << "|" << "[second]	: " << total_usec << "us" << std::endl;

	exit(num);
}

pid_t SpawnProcess(int i, int request) {
	pid_t pid = fork();

	switch (pid) {
	case -1:
		exit(-1);
		break;

	case 0:
		Handle(i, request);
		break;
	}
	return pid;
}

int main(int argc, char *argv[]) {
	// 开始微妙时间
	int64_t start_usec = misc::GetTimeofday();

	const int worker = 10;
	const int request = 5000;

	// 创建进程
	std::vector<pid_t> process(worker);
	for (int i = 0; i < worker; i++) {
		pid_t pid = SpawnProcess(i, request);
		if (pid > 0) {
			std::cout << "fork children:" << i << "|" << pid << std::endl;
			process[i] = pid;
		}
	}

	int error = 0;

	// 回收进程
	int status;
	while (!process.empty()) {
		pid_t pid = wait(&status);

		// 是否有错误请求
		if (WIFEXITED(status) != 0) {
			if (WEXITSTATUS(status) > 0) {
				error += WEXITSTATUS(status);
			}
		}

		std::vector<pid_t>::iterator it = std::find(process.begin(), process.end(), pid);
		if (it != process.end()) {
			std::cout << "recycle children:" << pid << std::endl;
			process.erase(it);
		}
	}

	int64_t total_usec = (misc::GetTimeofday() - start_usec)/1000000;

	std::cout << "[error]	:	" << error << std::endl;
	std::cout << "[success]	:	" << request*worker - error << std::endl;
	std::cout << "[second]	:	" << total_usec << "s" << std::endl;
	std::cout << "[qps]		:	" << request*worker/total_usec << "req/s" << std::endl;
	return 0;
}
