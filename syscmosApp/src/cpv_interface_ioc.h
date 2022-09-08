#ifndef cpv_interface_ioc_H
#define cpv_interface_ioc_H

#include "sydor_scmos_params.h"

#include "ADDriver.h"

#include <stdio.h>
#include <string.h>

#include <asynOctetSyncIO.h>

#define kSTR_SETPV "setpv"
#define kSTR_ASYNC "async"
#define kSTR_GETPV "getpv"

#define kSTR_DOUBLE "d"
#define kSTR_INT32 "i32"
#define kSTR_STRING "s"

#include <unordered_map>

enum SD_Param_Type
  {
   SD_INT32,
   SD_DOUBLE,
   SD_STRING
  };

typedef struct CPV_Type
{
  char *asynParamName;		// The asyn string in the Database file
  char *asynParamQName;		// The asyn string in the Database file
				// for the Querier
  int param_num;		// The parameter number for the setter/getter
  int param_q_num;		// The parameter number for the querier
  char *command_string;		// The string for the command to DataViewer
  enum SD_Param_Type pv_type;	// Data type of the PV
} CPV_Data_t;

class syscmos; // fwd proto

class CPV_Interface_IOC
{
	// constructor
public:

    struct _structPR
    {
        int nType; // 1 = Int32, 2 = Float64, 3 = string
        epicsInt32 ival;
        epicsFloat64 fval;
        char *sval;
    };

    typedef struct _structPR PRType;
	const char kSPECIALCHAR_FLAG = '@';
  const char kGETCHAR_FLAG = '$';
	static const int kSizeOfPrivateBuffer = 1024;


public:
	//
	//  Create a table (HASH) aka Dictionary of 
	//    Key=PVName, Value = DataViewer Epics Plugin String
	//
	std::unordered_map<std::string, std::string> pv_dv_cmds = {
        {ADAcquireTimeString,"IntegTime"},		 // PV: AcquireTime
		{SDInterframeTimeString, "InterTime"},	 // PV:InterframeTime 
        {SDNumFramesString,"NFrames"},  // PV: NumFrames
        {ADTriggerModeString,"Trigger"},		 // PV:TriggerMode
		{ADNumImagesString,"NImages"},	     // PV:NumImages
		//[0]
		// Special SetName, SetDescription, and RunName must all be set
		// and then StartRun is selected. 
		// {SDSetNameString, "SetName"},			 // NYI
		//{SDSetDescriptionString, "SetDescription"},  // NYI
		//{SDRunNameString, "RunName"},			 // NYI
		//{SDStartRunString, "actionRun {setName:$1,description:$2,runName:$3}"
		{SDStartRunString, "actionCapture"},
	//{SDSelectRunString, "@SPECIAL_SELECT_RUN"},

		//[0]

		{SDDoTriggerString, "actionTrigger"},	 // NYT
		{ADShutterControlString, "Shutter"},	 	 // NYT
		{SDSensorPowerString, "SensorPower"},	 // NYT
	{SDLinkStatusString, "Connected"},
	{SDOutMuxString, "$OutMux"},
	{SDRunStartString, "actionStartRun"},
	{ADReverseXString, "HFlip"},
	{ADReverseYString, "VFlip"},
	{SDDSNUString, "DSNU"},
	{SDPRNUString, "PRNU"},
    };


	CPV_Interface_IOC(syscmos *psyscmos); 
	
	int Init( asynOctetSyncIO *pasynOctetSyncIO, asynUser *pasynUserMeter );
	
	int SetPV(const int pvNum, epicsInt32 val);
	int SetPV(const int pvNum, epicsFloat64 val);
	int SetPV(const int pvNum, const char *pval);
  int GetPV(const char *cmdName, int paramNum);
	int ParseResponse(const char *strResponse, int *nFunction, PRType *prt);

public:
	epicsEventId cmdEventId;

protected:
	asynStatus writeWithReply(char *pstr);
  bool _FindMatchingPV( const char *cmd_num, char *cmdName);
  bool _HandleSpecialCommands(const char *cmdName, int req_type);
  int _FindMatchingCmd(const int cmd_num, char *cmdName, enum SD_Param_Type *data_type = NULL);
  bool _FindResponsePV(const char *cmd_str, int *pv_num); ///TODO Fix this and add functionality
  bool _FindSpecialResponse(const std::string &cmd_str);
protected:
	asynOctetSyncIO *m_pasynOctetSyncIO = NULL;	
	asynUser        *m_pasynUserMeter = NULL;
	syscmos          *m_syscmos=NULL;
	uint32_t      m_sendCommandCounter = 0;
	char m_privateBuffer[kSizeOfPrivateBuffer]; // outgoing buffer
	//char m_outBuffer[128]; // outgoing buffer
	char m_inBuffer[128]; // incoming buffer
	
	


};	



#endif

