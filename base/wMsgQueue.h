
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  消息传递的共享内存队列
 */
#ifndef _W_MSG_QUEUE_
#define _W_MSG_QUEUE_

#include "wType.h"
#include "wAssert.h"
#include "wShm.h"

#define MSG_QUEUE_RESERVE_LEN 8

class wMsgQueue
{
	public:
		wMsgQueue()
		{
			Initialize();
		}

		~wMsgQueue() {}

		void Initialize();

		void SetBuffer(char *vBuffer, int vBufferLen);
		
		/**
		 * 取出第一个消息，本函数只改变mBeginIdx
		 * @param  pBuffer    [out]
		 * @param  vBufferLen [一次最多取多少字节数据，大多数时刻返回值小于该值，取完一整条消息即可]
		 * @return            [< 0 出错，= 0 没消息，> 0 消息长度]
		 */
		int Pop(char *pBuffer, int vBufferLen);

		/**
		 * 放入第一个消息，本函数只改变mEndIdx
		 * @param  pBuffer    
		 * @param  vBufferLen 
		 * @return            = 0表示成功，< 0表示失败
		 */
		int Push(char *pBuffer, int vBufferLen);

		int IsEmpty() 
		{
			return *mBeginIdxPtr == *mEndIdxPtr ? 1 : 0; 
		}
		
	private:
		int *mBeginIdxPtr;	//前4位记录开始地址
		int *mEndIdxPtr;	//后4位开始地址
		int mQueueSize;		//实际数据长度
		char *mBufferPtr;	//实际数据地址
};

#endif
