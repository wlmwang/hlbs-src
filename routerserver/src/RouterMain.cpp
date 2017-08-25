
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wMisc.h"
#include "wDaemon.h"
#include "wEnv.h"
#include "Define.h"
#include "RouterConfig.h"
#include "RouterServer.h"
#include "RouterMaster.h"

using namespace hnet;

int main(int argc, char *argv[]) {
	// 设置运行目录
	if (misc::SetBinPath() == -1) {
		std::cout << "set bin path failed" << std::endl;
		return -1;
	}

	RouterConfig *config;
	HNET_NEW(RouterConfig, config);
	if (!config) {
		std::cout << "config new failed" << std::endl;
		return -1;
	}

	// 解析命令行
	if (config->GetOption(argc, argv) == -1) {
		std::cout << "get configure failed" << std::endl;
		HNET_DELETE(config);
		return -1;
	}

	// 日志路径
	std::string log_path;
	if (config->GetConf("log_path", &log_path)) {
		soft::SetLogdirPath(log_path);
	}

	// 相对目录路径
	std::string runtime_path;
	if (config->GetConf("runtime_path", &runtime_path)) {
		soft::SetRuntimePath(runtime_path);
	}

	// 设置相关相关配置
	std::string hlbsName = kHlbsSoftwareName + std::string("(*router*)");
	soft::SetSoftName(hlbsName + " - ");
	soft::SetSoftVer(kHlbsSoftwareVer);
	soft::SetAcceptFilename(kHlbsAcceptFilename);
	soft::SetLockFilename(kHlbsLockFilename);
	soft::SetPidFilename(kHlbsPidFilename);
	soft::SetLogFilename(kHlbsLogFilename);

	// 解析xml配置文件
	if (config->ParseBaseConf() == -1) {
		std::cout << "parse config failed" << std::endl;
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
		wDaemon daemon;
		if (daemon.Start(soft::GetLockPath()) == -1) {
			std::cout << "create daemon failed" << std::endl;
			HNET_DELETE(config);
			return -1;
		}
	}

	// 创建服务器对象
	RouterServer* server;
	HNET_NEW(RouterServer(config), server);
	if (!server) {
		std::cout << "server new failed" << std::endl;
		HNET_DELETE(config);
		return -1;
	}

	// 创建master对象
	int ret = 0;
	RouterMaster* master;
	HNET_NEW(RouterMaster(hlbsName, server), master);
	if (master) {
	    std::string signal;
	    if (config->GetConf("signal", &signal) && signal.size() > 0) {	// 接受命令信号
	    	ret = master->SignalProcess(signal);
	    } else {
	    	// 解析xml配置文件
			if (config->ParseSvrConf() == -1) {
				ret = -1;
			} else if (config->ParseQosConf() == -1) {
				ret = -1;
			} else if (config->ParseAgntConf() == -1) {
				ret = -1;
			} else if (config->ParseRltConf() == -1) {
				ret = -1;
			}
			
	    	// 准备服务器
			if (ret == 0) {
				ret = master->PrepareStart();
				if (ret == 0) {
					ret = master->MasterStart();	// Master-Worker方式开启服务器
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
