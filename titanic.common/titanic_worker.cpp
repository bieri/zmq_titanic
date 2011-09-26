#include "StdAfx.h"
#include "titanic_worker.h"
#include "tmsg_api.h"

titanic_worker::titanic_worker(char* broker_address,int64_t hbeat_ivl)
{
	this->Broker_Address = broker_address;
	this->Heartbeat_Ivl = hbeat_ivl;
}


titanic_worker::~titanic_worker(void)
{
	delete this->Broker_Address;
		zsocket_destroy(this->Context,this->Socket);
				zctx_destroy(&this->Context);
}

zmsg_t* titanic_worker::get_work(void){
	if(!this->has_connected)
		this->connect();

	zmsg_t* r_msg = zmsg_new();
	this->append_framing(r_msg,TMSG_TYPE_READY,"");
	zmsg_send(&r_msg,this->Socket);

	
	while(TRUE){
		zmq_pollitem_t items [] = {
            { this->Socket,  0, ZMQ_POLLIN, 0 }
			};

        int rc = zmq_poll (items,1 ,1000  * ZMQ_POLL_MSEC);
		if (items [0].revents & ZMQ_POLLIN) {
			zmsg_t *msg = zmsg_recv (this->Socket);
            if (!msg)
                break;          //  Int
			char* cmd = this->strip_framing(msg);
			
			if(strcmp(cmd,TMSG_TYPE_REQUEST)==0){
				return msg;
			}
			
			if(strcmp(cmd,TMSG_TYPE_DISCONNECT)==0){
				zsocket_destroy(this->Context,this->Socket);
				zctx_destroy(&this->Context);
				break;
			}
		}
		if(zclock_time()>this->Heartbeat_At){
			this->heart_beat();
			this->Heartbeat_At = zclock_time() + this->Heartbeat_Ivl;
		}
	}
}

void titanic_worker::send_complete(zmsg_t* body){
	this->append_framing(body,TMSG_TYPE_REPLY,this->req_uuid);
	zmsg_send(&body,this->Socket);
	
	delete this->req_uuid;

}
	
void titanic_worker::connect(void){
	if(!this->has_connected){
		this->Context = zctx_new();
		this->Socket = zsocket_new(this->Context,ZMQ_ROUTER);
		zsocket_connect(this->Socket,this->Broker_Address);
	}
}

void titanic_worker::heart_beat(void){
	zmsg_t* r_msg = zmsg_new();
	append_framing(r_msg,TMSG_TYPE_HEARTBEAT,"");
	zmsg_send(&r_msg,this->Socket);
}

char* titanic_worker::strip_framing(zmsg_t* msg){
	//remove all the bs frames.
	zframe_t* env = zmsg_unwrap(msg);
	zframe_t* org = zmsg_pop(msg);
	zframe_t* svc = zmsg_pop(msg);
	char* cmd = zmsg_popstr(msg);
	this->req_uuid = zmsg_popstr(msg);
	return cmd;
}

void titanic_worker::append_framing(zmsg_t* msg,char* command,char* uuid){
	if(msg==NULL)
		msg = zmsg_new();
	
	if(strlen(uuid)!=0)
		zmsg_pushstr(msg,uuid);

	zmsg_pushstr(msg,command);
	zmsg_pushstr(msg,this->ServiceName);
	zmsg_pushstr(msg,TWRK_WRK_VER);
	zmsg_pushstr(msg,"");
	zmsg_pushstr(msg,"titanic.frontend");
	
}