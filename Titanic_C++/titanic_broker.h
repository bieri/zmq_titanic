#pragma once

#include <iostream>
#include <string>
#include "titanic_dispatcher.h"
#include "titanic_types.h"
using namespace std;

class titanic_broker
{
public:
	titanic_broker(string address,string compaddress);
	titanic_broker(string frontside,string compaddress,INT_ verbose);
	~titanic_broker(void);

	char* Sckt_Address;
	char* Comp_Address;
	zctx_t* Context;				//ZMQ context used to spin up the service.
	void* Z_Sckt;					//Public socket for everyone else to talk to.
	void* Inproc_Sckt;				//INPROC Pipe for Broker to service communciations
	int Verbose;					//Settable log level

	void init(void);
	void Start(void);

	titanic_dispatcher* Dispatcher;
private:
	void heartbeat(void);
	void process_msg(zmsg_t* msg);

	void send_to_component(zmsg_t* msg,char* command,char* service,char* origin,zframe_t* envelope,char* scktid);

	void message_from_client(zmsg_t* body,zframe_t* envelope,char* origin,char* service,char* command,char*uuid);
	void message_from_component(zmsg_t* body,zframe_t* envelope,char* origin,char* service,char* command,char*uuid);
	void message_from_worker(zmsg_t* body,zframe_t* envelope,char* origin,char* service,char* command,char*uuid);

};

