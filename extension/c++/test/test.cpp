
#include "wCore.h"
#include "agent_api.h"

int GPSvr()
{
	struct SvrNet_t stSvr;
	stSvr.mGid = 1;
	stSvr.mXid = 2;

	string s;
	cout << "res:" << QueryNode(stSvr, 30, s) << endl;
	cout << "host:" << stSvr.mHost << endl;
	cout << "port:" << stSvr.mPort << endl;

	if (stSvr.mHost[0] != 0 && stSvr.mPort > 0)
	{
		cout << "res:" << NotifyCallerRes(stSvr, 0, 2000, s) << endl;
	}
}

int main(int argc, char *argv[])
{
	(ConnectAgent() >= 0) && GPSvr();
	CloseAgent();
	return 0;
}
