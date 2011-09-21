
#include <list>
#include <RpcDce.h>
#include <uuids.h>

using namespace std;
#pragma once
namespace titanic_persistence
{
	__declspec(dllexport) string gen_uuid();
	//returns 1 for successful persistence.
	__declspec(dllexport) bool store(char* msgtype,char* uuid,zmsg_t* msg);
	__declspec(dllexport) bool exists(char* msgtype,char* uuid);
	__declspec(dllexport) bool finalize(char* uuid);
	__declspec(dllexport) string get_filename(char* msgtype,char* uuid);
	__declspec(dllexport) zmsg_t* get(char* msgtype,char* uuid);
	__declspec(dllexport) void Test(void);
};

