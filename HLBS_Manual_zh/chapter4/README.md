# 总述

HLBS系统由RouterSvrd、AgentSvrd两部分组件组成，~~同时随包发放的还有AgentCmd组件（命令行工具）、~~PHP扩展。

# CGI使用HLBS步骤：

1. CGI从AgentSvrd获取某一类(GID、XID)Svr服务的HOST/PORT。一般使用PHP扩展。

2. 然后对该Svr的HOST/PORT进行直连访问。

3. 访问结束后，CGI将本次访问结果（成功与否）、微妙时延上报至本地AgentSvrd（AgentSvrd需与CGI同处在同一台机器，即一一对应）。

# 部署建议

1. RouterSvrd单独部署一台机器。

2. AgentSvrd须与调用方CGI部署同一台机器。

3. 被调方Svr不要与调用方CGI处于同一台机器（同一台机器ping返回头类型不正常，会导致Svr不能被宕机拉起）

# 部署图

~
