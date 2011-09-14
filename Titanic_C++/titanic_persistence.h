#include <iostream>
#include <czmq.h>
#include <list>
#include <RpcDce.h>
#include <uuids.h>

using namespace std;
#pragma once
namespace titanic_persistence
{
	string gen_uuid();
	//returns 1 for successful persistence.
	int store(char* msgtype,char* uuid,zmsg_t* msg);
	int exists(char* msgtype,char* uuid);
	int remove(char* uuid);
	zmsg_t* get(char* msgtype,char* uuid);
};

