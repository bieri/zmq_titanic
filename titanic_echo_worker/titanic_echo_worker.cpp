// titanic_echo_worker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <czmq.h>
#include <tmsg_api.h>
#include <string>
#include <titanic_worker.h>

using namespace std;

int64_t static inline SleepAndSquare(int64_t* duration){
  int64_t val = reinterpret_cast<int64_t>(duration);
  zclock_sleep(val- (val)+100);
  return val * val;
}

zmsg_t*  process_message(zmsg_t* msg){
  zframe_t* data = zmsg_pop(msg);
  int64_t* vOut = (int64_t*) zframe_data(data);
  int64_t res = SleepAndSquare(vOut);
  zmsg_t* rep = zmsg_new();

  zmsg_add(rep,zframe_new(&res,sizeof(res)));
  return rep;
}

int _tmain(int argc, _TCHAR* argv[])
{
  zclock_sleep(1000);
  char* broker_loc = "tcp://nyc-ws093:5555";
  titanic_worker* worker = new titanic_worker(broker_loc,"Echo",10000);
  while(TRUE){
    zmsg_t* req_msg = worker->get_work();
    zmsg_t* rep_msg = process_message(req_msg);
    worker->send_complete(rep_msg);
    zmsg_destroy(&req_msg);
  }
  return 0;
}

  /*
  zmsg_t* r_msg = zmsg_new();
  zmsg_addstr(r_msg,TWRK_WRK_VER);
  zmsg_addstr(r_msg,"Echo");
  zmsg_addstr(r_msg,TMSG_TYPE_READY);
  zmsg_pushstr(r_msg,"");
  zmsg_pushstr(r_msg,"titanic.frontend");
  zmsg_dump(r_msg);
  zclock_sleep(1000);
  zmsg_send(&r_msg,scket);
  int64_t heartbeat_at = zclock_time()+10000;
  while (TRUE) {
        zmq_pollitem_t items [] = {
            { scket,  0, ZMQ_POLLIN, 0 }
      };

        int rc = zmq_poll (items,1 ,1000  * ZMQ_POLL_MSEC);
        if (rc == -1)
            break;              //  Interrupted

        //  Process next input message, if any
    if (items [0].revents & ZMQ_POLLIN) {
      zmsg_t *msg = zmsg_recv (scket);
            if (!msg)
                break;          //  Interrupted
            zclock_log ("I: received message:");
            zmsg_dump (msg);

      zframe_t* env = zmsg_unwrap(msg);
      zframe_t* org = zmsg_pop(msg);
      zframe_t* svc = zmsg_pop(msg);
      char* cmd = zmsg_popstr(msg);
      char* uuid = zmsg_popstr(msg);
      if(strcmp(cmd,TMSG_TYPE_REQUEST)==0){
        zframe_t* data = zmsg_pop(msg);
        int64_t* vOut = (int64_t*) zframe_data(data);
        //int64_t vOut = _atoi64(data);
        int64_t res = SleepAndSquare(vOut);

        zmsg_add(msg,zframe_new(&res,sizeof(res)));
        zmsg_pushstr(msg,uuid);
        zmsg_pushstr(msg,TMSG_TYPE_REPLY);
        zmsg_push(msg,svc);
        zmsg_pushstr(msg,TWRK_WRK_VER);
        zmsg_wrap(msg,env);
        zmsg_dump(msg);
        zmsg_send(&msg,scket);

        delete data;
      }
      
      delete cmd;
      zframe_destroy(&org);

      //lets get more work:
      zmsg_t* h_msg = zmsg_new();
      zmsg_addstr(h_msg,TWRK_WRK_VER);
      zmsg_addstr(h_msg,"Echo");
      zmsg_addstr(h_msg,TMSG_TYPE_READY);
      zmsg_pushstr(h_msg,"");
      zmsg_pushstr(h_msg,"titanic.frontend");
      zmsg_send(&h_msg,scket);
      zmsg_destroy(&h_msg);
      cout<<"Heartbeat Sent.";

      heartbeat_at = zclock_time () + 10000;
        }
    if (zclock_time () > heartbeat_at) {
      zmsg_t* h_msg = zmsg_new();
      zmsg_addstr(h_msg,TWRK_WRK_VER);
      zmsg_addstr(h_msg,"Echo");
      zmsg_addstr(h_msg,TMSG_TYPE_HEARTBEAT);
      zmsg_pushstr(h_msg,"");
      zmsg_pushstr(h_msg,"titanic.frontend");
      zmsg_send(&h_msg,scket);
      zmsg_destroy(&h_msg);
      cout<<"Heartbeat Sent.";
            heartbeat_at = zclock_time () + 10000;
        }
  }*/
