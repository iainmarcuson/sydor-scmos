#ifndef SYDOR_SCMOS_PARAMS_H
#define SYDOR_SCMOS_PARAMS_H


#define SDSettingString         "SD_SETTING"
#define SDDelayTimeString       "SD_DELAY_TIME"
#define SDThresholdString       "SD_THRESHOLD"
#define SDEnergyString          "SD_ENERGY"
#define SDUseFlatFieldString    "SD_USE_FLATFIELD"
#define SDUseCountRateString    "SD_USE_COUNTRATE"
#define SDTauString             "SD_TAU"
#define SDUseBadChanIntrplString "SD_USE_BADCHANNEL_INTRPL"
#define SDBitDepthString        "SD_BIT_DEPTH"
#define SDUseGatesString        "SD_USE_GATES"
#define SDNumGatesString        "SD_NUM_GATES"
#define SDNumFramesString       "SD_NUM_FRAMES"
#define SDInterframeTimeString  "SD_INTERFRAME_TIME"
#define SDSensorPowerString     "SD_SENSOR_POWER"
#define SDRunNameString         "SD_RUN_NAME"
#define SDSetNameString         "SD_SET_NAME"
#define SDSetDescriptionString  "SD_SET_DESCRIPTION"

#define SDStartRunString        "SD_START_RUN"
// #define SDTriggerString         "SD_TRIGGER"
#define SDResetString           "SD_RESET"
#define SDNModulesString        "SD_NMODULES"
#define SDFirmwareVersionString "SD_FIRMWARE_VERSION"  /* asynOctet    ro */
#define SDReadModeString        "SD_READ_MODE"
#define SDDoTriggerString       "SD_DO_TRIGGER"

#define SDDSNUString            "SD_DSNU"
#define SDPRNUString            "SD_PRNU"

#define SDLinkStatusString      "SD_LINK_STATUS"
#define SDEPICSLinkStatusString "SD_EPICS_STATUS"

#define SDRunStartString            "SD_RUN_START"
#define SDSelectRunString       "SD_SELECT_RUN"
#define SDLoadNumFramesString   "SD_LD_NUM_FRAMES"
#define SDCommandOutString      "SD_COMMAND_OUT"
#define SDOutMuxString          "SD_OUT_MUX"

// A fixed terminator can be added easily by butting the terminator in quotes with the relevant #define
// TODO Should SY*_STR being just the command word be separate from SY*_CMD being the command word and format?

#define SY_SCMOS_RESERVED_NUM 0
#define SY_SCMOS_RESERVED_STR "\0"



#endif 
