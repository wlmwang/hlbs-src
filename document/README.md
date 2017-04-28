# 概要

HLBS全称为Hupu Load Balance System。顾名思义，它是一个负载均衡系统（动态），且带有过载保护功能。该系统致力于在使用方（PHP、C/C++客户端）与服务（SOA服务）方之间提供一个快速可靠的通路索引查询器。

**HLBS系统由RouterSvr、AgentSvr两部分组件组成**；同时随包发放有：~~AgentCmd组件（命令行工具）~~、C++客户端、C++压测工具、PHP扩展。


# 使用步骤

1. 启动RouterSvr服务器，编辑config/svr.xml文件，添加SVR记录。
2. 修改AgentSvr服务器配置文件config/router.xml将其指向可用的RouterSvr服务器地址，启动AgentSvr服务器。

3. 使用方（PHP、C/C++客户端）从AgentSvr获取某一类(GID、XID)服务的HOST/PORT。
4. 使用方使用得到的目标主机HOST、PORT进行直连访问。
5. 访问结束后，使用方将本次访问结果（成功与否、微妙时延）上报至AgentSvr。


```
HLBS服务方式极其类似于DNS。使用者凭借GID、XID（域名），请求AgentSvr（本地DNS服务器）获取目标主机地址（HOST、PORT），然后使用者直连目标主机获取服务；RouterSvr则充当为权威DNS服务器。

当然相比DNS区别也很明显：
1. HLBS的AgentSvr服务器上的所有SVR记录（域名记录），与RouterSvr服务器是强制同步的（AgentSvr会有一份SVR记录拷贝），这就为它能成为权威域名服务器提供了保障。即使用方在获取目标主机地址时，只会查找AgentSvr（本地DNS服务器）服务器上SVR记录（域名记录）。查到返回，否则出错。
2. 使用方在与目标主机进行通信后，需上报本次访问结果（成功与否，微妙时延）上报至AgentSvr。为容错系统提供保障。
3. 当故障机器恢复服务后，AgentSvr将自动将该SVR记录重新加入索引列表。
```

# 文档约定

CGI：通过HLBS系统获取后端服务来使用的前端业务。相对Svr属于消费者。

Svr：配置在HLBS系统中的后端SOA服务。相对Svr属于生产者（主机ip、port、weight等配置在svr.xml文件中）。

Qos：服务质量。标识各Svr负载大小、服务能力。容错体系的关键。
