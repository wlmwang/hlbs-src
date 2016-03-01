
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_CHANNEL_H_
#define _W_CHANNEL_H_

#include <sys/un.h>
#include <sys/socket.h>

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"
 
#define CMD_OPEN_CHANNEL   1 
#define CMD_CLOSE_CHANNEL  2 
#define CMD_QUIT           3 
#define CMD_TERMINATE      4 
#define CMD_REOPEN         5
	
class wChannel : private wNoncopyable
{
	public:
		//channel数据结构
		struct channel_t
		{
		     int	mCommand;
		     pid_t	mPid;	//发送方进程id
		     int	mSlot;	//发送方进程表中偏移(下标)
		     int	mFD;	//发送方ch[0]描述符
		};

		wChannel();
		virtual ~wChannel();

		/**
		 * 创建非阻塞channel
		 * @return 0成功 -1发生错误
		 */
		int Open();

		/**
		 * 发送channel_t消息
		 * @param  iFD  channel FD
		 * @param  pCh  channel_t消息
		 * @param  size channel_t长度
		 * @return 0成功，-1失败     
		 */
		int Send(int iFD, channel_t *pCh, size_t size);

		/**
		 * 接受channel数据
		 * @param  iFD  channel FD
		 * @param  pCh  接受缓冲
		 * @param  size 缓冲长度
		 * @return      <0 失败 >0实际接受长度
		 */
		int Recv(int iFD, channel_t *pCh, size_t size);

		/**
		 * 关闭channel[0] channel[1] 描述符
		 */
		void Close();

		int& operator[](int i);

	private:
		int	mChannel[2];
};

#endif
