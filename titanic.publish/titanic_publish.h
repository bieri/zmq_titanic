#pragma once
class titanic_publish :	public titanic_component
{
public:
	titanic_publish(zctx_t* ctx,string brokername,char* port,int hbeat,int reconn);
	~titanic_publish(void);
	void Start();
private:
	void* pub_socket;
};

