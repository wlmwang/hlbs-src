
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "AgentConfig.h"
#include "AgentServer.h"
#include "AgentMaster.h"

using namespace hnet;

int main(int argc, const char *argv[]) {

	// 创建配置对象
	AgentConfig *config;
	SAFE_NEW(AgentConfig, config);
	if (config == NULL) {
		return -1;
	}
	wStatus s;

	// 解析命令行
	s = config->GetOption(argc, argv);
	if (!s.Ok()) {
		std::cout << s.ToString() << std::endl;
		return -1;
	}

	// 版本输出 && 守护进程创建
	bool version, daemon;
	if (config->GetConf("version", &version) && version == true) {
		std::cout << kSoftwareName << kSoftwareVer << std::endl;
		return -1;
	} else if (config->GetConf("daemon", &daemon) && daemon == true) {
		std::string lock_path;
		config->GetConf("lock_path", &lock_path);
		if (!misc::InitDaemon(lock_path).Ok()) {
			std::cout << "create daemon failed" << std::endl;
			return -1;
		}
	}

	// 解析xml配置文件
	if (!config->ParseBaseConf().Ok()) {
		return -1;
	} else if (!config->ParseRouterConf().Ok()) {
		return -1;
	} else if (!config->ParseQosConf().Ok()) {
		return -1;
	}

	// 创建服务器对象
	AgentServer* server;
	SAFE_NEW(AgentServer(config), server);
	if (server == NULL) {
		return -1;
	}

	// 创建master对象
	AgentMaster* master;
	SAFE_NEW(AgentMaster("HLBS(agent)", server), master);
	if (master != NULL) {
		// 接受命令信号
	    std::string signal;
	    if (config->GetConf("signal", &signal) && signal.size() > 0) {
	    	if (master->SignalProcess(signal).Ok()) {
	    		return 0;
	    	} else {
	    		return -1;
	    	}
	    } else {
	    	// 准备服务器
			s = master->PrepareStart();
			if (s.Ok()) {
				// Master-Worker方式开启服务器
				master->MasterStart();
			} else {
				return -1;
			}
	    }
	}
	return 0;
}
