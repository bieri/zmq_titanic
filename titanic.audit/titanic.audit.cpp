// titanic.audit.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "titanic_audit.h"

int _tmain(int argc, _TCHAR* argv[])
{
  int interval = 100 * 1000; // milliseconds
  zctx_t* context = zctx_new();
  titanic_audit* component = new titanic_audit( context,
                                                TADD_COMP,
                                                TADD_AUDIT,
                                                interval,
                                                interval );
  
  // give the broker time to get bound up 
  // and set up all the pollers on its sockets
  zclock_sleep(1000);
  cout << "starting audit stream\n\r";
  component->Start();
  
  delete component;
  return 0;
}
