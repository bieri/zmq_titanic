#include "StdAfx.h"
#include "titanic_dispatcher.h"
#include <hash_map>
#include <hash_set>
#include <tmsg_api.h>
//#include <titanic_types.h>

using namespace std;
using namespace stdext;

#pragma region Helper Types
//Helper types.
service_t::service_t(void){
	this->name="";
}
service_t::service_t(string name){
	this->name.assign(name);
}
service_t::service_t(string name,deque<worker_t*> wrkrs){
	this->name.assign(name);
	this->avail_workers.assign(wrkrs.begin(),wrkrs.end());
}
service_t::~service_t(){
	this->name.erase();
}

worker_t::worker_t(){
	this->identity="";
	this->address=zframe_new(&"error",5);
	this->service=NULL;
	this->expiry=0;
}
worker_t::worker_t(string identity,zframe_t* addy,service_t* svc,INT_ heartbeat_ivl){
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
#pragma endregion 
//Includes service_t, worker_t and various typedefs

titanic_dispatcher::titanic_dispatcher(string brokername,INT_ hbeat,INT_ reconn,void* skt)
	:titanic_component(NULL,"titanic.discovery",brokername,ZMQ_ROUTER,hbeat,reconn)
{
	//This is only to be used in the dispatch method. remember that this is not in fact thread safe.
	//so this entire class must be treated as non thread safe and must be on the same thread as the 
	//broker that is calling it. Eventually i will fix this.
	this->socket=skt;
}
titanic_dispatcher::~titanic_dispatcher(void)
{
	
}

void titanic_dispatcher::Start(){
	
}
string titanic_dispatcher::envelope_strdup(zframe_t* self){
	if(!self){
		throw exception("cannot be null:");
	}
    byte *data = zframe_data (self);
    size_t size = zframe_size (self);
	string ret_str ("");
	char* ret =(char*) malloc(sizeof(char)*size);
    int is_bin = 0;
    uint char_nbr;
    for (char_nbr = 0; char_nbr < size; char_nbr++)
        if (data [char_nbr] < 9 || data [char_nbr] > 127)
            is_bin = 1;
    
    for (char_nbr = 0; char_nbr < size; char_nbr++) {
        if (is_bin){
             sprintf (ret, "%02X", (unsigned char) data [char_nbr]);
			ret_str.append(ret);
		}
        else{
			sprintf (ret, "%c", data [char_nbr]);
			ret_str.append(ret);
		}
    }
    return ret_str;
}

void titanic_dispatcher::Handle_HeartBeat(zframe_t* address,string svcname){
	Hash_str_svc::iterator i = this->Svcs.find(svcname);
	if(i==this->Svcs.end()){
		worker_add(svcname,address,TWRK_HBT_IVL);
	}
	else{
		service_t* svc = (i)->second;
		string address_s = envelope_strdup(address);
		bool found_worker = false;
		for(int ii=0;ii<svc->avail_workers.size();ii++){
			const char* w_id = svc->avail_workers[ii]->identity.c_str();
			if(strcmp(w_id,address_s.c_str() )==0){
				svc->avail_workers[ii]->expiry = zclock_time() + svc->avail_workers[ii]->heartbeat_ivl;
				found_worker=true;
			}
		}

		if(!found_worker){
			worker_add(svcname,address,TWRK_HBT_IVL);
		}
	}
}

void titanic_dispatcher::Handle_Ready(zframe_t* address,string svcname){
	worker_add(svcname,address,TWRK_HBT_IVL);
}

void titanic_dispatcher::Enqueue(string uuid,string svc,zmsg_t* opaque_frms){
	if(!this->Service_Avail(svc)){
		//need to send a client back a 404. or something. but this should more than likely be checked
		//before we get to here.
		throw exception("bang");
	}
	service_t* serv = this->Svcs.find(svc)->second;
	if(serv->avail_workers.size()==0){
		//We queue it and expect someone to pick it up later from the msg directory.
		serv->requests.push_back(uuid);
		zmsg_destroy(&opaque_frms);
	}
	else{
		worker_t* wrk = this->worker_get(serv);
		this->work_status_set(wrk,uuid);
		this->send_work(wrk,TMSG_TYPE_REQUEST,(char*)svc.c_str(),(char*)uuid.c_str(),opaque_frms);
	}
}
void titanic_dispatcher::Dequeue(string uuid,string svc){
	this->work_status_clear(svc,uuid);
}

bool titanic_dispatcher::Service_Avail(string name){
	return (this->Svcs.find(name)!=Svcs.end());
}

void titanic_dispatcher::Services_Purge(void){
	Hash_str_svc::iterator svcs_iter = this->Svcs.begin();
	service_t* service;
	while(svcs_iter !=this->Svcs.end()){
		service = svcs_iter->second;

		if (service->avail_workers.size()!=0){
			workers_purge(&service->avail_workers); //Clean up the service parts.
			                  //  Services are around to accept requests.
		}

		//now that we have 
		if(	service->avail_workers.size()==0 && 
			service->requests.size()==0 && 
			service->processing_request.size()==0){
				string key;
				key.assign(service->name);
				if (this->Verbose)
					zclock_log ("I: deleting expired service: %s",key);
				svcs_iter++;
				//service_del (key);
				continue;
		}
		svcs_iter++;
	}
}

//PRIVATE Methods::
void titanic_dispatcher::service_add(service_t* sv){
	this->Svcs.insert(pair<string,service_t*>(sv->name,sv));
}
void titanic_dispatcher::service_add(string name){
	service_t* sv = new service_t(name);
	service_add(sv);
}
//This is called only when we are destructing this bad boy.
void titanic_dispatcher::service_del(string name){
	service_t* svc = (this->Svcs.find(name) ->second);
	svc->avail_workers.clear();
	this->Svcs.erase(name);
	delete svc;
}

//this is called before work is dispatched to ensure we dont have any dead workers.
worker_t* titanic_dispatcher::worker_get(service_t* serv){
	worker_t* wrk =serv->avail_workers.front();
	serv->avail_workers.pop_front();
	return wrk;
}
void titanic_dispatcher::workers_purge(deque<worker_t*>* workers){
	for (int i=0; i <workers->size(); i++){
		int64_t zc = zclock_time();
		worker_t* wrk = workers->at(i);
		if (zc  < wrk->expiry + 100){
            continue;                  //  Worker is alive, we're done here
		}
		if (this->Verbose){
            zclock_log ("I: deleting expired worker: %s",wrk->identity.c_str());
		}
		workers->erase(workers->begin()+i);
		delete wrk;
		i--;
	}
}
void titanic_dispatcher::worker_add(string svcname,zframe_t* addr,INT_ hbeatby){
	
	if(!this->Service_Avail(svcname)){
		this->service_add(svcname);
	}
	string key;
	key.assign(svcname);
	service_t* svc = (this->Svcs.find(key) ->second);
	string wrk_id = envelope_strdup(addr);
	worker_t* n_wrk = new worker_t(wrk_id,addr,svc,hbeatby);
	
	if(svc->requests.size()!=0){
		string uuid = svc->requests.front();
		svc->requests.pop_front();

		zmsg_t* msg = titanic_persistence::get(TMSG_TYPE_REQUEST,(char*)uuid.c_str());
		while(!zframe_streq(zmsg_first(msg),(char*) uuid.c_str())){
			zframe_t* eFrame = zmsg_pop(msg);
			zframe_destroy(&eFrame);
		}
		
		char* uuid_v = zmsg_popstr(msg);

		this->work_status_set(n_wrk,uuid);
		this->send_work(n_wrk,TMSG_TYPE_REQUEST,(char*)svcname.c_str(),uuid_v,msg);
		
		//Clean Up
		zmsg_destroy(&msg);
		return;
	}
	
	svc->avail_workers.push_back(n_wrk);
}

void titanic_dispatcher::send_work(worker_t* worker,char *command,char *option,char* uuid, zmsg_t *msg){
	msg = msg? zmsg_dup (msg): zmsg_new ();

	zmsg_pushstr(msg,uuid);
	//  Stack protocol envelope to start of message
	zmsg_pushstr (msg, command);
	zmsg_pushstr (msg, option);
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
void titanic_dispatcher::work_status_set(worker_t* wrk,string uuid){
	wrk->service->processing_request.insert(pair<string,worker_t*>(uuid,wrk));
}

//This is used if a worker is killed for a heartbeating issue to reapportion the work. 
//The work should definitely go back on the front of the queue.
void titanic_dispatcher::work_status_clear(string svc,string uuid){
	if(this->Service_Avail(svc)){
	service_t* service =( this->Svcs.find(svc)->second);
	Hash_str_wrkr::iterator it =  service->processing_request.find(uuid);
	if(it!=service->processing_request.end()){
		delete it->second;
		service->processing_request.erase(it);
	}}
	//Hash_wkrid_vuuid::iterator l_it = this->working_workers.find(workerid);
	//if(l_it!=this->working_workers.end()){
	//	Hash_str* h_s = l_it->second;

	//	//I definitely do not like this section of code. Listed at issue#2 at Git.
	//	for (Hash_str::const_iterator it=h_s->begin(); it != h_s->end(); ++it){
	//		this->work_requeue(string(it->c_str()));
	//	}
	//	this->working_workers.erase(l_it);
	//}			
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
	//wish i could just calll Enqueue here, but i need them at the front of the queue, after all
	//they were already being processed so we should at least put them at the front of the line.
	service_t* serv = this->Svcs.find(svc)->second;
	if(serv->avail_workers.size()==0){
		//We queue it and expect someone to pick it up later from the msg directory.
		serv->requests.push_front(string(uuid));
		//zlist_push(serv->requests, &uuid);
		zmsg_destroy(&msg);
	}
	else{
		worker_t* wrk = this->worker_get(serv);
		this->work_status_set(wrk,uuid);
		this->send_work(wrk,TMSG_TYPE_REQUEST,(char*)serv->name.c_str(),(char*) uuid.c_str(),msg);
	}
}


void titanic_dispatcher::Test(void){
	char* addy = "tcp:://test:100";
	char* addy2 = "tcp:://test:101";
	char* svcname = "Test.Service";
	//zframe_t* add_frame = zframe_new(addy,strlen(addy));
	string addy_str = string(addy);
	string svcname_str = string(svcname);


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
	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)),TWRK_HBT_IVL);
	if(!this->Service_Avail(svcname_str))
		assert("fails");

	worker_t* wrk_svc = 	this->Svcs.begin()->second ->avail_workers.front();

	//This one should delete the worker. but leave the service.
	//this->worker_del(wrk_svc);

	if(!this->Service_Avail(svcname_str))
		assert("fails");

	////This service delete call should clean up the workers as well.
	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)),10000);
	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)),100);
	this->service_del(svcname_str);
	////We have now validated both the worker add and delete functionality.

	////Test the purge functionality. this should only affect those that have 
	////expired based on heartbeating issues.
	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)+1),20*1000);
	this->worker_add(svcname_str,zframe_new(addy2,strlen(addy2)),0);
	zclock_sleep(100);
	service_t* svc_ = this->Svcs.begin()->second;
	this->workers_purge(&svc_ ->avail_workers);
	if(svc_->avail_workers.size()>0)
		assert("fails");
	worker_t* wrk_ =svc_->avail_workers.front();
		//since we are manually popping better clean up the service
	const char * w_id = wrk_->identity.c_str();
	const char * a_id = addy_str.c_str();
	int scp = strcmp(addy_str.c_str(),w_id);
	if(scp!=0)
		assert("fails");
//	this->worker_del(wrk_);

	//time to test all the worker status manipulators:
	//	--work_status_set
	//	--work_status_clear

	this->worker_add(svcname_str,zframe_new(addy,strlen(addy)+1),20*1000);
	worker_t* wrk__ = this->worker_get(this->Svcs.begin()->second);
	string uuid = titanic_persistence::gen_uuid();
	
	/* These tests require a message to be persisted to disk in order to get teh thing going
	this->work_status_set(wrk__->identity,uuid);
	this->work_status_clear(wrk__->identity);*/

	this->work_status_set(wrk__,uuid);
	this->work_status_clear(wrk__->service->name,uuid);
	this->work_status_set(wrk__,uuid);
	this->work_status_clear(wrk__->service->name,uuid);


	this->Enqueue(uuid,wrk__->service->name,zmsg_new());
	this->Dequeue(uuid,wrk__->service->name);


}