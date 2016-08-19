
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "agent_api.h"

int GPSvr()
{
	struct SvrNet_t stSvr;
	stSvr.mGid = 1;
	stSvr.mXid = 2;

	string s;
	int iRet = QueryNode(stSvr, 30, s);

	cout << "ret:" << iRet << endl;
	cout << "host:" << stSvr.mHost << endl;
	cout << "port:" << stSvr.mPort << endl;

	if (iRet >= 0)	return NotifyCallerRes(stSvr, 0, 2000, s);
	
	return -1;
}

int main(int argc, char *argv[])
{
	//ConnectAgent();

	int errnum = 0;
	for (int i = 0; i < 1; i++)
	{
		if (GPSvr() < 0) errnum++;
	}
	cout << "errnum" << errnum << endl;

	//CloseAgent();
	return 0;
}
