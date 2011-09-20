#pragma once
#include "titanic_component.h"
class titanic_request :
	public titanic_component
{
public:
	titanic_request(zctx_t* ctx,string brokername,int hbeat,int reconn);
	~titanic_request(void);
	void Start();
};

