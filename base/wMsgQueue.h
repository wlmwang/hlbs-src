
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

#include <string.h>

#define MSG_QUEUE_RESERVE_LEN 8

//template <unsigned int mQueueSize>	//消息缓冲队列大小
class wMsgQueue
{
	public:
		wMsgQueue()
		{
			Initialize();
		}

		~wMsgQueue() {}

		void Initialize()
		{
			//memset(mRealQueue, 0, sizeof(mRealQueue));
			mBeginIdxPtr = NULL;
			mEndIdxPtr = NULL;
		}

		void SetBuffer(char *vBuffer, int vBufferLen)
		{
			char *pBuffer = vBuffer;
			mBeginIdxPtr = (int *)pBuffer;
			pBuffer += sizeof(int);
			mEndIdxPtr = (int *)pBuffer;
			pBuffer += sizeof(int);
			mBufferPtr = pBuffer;
			mQueueSize = vBufferLen - 2 * sizeof(int);
		}
		
		/**
		 * 取出第一个消息，本函数只改变mBeginIdx
		 * @param  pBuffer    [out]
		 * @param  vBufferLen [一次最多取多少字节数据，大多数时刻返回值小于该值，取完一整条消息即可]
		 * @return            [< 0 出错，= 0 没消息，> 0 消息长度]
		 */
		int Pop(char *pBuffer, int vBufferLen)
		{
			W_ASSERT(pBuffer != NULL, return -1);
			
			//临时值，防止发送端同时操作产生同步问题
			int iBeginIdx = *mBeginIdxPtr;
			int iEndIdx = *mEndIdxPtr;

			if(iBeginIdx == iEndIdx)
			{
				return 0;
			}

			// 当前所有消息占据的长度
			int iAllMsgLen;
			if(iBeginIdx > iEndIdx)
			{
				iAllMsgLen = (int)mQueueSize - iBeginIdx + iEndIdx;
			}
			else
			{
				iAllMsgLen = iEndIdx - iBeginIdx;
			}
			
			//如果消息有问题，则直接忽略
			if(iAllMsgLen <= (int)sizeof(int))
			{
				*mBeginIdxPtr = iEndIdx;
				return -2;
			}

			//消息长度
			int iHeadMsgLen;
			//分段长度
			if(iBeginIdx > (int)mQueueSize - (int)sizeof(int)) 
			{
				int iLenCopyOnce = (int)mQueueSize - iBeginIdx;
				memcpy((void *)&iHeadMsgLen, (const void *)(mBufferPtr + iBeginIdx), iLenCopyOnce);
				memcpy((void *)(((char *)&iHeadMsgLen) + iLenCopyOnce), (const void *)mBufferPtr, (int)sizeof(int) - iLenCopyOnce);
			}
			else 
			{
				iHeadMsgLen = *(int *)(mBufferPtr + iBeginIdx);
			}

			// 消息长度判断
			if(iHeadMsgLen < 0 || iHeadMsgLen > iAllMsgLen - (int)sizeof(int)) 
			{
				return -3;
			}
			
			// 如果接收长度过短
			if(iHeadMsgLen > vBufferLen) 
			{
				return -4;
			}

			int iMsgEndIdx = iBeginIdx + (int)sizeof(int) + (int)iHeadMsgLen;
			//如果消息没有分段
			if(iMsgEndIdx <= (int)mQueueSize) 
			{
				memcpy((void *)pBuffer, (const void *)(mBufferPtr + iBeginIdx + (int)sizeof(int)), iHeadMsgLen);
				*mBeginIdxPtr = iMsgEndIdx;
				return iHeadMsgLen;
			}
			else 
			{
				//前半段的长度
				int iTmpLen = (int)mQueueSize - (int)sizeof(int) - iBeginIdx;
				if(iTmpLen <= 0) 
				{
					memcpy((void *)pBuffer, (const void *)(mBufferPtr - iTmpLen), iHeadMsgLen);
					*mBeginIdxPtr = iHeadMsgLen - iTmpLen;
					return iHeadMsgLen;
				}
				//拷贝前半段
				memcpy((void *)pBuffer, (const void *)(mBufferPtr + iBeginIdx + (int)sizeof(int)), iTmpLen);
				//后半段的长度
				int iLastLen = iHeadMsgLen - iTmpLen;
				//拷贝后半段
				memcpy((void *)(pBuffer + iTmpLen), (const void *)mBufferPtr, iLastLen);
				//调整起始位置
				*mBeginIdxPtr = iLastLen;
				return iHeadMsgLen;
			}
		}

