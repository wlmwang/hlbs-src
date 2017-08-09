# routersvrd管理RestFul接口

**一、SVR管理**

    * http://192.168.9.180:10005/?cmd=70&para=1    # 更新SVR记录接口
    请求方式：POST

请求参数：

| 序号 | 名称 | 类型 | 备注 |
| -- | -- | -- | -- |
| 1 | svrs | string(json) | [{"gid":1000,"xid":20000,"host":"192.168.9.28","port":10016,"weight":1000,"name":"test1","idc":0}, {"gid":1000,"xid":20000,"host":"192.168.9.29","port":10016,"weight":1000,"name":"test2","idc":0}]|

请求返回：

成功：{"msg": "ok","status": "200"}

错误：非200的http状态码，均为错误。

接口说明：

更新SVR记录接口。主要用于添加、删除（weight=0）、修改（weight>0，name，idc）SVR记录。
运维管理时应尽量使用该接口。router服务器将自动识别新增、修改或删除的SVR记录并定向下发到已连接并注册的agent服务器上。


    * http://192.168.9.180:10005/?cmd=70&para=2      # 覆盖SVR记录接口
    请求方式：POST

请求参数：

| 序号 | 名称 | 类型 | 备注 |
| -- | -- | -- | -- |
| 1 | svrs | string(json) | [{"gid":1000,"xid":20000,"host":"192.168.9.28","port":10016,"weight":1000,"name":"test1","idc":0}, {"gid":1000,"xid":20000,"host":"192.168.9.29","port":10016,"weight":1000,"name":"test2","idc":0}]|

请求返回：

成功：{"msg": "ok","status": "200"}

错误：非200的http状态码，均为错误。

接口说明：

覆盖SVR记录接口。主要统一整理SVR记录，强行将配置全部重置为请求参数中的SVR记录。
运维管理时应避免使用该接口。router服务器将并定向下发所有SVR记录到已连接并注册的agent服务器上。

    * http://192.168.9.180:10005/?cmd=70&para=3     # SVR记录列表接口
    请求方式：GET

请求参数：无

请求返回：

成功：["msg":"ok", "status":"200","svrs":[{"gid":1000,"xid":20000,"host":"192.168.9.28","port":10016,"weight":1000,"name":"test1","idc":0},{"gid":1000,"xid":20000,"host":"192.168.9.29","port":10016,"weight":1000,"name":"test2","idc":0}]]

错误：非200的http状态码，均为错误。


**二、AGENT管理（从 v3.0.8 开始AGNT将自动注册。router服务器也将只保留列表接口，即第3点）**

    * http://192.168.9.180:10005/?cmd=71&para=1      # 更AGENT记录接口
    请求方式：POST

请求参数：

| 序号 | 名称 | 类型 | 备注 |
| -- | -- | -- | -- |
| 1 | agnts | string(json) |[{"host":"192.168.8.13","weight":1,"idc":0},{"host":"192.168.8.107","weight":1,"idc":0}]|

请求返回：

成功：{"msg": "ok","status": "200"}

错误：非200的http状态码，均为错误。

接口说明：

更新AGENT记录接口。主要用于添加、删除（weight=0）、修改（idc）AGENT记录。
运维管理时应尽量使用该接口。router服务器将自动识别新增、修改或删除的AGENT记录下发到已连接并注册的agent服务器上。

    * http://192.168.9.180:10005/?cmd=71&para=2      # 覆盖AGNT记录接口
    请求方式：POST

请求参数：

| 序号 | 名称 | 类型 | 备注 |
| -- | -- | -- | -- |
| 1 | agnts | string(json) |[{"host":"192.168.8.13","weight":1,"idc":0},{"host":"192.168.8.107","weight":1,"idc":0}]|

请求返回：

成功：{"msg": "ok","status": "200"}

错误：非200的http状态码，均为错误。

接口说明：

覆盖AGENT记录接口。主要统一整理SVR记录，强行将配置全部重置为请求参数中的AGENT记录。
运维管理时应避免使用该接口。router服务器将下发所有AGENT记录到已连接并注册的agent服务器上。

    * http://192.168.9.180:10005/?cmd=71&para=3     # AGENT记录列表接口
    请求方式：GET

请求返回：

