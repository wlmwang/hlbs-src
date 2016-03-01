
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "RouterMaster.h"

RouterMaster::RouterMaster()
{
	Initialize();
}

RouterMaster::~RouterMaster()
{
	//
}

void RouterMaster::Initialize()
{
	//
}

void RouterMaster::PrepareRun()
{
	//
}

void RouterMaster::Run()
{
	//
}

wWorker* RouterMaster::NewWorker()
{
	return new RouterWorker();
}