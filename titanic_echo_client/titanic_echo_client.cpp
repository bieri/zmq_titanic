// titanic_echo_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <czmq.h>
#include "tmsg_api.h"

int _tmain(int argc, _TCHAR* argv[])
{
	zclock_sleep(10000);
	char* broker_loc = "tcp://localhost:5555";

	zctx_t* ctx = zctx_new();
	void* scket = zsocket_new(ctx,ZMQ_REQ);
	zsocket_connect(scket,broker_loc);
	zmsg_t* msg = zmsg_new();
	zmsg_addstr(msg,TWRK_CLI_VER);
	zmsg_addstr(msg,"Echo");
	zmsg_addstr(msg,TMSG_TYPE_REQUEST);
	
	int64_t sleep = 1000*1000;
	zclock_sleep(sleep);
	zmsg_add(msg,zframe_new(&sleep,sizeof(sleep)));
	zmsg_send(&msg,scket);
	zmsg_t* reply =  zmsg_recv(scket);

	return 0;
}

