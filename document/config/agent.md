# 基础配置 conf.xml

一般无需改动。

```
<?xml version="1.0" encoding="UTF-8"?>
<ROOT>
	<!--服务器监听地址-->
	<SERVER HOST="/var/run/hlbs.sock" PORT="0" PROTOCOL="UNIX" WORKER="0"/>
	<!--
	<SERVER HOST="0.0.0.0" PORT="10007" PROTOCOL="TCP" WORKER="0"/>
	-->
</ROOT>
```


# Qos配置 qos.xml

一般无需改动。

```
<?xml version="1.0" encoding="UTF-8"?>
<QOS>
	<!-- @TYPE:访问量配置 -->
	<!-- @REBUILD:路由重建时间间隔 -->
	<!-- @TIMEOUT:超时时间 -->
	<CFG TYPE="REQ" REBUILD="30" TIMEOUT="30"/>

	<!-- @RATE_WEIGHT:成功率负载权重 -->
	<!-- @DELAY_WEIGHT:时延负载权重 -->
	<FACTOR RATE_WEIGHT="7" DELAY_WEIGHT="1"/>

	<!-- @REQ_MAX:每周期最大访问次数（门限最大值） -->
	<!-- @REQ_MIN:每个周期最小访问次数（门限最小值（宕机依据）） -->
	<!-- @REQ_ERR_MIN:错误率阈值（宕机嫌疑，门限扩缩依据） -->
	<!-- @REQ_EXTEND_RATE:门限扩缩系数 -->
	<!-- @PRE_TIME:预取时间长度 -->
	<!-- @REQ_LIMIT_DEFAULT:当新增节点时，对此节点的预分配门限值按req_limit_default的值来指定 -->
	<REQ REQ_MAX="100000" REQ_MIN="10" REQ_ERR_MIN="0.1" REQ_EXTEND_RATE="0.2" PRE_TIME="4" REQ_LIMIT_DEFAULT="0"/>

	<!-- 宕机控制&恢复配置 -->
	<!-- @PROBE_BEGIN:路由探测尝试恢复时间间隔 -->
	<!-- @PROBE_TRIGER_REQ:路由探测尝试恢复请求量条件 -->
	<!-- @PROBE_TIMES:路由刚从探测线程恢复，初始门限 -->
	<!-- @PROBE_INTERVAL:路由探测时间间隔 -->
	<!-- @PROBE_EXPIRED:路由探测过期时间。v2.3.0版本过期后由主线程继续加入到探测线程中继续探测 -->
	<!-- @DOWN_ERR_REQ:路由连续失败次数阈值（宕机嫌疑） -->
	<!-- @DOWN_ERR_RATE:路由有效错误率阈值（宕机嫌疑） -->
	<!-- @DOWN_TIME:路由恢复时间间隔条件 -->
	<DOWN PROBE_BEGIN="3" PROBE_TRIGER_REQ="500" PROBE_TIMES="3" PROBE_INTERVAL="3" PROBE_EXPIRED="600" DOWN_ERR_REQ="10" DOWN_ERR_RATE="0.5" DOWN_TIME="300"/>
</QOS>
```


# routersvrd配置 router.xml

```
<?xml version="1.0" encoding="UTF-8"?>
<ROOT>
	<ROUTERS>
		<!--配置中心ROUTER地址-->
		<ROUTER HOST="0.0.0.0" PORT="10006" PROTOCOL="TCP"/>
	</ROUTERS>
</ROOT>
```
