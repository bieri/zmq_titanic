#pragma once
#include "stdafx.h"
#include <hash_map>
#include <hash_set>
#include <string>
#include <xstring>
#include <uuids.h>
#include <list>
#include <titanic_types.h>
#include <deque>
using namespace std;


class service_t;
//class worker_t;
class worker_t {
public:
	worker_t(string identity,zframe_t* addy,service_t* svc,INT_ heartbeat_ivl);
	worker_t(void);
	~worker_t();
	string identity;             //  Identity of worker
    zframe_t *address;          //  Address frame to route to
    service_t *service;         //  Owning service, if known
    INT_ expiry;             //  Expires at unless heartbeat
	INT_ heartbeat_ivl;			// Like it says
};

class service_t {
public:
	service_t(string name);
	service_t(string name,deque<worker_t*> wrkrs);
	service_t(void);
	~service_t(void);
	string name;					//  Service name
	deque<string> requests;			//	UUIDs of strings that need to be distributed.
	deque<worker_t*> avail_workers;	//	List of workers that are available to receive work.
    //size_t avail_count;             //  Number of workers that are available.
};

typedef hash_set<string> Hash_str;
typedef hash_map<string,service_t*> Hash_str_svc;
typedef hash_map<string,worker_t*> Hash_str_wrkr;
typedef hash_map<string,Hash_str*> Hash_wkrid_vuuid;

#pragma endregion Includes service_t, worker_t and various typedefs

class titanic_dispatcher :
	public titanic_component
{
public:
	titanic_dispatcher(string brokername,INT_ hbeat,INT_ reconn,void* socket );
	~titanic_dispatcher(void);

	void Start();

	//Method
	
	void Handle_Ready(zframe_t* address,string svcname);
	void Handle_HeartBeat(zframe_t* address,string svcname);
	
	void Dispatch(void);
	
	//void Enqueue(string uuid,string svc);
	void Enqueue(string uuid,string svc,zmsg_t* opaque_frames);
	void Dequeue(string uuid,string svc);
	void Services_Purge(void);
	
	bool Service_Avail(string name);

	//Worker / Service Ms
	void Test(void);
private:
	//Properties
	Hash_str_svc Svcs;
	Hash_wkrid_vuuid working_workers;
	void* socket;

	//Methods
	void workers_purge(deque<worker_t*>* workers);
	void worker_add(string svcname,zframe_t* address,INT_ hbeatby);
	void worker_del(worker_t* worker);
	worker_t* worker_get(service_t* svcname);

	
	void service_add(service_t* sv);
	void service_add(string name);
	void service_del(string name);

	void send_work(worker_t* worker,char *command,char *option,char* uuid, zmsg_t *msg);
	
	void work_status_set(string workerid,string uuid);
	void work_status_clear(string workerid);
	void work_status_clear(string svcname,string workerid,string uuid);
	void work_requeue(string uuid);
};



