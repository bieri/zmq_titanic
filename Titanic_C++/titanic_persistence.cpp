#include "StdAfx.h"
#include "titanic_persistence.h"
#include <czmq.h>
#include <RpcDce.h>
#include <uuids.h>

using namespace std;

namespace titanic_persistence{
	string gen_uuid(){
		UUID uid;
		unsigned char *str;
		string str_res;
		
		try{

			if(UuidCreateSequential(&uid)!=RPC_S_OK)
			{
				if(UuidCreate(&uid)!=RPC_S_OK){
					throw runtime_error("Unable to get guid.");
				}
			}
			if(UuidToStringA(&uid,&str)!=RPC_S_OK){
				throw runtime_error("Unable to convert guid to string.");
			}

				str_res = (char *)str;
		}
		catch (...) {
			free(str);
			throw;
		}
		return str_res;
		
	}

	int store(char* msgtype,char* uuid,zmsg_t* msg){
			return 1;
	}
	
	int exists(char* msgtype,char* uuid){
		return 1;	
	}

	int remove(char* uuid){
		return 1;
	}

	zmsg_t* get(char* msgtype,char* uuid){
		zmsg_t* fs = zmsg_new();
		return fs;
	}
}