		//放入第一个消息，本函数只改变mEndIdx
		//返回值：= 0表示成功，< 0表示失败
		int Push(char *pBuffer, int vBufferLen)
		{
			W_ASSERT(pBuffer != NULL || vBufferLen <= 0 || vBufferLen > mQueueSize - MSG_QUEUE_RESERVE_LEN, return -1);

			int iBeginIdx = *mBeginIdxPtr;
			int iEndIdx = *mEndIdxPtr;

			//剩余的空间长度
			int iLastLen;
			if(iBeginIdx == iEndIdx)
			{
				iLastLen = (int)mQueueSize;
			}
			else if(iBeginIdx > iEndIdx)
			{
				iLastLen = iBeginIdx - iEndIdx;
			}
			else
			{
				iLastLen = (int)mQueueSize - iEndIdx + iBeginIdx;
			}

			//减去保留长度
			iLastLen -= MSG_QUEUE_RESERVE_LEN;

			//如果剩余长度不够
			if(iLastLen < vBufferLen + (int)sizeof(int))
			{
				return -2;
			}

			//消息长度判断
			if((int)vBufferLen <= 0 || (int)vBufferLen > (int)mQueueSize - MSG_QUEUE_RESERVE_LEN)
			{
				return -3;
			}

			//拷贝长度，如果分段
			if(iEndIdx > (int)mQueueSize - (int)sizeof(int)) 
			{
				int iLenCopyOnce = (int)mQueueSize - iEndIdx;
				memcpy((void *)(mBufferPtr + iEndIdx), (const void *)&vBufferLen, iLenCopyOnce);
				memcpy((void *)mBufferPtr, (const void *)(((char *)&vBufferLen) + iLenCopyOnce), sizeof(int) - iLenCopyOnce);
			}
			// 如果不分段
			else
			{
				*(int *)(mBufferPtr + iEndIdx) = vBufferLen;
			}

			//确定当前的拷贝入口
			int iNowEndIdx = iEndIdx + (int)sizeof(int);
			if(iNowEndIdx >= (int)mQueueSize)
			{
				iNowEndIdx -= (int)mQueueSize;
			}

			int iMsgEndIdx = iNowEndIdx + vBufferLen;
			//不需要分段
			if(iMsgEndIdx <= (int)mQueueSize)
			{
				memcpy((void *)(mBufferPtr + iNowEndIdx), (const void *)pBuffer, vBufferLen);
				*mEndIdxPtr = iNowEndIdx + vBufferLen;
				return 0;
			}
			else
			{
				//前半段的长度
				int iTmpLen = (int)mQueueSize - iNowEndIdx;
				//拷贝消息前半段
				memcpy((void *)(mBufferPtr + iNowEndIdx), (const void *)pBuffer, iTmpLen);
				//后半段的长度
				int iLastLen = vBufferLen - iTmpLen;
				memcpy((void *)mBufferPtr, (const void *)(pBuffer + iTmpLen), iLastLen);
				*mEndIdxPtr = iLastLen;
				return 0;
			}
		}

		int IsEmpty() 
		{
			return *mBeginIdxPtr == *mEndIdxPtr ? 1 : 0; 
		}
		
	private:
		//char mRealQueue[mQueueSize];	// 实际的消息队列
		int *mBeginIdxPtr;
		int *mEndIdxPtr;
		int mQueueSize;
		char *mBufferPtr;
};

#endif
