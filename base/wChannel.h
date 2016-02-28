
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

		wChannel() 
		{
			memset(mChannel, 0, sizeof(mChannel));
		}

		int Open()
		{
	        int iRt = socketpair(AF_UNIX, SOCK_STREAM, 0, mChannel);
	        if (iRt == -1)
	        {
				LOG_ERROR(ELOG_KEY, "[runtime] socketpair failed");
	        	return -1;
	        }

	        if (fcntl(mChannel[0], F_SETFL, fcntl(mChannel[0], F_GETFL) | O_NONBLOCK) == -1)
	        {
	        	Close();
	        	return -1;
	        }

	        if (fcntl(mChannel[1], F_SETFL, fcntl(mChannel[1], F_GETFL) | O_NONBLOCK) == -1)
	        {
	        	Close();
	        	return -1;
	        }

	        if (fcntl(mChannel[0], F_SETFD, FD_CLOEXEC) == -1) 
	        {
				LOG_ERROR(ELOG_KEY, "[runtime] fcntl(FD_CLOEXEC) failed");
	        }

	        if (fcntl(mChannel[1], F_SETFD, FD_CLOEXEC) == -1) 
	        {
				LOG_ERROR(ELOG_KEY, "[runtime] fcntl(FD_CLOEXEC) failed");
	        }
	        return 0;
		}

		int Send(int iFD, channel_t *pCh, size_t size)
		{
		    ssize_t	n;
		    int	err;
		    struct iovec	iov[1];
		    struct msghdr	msg;

		    union 
		    {
		        struct cmsghdr  cm;
		        char	space[CMSG_SPACE(sizeof(int))];
		    } cmsg;

		    if (pCh->mFD == -1) 
		    {
		        msg.msg_control = NULL;
		        msg.msg_controllen = 0;

		    } 
		    else 
		    {
		        msg.msg_control = (caddr_t) &cmsg;
		        msg.msg_controllen = sizeof(cmsg);

		        memzero(&cmsg, sizeof(cmsg));

		        cmsg.cm.cmsg_len = CMSG_LEN(sizeof(int));
		        cmsg.cm.cmsg_level = SOL_SOCKET;
		        cmsg.cm.cmsg_type = SCM_RIGHTS;

		        memcpy(CMSG_DATA(&cmsg.cm), &pCh->fd, sizeof(int));
		    }
		    msg.msg_flags = 0;

		    //
		    iov[0].iov_base = (char *) pCh;
		    iov[0].iov_len = size;

		    msg.msg_name = NULL;
		    msg.msg_namelen = 0;
		    msg.msg_iov = iov;
		    msg.msg_iovlen = 1;

		    n = sendmsg(iFD, &msg, 0);

		    if (n == -1) 
		    {
		        err = errno;
		        if (err == EAGAIN) 
		        {
		            return EAGAIN;
		        }
				
				LOG_ERROR(ELOG_KEY, "[runtime] sendmsg() failed");
		        return -1;
		    }

		    return 0;

		}

		int Recv(int iFD, channel_t *pCh, size_t size)
		{
		    ssize_t	n;
		    int	err;
		    struct iovec	iov[1];
		    struct msghdr	msg;

		    union 
		    {
		        struct cmsghdr  cm;
		        char	space[CMSG_SPACE(sizeof(int))];
		    } cmsg;

		    iov[0].iov_base = (char *) ch;
		    iov[0].iov_len = size;

		    msg.msg_name = NULL;
		    msg.msg_namelen = 0;
		    msg.msg_iov = iov;
		    msg.msg_iovlen = 1;

		    msg.msg_control = (caddr_t) &cmsg;
		    msg.msg_controllen = sizeof(cmsg);

		    n = recvmsg(iFD, &msg, 0);

		    if (n == -1)
		    {
		        err = errno;
		        if (err == EAGAIN) 
		        {
		            return EAGAIN;
		        }
				
				LOG_ERROR(ELOG_KEY, "[runtime] recvmsg() failed");
		        return -1;
		    }

		    if (n == 0) 
		    {
				LOG_ERROR(ELOG_KEY, "[runtime] recvmsg() returned zero");
		        return -1;
		    }

		    if ((size_t) n < sizeof(channel_t)) 
		    {
				LOG_ERROR(ELOG_KEY, "[runtime] recvmsg() returned not enough data: %d", n);
		        return -1;
		    }

		    if (pCh->mCommand == CMD_OPEN_CHANNEL) 
		    {

		        if (cmsg.cm.cmsg_len < (socklen_t) CMSG_LEN(sizeof(int))) 
		        {
					LOG_ERROR(ELOG_KEY, "[runtime] recvmsg() returned too small ancillary data");
		            return -1;
		        }

		        if (cmsg.cm.cmsg_level != SOL_SOCKET || cmsg.cm.cmsg_type != SCM_RIGHTS)
		        {
					LOG_ERROR(ELOG_KEY, "[runtime] recvmsg() returned invalid ancillary data");
		            return -1;
		        }

		        /* pCh->mFD = *(int *) CMSG_DATA(&cmsg.cm); */
		        memcpy(&pCh->mFD, CMSG_DATA(&cmsg.cm), sizeof(int));
		    }

		    if (msg.msg_flags & (MSG_TRUNC|MSG_CTRUNC)) 
		    {
				LOG_ERROR(ELOG_KEY, "[runtime] recvmsg() truncated data");
		    }

		    return n;
		}

		void Close()
		{
		    if (close(mChannel[0]) == -1) 
		    {
				LOG_ERROR(ELOG_KEY, "[runtime] close() channel failed");
		    }

		    if (close(mChannel[1]) == -1) 
		    {
				LOG_ERROR(ELOG_KEY, "[runtime] close() channel failed");
		    }
		}

	private:
		int	mChannel[2];
}