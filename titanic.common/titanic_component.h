#include <iostream>
#include <string.h>
#include <czmq.h>
using namespace std;
#pragma once


class __declspec(dllexport) titanic_component
{
protected:
	//Constructor Properties
	string Svc_Name;			//Name of this service for communication purposes.
	string Broker_Name;			//Name of the broker that we are going to chat with for Inproc
	int64_t HeartBeat_Ivl;			//Interval at which we send our "I'm Alive!"
	int64_t Reconnect_Ivl;			//Interval at which we retry to get to the broker.
	int64_t ZmqSocketType;			//ZMQ_DEALER,ZMQ_XREP, etc.... As defined in czmq.h

	zctx_t* Context;			//ZMQ context used to spin up the service.
	void* Pipe;					//INPROC Pipe for Broker to service communciations
	int Verbose;				//Settable log level

	int connect_to_broker();	//Returns true or false (1 / 0)
	int send_to_broker(char* messagetype,zmsg_t* msg);
	int work_finished(zmsg_t* msg);
	zmsg_t* get_work();

private:
	//properties
	int64_t heartbeat_at;			//Time in MS (ticks) when the heart beat needs to happen. Saves cycles.
	bool isConnected;
public:
	//Constructor
	titanic_component(zctx_t* ctx,string svcname,string brokername,int zmqsckttype,int64_t hbeativl,int64_t reconivl);
	titanic_component(zctx_t* ctx,string svcname,string brokername,int zmqsckttype,int64_t hbeativl,int64_t reconivl,int verbose);
	
	void set_Heartbeat_Ivl(int64_t newval);
	void set_Reconnect_Ivl(int64_t newval);

	~titanic_component();
	virtual void Start(){};
};

