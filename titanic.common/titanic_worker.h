#include "stdafx.h"
#pragma once


class __declspec(dllexport) titanic_worker
{
public:
	titanic_worker(char* broker_address,char* svcname,int64_t ivl);
	~titanic_worker(void);
	
	zmsg_t* get_work(void);
	void send_complete(zmsg_t* body);
	
	char* ServiceName;
	
	//Zmq Pieces.
	zctx_t* Context;
	void* Socket;
	int32_t Heartbeat_At;
	int64_t Heartbeat_Ivl;
	char* Broker_Address;

private:

	bool has_connected;
	char* req_uuid;
	void connect(void);
	void heart_beat(void);
	char* strip_framing(zmsg_t* msg);
	void append_framing(zmsg_t* msg,char* command,char* uuid);
};

