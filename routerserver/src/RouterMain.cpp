
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wMisc.h"
#include "wEnv.h"
#include "Define.h"
#include "RouterConfig.h"
#include "RouterServer.h"
#include "RouterMaster.h"

using namespace hnet;

int main(int argc, const char *argv[]) {
	// 设置运行目录
	if (misc::SetBinPath() == -1) {
		std::cout << "set bin path failed" << std::endl;
		return -1;
	}

	// 设置相关相关配置
	std::string hlbsName = kHlbsSoftwareName + std::string("(*router*)");
	soft::SetSoftName(hlbsName + " - ");
	soft::SetSoftVer(kHlbsSoftwareVer);
	soft::SetLockPath(kHlbsLockPath);
	soft::SetPidPath(kHlbsPidPath);
	soft::SetLogPath(kHlbsLogPath);

	RouterConfig *config;
	SAFE_NEW(RouterConfig, config);
	if (config == NULL) {
		return -1;
	}
	
	// 解析xml配置文件
	if (config->ParseBaseConf() == -1) {
		std::cout << "parse config failed" << std::endl;
		return -1;
	}

	// 解析命令行
	if (config->GetOption(argc, argv) == -1) {
		std::cout << "get configure failed" << std::endl;
		return -1;
	}

	// 版本输出 && 守护进程创建
	bool version, daemon;
	if (config->GetConf("version", &version) && version == true) {
		std::cout << soft::GetSoftName() << soft::GetSoftVer() << std::endl;
		return -1;
	} else if (config->GetConf("daemon", &daemon) && daemon == true) {
		std::string lock_path;
		config->GetConf("lock_path", &lock_path);
		if (misc::InitDaemon(lock_path) == -1) {
			std::cout << "create daemon failed" << std::endl;
			return -1;
		}
	}

	// 创建服务器对象
	RouterServer* server;
	SAFE_NEW(RouterServer(config), server);
	if (server == NULL) {
		return -1;
	}

	// 创建master对象
	RouterMaster* master;
	SAFE_NEW(RouterMaster(hlbsName, server), master);
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
	    	// 解析xml配置文件
			if (config->ParseSvrConf() == -1) {
				return -1;
			} else if (config->ParseQosConf() == -1) {
				return -1;
			} else if (config->ParseAgntConf() == -1) {
				return -1;
			}
			
	    	// 准备服务器
			if (master->PrepareStart().Ok()) {
				// Master-Worker方式开启服务器
				// 考虑到router服务压力很小因素，故让其单进程运行。RouterMaster::mWorkerNum=1
				// 单进程模式RouterMaster::SingleStart()，目前版本（HNET0.0.2）并不完善
				master->MasterStart();
			} else {
				return -1;
			}
	    }
	}
	return 0;
}
