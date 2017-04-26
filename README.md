# 概要

HLBS全称为Hupu Load Balance System。顾名思义，它是一个负载均衡系统（动态），且带有过载保护功能。该系统致力于在使用方（PHP、C/C++客户端）与服务（SOA服务）方之间提供一个快速可靠的通路索引查询器。

**HLBS系统由routersvrd、agentsvrd两部分组件组成**，~~同时随包发放的还有AgentCmd组件（命令行工具）~~、C++客户端、C++压测工具、PHP扩展。


# 使用步骤

1. 使用方（PHP、C/C++客户端）从agentsvrd获取某一类(GID、XID)服务的HOST/PORT。

2. 使用方使用得到的主机HOST、PORT进行直连访问。

3. 访问结束后，使用方将本次访问结果（成功与否）、微妙时延上报至agentSvrd。

```
HLBS服务方式极其类似于DNS。使用者拿着域名（GID、XID），请求DNS服务器（agentsvrd）获取实际主机地址（HOST、PORT），最后由使用者再去直连该主机地址获取服务。
```

#详情
见目录：[用户手册](document/SUMMARY.md)
