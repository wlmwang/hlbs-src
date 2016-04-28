# 基础配置 conf.xml

一般无需改动。

```
<?xml version="1.0" encoding="UTF-8"?>
<ROOT>
	<!--服务器监听地址-->
	<SERVER IPADDRESS="0.0.0.0" PORT="10007" BACKLOG="1024" WORKERS="1"/>

	<!--日志文件-->
	<LOG>
		<LOG_INFO KEY="error"	FILE="../log/error.log" LEVEL="700" MAX_FILE_SIZE="10485760"/>
	</LOG>
</ROOT>
```


# 服务质量配置 qos.xml

一般无需改动。

```
<?xml version="1.0" encoding="UTF-8"?>
<QOS>

	<CFG TYPE="REQ" REBUILD="30"/>

	<!--成功率权重 时延权重-->
	<FACTOR RATE_WEIGHT="7" DELAY_WEIGHT="1"/>

	<!--请求超时时间500ms-->
	<TIMEOUT REQ_TIMEOUT="500"/>

	<!--访问量的配置信息-->
	<REQ REQ_MAX="10000" REQ_MIN="10" REQ_ERR_MIN="0.1" REQ_EXTEND_RATE="0.2" PRE_TIME="4"/>

	<!--并发量的配置信息-->
	<LIST LIST_MAX="100" LIST_MIN="10" LIST_ERR_MIN="0.001" LIST_EXTEND_RATE="0.2" LIST_TIMEOUT="2"/>

	<!--宕机控制&恢复配置-->
	<DOWN BEGIN="3" INTERVAL="10" EXPIRE="600" TIMES="3" REQ_COUNT="1000" DOWN_TIME="600" DOWN_ERR_REQ="10" DOWN_ERR_RATE="0.5"/>

	<!--时延配置-->
	<DELAY>
		<TIME BEGIN="0" END="100"/>
		<TIME BEGIN="100" END="200"/>
		<TIME BEGIN="200" END="500"/>
		<TIME BEGIN="500" END="100000000"/>
	</DELAY>
</QOS>
```

* CFG：REBUILD为周期（时间片）大小（秒）。

* FACTOR：RATE_WEIGHT、DELAY_WEIGHT分别为成功率负载、时延负载因子。

* REQ：访问量阈值配置信息。


# RouterSvrd配置 router.xml

```
<?xml version="1.0" encoding="UTF-8"?>
<ROOT>
	<ROUTERS>
		<!--配置中心ROUTER地址-->
		<ROUTER IPADDRESS="127.0.0.1" PORT="10006"  DISABLED="0"/> <!--master-->
	</ROUTERS>
</ROOT>

```
