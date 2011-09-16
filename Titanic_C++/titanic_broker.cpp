#include "StdAfx.h"
#include <czmq.h>
#include "titanic_broker.h"
#include "tmsg_api.h"


titanic_broker::titanic_broker(string frontside)
{
	this->Context = zctx_new();
	this->Z_Sckt = zsocket_new(this->Context,ZMQ_DEALER);
	this->Verbose = 0;
}
titanic_broker::titanic_broker(string frontside,int verbose)
{
	this->Context = zctx_new();
	this->Z_Sckt = zsocket_new(this->Context,ZMQ_DEALER);
	this->Verbose = verbose;
}

titanic_broker::~titanic_broker(void)
{
	zsocket_destroy(this->Context,this->Z_Sckt);

	zctx_destroy(&this->Context);
	this->Sckt_Address.clear();
	this->Verbose = NULL;

    free (this);
}

void titanic_broker::Start(void){

    //  Get and process messages forever or until interrupted
    while (TRUE) {
        zmq_pollitem_t items [] = {
            { this->Z_Sckt,  0, ZMQ_POLLIN, 0 } };
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
            zframe_t *sender = zmsg_pop (msg);
            zframe_t *empty  = zmsg_pop (msg);
            zframe_t *header = zmsg_pop (msg);

           
            zframe_destroy (&sender);
            zframe_destroy (&empty);
            zframe_destroy (&header);
        }
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
    if (zctx_interrupted)
        printf ("W: interrupt received, shutting down…\n");

    s_broker_destroy (&self);
    return 0;
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

