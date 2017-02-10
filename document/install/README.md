# 环境最低要求

```
Linux2.4.x+epoll补丁

Linux2.6.x #系统最好为2.6.x：基础框架涉及TCP_USER_TIMEOUT的TCP选项来控制ack超时

g++ (GCC) 4.8.x

运行需用root身份：系统中包含发送原生icmp包，用于宕机恢复
```

# 软件包依赖

请先安装tinyxml、protobuf、hnet软件包。详细步骤请查看hnet安装文档。

