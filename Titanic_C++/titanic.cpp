// Titanic_C++.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <zmq.hpp>
#include "tmsg_api.h"
#include <Windows.h>
#include <Rpc.h>
#include <RpcDce.h>
#include <iostream>
#include <atlstr.h>
#include "titanic_persistence.h"
#include "titanic_dispatcher.h"
#include "titanic_broker.h"
#include "titanic_reply.h"
#include "titanic_request.h"
#include "titanic_finalize.h"
#include <czmq.h>

inline static void* start_reply(void* context){
	titanic_reply* reply = new titanic_reply((zctx_t*)context,TADD_INPROC,100000,10000);
	//Lets give the broker time to get bound up and set up all the pollers on its sockets
	zclock_sleep(100);
	reply->Start();
	delete reply;
	return NULL;
}
inline static void* start_finalize(void* context){
	titanic_finalize* finalize = new titanic_finalize((zctx_t*)context,TADD_INPROC,100000,10000,48*60*60*1000);
	//Lets give the broker time to get bound up and set up all the pollers on its sockets
	zclock_sleep(100);
	finalize->Start();
	delete finalize;
	return NULL;
}
inline static void* start_request(void* context){
	titanic_request* request = new titanic_request((zctx_t*)context,TADD_INPROC,100000,10000);
	//Lets give the broker time to get bound up and set up all the pollers on its sockets
	zclock_sleep(100);
	request->Start();
	delete request;
	return NULL;
}
int _tmain(int argc, _TCHAR* argv[])
{
	//we can instantiate this so that its binds up to all the sockets that we need
	//in the constructor.
	titanic_broker* broker = new titanic_broker("tcp://*:5555",TADD_INPROC,1);
	
	//zthread_new(start_request,broker->Context);
	zthread_new(start_reply,broker->Context);
	//zthread_new(start_finalize,broker->Context);

	broker->Start();

	delete broker;

}

