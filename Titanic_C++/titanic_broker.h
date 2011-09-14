#pragma once

#include <iostream>
#include <string>

using namespace std;

class titanic_broker
{
public:
	titanic_broker(string frontside,string backside);
	titanic_broker(string frontside,string backside,int verbose);
	~titanic_broker(void);

	string FrontSide;
	string BackSide;

	zctx_t* Context;				//ZMQ context used to spin up the service.
	void* FrontSide_Sckt;			//INPROC Pipe for Broker to service communciations
	void* BackSide_Sckt;			//INPROC Pipe for Broker to service communciations
	int Verbose;					//Settable log level

	void Start(void);
private:
	void process_msg(zmsg_t* msg);
	void message_from_client(zmsg_t* msg);
	void message_from_worker(zmsg_t* msg);

	void message_to_worker(zmsg_t* msg);
	void message_to_client(zmsg_t* msg);

	bool service_available(string svc_name);

};

