# AgentSvrd日志

与过载相关日志

```
断线重连（与RouterSvrd服务）：
[system] reconnect success: ip(%s) port(%d)
[system] reconnect failed: ip(%s) port(%d)

某一节点过载：
[svr] Overload(one ready) RebuildRoute, gid(%d),xid(%d),host(%s),port(%d),limit(%d),reqall(%d),reqsuc(%d),reqrej(%d),err(%d),errtm(%d), service err rate[%f][%f]>config err_rate[%f]

全部过载：
[svr] Overload(all ready) RebuildRoute, gid(%d),xid(%d),avg err rate=%f, all service err rate>config err_rate[%f]
[svr] Overload(all) RebuildRoute, mod=%d cmd=%d avg err rate=%f, all req to min>config err_rate[%f]

宕机检测日志：
[detect] succ detect %d us,ip %s:%u rc %d, ping %d,connect %d; elapse=%d,ret=%d
[detect] fail detect %d us,ip %s:%u rc %d, ping %d,connect %d; elapse=%d,ret=%d

```
