#include "StdAfx.h"
#include "titanic_persistence.h"
#include <czmq.h>
#include <RpcDce.h>
#include <uuids.h>
#include <iostream>
#include <fstream>
#include <string>
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
    string fn = titanic_persistence::get_filename(msgtype,uuid);
    ofstream  outstream;
    outstream.open(fn.c_str(),ios::out);
    cout << "Storing at: " << fn.c_str() << endl;
    zframe_t *frame = zmsg_first (msg);
    while (frame) {
      size_t frame_size = zframe_size (frame);
      string strval =string( zframe_strdup(frame));
      outstream << strval.c_str() << endl;
      frame = zmsg_next (msg);
    }
    outstream.close();
    return true;
  }

  bool exists(char* msgtype,char* uuid){
    string f = titanic_persistence::get_filename(msgtype,uuid);
    FILE* fp = NULL;
    fp = fopen( f.c_str(), "rb" );
    if( fp != NULL )
    {
      fclose( fp );
      return true;
    }
    return false;
  }

  bool finalize(char* uuid){
    string req_f = titanic_persistence::get_filename(TMSG_TYPE_REQUEST,uuid);
    string rep_f = titanic_persistence::get_filename(TMSG_TYPE_REPLY,uuid);
    int req=0;
    int rep=0;
    if(titanic_persistence::exists(TMSG_TYPE_REQUEST,uuid))
      req = remove(req_f.c_str());
    if(titanic_persistence::exists(TMSG_TYPE_REPLY,uuid))
      rep = remove(req_f.c_str());

    return (req==rep && req==1);
  }

  zmsg_t* get(char* msgtype,char* uuid){
    zmsg_t* m = zmsg_new();

    string fn = titanic_persistence::get_filename(msgtype,uuid);	
    ifstream f (fn);
    string line;
    if(f.is_open()){
      while(getline(f,line)){
        zframe_t* frm = zframe_new(line.c_str(),strlen(line.c_str()));
        zmsg_add(m,frm);
      }
      f.close();
    }
    return m;
  }

  string get_filename(char* msgtype,char* uuid){
    string s_wd = string(TMSG_DIR);
    if(_chdir(s_wd.c_str())!=0)
      _mkdir(s_wd.c_str());

    s_wd.append(string("\\"));
    s_wd.append(string(msgtype));

    int ex = _chdir(s_wd.c_str());
    if(ex!=0)
      _mkdir(s_wd.c_str());
    s_wd.append(string("\\"));
    s_wd.append(string(uuid));
    s_wd.append(string(".txt"));

    return  s_wd;

  }
  void Test(){
    string uuid = titanic_persistence::gen_uuid();
    char* c_uuid = (char*) uuid.c_str();
    zmsg_t* msg = zmsg_new();
    zmsg_addstr(msg,"test");
    titanic_persistence::store(TMSG_TYPE_REQUEST,c_uuid,msg);
    if(!titanic_persistence::exists(TMSG_TYPE_REQUEST,c_uuid)){
      assert("failed:");
    }

    zmsg_t* msg_back  = titanic_persistence::get(TMSG_TYPE_REQUEST,c_uuid);
    if(strcmp(zmsg_popstr(msg),"test")!=0){
      assert("failed");
    }
    zmsg_destroy(&msg);
    zmsg_destroy(&msg_back);

    titanic_persistence::finalize(c_uuid);
    if(titanic_persistence::exists(TMSG_TYPE_REQUEST,c_uuid)){
      assert("failed:");
    }

  }
}
