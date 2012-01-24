// titanic.publish.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "titanic_publish.h"

int _tmain(int argc, _TCHAR* argv[])
{
	zctx_t* context = zctx_new();
	titanic_publish* component = new titanic_publish(context,TADD_COMP,TADD_PUBSUB,100*1000,100*1000);
	//Lets give the broker time to get bound up and set up all the pollers on its sockets
	zclock_sleep(1000);
	cout << "Starting Publisher";
	component->Start();
	delete component;
	return NULL;
}

