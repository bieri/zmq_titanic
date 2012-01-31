#include "StdAfx.h"
#include "titanic_publish.h"


titanic_publish::titanic_publish(zctx_t* ctx,string brokername,char* pubPort,int hbeat,int reconn):
	titanic_component(ctx,"titanic.publish",brokername,ZMQ_XREP,hbeat,reconn){
		this->pub_socket = zsocket_new (this->Context, ZMQ_PUB);
		zsocket_bind(this->pub_socket,pubPort);
}

titanic_publish::~titanic_publish(void){
	zmq_close(this->pub_socket);
}

void titanic_publish::Start(){
	
	this->connect_to_broker();
	cout<<"Starting publish \n\r";
    while (TRUE) {
        zmsg_t *incoming = this->get_work();
        if (!incoming)
            break;      //  Interrupted, exit
		
		zframe_t* f_1 = zmsg_first(incoming);
		zframe_t* f_2 = zmsg_next(incoming);
		
		if(!zframe_streq(f_2,"")){
			zframe_t* f = zmsg_pop(incoming);
			zframe_destroy(&f);
		}
		
		zframe_t* envelope = zmsg_unwrap(incoming);
		char* origin = zmsg_popstr(incoming);
		zframe_t* service = zmsg_pop(incoming);
		char* command = zmsg_popstr(incoming);
		char* uuid = zmsg_popstr(incoming);
		cout << "Publishing :" << uuid << "  \n\r";
		zmsg_pushstr(incoming,uuid);
		//zclock_sleep(10);
		zmsg_send(&incoming,this->pub_socket);
        zmsg_destroy (&incoming);
    }
}
