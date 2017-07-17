
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _COMMON_MISC_H_
#define _COMMON_MISC_H_

#include "wCore.h"

// 过滤router与agent部署在同一台机器的特殊情况（返回本机第一个不是127.0.0.1网卡地址）
inline std::string FilterLocalIp(const std::string& ip) {
	std::string host = ip;
	if (misc::Text2IP(ip.c_str()) == misc::Text2IP("127.0.0.1") || misc::Text2IP(ip.c_str()) == misc::Text2IP("0.0.0.0")) {
		std::vector<unsigned int> ips;
		if (misc::GetIpList(ips) == 0) {
			std::sort(ips.begin(), ips.end());
			for (size_t i = 0; i < ips.size(); i++) {
				if (ips[i] != misc::Text2IP("127.0.0.1")) {
					host = misc::IP2Text(ips[i]);
					break;
				}
			}
		}
	}
	return host;
}

#endif
