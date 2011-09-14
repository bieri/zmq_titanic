#include "StdAfx.h"
#include "titanic_reply.h"
#include "tmsg_api.h"
#include "titanic_persistence.h"
#include <czmq.h>
#include <list>
#include <iostream>
/*
The titanic.reply service accepts a UUID and if a reply exists for that UUID, returns the reply message. It accepts a request message with 1 frame, as follows:
	Frame 0: UUID (as returned by titanic.request)
	
	The titanic.reply service MUST reply with 1 or more frames, as follows:
		Frame 0: Status code (explained below)
		Frames 1+: Request body (opaque binary), if OK

	The status code MUST be one of the codes listed below in the section "Status Frames". The UUID MUST be formatted as 32 printable 
	hexadecimal characters ('0' to '9' and 'A' to 'Z' or 'a' to 'z').

The titanic.reply service is idempotent and MUST NOT delete a reply when successfully delivered to the client. Multiple requests to titanic.reply with 
the same UUID should result in the same response back to the client, until and unless the request is executed. See "Request Execution" below.
*/

//Sole Constructor:
//Implement the constructor and the base class layer, this will add some default settings.
titanic_reply::titanic_reply(string brokername,int hbeat,int reconn):
	titanic_component("titanic.reply",brokername,ZMQ_ROUTER,hbeat,reconn){
}

titanic_reply::~titanic_reply(void){
}

//Main loop for this. This is done this way so this can be moved to a different process.
void titanic_reply::Start(){
	
	this->connect_to_broker();
    zmsg_t *reply = NULL;

    while (TRUE) {
        zmsg_t *incoming = this->get_work(TMSG_TYPE_REPLY);
        if (!incoming)
            break;      //  Interrupted, exit
        
		zframe_t* empty2 = zmsg_pop(incoming);
		
		//disassemble our incoming and assemble the reply, all in one operation.
        reply = zmsg_new ();
		zmsg_add(reply,zmsg_pop(incoming));
		zmsg_add(reply,zmsg_pop(incoming));
		zmsg_add(reply,zmsg_pop(incoming));
		zmsg_add(reply,zmsg_pop(incoming));
		zmsg_add(reply,zmsg_pop(incoming));
		
		//last incoming frame will be our UUID.
		char *uuid = zmsg_popstr (incoming);

		if(titanic_persistence::exists(TMSG_TYPE_REPLY,uuid)){
			zmsg_t* frames;
			frames = titanic_persistence::get(TMSG_TYPE_REPLY,uuid);
			zmsg_addstr(reply, TMSG_STATUS_OK);
			zframe_t* f = NULL;
			f=zmsg_pop(frames);
			while(f != NULL){
				zmsg_add(reply,f);
				f = zmsg_pop(frames);
			}
		}
        else {
			if (titanic_persistence::exists(TMSG_TYPE_REQUEST,uuid))
                zmsg_pushstr (reply, TMSG_STATUS_PENDING); //Pending
            else
                zmsg_pushstr (reply, TMSG_STATUS_UKNOWN); //Unknown
        }
        zmsg_destroy (&incoming);
		zmsg_destroy (&reply);
        free (uuid);
        /*free (req_filename);
        free (rep_filename);*/
    }
}
