
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wTask.h"

wTask::wTask()
{
    Initialize();
}

wTask::wTask(wSocket* pSocket) : mSocket(pSocket)
{
    Initialize();
}

wTask::~wTask()
{
    SAFE_DELETE(mSocket);
}

void wTask::Initialize()
{
    mRecvRead = mRecvWrite = mRecvBuff;
    mSendRead = mSendWrite = mSendBuff;
}

int wTask::Heartbeat()
{
    mHeartbeatTimes++;
    struct wCommand vCmd;
    int iRet = SyncSend((char *)&vCmd, sizeof(vCmd));
    return iRet;
}

int wTask::TaskRecv()
{
    char *pBuffEnd = mRecvBuff + MSG_BUFF_LEN;  //超尾指针
    if ((pBuffEnd - mRecvWrite) < (int)((sizeof(int) + MIN_PACKAGE_LEN)))   //性能考虑
    {
        //memmove(mRecvBuff, mRecvRead, iLen);
        //mRecvRead = mRecvBuff;
        //mRecvWrite = mRecvBuff + iLen;
    }
    if (mRecvRead != mRecvBuff && mRecvWrite == pBuffEnd) mRecvWrite = mRecvBuff;

    int iLen =  mRecvWrite - mRecvRead;
    int iLeftLen = 0;
    if (iLen >= 0)  //正向剩余
    {
        iLeftLen = pBuffEnd - mRecvWrite;   //可写长度
    }
    else if (iLen < 0)  //分段
    {
        iLeftLen = -iLen;   //可写长度
    }
    
    int iRecvLen = 0;
    if (mRecvLen != MSG_BUFF_LEN)   //缓冲区未满
    {
       if (mRecvWrite == pBuffEnd) mRecvWrite = mRecvBuff;
       iRecvLen = mSocket->RecvBytes(mRecvWrite, iLeftLen);
       
        if (iRecvLen <= 0) return  iRecvLen;
        mRecvLen += iRecvLen;
        mRecvWrite += iRecvLen;
    }
    //LOG_DEBUG(ELOG_KEY, "[system] recv buf(%d-%d) rw(%d-%d) iLen(%d) leftlen(%d) movelen(%d) recvLen(%d)", mRecvBuff, pBuffEnd, mRecvRead, mRecvWrite, iLen, iLeftLen, mRecvRead-mRecvBuff, mRecvLen);

    int iRealLen = 0;
    do {
        iLen =  mRecvWrite - mRecvRead;
        
        if (mRecvLen == 0)  //缓冲区已空
        {
            break;
        }
        else if (iLen > 0)
        {
            iRealLen = *(int *)mRecvRead;
            if ((iRealLen < MIN_PACKAGE_LEN) || (iRealLen > MAX_PACKAGE_LEN))
            {
                LOG_ERROR(ELOG_KEY, "[system] recv message invalid len: %d , fd(%d)", iRealLen, mSocket->FD());
                return ERR_MSGLEN;
            }
            if (iRealLen > (int)(iLen - sizeof(int)))
            {
                LOG_DEBUG(ELOG_KEY, "[system] recv a part of message: real len = %d, now len = %d", iRealLen, iLen - sizeof(int));
                return 0;
            }
            
            HandleRecvMessage(mRecvRead + sizeof(int), iRealLen);  //消息解析，去除4字节消息长度标识位
            mRecvLen -= iRealLen + sizeof(int);
            mRecvRead += iRealLen + sizeof(int);
        }
        else if (iLen <= 0)
        {
            iLeftLen = pBuffEnd - mRecvRead;
            if (iLeftLen >= (int)sizeof(int))
            {
                iRealLen = *(int *)mRecvRead;
            }
            else
            {
                iRealLen = 0;
                //小端
                int tmp;
                for (int i = iLeftLen; i > 0; i--)
                {
                    tmp = *(mRecvRead + (sizeof(int) - i -1));
                    iRealLen |= tmp << (8 * (i + 1));
                }
                for (int i = sizeof(int) - iLeftLen; i > 0; i--)
                {
                    tmp = *(mRecvRead + (sizeof(int) - i -1));
                    iRealLen |= tmp << (8 * (i + 1));
                }
            }
            LOG_DEBUG(ELOG_KEY, "[system] handle recv buf(%d-%d) rw(%d-%d) iLen(%d) iRealLen(%d) iLeftLen(%d)", mRecvBuff, pBuffEnd, mRecvRead, mRecvWrite, iLen, iRealLen, iLeftLen);
            
            if ((iRealLen < MIN_PACKAGE_LEN) || (iRealLen > MAX_PACKAGE_LEN))
            {
                LOG_ERROR(ELOG_KEY, "[system] recv message invalid len: %d , fd(%d)", iRealLen, mSocket->FD());
                return ERR_MSGLEN;
            }
            if (iRealLen > (int)(MSG_BUFF_LEN + iLen - sizeof(int)))
            {
                LOG_DEBUG(ELOG_KEY, "[system] recv a part of message: real len = %d, now len = %d", iRealLen, MSG_BUFF_LEN - iLen - sizeof(int));
                return 0;
            }
            
            if (iRealLen <= (int)(iLeftLen - sizeof(int)))
            {
                HandleRecvMessage(mRecvRead + sizeof(int), iRealLen);  //消息解析，去除4字节消息长度标识位
                mRecvLen -= iRealLen + sizeof(int);
                mRecvRead += iRealLen + sizeof(int);
            }
            else
            {
                memcpy(mTmpBuff, mRecvRead, iLeftLen);
                memcpy(mTmpBuff + iLeftLen, mRecvBuff, iRealLen - iLeftLen + sizeof(int));
                
                HandleRecvMessage(mTmpBuff + sizeof(int), iRealLen);
                mRecvLen -= iRealLen + sizeof(int);
                mRecvRead = mRecvBuff + iRealLen - iLeftLen + sizeof(int);
            }
        }
    } while (true);
    
    return iRecvLen;
}

