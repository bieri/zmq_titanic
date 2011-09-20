#include <czmq.h>
#include "titanic_component.h"
#include <iostream>

using namespace std;

class titanic_reply:public titanic_component
{
public:
	titanic_reply(zctx_t* ctx,string brokername,int hbeat,int reconn);
	~titanic_reply(void);
	void Start();
private:
	void message_from_worker(zmsg_t* reply,zframe_t* envelope,char* origin,zframe_t* service,char* command,char* uuid);
	zmsg_t* message_from_client(zframe_t* envelope,char* origin,zframe_t* service,char* command,char* uuid);
};

