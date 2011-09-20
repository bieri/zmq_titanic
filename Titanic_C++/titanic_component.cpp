#include <czmq.h>
#include <iostream>
#include "titanic_component.h"
#include "tmsg_api.h"

using namespace std;

//Constructor -----------------------------------------------------------------------------------------------------------------
titanic_component::titanic_component(string svcname,string brokername,int zmqsvctype,int64_t hbeat,int64_t reconivl){
	this->Context = zctx_new();
	this->Svc_Name = svcname;
	this->Broker_Name = brokername;
	this->ZmqSocketType =zmqsvctype;
	this->HeartBeat_Ivl = hbeat;
	this->Reconnect_Ivl = reconivl;
	this->Verbose =0;
}
titanic_component::titanic_component(string svcname,string brokername,int zmqsvctype,int64_t hbeat,int64_t reconivl,int verb){
	this->Context = zctx_new();
	this->Svc_Name = svcname;
	this->Broker_Name = brokername;
	this->ZmqSocketType =zmqsvctype;
	this->HeartBeat_Ivl = hbeat;
	this->Reconnect_Ivl = reconivl;
	this->Verbose = verb;
}

//Destructor -----------------------------------------------------------------------------------------------------------------
titanic_component::~titanic_component(){
	
	zsocket_destroy(this->Context,this->Pipe);
	zctx_destroy (&this->Context);

	this->Broker_Name.clear();
	this->Svc_Name.clear();
	this->heartbeat_at = NULL;
	this->HeartBeat_Ivl = NULL;
	this->Reconnect_Ivl = NULL;
	this->Verbose = NULL;
    free (this);
}

//First part of the initialization process.
int titanic_component::connect_to_broker(){
	if (this->Pipe)
        zsocket_destroy (this->Context, this->Pipe);
    this->Pipe = zsocket_new (this->Context, this->ZmqSocketType);
    zmq_connect (this->Pipe, this->Broker_Name.c_str());
    if (this->Verbose)
        zclock_log ("I: connecting to broker at %s...", this->Broker_Name);

    //  Register service with broker
	this->send_to_broker (TMSG_TYPE_READY, NULL);
    
	this->heartbeat_at = zclock_time () + this->HeartBeat_Ivl;

	return 1;
}

//Send a message to the broker. Return 1 for success, 0 for failure.
//expects that you will be sending any frames that are not part of the
//message envelope.
int titanic_component::send_to_broker(char* messagetype,zmsg_t* msg){
	
	msg = msg? zmsg_dup (msg): zmsg_new ();

    //  Stack protocol envelope to start of the message in reverse order.
	//	that way that our message confirms to the specification set in "tmsg_api.h"
    zmsg_addstr (msg, this->Svc_Name.c_str());
	zmsg_addstr (msg, TWRK_SVC_VER);
	zmsg_pushstr(msg,"");

    if (this->Verbose) {
        zclock_log ("I: sending %s to broker",TMSG_TYPES [(int) *messagetype]);
        zmsg_dump (msg);
    }

    zmsg_send (&msg, this->Pipe);
	return 1;
}

//This makes an assumption that we are only handling one type of message.
zmsg_t* titanic_component::get_work(){
	while (TRUE) {
        zmq_pollitem_t items [] = {
            { this->Pipe,  0, ZMQ_POLLIN, 0 } };
        int rc = zmq_poll (items, 1, this->HeartBeat_Ivl* ZMQ_POLL_MSEC);
        if (rc == -1)
            break;              //  Interrupted

        if (items [0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = zmsg_recv (this->Pipe);
            if (!msg)
                break;          //  Interrupted
            if (this->Verbose) {
                zclock_log ("I: received message from broker:");
                zmsg_dump (msg);
            }
            
            //  Don't try to handle errors, just assert noisily
            assert (zmsg_size (msg) >= 3);
            return msg;     //  We have a request to process
        }
        
        //  Send HEARTBEAT if it's time
        if (zclock_time () > this->heartbeat_at) {
			this->send_to_broker (TMSG_TYPE_HEARTBEAT,NULL);
            this->heartbeat_at = zclock_time () + this->HeartBeat_Ivl;
        }
    }
    if (zctx_interrupted)
        printf ("W: interrupt received, killing worker...\n");
    return NULL;
}

void titanic_component::set_Heartbeat_Ivl(int64_t newval){
	this->HeartBeat_Ivl = newval;
}
void titanic_component::set_Reconnect_Ivl(int64_t newval){
	this->HeartBeat_Ivl = newval;
}