int wTask::TaskSend()
{
    char *pBuffEnd = mSendBuff + MSG_BUFF_LEN;  //超尾指针
    int iLen = 0, iSendLen = 0, iLeftLen = 0;
    do {        
        iLen = mSendWrite - mSendRead;
        if (mSendLen == 0)
        {
            return 0;
        }
        else if (iLen > 0)
        {
            iSendLen = mSocket->SendBytes(mSendRead, mSendLen); //iLen == mSendLen
            
            if (iSendLen < 0) return iSendLen;
            mSendLen -= iSendLen;
            mSendRead += iSendLen;
        }
        else if (iLen <= 0)
        {
            iLeftLen = pBuffEnd - mSendRead;
            memcpy(mTmpBuff, mSendRead, iLeftLen);
            memcpy(mTmpBuff + iLeftLen, mSendBuff, mSendLen - iLeftLen);
            iSendLen = mSocket->SendBytes(mTmpBuff, mSendLen);
            
            if (iSendLen < 0) return iSendLen;
            mSendLen -= iSendLen;
            mSendRead = mSendBuff + mSendLen - iLeftLen;
        }
        
        LOG_DEBUG(ELOG_KEY, "[system] send message len: %d, fd(%d)", iSendLen, mSocket->FD());
    } while (true);
    
    return iSendLen;
}

