# 说明

在单台机器上同时启动运行RouterSvrd、AgentSvrd服务。

# 启动

```
$ cd /usr/local/hlbs/bin
$ ./restart.sh  #可能有报错：/bin/bash/^M:bad interpreter。
```

```
出错运行（讲道理无需运行）~

$ vim restart.sh
:set ff
fileformat=dos		#说明文件格式不正确。需执行:set ff=unix
```

# 停止

```
$ cd /usr/local/hlbs/bin
$ ./stop.sh #可能有报错：/bin/bash/^M:bad interpreter。
```

```
出错运行（讲道理无需运行）~

$ vim stop.sh
:set ff
fileformat=dos		#说明文件格式不正确。需执行:set ff=unix
```


