# 目录结构

```
v3.0.2之前版本目录结构（从v3.0.2 + hnet(v0.0.12+)起将不再依赖表格中的第三方库）：

PHP扩展库hlbs.so依赖第三方库： libhnet libtinyxml；若未用到PHP，则可不安装第三方运行库（routersvrd、agentsvrd静态链接了这些库）
libhnet libtinyxml libprotobuf具体版本由实际安装版本确定。
若这些第三方库是已分发安装方式（复制hlbs.so、libhnet.so.0.0.9、libprotobuf.so.7.0.0、libtinyxml.so到对应服务器目录），则在复制安装hlbs.so之前，需运行命令ldconfig

```

* 建议目录结构

| /usr/local |  |   |   |  |
| -- | -- | -- | --|-- |
|   | lib/ |     |||
|   |  | libhnet.so.0.0.9 |||
|   |  | libprotobuf.so.7.0.0 |||
|   |  | libtinyxml.so |||
|   | webserver/hlbs/ |  |||
|   |   |   routerserver/|||
|   |   |   |bin/|  |
|   |   |   |   | routersvrd |
|   |   |   |config/|  |
|   |   |   |   | conf.xml |
|   |   |   |   | qos.xml |
|   |   |   |   | svr.xml |
|   |   |   |log/|  |
|   |   |   |   | hlbs.pid |
|   |   |   |   | hlbs.log |
|   |   |   agentserver/|||
|   |   |   |bin/|  |
|   |   |   |   | agentsvrd |
|   |   |   |config/|  |
|   |   |   |   | conf.xml |
|   |   |   |   | qos.xml |
|   |   |   |   | router.xml |
|   |   |   |log/|  |
|   |   |   |   | hlbs.pid |
|   |   |   |   | hlbs.log |

