#include "StdAfx.h"
#include <czmq.h>
#include "titanic_broker.h"
#include "tmsg_api.h"
#include "titanic_persistence.h"


titanic_broker::titanic_broker(string frontside)
{
	this->Context = zctx_new();
	this->Z_Sckt = zsocket_new(this->Context,ZMQ_ROUTER);  //Public facing socket
	this->Inproc_Sckt = zsocket_new(this->Context,ZMQ_DEALER); //Socket to connect our services to.
	this->Verbose = 0;
}
titanic_broker::titanic_broker(string frontside,int verbose)
{
	this->Context = zctx_new();
	this->Z_Sckt = zsocket_new(this->Context,ZMQ_DEALER);
	this->Inproc_Sckt = zsocket_new(this->Context,ZMQ_DEALER); //Socket to connect our services to.
	this->Verbose = verbose;
}

titanic_broker::~titanic_broker(void)
{
	zsocket_destroy(this->Context,this->Z_Sckt);
	zsocket_destroy(this->Context,this->Inproc_Sckt);
	zctx_destroy(&this->Context);
	free(this->Sckt_Address);
	this->Verbose = NULL;

    free (this);
}
void titanic_broker::send_to_component(zmsg_t* msg,char* command,char* service,char* origin,zframe_t* envelope,char* scktid){
	zmsg_pushstr(msg,command);
	zmsg_pushstr(msg,service);
	zmsg_pushstr(msg,origin);
	zmsg_wrap(msg,envelope);
	zmsg_pushstr(msg,scktid);
	zmsg_send(&msg,this->Inproc_Sckt);
}

void titanic_broker::Start(void){
	//bind up our sockts
	zsocket_bind(Z_Sckt,this->Sckt_Address);
	zsocket_bind(this->Inproc_Sckt,TADD_INPROC);

    //  Get and process messages forever or until interrupted
    while (TRUE) {
        zmq_pollitem_t items [] = {
            { this->Z_Sckt,  0, ZMQ_POLLIN, 0 },
			{ this->Inproc_Sckt,0,ZMQ_POLLIN,0}};

        int rc = zmq_poll (items, 1,100  * ZMQ_POLL_MSEC);
        if (rc == -1)
            break;              //  Interrupted

        //  Process next input message, if any
        if (items [0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = zmsg_recv (this->Z_Sckt);
            if (!msg)
                break;          //  Interrupted
            if (this->Verbose) {
                zclock_log ("I: received message:");
                zmsg_dump (msg);
            }
			zframe_t* envelope = zmsg_unwrap(msg);  //address and empty frame.
            char* origin = zmsg_popstr (msg);		//origin frame.
            char* service = zmsg_popstr (msg);		//service frame.
            char* command = zmsg_popstr (msg);		//command frame.
			char* uuid;
			//Remaining frames are the 

			if(strcmp(origin,TWRK_CLI_VER)==0){
				//Client
				if(strcmp(command,TMSG_TYPE_REQUEST)){
					//send it to the request handler
					this->send_to_component(msg,command,service,origin,envelope,"titanic.request");
				}
				else if(strcmp(command,TMSG_TYPE_REPLY)){
					this->send_to_component(msg,command,service,origin,envelope,"titanic.reply");
				}
				else if(strcmp(command,TMSG_TYPE_FINALIZE)){
					//send it to the reply handler on a different thread.
					uuid = zmsg_popstr(msg);
					if(!titanic_persistence::exists(TMSG_TYPE_REPLY,uuid)){
						this->Dispatcher->Dequeue(string(uuid),string(""));
					}
					else{
						//send to finalizer.
						zmsg_pushstr(msg,uuid);
						this->send_to_component(msg,command,service,origin,envelope,"titanic.finalize");
					}
				}
			}
			else if(strcmp(origin,TWRK_SVC_VER)==0){
				//Internal Titanic Component
				//We are assuming that they will be ready and alive by the time that we get to here.
				//this is prolly a bad assumption but its one i am willing to make for right now.
				if(strcmp(command,TMSG_TYPE_REQUEST)==0){
					//this is going to be coming back from the request service.
					uuid = zmsg_popstr(msg);
					//send the value back to the 
					zmsg_t* clientmsg = zmsg_new();
					zmsg_add(clientmsg,zframe_dup(envelope));
					char* i1;
					zmsg_addstr(clientmsg,strcpy(i1,origin));
					char* i2;
					zmsg_addstr(clientmsg,strcpy(i2,service));
					char* i3;
					zmsg_addstr(clientmsg,strcpy(i3,command));
					char* i4;
					zmsg_addstr(clientmsg,strcpy(i4,uuid));
					
					zmsg_send(&clientmsg,this->Z_Sckt);
					zmsg_destroy(&clientmsg);

					//lets queue up the work now.
					this->Dispatcher->Enqueue(string(uuid),string(service),msg);
				}
				else if(strcmp(command,TMSG_TYPE_REPLY)){
					this->send_to_component(msg,command,service,origin,envelope,"titanic.reply");
				}
			}
			else if(strcmp(origin,TWRK_WRK_VER)==0){
				//Worker
				if(strcmp(command,TMSG_TYPE_READY)){
					this->Dispatcher->Handle_Ready(envelope,string(service));
				}
				else if(strcmp(command,TMSG_TYPE_HEARTBEAT)){
					this->Dispatcher->Handle_HeartBeat(envelope,string(service));
				}
				else if(strcmp(command,TMSG_TYPE_REPLY)){
					//Need to mark this as completed.
					char* uuid = zmsg_popstr(msg);
					this->Dispatcher->Dequeue(string(uuid),string(service));
					zmsg_pushstr(msg,uuid);
					//send it to the reply handler on a different thread to save to the file system.
					this->send_to_component(msg,command,service,origin,envelope,"titanic.request");
					//send it to the publisher so that we can notify those clients that dont want to ping.
					this->send_to_component(msg,command,service,origin,envelope,"titanic.publish");
					
					
				}
			}
        }
        
    }
    if (zctx_interrupted)
        printf ("W: interrupt received, shutting down…\n");
}
void titanic_broker::heartbeat(void){
	//  Disconnect and delete any expired workers
        //  Send heartbeats to idle workers if needed
        if (zclock_time () > this->heartbeat_at) {
            s_broker_purge_workers (self);
            worker_t *worker = (worker_t *) zlist_first (self->waiting);
            while (worker) {
                s_worker_send (self, worker, MDPW_HEARTBEAT, NULL, NULL);
                worker = (worker_t *) zlist_next (self->waiting);
            }
            self->heartbeat_at = zclock_time () + HEARTBEAT_INTERVAL;
        }
}

void titanic_broker::process_msg(zmsg_t* msg){
	 assert (zmsg_size (msg) >= 1);     //  At least, command

	  if (zframe_streq (header, TWRK_CLI_VER)){
		        s_client_process (self, sender, msg);
				this->message_from_client(msg);
	  }
      else{
		  this->message_from_worker(msg);
            if (zframe_streq (header, TWRK_WRK_VER))
                s_worker_process (self, sender, msg);
            else {
                zclock_log ("E: invalid message:");
                zmsg_dump (msg);
                zmsg_destroy (&msg);
            }
	  }
	 //for client
	 

	 //for services.
	 

}

//Process an incoming message from a client.
void titanic_broker::message_from_client(zmsg_t* msg){

}
//Process an incoming message from a worker. This is where we store the result for the UUID.
void titanic_broker::message_from_worker(zmsg_t* msg){
	
}

void titanic_broker::message_to_client(zmsg_t* msg){

}

