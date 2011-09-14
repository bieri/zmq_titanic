#include "StdAfx.h"
#include "titanic_finalize.h"
#include "titanic_persistence.h"
#include "tmsg_api.h"

using namespace std;

titanic_finalize::titanic_finalize(string brokername,int hbeat,int reconn,int cleanupivl)
	:titanic_component("titanic.finalize",brokername,ZMQ_XREP,hbeat,reconn)
{
	this->Cleanup_Ivl = cleanupivl;
}


titanic_finalize::~titanic_finalize(void){
}

void titanic_finalize::Start(){
	this->connect_to_broker();
    zmsg_t *reply = NULL;

    while (TRUE) {
        zmsg_t *incoming = this->get_work(TMSG_TYPE_FINALIZE);
        if (!incoming)
            break;      //  Interrupted, exit
        
		//take the message out of the envelope.
		zframe_t* envelope = zmsg_unwrap(incoming);

		//lets get the frame we want. the last one.
		char* uuid;
		reply = zmsg_new ();
		zmsg_add(reply,zmsg_pop(incoming)); //empty frame
		zmsg_add(reply,zmsg_pop(incoming)); //version of the titanic service
		zmsg_add(reply,zmsg_pop(incoming)); //TMSG_TYPE
		
		uuid = zmsg_popstr(incoming);

		if(titanic_persistence::remove(uuid)){
			//return a 200 back to the client
			zmsg_addstr(reply,TMSG_STATUS_OK);
			zmsg_addstr(reply,uuid);
			
		}
		else{
			//return a 500 to the client.
			zmsg_addstr(reply,TMSG_STATUS_ERROR);
			zmsg_addstr(reply,uuid);
		}
		//Since we are replying direcly to the client lets put the message back in the envelope.
		zmsg_wrap(reply,envelope);
		zmsg_send(&reply,this->Context);

		//Clean up after ourselves.
        zmsg_destroy (&incoming);
		zmsg_destroy (&reply);
        free (&uuid);
	}
}
