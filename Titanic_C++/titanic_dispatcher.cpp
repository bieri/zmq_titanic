#include "StdAfx.h"
#include "titanic_dispatcher.h"
#include <hash_map>
#include <hash_set>
#include <tmsg_api.h>
using namespace std;
using namespace stdext;

#pragma region Helper Types
//Helper types.
service_t::service_t(void){
	this->name="";
	this->avail_workers = zlist_new();
	this->requests = zlist_new();
	this->avail_count = 0;
}
service_t::service_t(string name){
	this->name.assign(name);
	this->avail_workers=zlist_new();
	this->requests=zlist_new();
	this->avail_count = 0;
}
service_t::service_t(string name,zlist_t* wrkrs){
	this->name.assign(name);
	this->avail_workers=wrkrs;
	this->requests=zlist_new();
	this->avail_count =0;
}
service_t::~service_t(){
	this->name.erase();
	zlist_destroy(&(this->avail_workers));
	zlist_destroy(&(this->requests));
}

worker_t::worker_t(){
	this->identity="";
	this->address=zframe_new(&"error",5);
	this->service=NULL;
	this->expiry=0;
}
worker_t::worker_t(string identity,zframe_t* addy,service_t* svc,int64_t heartbeat_ivl){
	this->identity.assign(identity);
	this->address = addy;
	this->service = svc;
	this->expiry = zclock_time() + heartbeat_ivl;
	this->heartbeat_ivl = heartbeat_ivl;
}
worker_t::~worker_t(){
	this->identity.erase();
	zframe_destroy(&this->address);
}
#pragma endregion Includes service_t, worker_t and various typedefs

titanic_dispatcher::titanic_dispatcher(string brokername,int hbeat,int reconn,void* skt)
	:titanic_component(NULL,"titanic.discovery",brokername,ZMQ_ROUTER,hbeat,reconn)
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

void titanic_dispatcher::Handle_HeartBeat(zframe_t* address,string svcname){
	service_t* svc = (this->Svcs.find(svcname))->second;
	const char* w_name = (const char*) zframe_strdup(address);
	worker_t* wrk =(worker_t*) zlist_first(svc->avail_workers);
	while(wrk){
		const char* w_id = wrk->identity.c_str();
		
		if(strcmp(w_id,w_name )==0){
			wrk->expiry = zclock_time() + wrk->heartbeat_ivl;
		}
		wrk = (worker_t*) zlist_next(svc->avail_workers);
		continue;

	}
}
void titanic_dispatcher::Handle_Ready(zframe_t* address,string svcname){
	worker_add(svcname,address,100);
}

void titanic_dispatcher::Enqueue(string uuid,string svc,zmsg_t* opaque_frms){
	if(!this->Service_Avail(svc)){
		//need to send a client back a 404. or something. but this should more than likely be checked
		//before we get to here.

	}
	service_t* serv = this->Svcs.find(svc)->second;
	if(serv->avail_count==0){
		//We queue it and expect someone to pick it up later from the msg directory.
		zlist_append(serv->requests, &uuid);
		zmsg_destroy(&opaque_frms);
	}
	else{
		worker_t* wrk = this->worker_get(serv);
		this->work_status_set(wrk->identity,uuid);
		this->send_work(wrk,TMSG_TYPE_REQUEST,NULL,opaque_frms);
	}
}
void titanic_dispatcher::Dequeue(string uuid,string svc){
	//if there is no service available for the dequeue something bizzare has happened.
	//should prolly do something about that.
	if(this->Service_Avail(svc)){
		service_t* s = this->Svcs.find(svc)->second;
		//I am pretty sure that this is going to have to be changed to a hash_set
		//I think that this is more than likely going to be slow. ugh.
		zlist_remove(s->requests,&svc);
	}
}


bool titanic_dispatcher::Service_Avail(string name){
	return (this->Svcs.find(name)!=Svcs.end());
}


//PRIVATE Methods::
void titanic_dispatcher::service_add(service_t* sv){
	this->Svcs[sv->name] = sv;
}
void titanic_dispatcher::service_add(string name){
	service_t* sv = new service_t(name);
	service_add(sv);
}
//This is called only when we are destructing this bad boy.
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
	
	//Clean up the workers.
	int i = 0;
	while(svc->avail_count !=0){
		worker_t* w = (worker_t*) zlist_next(svc->avail_workers);
		this->worker_del(w);
	}
	this->Svcs.erase(name);
	
	delete svc;
	
}

