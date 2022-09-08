#ifndef SYDOR_SCMOS_PARAMS_H
#define SYDOR_SCMOS_PARAMS_H

#include "gc_params.h"
#include "gc_query.h"

#define SDSettingString         "SD_SETTING"
#define SDNumFramesString       "SD_NUM_FRAMES"

#define SDStartRunString        "SD_START_RUN"
// #define SDTriggerString         "SD_TRIGGER"
#define SDFirmwareVersionString "SD_FIRMWARE_VERSION"  /* asynOctet    ro */
#define SDReadModeString        "SD_READ_MODE"
#define SDDoTriggerString       "SD_DO_TRIGGER"

#define SDLinkStatusString      "SD_LINK_STATUS"
#define SDEPICSLinkStatusString "SD_EPICS_STATUS"

#define SDRunStartString            "SD_RUN_START"
#define SDCommandOutString      "SD_COMMAND_OUT"
#define SDOutMuxString          "SD_OUT_MUX"

// A fixed terminator can be added easily by butting the terminator in quotes with the relevant #define
// TODO Should SY*_STR being just the command word be separate from SY*_CMD being the command word and format?

#define SY_SCMOS_RESERVED_NUM 0
#define SY_SCMOS_RESERVED_STR "\0"



#endif 
