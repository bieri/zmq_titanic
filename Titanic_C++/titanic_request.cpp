#include "StdAfx.h"
#include "titanic_request.h"
#include "tmsg_api.h"
#include "titanic_persistence.h"

using namespace std;
titanic_request::titanic_request(string brokername,int hbeat,int reconn):titanic_component("titanic.request",brokername,ZMQ_REP,hbeat,reconn)
{
}


titanic_request::~titanic_request(void)
{
}

void titanic_request::Start(){
	this->connect_to_broker();
    zmsg_t *reply = NULL;

    while (TRUE) {
        zmsg_t *incoming = this->get_work();
        if (!incoming)
            break;      //  Interrupted, exit
        
		zframe_t* envelope = zmsg_unwrap(incoming);
		zframe_t* origin_fr = zmsg_pop(incoming);
		zframe_t* service = zmsg_pop(incoming);
		zframe_t* command = zmsg_pop(incoming);

		//We know that its of the correct type. So lets go ahead and stash it.
		string uid = titanic_persistence::gen_uuid();
		
		//push the id onto the body
		zmsg_pushstr(incoming,uid.c_str());
		zmsg_push(incoming,command);
		zmsg_push(incoming,service);
		zmsg_pushstr(incoming,TWRK_SVC_VER);
		zmsg_wrap(incoming,envelope);
		//Send the uuid back to the server before we store the message as there is some magic in the broker 
		//to shuffle this work over to the dispatcher.
		zmsg_send(&incoming,this->Pipe);

		if(titanic_persistence::store(TMSG_TYPE_REQUEST,(char*)uid.c_str(),incoming)){
			throw runtime_error(strcat( "Unable to store request with id of " ,(char*) &uid));
		}

		//Clean up after ourselves.
		zframe_destroy(&origin_fr);
        zmsg_destroy (&incoming);
        free (&uid);
	}
}