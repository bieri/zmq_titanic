#include "StdAfx.h"
#include "titanic_reply.h"
#include "tmsg_api.h"
#include "titanic_persistence.h"
#include <czmq.h>
#include <list>
#include <iostream>

titanic_reply::titanic_reply(zctx_t* ctx,string brokername,int hbeat,int reconn):
	titanic_component(ctx,"titanic.reply",brokername,ZMQ_XREP,hbeat,reconn){
}

titanic_reply::~titanic_reply(void){
}

//Main loop for this. This is done this way so this can be moved to a different process.
void titanic_reply::Start(){
	
	this->connect_to_broker();
    zmsg_t *reply = NULL;
	cout<<"Starting Reply \n\r";
    while (TRUE) {
        zmsg_t *incoming = this->get_work();
        if (!incoming)
            break;      //  Interrupted, exit

		zframe_t* envelope = zmsg_unwrap(incoming);
		char* origin = zmsg_popstr(incoming);
		zframe_t* service = zmsg_pop(incoming);
		char* command = zmsg_popstr(incoming);
		char* uuid = zmsg_popstr(incoming);

				
		//disassemble our incoming and assemble the reply, all in one operation.
		if(strcmp(origin,TWRK_WRK_VER)==0){
			this->message_from_worker(incoming,envelope,origin,service,command,uuid);
		}
		if(strcmp(origin,TWRK_CLI_VER)==0){
			zmsg_destroy(&incoming);
			incoming = this->message_from_client(envelope,origin,service,command,uuid);
			zmsg_send(&incoming,this->Pipe);
		}
		
        zmsg_destroy (&incoming);
		zmsg_destroy (&reply);
        free (uuid);
    }

}
zmsg_t* titanic_reply::message_from_client(zframe_t* envelope,char* origin,zframe_t* service,char* command,char* uuid){
	zmsg_t* reply;
	if(titanic_persistence::exists(TMSG_TYPE_REPLY,uuid)){
		reply = titanic_persistence::get(TMSG_TYPE_REPLY,uuid);
	}
    else {
		reply = zmsg_new();
		if (titanic_persistence::exists(TMSG_TYPE_REQUEST,uuid))
            zmsg_pushstr (reply, TMSG_STATUS_PENDING); //Pending
        else
            zmsg_pushstr (reply, TMSG_STATUS_UKNOWN); //Unknown
		
		zmsg_pushstr(reply,uuid);
		zmsg_pushstr(reply,command);
		zmsg_push(reply,service);
    }
	
	zmsg_pushstr(reply,origin);
	zmsg_wrap(reply,envelope);
	return reply;
}
void titanic_reply::message_from_worker(zmsg_t* reply,zframe_t* envelope,char* origin,zframe_t* service,char* command,char* uuid){
	zmsg_pushstr(reply,TMSG_STATUS_OK);
	zmsg_pushstr(reply,uuid);
	zmsg_pushstr(reply,command);
	zmsg_push(reply,service);

	if(titanic_persistence::store(TMSG_TYPE_REPLY,uuid,reply)){
		assert("failed to save");
	}
}