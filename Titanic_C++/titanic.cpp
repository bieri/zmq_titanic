// Titanic_C++.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <zmq.hpp>
#include "tmsg_api.h"
#include <Windows.h>
#include <Rpc.h>
#include <RpcDce.h>
#include <iostream>
#include <atlstr.h>
#include "titanic_persistence.h"
#include "titanic_dispatcher.h"


int _tmain(int argc, _TCHAR* argv[])
{
	titanic_persistence::Test();


	titanic_dispatcher* d = new titanic_dispatcher("MYBk",100,100,NULL);
	d->Test();
	d->set_Reconnect_Ivl(1100);
	//d->service_add(&string("MySvc"));


	 int major, minor, patch;
	 
    // zmq_version(&major, &minor, &patch); // pure C version
    zmq::version(&major, &minor, &patch); // C++ wrapper version
    std::cout << "Current 0MQ version is " << major <<"." << minor << "." << patch << std::endl;
}