//this is called before work is dispatched to ensure we dont have any dead workers.
worker_t* titanic_dispatcher::worker_get(service_t* serv){
	worker_t* wrk =  (worker_t*) zlist_pop( serv->avail_workers);
	serv->avail_count--;
	return wrk;
}
void titanic_dispatcher::workers_purge(zlist_t* workers){
	worker_t *worker = (worker_t *) zlist_first (workers);
	//This whole thing doesnt look like it ever gets to the end of the list
	//double check this whole thing.
	int64_t zc = zclock_time();
    while (worker) {
        if (zc  < worker->expiry){
			worker = (worker_t*) zlist_next(workers);
            continue;                  //  Worker is alive, we're done here
		}
        if (this->Verbose){
            zclock_log ("I: deleting expired worker: %s",worker->identity);
		}
		this->worker_del(worker);
        worker = (worker_t *) zlist_next (workers);
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
	//Do some memory management.
	delete worker;
}
void titanic_dispatcher::worker_add(string svcname,zframe_t* addr,int hbeatby){
	
	if(!this->Service_Avail(svcname)){
		this->service_add(svcname);
	}
	string key;
	key.assign(svcname);
	service_t* svc = (this->Svcs.find(key) ->second);
	char* w_name =zframe_strdup(addr);
	string wrk_id = string(w_name);
	worker_t* n_wrk = new worker_t(wrk_id,addr,svc,hbeatby);
	
	zlist_append( svc->avail_workers,n_wrk);
	svc->avail_count++;


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
		zclock_log ("I: sending %s to worker",command);
		zmsg_dump (msg);
	}
	zmsg_send (&msg, this->socket);
}

//We are setting the status to maintain state so we know if a worker dies what we need to do.
void titanic_dispatcher::work_status_set(string workerid,string uuid){
	Hash_wkrid_vuuid::iterator it =  this->working_workers.find(workerid);
	if(it == this->working_workers.end()){
		Hash_str* uids = new Hash_str();
		uids->insert(uuid);
		this->working_workers[workerid]= uids;
	}
	else{
		it->second->insert(uuid);
	}
}

//This is used if a worker is killed for a heartbeating issue to reapportion the work. 
//The work should definitely go back on the front of the queue.
void titanic_dispatcher::work_status_clear(string workerid){
	
	Hash_wkrid_vuuid::iterator l_it = this->working_workers.find(workerid);
	if(l_it!=this->working_workers.end()){
		Hash_str* h_s = l_it->second;

		//I definitely do not like this section of code. Listed at issue#2 at Git.
		for (Hash_str::const_iterator it=h_s->begin(); it != h_s->end(); ++it){
			this->work_requeue(string(it->c_str()));
		}
		this->working_workers.erase(l_it);
	}			
}

//This is used to clean up the state that is being maintained, since we are passing 
//the particular request we have to assume that we are only going to be expiring one
//atomic request that has either been completed or expired by the requester.
void titanic_dispatcher::work_status_clear(string svcname,string workerid,string uuid){
	//we are always hoping for a worker id. cheapest way to expire this thing in
	//the current set up as well as the most frequently received way because  we normally are 
	//going to recieve an expiration of a uuid when work has been completed.
	if(!workerid.empty()){
		Hash_wkrid_vuuid::iterator l_it = this->working_workers.find(workerid);
		Hash_str* h_s = l_it->second;
		if(h_s->count(uuid)==1){
			l_it->second->erase(uuid);
		}
	}
	else{
		//This is only hit when a client is expiring a bit of work.
		Hash_wkrid_vuuid::const_iterator it =  this->working_workers.begin();
		for (; it != this->working_workers.end(); ++it){
			if(it->second->count(uuid)==1){
				it->second->erase(uuid);
				break;
			}
		}
	}
}
void titanic_dispatcher::work_requeue(string uuid){
	//Since we will never have a service when this is called. We will go and fetch the message from the 
	//persistence layer and then pop off the Service Frame. Once we have that we can call the Enqueue bit.
	//just for some reason i really didnt want to encapsulate this functionality in the work_status_clear()
	//I suppose its just for separation of concerns more than anything.
	zmsg_t* msg = titanic_persistence::get(TMSG_TYPE_REQUEST,(char*)uuid.c_str());
	//lets pop off the service frame which should be frame one.
	string svc = string(zmsg_popstr(msg));
	//according to the api this is an empty frame.
	zframe_t* e_fr = zmsg_pop(msg);
	zframe_destroy(&e_fr);
	//wish i could just calll Enqueue here, but i need them at the frotn of the queue, after all
	//they were already being processed so we should at least put them at the front of the line.
	service_t* serv = this->Svcs.find(svc)->second;
	if(serv->avail_count==0){
		//We queue it and expect someone to pick it up later from the msg directory.
		zlist_push(serv->requests, &uuid);
		zmsg_destroy(&msg);
	}
	else{
		worker_t* wrk = this->worker_get(serv);
		this->work_status_set(wrk->identity,uuid);
		this->send_work(wrk,TMSG_TYPE_REQUEST,NULL,msg);
	}
}


