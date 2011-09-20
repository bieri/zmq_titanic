// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TITANICCOMMON_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TITANICCOMMON_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TITANICCOMMON_EXPORTS
#define TITANICCOMMON_API __declspec(dllexport)
#else
#define TITANICCOMMON_API __declspec(dllimport)
#endif

// This class is exported from the titanic.common.dll
class TITANICCOMMON_API Ctitaniccommon {
public:
	Ctitaniccommon(void);
	// TODO: add your methods here.
};

extern TITANICCOMMON_API int ntitaniccommon;

TITANICCOMMON_API int fntitaniccommon(void);
