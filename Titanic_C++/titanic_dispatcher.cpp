#include "StdAfx.h"
#include "titanic_dispatcher.h"
#include "tmsg_api.h"
#include "titanic_persistence.h"
#include <czmq.h>
#include <hash_map>
using namespace std;
using namespace stdext;


//Helper types.
service_t::service_t(string name){
	this->name.assign(name);
	this->avail_workers;
	this->requests;
	this->avail_count = 0;
}
service_t::service_t(string name,list<worker_t> wrkrs){
	this->name.assign(name);
	this->avail_workers.assign(wrkrs.begin(),wrkrs.end());
	this->avail_workers=wrkrs;
	this->requests;
	this->avail_count = 0;
}

service_t::~service_t(){
	this->name.erase();
	delete &(this->avail_workers);
	delete &(this->requests);
}

worker_t::worker_t(string identity,zframe_t* addy,service_t* svc,int expiry){
	this->identity.assign(identity);
	this->address = addy;
	this->service = svc;
	this->expiry = expiry;
}
worker_t::~worker_t(){
	this->identity.erase();
	zframe_destroy(&this->address);
}

titanic_dispatcher::titanic_dispatcher(string brokername,int hbeat,int reconn,void* skt)
	:titanic_component("titanic.discovery",brokername,ZMQ_ROUTER,hbeat,reconn)
{
	//This is only to be used in the dispatch method. remember that this is not in fact thread safe.
	//so this entire class must be treated as non thread safe and must be on the same thread as the 
	//broker that is calling it. Eventually i will fix this.
	this->socket=socket;
}


titanic_dispatcher::~titanic_dispatcher(void)
{

}

void titanic_dispatcher::Start(){
	
	//this->connect_to_broker();
 //   zmsg_t *reply = NULL;
	//Hash_str_svc::iterator iter;

 //   while (TRUE) {
	//	//We are assured that we only want HeartBeats and Handshakes.
 //       zmsg_t *incoming = this->get_work(TMSG_TYPE_READY);
	//	
	//	int returnCode=0;
 //       if (!incoming)
 //           break;      //  Interrupted, exit
 //       
	//	zframe_t* empty2 = zmsg_pop(incoming);
	//	
	//	//disassemble our incoming and assemble the reply, all in one operation.
 //       reply = zmsg_new ();
	//	zmsg_add(reply,zmsg_pop(incoming));
	//	
	//	char* command= zmsg_popstr(incoming);
	//	
	//	//service please!
	//	char* svcname= zmsg_popstr(incoming);
	//	iter;// = lookup.find(svcname);
	//	
	//	if(command=="AVAILABLE"){
	//		if(iter !=Svcs.end() && (zclock_time() -  iter->second)>this->HeartBeat_Ivl){
	//			returnCode = 200;
	//		}
	//		else{
	//			if(iter !=Svcs.end()){
	//				Svcs.erase(iter);
	//			}
	//			returnCode=400;
	//		}
	//	}
	//	if(command=="READY" || command=="HEARTBEAT"){
	//		if(iter!=Svcs.end()){
	//			Svcs.erase(iter);
	//		}
	//		Svcs[svcname]=zclock_time();
	//	}

	//	if(returnCode>0){
	//		//build up a response b/c we are expecting one on the other end.
	//		zmsg_addstr(reply,(char*) returnCode);
	//		zmsg_send(&reply,this->Pipe);
	//	}

 //       zmsg_destroy (&incoming);
	//	zmsg_destroy (&reply);
	//	free(&returnCode);

    //}
}

void titanic_dispatcher::Handle_HeartBeat(zframe_t* address,zmsg_t* msg){
	
}
void titanic_dispatcher::Handle_Ready(zframe_t* address,string svcname){
	worker_add(svcname,address,100);
}
bool titanic_dispatcher::Service_Avail(string name){
//	string key = name->substr(0,name->length());
	return (this->Svcs.find(name)!=Svcs.end());
}

void titanic_dispatcher::service_add(service_t* sv){
	//this->Svcs.insert(Pair_str_svc(sv->name->substr(0,sv->name->length()),sv));
	string key;
	key.assign(sv->name);
	this->Svcs[key] = sv;
}
void titanic_dispatcher::service_add(string name){
	//string key =name->substr(0,name->length());
	service_t* sv = new service_t(name);
	this->Svcs[name] = sv;
}


