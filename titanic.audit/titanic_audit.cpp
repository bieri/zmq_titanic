#include "stdafx.h"
#include "titanic_audit.h"

titanic_audit::titanic_audit( zctx_t* ctx,
                              string  brokername,
                              char*   pubPort,
                              int     hbeat,
                              int     reconn )
  : titanic_component(ctx,"titanic.audit",brokername,ZMQ_XREP,hbeat,reconn)
    {
      this->pub_socket = zsocket_new(this->Context,ZMQ_PUB);
      zsocket_bind(this->pub_socket,pubPort);
    }

titanic_audit::~titanic_audit(void) { zmq_close(this->pub_socket); }

void titanic_audit::Start()
{
  this->connect_to_broker();
  
  while (TRUE)
  {
    zmsg_t *msg = this->get_work();
    if (!msg) break; // EXIT: interrupted

    zmsg_pushstr(msg,"titanic.audit"); //TODO: better message topic(s)
    zmsg_send(&msg,this->pub_socket);
    zmsg_destroy(&msg);
  }
}
