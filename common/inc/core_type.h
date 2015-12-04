#ifndef _CORE_TYPE_H_
#define _CORE_TYPE_H_

// 服务器之间如果断连，下一次检测重连的时间
#define RECONNECT_TIME 60000 /* 60 * 1000 */

// 向服务器注册，下一次检测重连的时间
#define REGISTER_TIME 3000 /* 3 * 1000 */

// 心跳包间隔时间
#define KEEPALIVE_TIME 1000 /* 1 * 1000 */

// SQL运行超时检查时间
#define SQL_CHECK_TIME 1000

// 每一个服务器之间的发送接收缓冲区的大小
#define MAX_TCP_BUFFER_LEN 10485760 /* 10 * 1024 * 1024 */

// 最长的IP地址字符串长度
#define MAX_IP_ADDR_LEN 24

// 网关服务器对客户端的缓冲区大小
#define MAX_CLIENT_MSG_BUFFER_LEN 153600 /* 150 * 1024 */

// 名字的最大长度
#define MAX_NAME_LEN 32

// 发服务器消息时的服务器ID
enum SERVER_FE
{
	SERVER_LOGIN = 1,
	SERVER_SCENE,
	SERVER_DB,
};

// 服务器中最多的玩家人数
#ifdef _DEBUG_
#define MAX_PLAYER_NUM 1
#else
#define MAX_PLAYER_NUM 1
#endif

// 最大场景服务器数量
#ifdef _DEBUG_
#define MAX_SCENE_NUM 1
#else
#define MAX_SCENE_NUM 4
#endif

// 最长的SQL长度
#define MAX_SQL_LEN 10240

// 断连客户端时所需要的缓冲区长度
#define DISCONNECT_MSG_LEN 30

// SQL执行的超时时间
#define EXECUTESQL_TIMEOUT 30000

// 消息队列的长度
#ifdef _DEBUG_
#define MSG_QUEUE_LEN 4194304 /* 1024 * 1024 * 4 */
#else
#define MSG_QUEUE_LEN 16777216 /* 1024 * 1024 * 16 */
#endif

// 消息缓冲区大小
#define MAX_PACKAGE_LEN 100 * 1024

// 客户端消息的最长长度
#define MAX_CLIENT_MSG_LEN MAX_PACKAGE_LEN

// 客户端消息的最短长度
#define MIN_CLIENT_MSG_LEN 4

#endif
