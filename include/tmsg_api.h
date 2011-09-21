//#define TADD_PUB "tcp://*:5555"
//#define TADD_INPROC "inproc://titanic_broker"
////#define TADD_INPROC "tcp://127.0.0.1:5556"
//#define TMSG_DIR "C:\\titanic.messages"
//
//#define TWRK_SVC_VER "TITS01"
//#define TWRK_CLI_VER "TITC01"
//#define TWRK_WRK_VER "TITW01"
//
////Define the Work Status for Tasks that have been sent for work.
//#define TMSG_STATUS_OK "200"
//#define TMSG_STATUS_PENDING "300"
//#define TMSG_STATUS_UKNOWN "400"
//#define TMSG_STATUS_ERROR "500"
//
////Define the message types. This will allow us to create and manage service discovery.
//#define TMSG_TYPE_READY  "1"
//#define TMSG_TYPE_REQUEST  "2"
//#define TMSG_TYPE_REPLY  "3"
//#define TMSG_TYPE_PUBLISH  "4"
//#define TMSG_TYPE_HEARTBEAT  "5"
//#define TMSG_TYPE_DISCONNECT  "6"
//#define TMSG_TYPE_FINALIZE "7"
//
//static const char* TMSG_TYPES [] = {
//    NULL, "READY", "REQUEST", "REPLY","PUBLISH", "HEARTBEAT", "DISCONNECT","FINALIZE","AVAILABLE"
//};
//
///*
//ALL FRAMES ARE PREPENDED WITH THE REQUIRED ZMQ MSG ENVELOPE.
//DEFAULT FRAMING:
//Frame 0: Message Envelope (Address + Empty Frame comes as part of DEALER Socket)
//Frame 1: TITANIC VERSION (TWRK_SVC_VER,TWRK_CLI_VER)
//Frame 2: 0x01 (one byte, representing the desired COMMAND)
//Frame 3: Service name (printable string)
//**************************************************************************************************
//--------------------------------------------------------------------------------------------------
//COMMAND:	READY / HEARTBEAT  
//SUMMARY:	Consists of a multipart message of 4 frames, formatted on the wire as follows:
//
//--------------------------------------------------------------------------------------------------
//ORIGIN:		WORKER
//Only default framing.
//
//
//ORIGIN:		SERVER
//Only default framing.
//
//**************************************************************************************************
//--------------------------------------------------------------------------------------------------
//COMMAND:	REQUEST
//SUMMARY:	Consists of a multipart message of 6 or more frames, formatted on the wire as follows:
//--------------------------------------------------------------------------------------------------
//ORIGIN:		WORKER
//Frame 4:	Empty (zero bytes, envelope delimiter)
//Frames 5+:	Request body (opaque binary)
//
//ORIGIN:		SERVER
//Frame 4:	Empty (zero bytes, envelope delimiter)
//Frames 5:	UUID
//
//**************************************************************************************************
//--------------------------------------------------------------------------------------------------
//COMMAND:	REPLY
//SUMMARY:	Consists of a multipart message of 6 or more frames, formatted on the wire as follows:
//--------------------------------------------------------------------------------------------------
//ORIGIN:		WORKER
//Frame 3:	Empty (zero bytes, envelope delimiter)
//Frame 4:	UUID
//
//ORIGIN:		SERVER
//Frame 3:	Empty (zero bytes, envelope delimiter)
//Frame 4:	UUID
//Frame 5:	Status (OK,PENDING,UKNOWN,ERROR)
//Frame 5(+):	Result
//
//**************************************************************************************************
//--------------------------------------------------------------------------------------------------
//COMMAND:	PUBLISH
//SUMMARY:	Consists of a multipart message of 4 or more frames, formatted on the wire as follows:
//--------------------------------------------------------------------------------------------------
//ORIGIN:		WORKER
//Frame 3:	Topic
//Frame 4(+):	Opaque Data to publish.
//
//ORIGIN:		SERVER
//No response.
//
//**************************************************************************************************
//--------------------------------------------------------------------------------------------------
//COMMAND:	DISCONNECT
//SUMMARY:	Consists of a multipart message of 3 frames, formatted on the wire as follows:
//--------------------------------------------------------------------------------------------------
//ORIGIN:		WORKER
//Frame 3:	 Service name (printable string)
//
//ORIGIN:		SERVER
//Frame 3:	Service name (printable string)
//Frame 4:	200 (Successful) | 400 (Uknown) | 500 (Error)
//
//**************************************************************************************************
//--------------------------------------------------------------------------------------------------
//COMMAND:	FINALIZE
//SUMMARY:	Consists of a multipart message of 4 frames, formatted on the wire as follows:
//--------------------------------------------------------------------------------------------------
//ORIGIN:		WORKER
//Frame 3:	FRAME containing the UUID that is needed to be finalized.
//
//ORIGIN:		SERVER
//Frame 3:	FRAME containing the UUID that is needed to be finalized.
//Frame 4:	200 (Successful) | 500 (Error)
//*/