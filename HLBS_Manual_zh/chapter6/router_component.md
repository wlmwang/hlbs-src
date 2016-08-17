# RouterSvrd功能列表

* 接受并验证所有连接的AgentSvrd，记录日志中。

* 对所有连接的AgentSvrd保存心跳检测，心跳异常，记录到日志中。

* 检测某一Svr的配置（svr.xml）发生变化，同步变化（新增或修改version值）的Svr到所有AgentSvrd。

* 同步失败的AgentSvrd告警。

* 检测svr.xml配置文件，因非法配置被忽略的项，输出到日志中。