int wTask::Send2Buf(const char *pCmd, int iLen)
{
    //判断消息长度
    if ((iLen < MIN_PACKAGE_LEN) || (iLen > MAX_PACKAGE_LEN))
    {
        LOG_ERROR(ELOG_KEY, "[system] write message invalid len: %d , fd(%d)", iLen, mSocket->FD());
        return ERR_MSGLEN;
    }
    if (iLen > (int)(MSG_BUFF_LEN - mSendLen - sizeof(int)))
    {
        LOG_ERROR(ELOG_KEY, "[system] send buf not enough. send(%d) need(%d)", MSG_BUFF_LEN - mSendLen, iLen + sizeof(int));
        return ERR_NOBUFF;
    }
    
    char *pBuffEnd = mSendBuff + MSG_BUFF_LEN;  //超尾指针
    if (pBuffEnd - mSendWrite >= (int)(iLen + sizeof(int)))
    {
        *(int *)mSendWrite = iLen;
        memcpy(mSendWrite + sizeof(int), pCmd, iLen);
        mSendWrite += iLen + sizeof(int);
    }
    else
    {
        //先到临时缓冲区
        *(int *)mTmpBuff = iLen;
        memcpy(mTmpBuff + sizeof(int), pCmd, iLen);
        
        memcpy(mSendWrite, mTmpBuff, pBuffEnd - mSendWrite);
        memcpy(mSendBuff, mTmpBuff + (pBuffEnd - mSendWrite), iLen - (pBuffEnd - mSendWrite));
        mSendWrite = mSendBuff + iLen - (pBuffEnd - mSendWrite);
    }
    mSendLen += iLen + sizeof(int);
    
    LOG_DEBUG(ELOG_KEY, "[system] write message to buf(%d), len(%d)", MSG_BUFF_LEN - mSendLen, iLen);
    return iLen;
}

int wTask::SyncSend(const char *pCmd, int iLen)
{
    //判断消息长度
    if ((iLen < MIN_PACKAGE_LEN) || (iLen > MAX_PACKAGE_LEN))
    {
        LOG_ERROR(ELOG_KEY, "[system] write message invalid len: %d , fd(%d)", iLen, mSocket->FD());
        return ERR_MSGLEN;
    }
    
    *(int *)mTmpBuff = iLen;
    memcpy(mTmpBuff + sizeof(int), pCmd, iLen);
    return mSocket->SendBytes(mTmpBuff, iLen + sizeof(int));
}

int wTask::SyncRecv(char vCmd[], int iLen, int iTimeout)
{
    long long iSleep = 100;
    int iTryCount = iTimeout*1000000 / iSleep;
    int iCmdMsgLen = sizeof(int) + sizeof(struct wCommand);
    struct wCommand* pTmpCmd = 0;
    
    int iSize = 0, iRecvLen = 0;
    do {
        iSize = mSocket->RecvBytes(mTmpBuff + iRecvLen, iLen + sizeof(int) - iRecvLen);
        if (iSize < 0) break;
        iRecvLen += iSize;
        
        if ((iSize == 0) || (iRecvLen < iCmdMsgLen))
        {
            if (iTryCount-- < 0) break;
            usleep(iSleep);
            continue;
        }

        pTmpCmd = (struct wCommand*) (mTmpBuff + sizeof(int));	//心跳包干扰
        if (pTmpCmd != NULL && pTmpCmd->GetCmd() == CMD_NULL && pTmpCmd->GetPara() == PARA_NULL)
        {
            memmove(mTmpBuff, mTmpBuff + iCmdMsgLen, iRecvLen -= iCmdMsgLen);
            continue;
        }

        if (((size_t)iRecvLen < iLen + sizeof(int)) && (iTryCount-- > 0))
        {
            usleep(iSleep);
            continue;
        }
        break;
    } while (true);
    
    int iMsgLen = *(int *)mTmpBuff;
    if ((iRecvLen <= 0) || (iMsgLen < MIN_PACKAGE_LEN) || (iMsgLen > MAX_PACKAGE_LEN))
    {
        LOG_ERROR(ELOG_KEY, "[system] sync recv message invalid len: %d, fd(%d)", iMsgLen, mSocket->FD());
        return ERR_MSGLEN;
    }
    else if (iMsgLen > (int)(iRecvLen - sizeof(int)))	//消息不完整
    {
        LOG_DEBUG(ELOG_KEY, "[system] sync recv a part of message: real len = %d, now len = %d, call len = %d", iMsgLen, iRecvLen, iLen);
        return ERR_MSGLEN;
    }
    else if (iMsgLen != iLen)
    {
        LOG_DEBUG(ELOG_KEY, "[system] sync recv error buffer len");
        return ERR_MSGLEN;
    }
    
    memcpy(vCmd, mTmpBuff + sizeof(int), iLen);
    return iRecvLen - sizeof(int);
}
