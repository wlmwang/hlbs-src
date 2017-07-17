
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wMisc.h"
#include "wDaemon.h"
#include "Define.h"
#include "AgentConfig.h"
#include "AgentServer.h"
#include "AgentMaster.h"

using namespace hnet;

int main(int argc, char *argv[]) {
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
	HNET_NEW(AgentConfig, config);
	if (!config) {
		std::cout << "config new failed" << std::endl;
		return -1;
	}
	
	// 解析xml配置文件
	if (config->ParseBaseConf() == -1) {
		std::cout << "parse config failed" << std::endl;
		HNET_DELETE(config);
		return -1;
	}

	// 解析命令行
	if (config->GetOption(argc, argv) == -1) {
		std::cout << "get configure failed" << std::endl;
		HNET_DELETE(config);
		return -1;
	}

	// 版本输出 && 守护进程创建
	bool vn, dn;
	if (config->GetConf("version", &vn) && vn) {
		std::cout << soft::GetSoftName() << soft::GetSoftVer() << std::endl;
		HNET_DELETE(config);
		return -1;
	} else if (config->GetConf("daemon", &dn) && dn) {
		std::string lock_path;
		config->GetConf("lock_path", &lock_path);
		wDaemon daemon;
		if (daemon.Start(lock_path) == -1) {
			std::cout << "create daemon failed" << std::endl;
			HNET_DELETE(config);
			return -1;
		}
	}

	// 创建服务器对象
	AgentServer* server;
	HNET_NEW(AgentServer(config), server);
	if (!server) {
		std::cout << "server new failed" << std::endl;
		HNET_DELETE(config);
		return -1;
	}

	// 创建master对象
	int ret = 0;
	AgentMaster* master;
	HNET_NEW(AgentMaster(hlbsName, server), master);
	if (master) {
		// 接受命令信号
	    std::string signal;
	    if (config->GetConf("signal", &signal) && signal.size() > 0) {
	    	ret = master->SignalProcess(signal);
	    } else {
	    	// 解析xml配置文件
			if (config->ParseRouterConf() == -1) {
				ret = -1;
			} else if (config->ParseQosConf() == -1) {
				ret = -1;
			}

			if (ret == 0) {
				ret = master->PrepareStart();
				if (ret == 0) {
					ret = master->MasterStart();
					if (ret == -1) {
						std::cout << "MasterStart failed" << std::endl;
					}
				} else {
					std::cout << "PrepareStart failed" << std::endl;
				}
			}
	    }
	}

	HNET_DELETE(config);
	HNET_DELETE(server);
	HNET_DELETE(master);
	return ret;
}
