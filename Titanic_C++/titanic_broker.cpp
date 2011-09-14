#include "StdAfx.h"
#include "titanic_broker.h"


titanic_broker::titanic_broker(string frontside,string backside)
{
	this->Context = zctx_new();
	this->FrontSide = frontside;
	this->BackSide = backside;
	this->Verbose = 0;
}
titanic_broker::titanic_broker(string frontside,string backside,int verbose)
{
	this->Context = zctx_new();
	this->FrontSide = frontside;
	this->BackSide = backside;
	this->Verbose = verbose;
}

titanic_broker::~titanic_broker(void)
{
	zsocket_destroy(this->Context,this->FrontSide_Sckt);
	zsocket_destroy(this->Context,this->BackSide_Sckt);


	zctx_destroy(&this->Context);
	this->FrontSide.clear();
	this->BackSide.clear();
	this->Verbose = NULL;

    free (this);
}

void titanic_broker::Start(void){
	this->FrontSide_Sckt = zsocket_new(this->Context,ZMQ_ROUTER);
	this->BackSide_Sckt = zsocket_new(this->Context,ZMQ_ROUTER);
	zsocket_bind(this->FrontSide_Sckt,this->FrontSide.c_str());
	zsocket_bind(this->BackSide_Sckt,this->BackSide.c_str());

	zmq_pollitem_t items [] = {
		{this->FrontSide_Sckt,0,ZMQ_POLLIN,0},
		{this->BackSide_Sckt,0,ZMQ_POLLIN,0}
	};
	while(true){
		int rc = zmq_poll(items,2,-1);
		if(rc==-1)
			break;
		if(items[0].revents && ZMQ_POLLIN){

		}
		if(items[0].revents && ZMQ_POLLIN){

		}
	}
}
void titanic_broker::process_msg(zmsg_t* msg){
	 assert (zmsg_size (msg) >= 1);     //  At least, command

	 //for client
	 this->message_from_client(msg);

	 //for services.
	 this->message_from_worker(msg);

}

//Process an incoming message from a client.
void titanic_broker::message_from_client(zmsg_t* msg){

}
//Process an incoming message from a worker. This is where we store the result for the UUID.
void titanic_broker::message_from_worker(zmsg_t* msg){
	
}

void titanic_broker::message_to_client(zmsg_t* msg){

}

