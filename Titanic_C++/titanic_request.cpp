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
        zmsg_t *incoming = this->get_work(TMSG_TYPE_REQUEST);
        if (!incoming)
            break;      //  Interrupted, exit
        
		//We know that its of the correct type. So lets go ahead and stash it.
		string uid = titanic_persistence::gen_uuid();
		if(titanic_persistence::store(TMSG_TYPE_REQUEST,(char*) &uid,incoming)){
			throw runtime_error(strcat( "Unable to store request with id of " ,(char*) &uid));
		}

		zframe_t* empty2 = zmsg_pop(incoming);
		
		//disassemble our incoming and assemble the reply, all in one operation.
        reply = zmsg_new ();
		zmsg_add(reply,zmsg_pop(incoming)); //empty frame
		zmsg_add(reply,zmsg_pop(incoming)); //version of the titanic service
		zmsg_add(reply,zmsg_pop(incoming)); //TMSG_TYPE
		zmsg_add(reply,zmsg_pop(incoming)); //client address
		zmsg_add(reply,zmsg_pop(incoming)); //empty frame

		//lets go ahead and send the work over to the queue. 
		//We should make sure we can shuffle it before we reply to the client.
		//all we are going to do is send over the UUID. it can use the persistence layer
		//to pick up the message and send it down.
		zmsg_t* internalmsg = zmsg_new();
		zmsg_addstr(internalmsg,uid.c_str());
		zmsg_send(&internalmsg,this->Pipe);

		//lets send the uuid back to the client. We are after all a proxy.
		zmsg_addstr(reply,TMSG_STATUS_OK);
		zmsg_addstr(reply,uid.c_str());
		zmsg_send(&reply,this->Context);

		//Clean up after ourselves.
        zmsg_destroy (&incoming);
		zmsg_destroy (&internalmsg);
		zmsg_destroy (&reply);
        free (&uid);
	}
}