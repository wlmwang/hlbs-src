# 基础配置 conf.xml

一般无需改动。

```
<?xml version="1.0" encoding="UTF-8"?>
<ROOT>
	<!--服务器监听地址-->
	<SERVER HOST="0.0.0.0" PORT="10006" PROTOCOL="TCP" WORKER="1"/>
</ROOT>
```


# 被调Svr配置 svr.xml

```
<?xml version="1.0" encoding="UTF-8"?>
<ROOT>
	<SVRS>
		<!--SVR后端服务配置-->
		<SVR GID="1" XID="1" HOST="192.168.9.28" PORT="80" WEIGHT="100" VERSION="0" NAME="exam" />
		<SVR GID="1" XID="1" HOST="192.168.9.28" PORT="8081" WEIGHT="100" VERSION="0" NAME="exam" />
		<SVR GID="1" XID="1" HOST="192.168.9.28" PORT="6770" WEIGHT="100" VERSION="0" NAME="exam" />
		<SVR GID="1" XID="1" HOST="192.168.9.28" PORT="9527" WEIGHT="100" VERSION="0" NAME="exam" />

		<SVR GID="1" XID="2" HOST="192.168.8.49" PORT="8184" WEIGHT="80" VERSION="0" NAME="school" />
		<SVR GID="1" XID="2" HOST="192.168.8.49" PORT="8284" WEIGHT="60" VERSION="0" NAME="school" />
		<SVR GID="1" XID="2" HOST="192.168.8.49" PORT="8384" WEIGHT="100" VERSION="0" NAME="school" />
	</SVRS>
</ROOT>
```

* GID、XID、HOST、PORT唯一确定一个Svr服务。

* WEIGHT为机器负载能力0~1000，weight值越高则机器性能越好，0表明该Svr无效。

* VERSION为Svr下发标志位。当运行时需要热更新某一svr的权重（不能修改GID、XID、HOST、PORT，会被当做新的Svr处理）时，需更改该字段，否则修改无效。本次更改会在agentsvrd一个周期后生效。

* 新增路由总是下发到所有agentsvrd，并立即生效。负载、统计数据被初始化为同类别(GID、XID)下最高负载（优先级最低）。
