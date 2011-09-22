#include "stdafx.h"
#include <czmq.h>
#include "titanic_finalize.h"
#include <titanic_component.h>
#include <tmsg_api.h>
#include <titanic_persistence.h>

using namespace std;

titanic_finalize::titanic_finalize(zctx_t* ctx,string brokername,int hbeat,int reconn,int cleanupivl)
	:titanic_component(ctx,"titanic.finalize",brokername,ZMQ_XREP,hbeat,reconn)
{
	this->Cleanup_Ivl = cleanupivl;
}


titanic_finalize::~titanic_finalize(void){
}

void titanic_finalize::Start(){
	this->connect_to_broker();
	cout<<"Starting Finalize \n\r";

    while (TRUE) {
        zmsg_t *incoming = this->get_work();
        if (!incoming)
            break;      //  Interrupted, exit
        
		//take the message out of the envelope.
		zframe_t* envelope = zmsg_unwrap(incoming);

		//lets get the frame we want. the last one.
		zframe_t* origin = zmsg_pop(incoming); //version of the titanic service
		zframe_t* service = zmsg_pop(incoming);//TMSG_TYPE
		zframe_t* command = zmsg_pop(incoming);
		char* uuid= zmsg_popstr(incoming);

		if(titanic_persistence::finalize(uuid)){
			//return a 200 back to the client
			zmsg_pushstr(incoming,TMSG_STATUS_OK);
		}
		else{
			//return a 500 to the client.
			zmsg_addstr(incoming,TMSG_STATUS_ERROR);
		}
		//Since we are replying direcly to the client lets put the message back in the envelope.
		zmsg_pushstr(incoming,uuid);
		zmsg_push(incoming,command);
		zmsg_push(incoming,service);
		zmsg_pushstr(incoming,TWRK_SVC_VER);
		zmsg_wrap(incoming,envelope);
		zmsg_send(&incoming ,this->Context);

		//Clean up after ourselves.
        zmsg_destroy (&incoming);
        free (&uuid);
	}
}