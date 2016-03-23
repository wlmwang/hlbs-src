
#include "wCore.h"
#include "agent_api.h"

int main(int argc,char **argv)
{
	struct postHandle_t handle;

	cout << "connect:" <<ConnectAgent(&handle) << endl;

	struct SvrNet_t stSvr;
	stSvr.mGid = 1;
	stSvr.mXid = 1;

	string s;
	cout << "res:" << QueryNode(stSvr, 0.2, s) << endl;
	cout << "host:" << stSvr.mHost << endl;
	return 0;
}
