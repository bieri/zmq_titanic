#pragma once

#include <iostream>
#include <string>
#include "titanic_dispatcher.h"

using namespace std;

class titanic_broker
{
public:
	titanic_broker(string address,string backside);
	titanic_broker(string frontside,string backside,int verbose);
	~titanic_broker(void);

	string Sckt_Address;
	zctx_t* Context;				//ZMQ context used to spin up the service.
	void* Z_Sckt;					//INPROC Pipe for Broker to service communciations
	int Verbose;					//Settable log level

	void Start(void);
	titanic_dispatcher* Dispatcher;
private:
	void process_msg(zmsg_t* msg);
	void message_from_client(zmsg_t* msg);
	void message_from_worker(zmsg_t* msg);

	void message_to_worker(zmsg_t* msg);
	void message_to_client(zmsg_t* msg);

	bool service_available(string svc_name);

};

