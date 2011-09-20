#include "stdafx.h"
#include <titanic_component.h>

using namespace std;

class titanic_finalize :
	public titanic_component
{
public:
	titanic_finalize(zctx_t* context,string brokername,int hbeat,int reconn,int cleanupivl);
	~titanic_finalize(void);

	//Properties
	int Cleanup_Ivl;
	//Required Methods.
	void Start();

};