//This is called on regular intervals to make sure that we dont have anything thats dead
//and have it accidently get work.
void titanic_dispatcher::services_purge(void){
	Hash_str_svc::iterator svcs_iter = this->Svcs.begin();
	service_t* service;
	for (; svcs_iter != Svcs.end(); ++svcs_iter){
		service = svcs_iter->second;

		if (0 != service->avail_count){
			workers_purge(service->avail_workers); //Clean up the service parts.
			break;                  //  Services are around to accept requests.
		}
        if (this->Verbose)
            zclock_log ("I: deleting expired worker: %s",service->name);
		//now that we have 
		if(0==service->avail_workers){
			string key;
			key.assign(service->name);
			service_del (key);
		}
        
	}
}

void titanic_dispatcher::service_del(string name){
	//string key = name->substr(0,name->length()); 
	service_t* svc = (this->Svcs.find(name) ->second);
	this->Svcs.erase(name);
	
	delete svc;
	
}
void titanic_dispatcher::workers_purge(zlist_t* workers){
	worker_t *worker = (worker_t *) zlist_first (workers);
	//This whole thing doesnt look like it ever gets to the end of the list
	//double check this whole thing.
    while (worker) {
        if (zclock_time () < worker->expiry)
            break;                  //  Worker is alive, we're done here
        if (this->Verbose)
            zclock_log ("I: deleting expired worker: %s",
                worker->identity);
		this->worker_del(worker);
        worker = (worker_t *) zlist_first (workers);
    }
}
void titanic_dispatcher::worker_del(worker_t* worker){
	
	//Send a disconnect so this thing fails gracefully. 
	//And the worker doesnt just sit there forever.
	if(!this->socket)
		send_work(worker,TMSG_TYPE_DISCONNECT,NULL,NULL);

	if (worker->service) {
		service_t* svc = worker->service;
        zlist_remove (svc->avail_workers, worker);
        worker->service->avail_count--;
	}
	{
	string key;
	key.assign(worker->identity);
	this->Wrkrs.erase(key);
	}
}
void titanic_dispatcher::worker_add(string svcname,zframe_t* addr,int hbeatby){
	service_t* svc;
	
	if(!this->Service_Avail(svcname)){
		this->service_add(svcname);
	}
	string key;
	key.assign(svcname);
	svc = (this->Svcs.find(key) ->second);
	char* w_name =zframe_strdup(addr);
	string wrk_id = string(w_name);
	worker_t* n_wrk = new worker_t(wrk_id,addr,svc,100);
	
	zlist_append( svc->avail_workers,n_wrk);
	svc->avail_count=svc->avail_count++;


}

void titanic_dispatcher::send_work(worker_t* worker,char *command,char *option, zmsg_t *msg){
	msg = msg? zmsg_dup (msg): zmsg_new ();

	//  Stack protocol envelope to start of message
	if (option)
		zmsg_pushstr (msg, option);
	zmsg_pushstr (msg, command);
	zmsg_pushstr (msg, TWRK_WRK_VER);

	//  Stack routing envelope to start of message
	zmsg_wrap (msg, zframe_dup (worker->address));

	if (this->Verbose) {
		zclock_log ("I: sending %s to worker",
			TMSG_TYPES [(int) *command]);
		zmsg_dump (msg);
	}

	//********************************************************************************
	//This right now is a compile hack and will have to pass the broker endpoint socket
	//into the constructor so that we can hack it for a single endpoint solution.
	zmsg_send (&msg, this->Pipe);
}

void titanic_dispatcher::Test(void){
	char* addy = "tcp:://test:100";
	char* svcname = "Test.Service";
	zframe_t* add_frame = zframe_new(addy,strlen(addy));
	string addy_str = string(addy);
	string svcname_str = string(svcname);

	//Service Tests.
	this->service_add(addy_str);
	if(!this->Service_Avail(addy_str)){
		assert("failed");
	}
	this->service_del(addy_str);
	if(this->Service_Avail(addy_str)){
		assert("failed");
	}

	service_t* svc = new service_t(addy_str);
	this->service_add(svc);
	if(!this->Service_Avail(addy_str)){
		assert("failed");
	}
	this->service_del(addy_str);
	if(this->Service_Avail(addy_str)){
		assert("failed");
	}

	//Worker Tests
	worker_t* wrk =  new worker_t(addy_str,add_frame,svc,100);
	this->worker_add(svcname_str,add_frame,100);
	if(!this->Service_Avail(svcname_str))
		assert("fails");

	this->worker_del(wrk);
	if(!this->Service_Avail(svcname_str))
		assert("fails");
}