
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wChannel.h"

wChannel::wChannel() 
{
	memset(mChannel, 0, sizeof(mChannel));
}

int wChannel::Open()
{
    int iRt = socketpair(AF_UNIX, SOCK_STREAM, 0, mChannel);
    if (iRt == -1)
    {
		LOG_ERROR(ELOG_KEY, "[runtime] socketpair failed");
    	return -1;
    }

    if (fcntl(mChannel[0], F_SETFL, fcntl(mChannel[0], F_GETFL) | O_NONBLOCK) == -1)
    {
    	LOG_ERROR(ELOG_KEY, "[runtime] fcntl(O_NONBLOCK) failed");
    	Close();
    	return -1;
    }

    if (fcntl(mChannel[1], F_SETFL, fcntl(mChannel[1], F_GETFL) | O_NONBLOCK) == -1)
    {
    	LOG_ERROR(ELOG_KEY, "[runtime] fcntl(O_NONBLOCK) failed");
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

int wChannel::Send(int iFD, channel_t *pCh, size_t size)
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

        memset(&cmsg, 0, sizeof(cmsg));

        cmsg.cm.cmsg_len = CMSG_LEN(sizeof(int));
        cmsg.cm.cmsg_level = SOL_SOCKET;
        cmsg.cm.cmsg_type = SCM_RIGHTS;

        memcpy(CMSG_DATA(&cmsg.cm), &pCh->mFD, sizeof(int));
    }
    msg.msg_flags = 0;

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

int wChannel::Recv(int iFD, channel_t *pCh, size_t size)
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

    iov[0].iov_base = (char *) pCh;
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

        /*pCh->mFD = *(int *) CMSG_DATA(&cmsg.cm); */
        memcpy(&pCh->mFD, CMSG_DATA(&cmsg.cm), sizeof(int));
    }

    if (msg.msg_flags & (MSG_TRUNC|MSG_CTRUNC)) 
    {
		LOG_ERROR(ELOG_KEY, "[runtime] recvmsg() truncated data");
    }

    return n;
}

void wChannel::Close()
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

int& wChannel::operator[](int i)
{
	if (i > 1)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] leap the pale channel");
		exit(1);
	}
	return mChannel[i];
}
