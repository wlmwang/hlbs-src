
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wCore.h"
#include "wLog.h"
#include "wMisc.h"
#include "wConfig.h"
#include "wPing.h"

int main(int argc,char **argv)
{
	wPing *pPing = new wPing();
	int iRet = 0;
	if(pPing->Open() > 0 && pPing->SetTimeout(0.1) >= 0)
	{
		iRet = pPing->Ping("192.168.8.13");
	}

	cout << "ret:" << iRet << endl;
	return 0;
}
