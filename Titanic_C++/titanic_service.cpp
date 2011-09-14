#include <czmq.h>
#include <iostream>
#include "titanic_component.h"
#include "tmsg_api.h"

using namespace std;

//Constructor -----------------------------------------------------------------------------------------------------------------
titanic_component::titanic_component(string svcname,string brokername,int zmqsvctype,int hbeat,int reconivl){
	this->Context = zctx_new();
	this->Svc_Name = svcname;
	this->Broker_Name = brokername;
	this->ZmqSocketType =zmqsvctype;
	this->HeartBeat_Ivl = hbeat;
	this->Reconnect_Ivl = reconivl;
	this->Verbose =0;
}
titanic_component::titanic_component(string svcname,string brokername,int zmqsvctype,int hbeat,int reconivl,int verb){
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
zmsg_t* titanic_component::get_work(char* msg_type_tohandle){
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
			
		//validate the message structure.
            //every api has an empty frame.
			zframe_t *empty = zmsg_pop (msg);
            assert (zframe_streq (empty, ""));
            //zframe_destroy (&empty);
			
			//double check to make sure that the message is coming in an acceptable version.
            zframe_t *header = zmsg_pop (msg);
            assert (zframe_streq (header, TWRK_SVC_VER));
            //zframe_destroy (&header);

			//get the message type.
            zframe_t * msgtype = zmsg_pop (msg);
            if (zframe_streq (msgtype, msg_type_tohandle)) {
				//lets reassemble the message and punt it off. Nothing we need to know about.
                zmsg_push(msg,msgtype);
				zmsg_push(msg,header);
				zmsg_push(msg,empty);
                return msg;     //  We have a request to process
            }
            else
            if (zframe_streq (msgtype, TMSG_TYPE_HEARTBEAT))
                ;               //  Do nothing for heartbeats
            else
            if (zframe_streq (msgtype, TMSG_TYPE_DISCONNECT))
				this->connect_to_broker();
            else {
                zclock_log ("E: invalid input message");
                zmsg_dump (msg);
            }
            zframe_destroy (&msgtype);
            zmsg_destroy (&msg);
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

void titanic_component::set_Heartbeat_Ivl(int newval){
	this->HeartBeat_Ivl = newval;
}
void titanic_component::set_Reconnect_Ivl(int newval){
	this->HeartBeat_Ivl = newval;
}
