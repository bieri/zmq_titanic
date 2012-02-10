#include "stdafx.h"

#define TADD_PUB "tcp://*:5555"
#define TADD_PRIV "tcp://*:5556"
#define TADD_COMP "tcp://127.0.0.1:5556" //This one is for the components to use.
#define TADD_PUBSUB "tcp://*:5557"
#define TADD_AUDIT "tcp://*:5558"
#define TMSG_DIR "C:\\working\\titanic.messages"

#define TWRK_SVC_VER "TITS01"
#define TWRK_CLI_VER "TITC01"
#define TWRK_WRK_VER "TITW01"

//Define the Work Status for Tasks that have been sent for work.
#define TMSG_STATUS_OK "200"
#define TMSG_STATUS_PENDING "300"
#define TMSG_STATUS_UKNOWN "400"
#define TMSG_STATUS_ERROR "500"

//Define the message types. This will allow us to create and manage service discovery.
#define TMSG_TYPE_READY  "1"
#define TMSG_TYPE_REQUEST  "2"
#define TMSG_TYPE_REPLY  "3"
#define TMSG_TYPE_PUBLISH  "4"
#define TMSG_TYPE_HEARTBEAT  "5"
#define TMSG_TYPE_DISCONNECT  "6"
#define TMSG_TYPE_FINALIZE "7"

//static const char* TMSG_TYPES [] = {
//    NULL, "READY", "REQUEST", "REPLY","PUBLISH", "HEARTBEAT", "DISCONNECT","FINALIZE","AVAILABLE"
//};

#define TWRK_HBT_IVL 10000