void titanic_dispatcher::Test(void){
	char* addy = "tcp:://test:100";
	char* addy2 = "tcp:://test:101";
	char* svcname = "Test.Service";
	//zframe_t* add_frame = zframe_new(addy,strlen(addy));
	string addy_str = string(addy);
	string svcname_str = string(svcname);



	zlist_t* testlist = zlist_new();
	


	//Service Tests.

	//Test Service Add, one for each overload.
	this->service_add(addy_str);
	service_t* svc =new  service_t(svcname);
	this->service_add(svc);

	if(!this->Service_Avail(addy_str)||!this->Service_Avail(svcname)){
		assert("failed");
	}
	//Test Service Del
	this->service_del(addy_str);
	this->service_del(svcname_str);
	
	if(this->Service_Avail(addy_str)||this->Service_Avail(svcname_str)){
		assert("failed");
	}
	
	//Worker Tests
	//This worker add creates a new service to go with it.
	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)),100);
	if(!this->Service_Avail(svcname_str))
		assert("fails");

	worker_t* wrk_svc = (worker_t*)	zlist_first(this->Svcs.begin()->second ->avail_workers);

	//This one should delete the worker. but leave the service.
	this->worker_del(wrk_svc);

	if(!this->Service_Avail(svcname_str))
		assert("fails");

	//This service delete call should clean up the workers as well.
	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)),10000);
	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)),100);
	this->service_del(svcname_str);
	//We have now validated both the worker add and delete functionality.

	//Test the purge functionality. this should only affect those that have 
	//expired based on heartbeating issues.
	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)+1),20*1000);
	this->worker_add(svcname_str,zframe_new(addy2,strlen(addy2)),0);
	zclock_sleep(100);
	service_t* svc_ = this->Svcs.begin()->second;
	this->workers_purge(svc_ ->avail_workers);
	if(svc_->avail_count>0)
		assert("fails");
	worker_t* wrk_ =(worker_t*) zlist_pop(svc_->avail_workers);
		//since we are manually popping better clean up the service
	const char * w_id = wrk_->identity.c_str();
	const char * a_id = addy_str.c_str();
	int scp = strcmp(addy_str.c_str(),w_id);
	if(scp!=0)
		assert("fails");
	this->worker_del(wrk_);

	//time to test all the worker status manipulators:
	//	--work_status_set
	//	--work_status_clear

	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)+1),20*1000);
	worker_t* wrk__ = this->worker_get(this->Svcs.begin()->second);
	string uuid = titanic_persistence::gen_uuid();
	
	/* These tests require a message to be persisted to disk in order to get teh thing going
	this->work_status_set(wrk__->identity,uuid);
	this->work_status_clear(wrk__->identity);*/

	this->work_status_set(wrk__->identity,uuid);
	this->work_status_clear(string(""),wrk__->identity,uuid);
	this->work_status_set(wrk__->identity,uuid);
	this->work_status_clear(wrk__->service->name,string(""),uuid);


	this->Enqueue(uuid,wrk__->service->name,zmsg_new());
	this->Dequeue(uuid,wrk__->service->name);


}