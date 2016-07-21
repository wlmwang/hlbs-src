# 环境最低要求

```
Linux2.4.x+epoll补丁

Linux2.6.x #系统最好为2.6.x：基础框架涉及TCP_USER_TIMEOUT的TCP选项来控制ack超时

g++ (GCC) 4.8.x
```


# 解压软件包

默认安装到/usr/local/webserver/hlbs/目录中，若需更改目录，请务必在安装RouterSvrd、AgentSvrd之前，修改两者目录下的Makefile文件，设置其ROOTPATH值为要安装的目录（php扩展config.m4文件也要做对应路径的修改）

```
$ tar –xvf hlbs.tar –C /usr/local/webserver
$ cd /usr/local/webserver/hlbs
```


