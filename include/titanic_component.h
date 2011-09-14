#include <iostream>
#include <czmq.h>

using namespace std;

class titanic_component{
protected:
	//Constructor Properties
	string Svc_Name;			//Name of this service for communication purposes.
	string Broker_Name;			//Name of the broker that we are going to chat with for Inproc
	int HeartBeat_Ivl;			//Interval at which we send our "I'm Alive!"
	int Reconnect_Ivl;			//Interval at which we retry to get to the broker.
	int ZmqSocketType;			//ZMQ_DEALER,ZMQ_XREP, etc.... As defined in czmq.h

	zctx_t* Context;			//ZMQ context used to spin up the service.
	void* Pipe;					//INPROC Pipe for Broker to service communciations
	int Verbose;				//Settable log level

	int connect_to_broker();	//Returns true or false (1 / 0)
	int send_to_broker(char* messagetype,zmsg_t* msg);
	int work_finished(zmsg_t* msg);
	zmsg_t* get_work(char* msg_type_tohandle);

private:
	//properties
	int heartbeat_at;			//Time in MS (ticks) when the heart beat needs to happen. Saves cycles.
	
public:
	//Constructor
	titanic_component(string svcname,string brokername,int zmqsckttype,int hbeativl,int reconivl);
	titanic_component(string svcname,string brokername,int zmqsckttype,int hbeativl,int reconivl,int verbose);
	
	void set_Heartbeat_Ivl(int newval);
	void set_Reconnect_Ivl(int newval);

	~titanic_component();
	virtual void Start(){};
};