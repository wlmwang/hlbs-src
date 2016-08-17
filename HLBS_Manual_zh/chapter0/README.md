# 文档约定

	CGI：通过HLBS系统获取后端服务来使用的前端业务。相对Svr属于消费者。

	Svr：配置在HLBS系统中的后端服务。相对Svr属于生产者（主机ip、port、weight等配置在svr.xml文件中）。

	Qos：服务质量。标识各Svr负载大小、服务能力。容错体系的关键。

# 简介

    HLBS为Hupu Load Balance System的英文缩写。

    该系统致力于为CGI在使用某一（GID、XID）Svr时，提供负载均衡与容错保护。

    为实现该目标，系统为所有Svr实现按类别（GID、XID）的动态负载均衡和过载保护功能。