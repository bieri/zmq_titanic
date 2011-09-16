#include "StdAfx.h"
#include "titanic_persistence.h"
#include <czmq.h>
#include <RpcDce.h>
#include <uuids.h>
#include <iostream>
#include "tmsg_api.h"

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

	bool store(char* msgtype,char* uuid,zmsg_t* msg){
		char * fn = titanic_persistence::get_filename(msgtype,uuid);	
		FILE * f  = fopen(fn,"w+");
		zmsg_save(msg,f);
		return 1;
	}
	
	bool exists(char* msgtype,char* uuid){
		return 1;	
	}

	bool finalize(char* uuid){
		char* req_f = titanic_persistence::get_filename(TMSG_TYPE_REQUEST,uuid);
		char* rep_f = titanic_persistence::get_filename(TMSG_TYPE_REPLY,uuid);

		int req = remove(req_f);
		int rep = remove(rep_f);

		return (req==rep && req==1);
	}

	zmsg_t* get(char* msgtype,char* uuid){
		char * fn = titanic_persistence::get_filename(msgtype,uuid);	
		FILE * f  = fopen(fn,"r+");
		zmsg_t* fs = zmsg_load(NULL,f);
		return fs;
	}
	char* get_filename(char* msgtype,char* uuid){
		char * wd;
		char flder='\\';
		strcat(&flder,msgtype);
		if( (wd = _getcwd( NULL, 0 )) != NULL ){
			strcat(wd,&flder);
			if(!_chdir(wd))
				mkdir(wd);
			char* fname = uuid;
			strcat(fname,".txt");
			strcat(wd,fname);
			return wd;
		}
	}
}
