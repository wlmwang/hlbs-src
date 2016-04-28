# 环境最低要求

```
Linux2.4.x+epoll补丁

Linux2.6.x #系统最好为2.6.x：基础框架涉及TCP_USER_TIMEOUT的TCP选项来控制ack超时

g++ (GCC) 4.8.x
```


# 解压软件包

```
$ tar –xvf hlbs.tar –C /usr/local
$ cd /usr/local/hlbs #若不解压到此目录，请务必在安装RouterSvrd、AgentSvrd之前，修改两者目录下的Makefile文件。设置其ROOTPATH值为要安装的目录。
```


