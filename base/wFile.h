
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_FILE_H_
#define _W_FILE_H_

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"

/**
 * 该类主要用于普通文件处理
 */
class wFile : private wNoncopyable
{
	public:
		wFile()
		{
			Initialize();
		}

		void Initialize()
		{
			mFileName = "";
			mFD = 0;
			mOffset = 0;
			memset(&mInfo, 0, sizeof(struct stat));
		}
		
		//成功则返回0, 失败返回-1, 错误原因存于mErrno
		int Open(const char *pFilename, int flags = O_RDWR| O_APPEND| O_EXCL, mode_t mode = 644)
		{
			mFileName = pFilename;
			mFD = open(pFilename, flags, mode);
			if (mFD == -1)
			{
				mErrno = errno;
				return -1;
			}
			fstat(mFD, &mInfo);
			return mFD;
		}

		//成功则返回0, 失败返回-1, 错误原因存于errno
		int Close(const char *pFilename, int flags = , mode_t mode = 777)
		{
			if(close(mFD) == -1)
			{
				mErrno = errno;
				return -1;
			}
			return 0;
		}

		//成功则返回0, 失败返回-1, 错误原因存于errno
		int Unlink()
		{
			if(unlink(mFileName.c_str()) == -1)
			{
				mErrno = errno;
				return -1;
			}
			return 0;
		}

		//新的偏移量（成功），-1（失败）
		off_t Lseek(off_t offset, int whence = SEEK_SET)
		{
			mOffset = lseek(mFD, offset, whence);
			return mOffset;
		}

		/**
		 * @param  pBuf 读取到的内容
		 * @param  nbyte 读取大小
		 * @return 成功时，返回实际所读的字节数，0表示已经读到文件的结束了，小于0表示出现了错误（EINTR 说明由中断引起的失败）
		 */
		ssize_t Read(void *pBuf, size_t nbyte, off_t offset)
		{
			ssize_t  n;
			n = pread(mFD, pBuf, nbyte, offset);
			if (n == -1)
			{
				mErrno = errno;
				LOG_ERROR(ELOG_KEY, "pread() \"%s\" failed", mFileName.c_str());
				return -1;
			}

			mOffset += n;
			return n;
		}

		/**
		 * @param  pBuf   写入内容
		 * @param  nbytes pBuf长度
		 * @return 成功时返回写的字节数.失败时返回-1（EINTR 说明由中断引起的失败；EPIPE表示网络连接出现了问题，如对方已经关闭了连接）
		 */
		ssize_t Write(const void *pBuf, size_t nbytes, off_t offset)
		{
		    ssize_t n, written;

		    written = 0;
		    while (true) 
		    {
		        n = pwrite(mFD, pBuf + written, nbytes, offset);

		        if (n == -1) 
		        {
					mErrno = errno;

		            if (mErrno == EINTR) 
		            {
		                LOG_DEBUG(ELOG_KEY, "pwrite() was interrupted");
		                continue;
		            }

		            LOG_ERROR(ELOG_KEY, "pwrite() \"%s\" failed", mFileName.c_str());
		            return -1;
		        }

		        mOffset += n;
		        written += n;

		        if ((size_t) n == nbytes) 
		        {
		            return written;
		        }

		        offset += n;
		        nbytes -= n;
		    }
		}

		struct stat Stat()
		{
			return mInfo;
		}
		
		int FD()
		{
			return mFD;
		}

		mode_t Mode()
		{
			return mInfo.st_mode;
		}

		off_t Size()
		{
			return mInfo.st_size;
		}

		uid_t Uid()
		{
			return mInfo.st_uid;
		}

	private:
	    int mFD;		//文件描述符
	    string  mFileName;	//文件名称
		int mErrno;

	    struct stat mInfo;	//文件大小等资源信息
	    off_t  mOffset;		//现在处理到文件何处了
};

#endif