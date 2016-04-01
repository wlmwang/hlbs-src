
#include "wCore.h"
#include "agent_api.h"

int Get()
{
	struct SvrNet_t stSvr;
	stSvr.mGid = 1;
	stSvr.mXid = 2;

	string s;
	cout << "res:" << QueryNode(stSvr, 0.2, s) << endl;
	cout << "host:" << stSvr.mHost << endl;
	cout << "port:" << stSvr.mPort << endl;
}

int Post()
{
	SvrNet_t stSvr;
	stSvr.mGid = 1;
	stSvr.mXid = 2;
	stSvr.mPort = 3306;
	memcpy(stSvr.mHost,"192.168.8.14", sizeof("192.168.8.14"));
	
	string s;
	cout << "res:" << NotifyCallerRes(stSvr, 0, 2000, s) << endl;
	cout << "host:" << stSvr.mHost << endl;
}

int GPSvr()
{
	struct SvrNet_t stSvr;
	stSvr.mGid = 1;
	stSvr.mXid = 2;

	string s;
	cout << "res:" << QueryNode(stSvr, 0.2, s) << endl;
	cout << "host:" << stSvr.mHost << endl;
	cout << "port:" << stSvr.mPort << endl;

	if (stSvr.mHost[0] != 0 && stSvr.mPort > 0)
	{
		cout << "res:" << NotifyCallerRes(stSvr, 0, 2000, s) << endl;
	}
}

int main(int argc,char **argv)
{
	//Get();
	//Post();
	GPSvr();
	return 0;
}
