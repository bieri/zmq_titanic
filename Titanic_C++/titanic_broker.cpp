#include "StdAfx.h"
#include <czmq.h>
#include "titanic_broker.h"


titanic_broker::titanic_broker(string frontside,string componentside)
{
  this->Sckt_Address = (char*) frontside.c_str();
  this->Comp_Address = (char*) componentside.c_str();
  this->Context = zctx_new();
  this->Z_Sckt = zsocket_new(this->Context,ZMQ_ROUTER);  //Public facing socket
  this->Inproc_Sckt = zsocket_new(this->Context,ZMQ_ROUTER); //Socket to connect our services to.
  this->Verbose = 0;

  //bind up our sockts
  zsocket_bind(Z_Sckt,this->Sckt_Address);
  zsocket_bind(this->Inproc_Sckt,this->Comp_Address);
  //set up the dispatcher
  this->Dispatcher = new titanic_dispatcher(this->Sckt_Address,10000,10000,this->Z_Sckt);
}
titanic_broker::titanic_broker(string frontside,string componentside,INT_ verbose)
{
  this->Sckt_Address = (char*) frontside.c_str();
  this->Comp_Address =(char*)  componentside.c_str();
  this->Context = zctx_new();
  this->Z_Sckt = zsocket_new(this->Context,ZMQ_ROUTER);
  zsockopt_set_identity(this->Z_Sckt,"titanic.frontend");

  this->Inproc_Sckt = zsocket_new(this->Context,ZMQ_ROUTER); //Socket to connect our services to.
  zsockopt_set_identity(this->Inproc_Sckt,"titanic.broker");

  this->Verbose = verbose;

  //bind up our sockts
  zsocket_bind(this->Z_Sckt,this->Sckt_Address);
  zsocket_bind(this->Inproc_Sckt,this->Comp_Address);

  //set up the dispatcher
  this->Dispatcher = new titanic_dispatcher(this->Sckt_Address,10000,10000,this->Z_Sckt);
}

titanic_broker::~titanic_broker(void)
{
  zsocket_destroy(this->Context,this->Z_Sckt);
  zsocket_destroy(this->Context,this->Inproc_Sckt);
  zctx_destroy(&this->Context);
  free(this->Sckt_Address);
  this->Verbose = NULL;
  delete this->Dispatcher;

    free (this);
}


void titanic_broker::send_to_component(zmsg_t* msg,char* command,char* service,char* origin,zframe_t* envelope,char* scktid){
  zmsg_pushstr(msg,command);
  zmsg_pushstr(msg,service);
  zmsg_pushstr(msg,origin);
  zmsg_wrap(msg,envelope);
  zmsg_pushstr(msg,scktid);
  zmsg_dump(msg);

  zmsg_send(&msg,this->Inproc_Sckt);

  
}

void titanic_broker::Start(void){
  
  cout<<"Starting Broker \n\r";
    //  Get and process messages forever or until interrupted
    while (TRUE) {
        zmq_pollitem_t items [] = {
            { this->Z_Sckt,  0, ZMQ_POLLIN, 0 },
      { this->Inproc_Sckt,0,ZMQ_POLLIN,0}};

        int rc = zmq_poll (items, 2,1000  * ZMQ_POLL_MSEC);
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
      this->process_msg(msg);
    }
    if(items [1].revents & ZMQ_POLLIN){
      zmsg_t *msg = zmsg_recv (this->Inproc_Sckt);
            if (!msg)
                break;          //  Interrupted
            if (this->Verbose) {
                zclock_log ("I: received message:");
                zmsg_dump (msg);
            }
      //Conditionally pop off the first frame. its a pain to use xrep.
      zframe_t* f_1 = zmsg_first(msg);
      zframe_t* f_2 = zmsg_next(msg);

      if(!zframe_streq(f_2,"")){
        zframe_t* f = zmsg_pop(msg);
        zframe_destroy(&f);
      }
      this->process_msg(msg);
    }
    this->Dispatcher->Services_Purge();
    }
    if (zctx_interrupted)
        printf ("W: interrupt received, shutting down…\n");
}
void titanic_broker::process_msg(zmsg_t* msg){
  zframe_t* envelope = zmsg_unwrap(msg);	//address and empty frame.
    char* origin = zmsg_popstr (msg);		//origin frame.
    char* service = zmsg_popstr (msg);		//service frame.
    char* command = zmsg_popstr (msg);		//command frame.
  char* uuid=NULL;
  //Remaining frames are the 

  if(strcmp(origin,TWRK_CLI_VER)==0){
    //Client
    this->message_from_client(msg,envelope,origin,service,command,uuid);
  }
  else if(strcmp(origin,TWRK_SVC_VER)==0){
    //Internal Titanic Component
    this->message_from_component(msg,envelope,origin,service,command,uuid);
  }
  else if(strcmp(origin,TWRK_WRK_VER)==0){
    //Worker
    this->message_from_worker(msg,envelope,origin,service,command,uuid);
  }
}

