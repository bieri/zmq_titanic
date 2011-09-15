#pragma once
#include <czmq.h>
#include "titanic_component.h"
#include <hash_map>
#include <string.h>
#include <uuids.h>
#include <list>

using namespace std;

//Lets stick this here as its the only place I really want this to be used. No sense in leaking it all over the codebase;
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
	service_t(string name,vector<worker_t> wrkrs);
	~service_t(void);
	string name;                 //  Service name
    list<UUID> requests;          //  List of client requests
    list<worker_t> avail_workers;           //  List of waiting workers
    size_t avail_count;             //  How many workers we have
};
class worker_t {
public:
	worker_t(string identity,zframe_t* addy,service_t* svc,int expiry);
	~worker_t();
	string identity;             //  Identity of worker
    zframe_t *address;          //  Address frame to route to
    service_t *service;         //  Owning service, if known
    int64_t expiry;             //  Expires at unless heartbeat
};

////  This defines a single service
//typedef struct {
//    string *name;                 //  Service name
//    zlist_t *requests;          //  List of client requests
//    zlist_t *waiting;           //  List of waiting workers
//    size_t workers;             //  How many workers we have
//} service_t;
//
//// Defines a single worker
//typedef struct {
//    string *identity;             //  Identity of worker
//    zframe_t *address;          //  Address frame to route to
//    service_t *service;         //  Owning service, if known
//    int64_t expiry;             //  Expires at unless heartbeat
//} worker_t;

typedef hash_map<string,service_t*,string_hash> Hash_str_svc;
typedef hash_map<string,worker_t*,string_hash> Hash_str_wrkr;
typedef hash_map<string,vector<UUID>*,string_hash> Hash_zframe_vuuid;

typedef pair<string,service_t*> Pair_str_svc;
typedef pair<string,worker_t*> Pair_str_wkr;
typedef pair<string,vector<UUID>*> Pair_str_vuuid;

class titanic_dispatcher :
	public titanic_component
{
public:
	titanic_dispatcher(string brokername,int hbeat,int reconn,void* socket );
	~titanic_dispatcher(void);

	void Start();

	//Method
	
	void Handle_Ready(zframe_t* address,string svcname);
	void Handle_HeartBeat(zframe_t* address,zmsg_t* msg);
	
	void Dispatch(void);
	
	void Enqueue(string uuid);
	void Enqueue(string uuid,string svc,zmsg_t* opaque_frames);
	void Dequeue(string uuid);
	
	bool Service_Avail(string name);

	//Worker / Service Ms
	void RecieveWork(zmsg_t msg);
	void Test(void);
private:
	//Properties
	Hash_str_svc Svcs;
	Hash_str_wrkr Wrkrs;
	Hash_zframe_vuuid working_workers;
	void* socket;

	//Methods
	void workers_purge(zlist_t* workers);
	void worker_add(string svcname,zframe_t* address,int hbeatby);
	void worker_del(worker_t* worker);

	void services_purge(void);
	void service_add(service_t* sv);
	void service_add(string name);
	void service_del(string name);

	void send_work(worker_t* worker,char *command,char *option, zmsg_t *msg);
};



