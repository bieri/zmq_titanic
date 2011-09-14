#include <czmq.h>
//  This defines a single broker
typedef struct {
    zctx_t *ctx;                //  Our context
    void *socket;               //  Socket for clients & workers
    int verbose;                //  Print activity to stdout
    char *endpoint;             //  Broker binds to this endpoint
    zhash_t *services;          //  Hash of known services
    zhash_t *workers;           //  Hash of known workers
    zlist_t *waiting;           //  List of waiting workers
    uint64_t heartbeat_at;      //  When to send HEARTBEAT
} broker_t;

//  This defines a single service
typedef struct {
    char *name;                 //  Service name
    zlist_t *requests;          //  List of client requests
    zlist_t *waiting;           //  List of waiting workers
    size_t workers;             //  How many workers we have
} service_t;


//  This defines one worker, idle or active
typedef struct {
    char *identity;             //  Identity of worker
    zframe_t *address;          //  Address frame to route to
    service_t *service;         //  Owning service, if known
    int64_t expiry;             //  Expires at unless heartbeat
} worker_t;

////  ---------------------------------------------------------------------
////  Broker functions
//static broker_t * s_broker_new (int verbose);
//static void s_broker_destroy (broker_t **self_p);
//static void s_broker_bind (broker_t *self, char *endpoint);
//static void s_broker_purge_workers (broker_t *self);
//
////  Service functions
//static service_t * s_service_require (broker_t *self, zframe_t *service_frame);
//static void s_service_destroy (void *argument);
//static void s_service_dispatch (broker_t *self, service_t *service, zmsg_t *msg);
//static void s_service_internal (broker_t *self, zframe_t *service_frame, zmsg_t *msg);
//
////  Worker functions
//static worker_t * s_worker_require (broker_t *self, zframe_t *address);
//static void s_worker_delete (broker_t *self, worker_t *worker, int disconnect);
//static void s_worker_destroy (void *argument);
//static void s_worker_process (broker_t *self, zframe_t *sender, zmsg_t *msg);
//static void s_worker_send (broker_t *self, worker_t *worker, char *command, char *option, zmsg_t *msg);
//static void s_worker_waiting (broker_t *self, worker_t *worker);
