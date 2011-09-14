#include <czmq.h>
#include "titanic_component.h"
#include <iostream>

using namespace std;

class titanic_reply:public titanic_component
{
public:
	titanic_reply(string brokername,int hbeat,int reconn);
	~titanic_reply(void);
	void Start();
};

