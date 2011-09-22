#pragma once
#include "stdafx.h"
#include <hash_map>
#include <hash_set>
#include <string.h>
#include <uuids.h>
#include <list>

using namespace std;

//Lets stick this here as its the only place I really want this to be used. No sense in leaking it all over the codebase;
#pragma region Helper Types
class string_hash : public hash_compare<string>{
public:
	size_t operator() (const string& s) const{
		size_t h = 0;
		std::string::const_iterator p, p_end;
		for(p = s.begin(), p_end = s.end(); p != p_end; ++p)
		{
		  h = 31 * h + (*p);
		}
		return h;
	}
	bool operator() (const string& s1,const string& s2) const{
		return strcmp((char*)&s1,(char*)&s2)==0;
	}
};

class worker_t;
class service_t {
public:
	service_t(string name);
	service_t(string name,zlist_t* wrkrs);
	service_t(void);
	~service_t(void);
	string name;                 //  Service name
    //list<UUID> requests;          //  List of client requests
    //list<worker_t> avail_workers;           //  List of waiting workers
	zlist_t* requests;
	zlist_t* avail_workers;
    size_t avail_count;             //  How many workers we have
};
class worker_t {
public:
	worker_t(string identity,zframe_t* addy,service_t* svc,int64_t heartbeat_ivl);
	worker_t(void);
	~worker_t();
	string identity;             //  Identity of worker
    zframe_t *address;          //  Address frame to route to
    service_t *service;         //  Owning service, if known
    int64_t expiry;             //  Expires at unless heartbeat
	int64_t heartbeat_ivl;			// Like it says
};

typedef hash_set<string,string_hash> Hash_str;
typedef hash_map<string,service_t*,string_hash> Hash_str_svc;
typedef hash_map<string,worker_t*,string_hash> Hash_str_wrkr;
typedef hash_map<string,Hash_str*,string_hash> Hash_wkrid_vuuid;

#pragma endregion Includes service_t, worker_t and various typedefs

class titanic_dispatcher :
	public titanic_component
{
public:
	titanic_dispatcher(string brokername,int hbeat,int reconn,void* socket );
	~titanic_dispatcher(void);

	void Start();

	//Method
	
	void Handle_Ready(zframe_t* address,string svcname);
	void Handle_HeartBeat(zframe_t* address,string svcname);
	
	void Dispatch(void);
	
	//void Enqueue(string uuid,string svc);
	void Enqueue(string uuid,string svc,zmsg_t* opaque_frames);
	void Dequeue(string uuid,string svc);

	
	bool Service_Avail(string name);

	//Worker / Service Ms
	void Test(void);
private:
	//Properties
	Hash_str_svc Svcs;
	Hash_wkrid_vuuid working_workers;
	void* socket;

	//Methods
	void workers_purge(zlist_t* workers);
	void worker_add(string svcname,zframe_t* address,int hbeatby);
	void worker_del(worker_t* worker);
	worker_t* worker_get(service_t* svcname);

	void services_purge(void);
	void service_add(service_t* sv);
	void service_add(string name);
	void service_del(string name);

	void send_work(worker_t* worker,char *command,char *option,char* uuid, zmsg_t *msg);
	
	void work_status_set(string workerid,string uuid);
	void work_status_clear(string workerid);
	void work_status_clear(string svcname,string workerid,string uuid);
	void work_requeue(string uuid);
};



