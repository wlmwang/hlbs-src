# 部署

1. routersvrd单独部署一台机器。

2. agentsvrd须与调用方CGI部署同一台机器。

3. 被调方Svr不要与调用方CGI处于同一台机器（同一台机器ping返回头类型不正常，会导致Svr不能被宕机拉起）

# 部署图

~
