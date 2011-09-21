// Titanic_C++.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <zmq.hpp>
#include <Windows.h>
#include <Rpc.h>
#include <RpcDce.h>
#include <iostream>
#include <atlstr.h>
#include "titanic_dispatcher.h"
#include "titanic_broker.h"
#include <czmq.h>

int _tmain(int argc, _TCHAR* argv[])
{
	//we can instantiate this so that its binds up to all the sockets that we need
	//in the constructor.
	titanic_broker* broker = new titanic_broker(TADD_PUB,TADD_PRIV,1);
	
	//zthread_new(start_request,broker->Context);
	//zthread_new(start_reply,broker->Context);
	//zthread_new(start_finalize,broker->Context);

	broker->Start();

	delete broker;

}

