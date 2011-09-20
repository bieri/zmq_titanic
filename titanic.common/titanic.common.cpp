// titanic.common.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "titanic.common.h"


// This is an example of an exported variable
TITANICCOMMON_API int ntitaniccommon=0;

// This is an example of an exported function.
TITANICCOMMON_API int fntitaniccommon(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see titanic.common.h for the class definition
Ctitaniccommon::Ctitaniccommon()
{
	return;
}
