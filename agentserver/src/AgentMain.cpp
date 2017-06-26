
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "Define.h"
#include "AgentConfig.h"
#include "AgentServer.h"
#include "AgentMaster.h"

using namespace hnet;

int main(int argc, const char *argv[]) {
	// 设置相关相关配置
	std::string hlbsName = kHlbsSoftwareName + std::string("(*agent*)");
	soft::SetSoftName(hlbsName + " - ");
	soft::SetSoftVer(kHlbsSoftwareVer);
	soft::SetLockPath(kHlbsLockPath);
	soft::SetPidPath(kHlbsPidPath);
	soft::SetLogPath(kHlbsLogPath);
	soft::SetAcceptmtxPath(kHlbsAcceptmtxPath);
	
	// 设置运行目录
	if (misc::SetBinPath() == -1) {
		std::cout << "set bin path failed" << std::endl;
		return -1;
	}

	// 创建配置对象
	AgentConfig *config;
	SAFE_NEW(AgentConfig, config);
	if (config == NULL) {
		return -1;
	}
	
	// 解析xml配置文件
	if (config->ParseBaseConf() == -1) {
		std::cout << "parse config failed" << std::endl;
		SAFE_DELETE(config);
		return -1;
	}

	// 解析命令行
	if (config->GetOption(argc, argv) == -1) {
		std::cout << "get configure failed" << std::endl;
		SAFE_DELETE(config);
		return -1;
	}

	// 版本输出 && 守护进程创建
	bool version, daemon;
	if (config->GetConf("version", &version) && version == true) {
		std::cout << soft::GetSoftName() << soft::GetSoftVer() << std::endl;
		SAFE_DELETE(config);
		return -1;
	} else if (config->GetConf("daemon", &daemon) && daemon == true) {
		std::string lock_path;
		config->GetConf("lock_path", &lock_path);
		if (misc::InitDaemon(lock_path) == -1) {
			std::cout << "create daemon failed" << std::endl;
			SAFE_DELETE(config);
			return -1;
		}
	}

	// 创建服务器对象
	AgentServer* server;
	SAFE_NEW(AgentServer(config), server);
	if (server == NULL) {
		SAFE_DELETE(config);
		return -1;
	}

	// 创建master对象
	int ret = 0;
	AgentMaster* master;
	SAFE_NEW(AgentMaster(hlbsName, server), master);
	if (master != NULL) {
		// 接受命令信号
	    std::string signal;
	    if (config->GetConf("signal", &signal) && signal.size() > 0) {
	    	if (!master->SignalProcess(signal).Ok()) {
	    		ret = -1;
	    	}
	    } else {
	    	// 解析xml配置文件
			if (config->ParseRouterConf() == -1) {
				ret = -1;
			} else if (config->ParseQosConf() == -1) {
				ret = -1;
			}

	    	// 准备服务器
			if (ret == 0 && master->PrepareStart().Ok()) {
				master->MasterStart();	// Master-Worker方式开启服务器
			}
	    }
	}
	SAFE_DELETE(config);
	SAFE_DELETE(server);
	SAFE_DELETE(master);

	return 0;
}
