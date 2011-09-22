// titanic.finalize.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "titanic_finalize.h"

int _tmain(int argc, _TCHAR* argv[])
{
	zctx_t* context = zctx_new();
	titanic_finalize* component = new titanic_finalize(context,TADD_COMP,100*1000,100*1000,48*60*1000);
	//Lets give the broker time to get bound up and set up all the pollers on its sockets
	zclock_sleep(1000);
	cout << "Starting Finalizer";
	component->Start();
	delete component;
	return NULL;
}