成功：["msg":"ok","status":"200","agnts":[{"config":0,"host":"192.168.8.13","port":0,"weight":1,"status":0,"idc":0,"name":"kq-8-13-dev.sh.hupu.com"},{"config":-1,"host":"192.168.8.107","port":0,"weight":1,"status":0,"idc":0,"name":"kq-9-180-dev.sh.hupu.com"}]]

    # 字段解释： config=0为router配置文件中已存在的记录（已注册）、config=-1为远程agent连接的记录。weight=0为删除节点、weight=1为使用中节点，status=0为已注册被可使用的agent服务器记录、status=-1为本地配置agent记录并刚初始化、status=-2为本地未注册且agent已连接到router服务器的记录、status=-3为连接agent由于某种原因断开router连接记录。  # 注册的的意思是在router中添加了agent服务器主机地址到配置文件中。

错误：非200的http状态码，均为错误。


**三、RLT管理（RELATION缩写，路由绑定，即定向下发）**

    * http://192.168.9.180:10005/?cmd=73&para=1     # 更新RLT记录接口
    请求方式：POST

请求参数：

| 序号 | 名称 | 类型 | 备注 |
| -- | -- | -- | -- |
| 1 | rlts | string(json) |[{"gid":1000,"xid":20000,"host":"192.168.9.28","weight":1000},{"gid":1000,"xid":20000,"host":"192.168.9.29","weight":1000}]|

请求返回：

成功：{"msg": "ok","status": "200"}

错误：非200的http状态码，均为错误。

接口说明：

更新RLT记录接口。主要用于添加、删除（weight=0）。
运维管理时应尽量使用该接口。router服务器将自动识别新增、删除的SVR记录并定向下发到已连接并注册的agent服务器上。注意：删除绑定，将只能影响在此之后修改SVR记录定向下发逻辑。

    * http://192.168.9.180:10005/?cmd=73&para=2      # 覆盖RLT记录接口
    请求方式：POST

请求参数：

| 序号 | 名称 | 类型 | 备注 |
| -- | -- | -- | -- |
| 1 | rlts | string(json) |[{"gid":1000,"xid":20000,"host":"192.168.9.28","weight":1000},{"gid":1000,"xid":20000,"host":"192.168.9.29","weight":1000}]|

请求返回：

成功：{"msg": "ok","status": "200"}

错误：非200的http状态码，均为错误。

接口说明：

覆盖RLT记录接口。主要统一整理RLT记录，强行将配置全部重置为请求参数中的RLT记录。
运维管理时应避免使用该接口。router服务器将定向下发所有SVR记录到已连接并注册的agent服务器上。注意：删除绑定，将只能影响在此之后修改SVR记录定向下发逻辑。

    * http://192.168.9.180:10005/?cmd=73&para=3     # SVR记录列表接口
    请求方式：GET

请求返回：

成功：["msg":"ok","status":"200","rlts":[{"gid":1000,"xid":20000,"host":"192.168.9.28","weight":1000},{"gid":1000,"xid":20000,"host":"192.168.9.29","weight":1000}]]

错误：非200的http状态码，均为错误。

**四、服务器管理**

    * http://192.168.9.180:10005/?cmd=72&para=1      # 重启所有worker进程
    请求方式：GET

请求参数：无

请求返回：

成功：{"msg": "ok","status": "200"}

错误：非200的http状态码，均为错误。

接口说明：

重启所有工作进程worker。

    * http://192.168.9.180:10005/?cmd=72&para=2      # 关闭所有worker进程、master进程
    请求方式：GET

请求参数：无

请求返回：无

接口说明：

关闭所有工作进程worker进程、master进程。该请求一般会让连接中断并无返回（相信一下类似secureCRT客户端工具）

    * http://192.168.9.180:10005/?cmd=72&para=3      # 所有worker进程列表
    请求方式：GET

请求参数：无

请求返回：

成功：["msg":"ok","status":"200","pid":3802,"version":"3.0.4", "name":"HLBS(*router*) - master","num":2,"workers":[{"pid":3803,"respawn":1,"timeline":1497316970,"name":"HLBS(*router*) - worker"},{"pid":3804,"respawn":1,"timeline":1497316970,"title":"HLBS(*router*) - worker"}]]

    # 字段解释： pid=3802为master主进程pid。version=3.0.1为版本号，name=HLBS(*router*) - master，主进程名称。num为工作进程worker数量。workers为进程列表。其中respawn=1为worker进程意外退出主进程master会重启一个worker进程。

错误：非200的http状态码，均为错误。