void titanic_broker::message_from_client(zmsg_t* msg,zframe_t* envelope,char* origin,char* service,char* command,char*uuid){
  if(strcmp(command,TMSG_TYPE_REQUEST)==0){
    //send it to the request handler
    if(this->Dispatcher->Service_Avail(string(service))){
      this->send_to_component(msg,command,service,origin,envelope,"titanic.request");
    }else{
      zmsg_pushstr(msg,TMSG_STATUS_UKNOWN);
      zmsg_pushstr(msg,command);
      zmsg_pushstr(msg,service);
      zmsg_pushstr(msg,origin);
      zmsg_wrap(msg,envelope);
      zmsg_dump(msg);
      zmsg_send(&msg,this->Z_Sckt);
    }
  }
  else if(strcmp(command,TMSG_TYPE_REPLY)==0){
    this->send_to_component(msg,command,service,origin,envelope,"titanic.reply");
  }
  else if(strcmp(command,TMSG_TYPE_FINALIZE)==0){
    //send it to the reply handler on a different thread.
    uuid = zmsg_popstr(msg);
    if(!titanic_persistence::exists(TMSG_TYPE_REPLY,uuid)){
      this->Dispatcher->Dequeue(string(uuid),string(""));
    }
    else{
      //send to finalizer.
      zmsg_pushstr(msg,uuid);
      this->send_to_component(msg,command,service,origin,envelope,"titanic.finalize");
    }
  }
}
void titanic_broker::message_from_component(zmsg_t* msg,zframe_t* envelope,char* origin,char* service,char* command,char*uuid){
  //We are assuming that they will be ready and alive by the time that we get to here.
        //this is prolly a bad assumption but its one i am willing to make for right now.
  if(strcmp(command,TMSG_TYPE_REQUEST)==0){
    //this is going to be coming back from the request service.
    uuid = zmsg_popstr(msg);
    //send the value back to the 
    zmsg_t* clientmsg = zmsg_new();
    zmsg_wrap(clientmsg,zframe_dup(envelope));
    char i1[200];
    zmsg_addstr(clientmsg,strcpy(i1,origin));
    char i2[200];
    zmsg_addstr(clientmsg,strcpy(i2,service));
    char i3[200];
    zmsg_addstr(clientmsg,strcpy(i3,command));
    zmsg_addstr(clientmsg,TMSG_STATUS_OK);
    char i4[200];
    zmsg_addstr(clientmsg,strcpy(i4,uuid));
    zmsg_dump(clientmsg);
    zmsg_send(&clientmsg,this->Z_Sckt);
    zmsg_destroy(&clientmsg);

    //lets queue up the work now.
    this->Dispatcher->Enqueue(string(uuid),string(service),msg);
  }
  else if(strcmp(command,TMSG_TYPE_REPLY)==0){
    zmsg_dump(msg);
    zmsg_pushstr(msg,command);
    zmsg_pushstr(msg,service);
    zmsg_pushstr(msg,origin);
    zmsg_wrap(msg,envelope);
    zmsg_send(&msg,this->Z_Sckt);
  }
}

void titanic_broker::message_from_worker(zmsg_t* msg,zframe_t* envelope,char* origin,char* service,char* command,char*uuid){
  
  if(strcmp(command,TMSG_TYPE_READY)==0){
    this->Dispatcher->Handle_Ready(envelope,string(service));
  }
  else if(strcmp(command,TMSG_TYPE_HEARTBEAT)==0){
    this->Dispatcher->Handle_HeartBeat(envelope,string(service));
  }
  else if(strcmp(command,TMSG_TYPE_REPLY)==0){
    //Need to mark this as completed.
    char* uuid = zmsg_popstr(msg);
    this->Dispatcher->Dequeue(string(uuid),string(service));
    zmsg_pushstr(msg,uuid);
    //send it to the reply handler on a different thread to save to the file system.
    
    zmsg_t* msg_dup = zmsg_dup(msg);
    zframe_t* envelope_dup = zframe_dup(envelope);
    char* origin_dup = strdup(origin);
    char* svc_dup = strdup(service);
    char* cmd_dup = strdup(command);

    this->send_to_component(msg,command,service,origin,envelope,"titanic.reply");
    //send it to the publisher so that we can notify those clients that dont want to ping.
    this->send_to_component(msg_dup,cmd_dup,svc_dup,origin_dup,envelope_dup,"titanic.publish");
          
          
  }
}
