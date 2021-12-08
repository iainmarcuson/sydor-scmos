// File: cpv_interface_ioc.cpp
//
// File History
// v 1.0 YF 20APR2021
//   format strings defined so far: setpv<i32>, setpv<d>
// v 1.1 YF 22APR2021 all the core PV's are wired in. Used hash (un-ordered map).
#include "cpv_interface_ioc.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEndian.h>
#include <epicsEvent.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <epicsStdio.h>
#include <cantProceed.h>
#include <iocsh.h>

#include <asynOctetSyncIO.h>

#include "ADDriver.h"
#include "NDPluginDriver.h"

#include <epicsExport.h>

/// #include "syscmosV3.h"
#include "sydor_scmos_params.h"
#include "syscmos.h"

#include <iostream>

#include <vector>

//extern epicsMutex *plock_ControlPortIO;
#define kSizeOfCmdName 32
static char s_cmdName[kSizeOfCmdName];
static char s_pvName[kSizeOfCmdName];

// ***************************************************************************
// ********    Define class to send PV requests to DataViewer plugin *********
// ***************************************************************************

// public constructor
CPV_Interface_IOC::CPV_Interface_IOC(syscmos *psyscmos)
{
    printf("CPV_Implementation_IOC constructed");
    m_syscmos = psyscmos;
    //memset(m_outBuffer, 0, sizeof(m_outBuffer));
    
    s_cmdName[kSizeOfCmdName-1] = '!'; // debug used to check for overrun.

    

    //debug
    ///printf(" DEBUG: %s %d \n\n", __func__,  s_cmdName[31]);
};

int CPV_Interface_IOC::Init(asynOctetSyncIO *pasynOctetSyncIO,
                            asynUser *pasynUserMeter)
{
    m_pasynOctetSyncIO = pasynOctetSyncIO;
    m_pasynUserMeter = pasynUserMeter;

    //   Create the epicsEvents
    this->cmdEventId = epicsEventCreate(epicsEventEmpty);
    if (!this->cmdEventId)
    {
        printf("epicsEventCreate failure for cmdEvent\n");

        return (-1);
    }

    return (0); // No Error
}

//
//  creates returned string in m_privateBuffer
//  returns true if special command was detected.
//
bool CPV_Interface_IOC::_HandleSpecialCommands(const char *acmdName, int req_type)
{
    // @SPECIAL_START_RUN
    const char *pcmd = acmdName + 1; // skip past the '@'
    bool bret = false;

    ///
    printf("Handling special command: %s", pcmd);
    if (strcmp(pcmd, "SPECIAL_START_RUN") == 0)
    {
        std::string runName, setName, setDescription;
        
        m_syscmos->getStringParam(m_syscmos->SDSetName, /* byref */ setName);
        m_syscmos->getStringParam( m_syscmos->SDSetDescription, /* byref */ setDescription);
        m_syscmos->getStringParam(m_syscmos->SDRunName, /* byref */  runName);
        

	/// DataViewer appears to show "description" as the magical phrase for the set description in the setings
	/*
        snprintf(m_privateBuffer,  kSizeOfPrivateBuffer-1,   
            "#%d:setpv<s>:actionStartRun:{setName\\:%s,setDescription\\:%s,runName\\:%s}\r\n", 
            m_sendCommandCounter++, setName.c_str(), setDescription.c_str(), runName.c_str() );
	*/                
	snprintf(m_privateBuffer,  kSizeOfPrivateBuffer-1,   
            "#%d:setpv<s>:actionStartRun:{setName\\:%s,description\\:%s,runName\\:%s}\r\n", 
            m_sendCommandCounter++, setName.c_str(), setDescription.c_str(), runName.c_str() );                

        bret = true;    
    }
    else if (strcmp(pcmd, "SPECIAL_SELECT_RUN") == 0)
      {
	std::string runName, setName, setDescription;
	int mode = 1;		// TODO Allow background frames
	int b_avg = 0;		// TODO Presumable for averaging BG frames
	int b_call_fetch_avg = 0; // TODO Find out what this does
        int num_frames = 1;	  // Allow this to be changed 
        m_syscmos->getStringParam(m_syscmos->SDSetName, /* byref */ setName);
        m_syscmos->getStringParam( m_syscmos->SDSetDescription, /* byref */ setDescription);
        m_syscmos->getStringParam(m_syscmos->SDRunName, /* byref */  runName);
        m_syscmos->getIntegerParam(m_syscmos->SDLoadNumFrames, &num_frames);

        snprintf(m_privateBuffer,  kSizeOfPrivateBuffer-1,   
            "#%d:setpv<s>:q2c_SelectRunItem:{mode\\:%i,setName\\:%s,setDescription\\:%s,runName\\:%s,NFrames\\:%i,bAverage\\:%i,bCallFetchAverage\\:%i}\r\n", 
		 m_sendCommandCounter++, mode, setName.c_str(), setDescription.c_str(), runName.c_str(), num_frames, b_avg, b_call_fetch_avg);                
	///
	printf("Select Run command string: %s\n", m_privateBuffer);
        bret = true;    
    }
    else if (strcmp(pcmd, "AcqROI") == 0)
      {
	int enable_roi, min_x, min_y, size_x, size_y;
	if (req_type > 0)	// We are doing a set
	  {
	    m_syscmos->getIntegerParam(m_syscmos->SDEnableROI, &enable_roi);
	    m_syscmos->getIntegerParam(m_syscmos->ADMinX, &min_x);
	    m_syscmos->getIntegerParam(m_syscmos->ADMinY, &min_y);
	    m_syscmos->getIntegerParam(m_syscmos->ADSizeX, &size_x);
	    m_syscmos->getIntegerParam(m_syscmos->ADSizeY, &size_y);
	    snprintf(m_privateBuffer,  kSizeOfPrivateBuffer-1,   
		     "#%d:setpv<s>:Region:%i,%i,%i,%i,%i\r\n", 
		     m_sendCommandCounter++, enable_roi, min_x, min_y, size_x, size_y);
	  }
	else			// We are doing a get
	  {
	    snprintf(m_privateBuffer, kSizeOfPrivateBuffer-1,
		     "#%d:getpv<s>:Region?\r\n",
		     m_sendCommandCounter++);
	  }
      }
    
    ///
    fflush(stdout);

    return bret;

}

//
//  Helper Function
//

int CPV_Interface_IOC::_FindMatchingCmd( const int cmd_num, char *cmdName, enum SD_Param_Type *data_type) //data_type = NULL
{
  ///printf("FindMatchingCmd: cmd: \"%s\"\n", cmdName);
  int ret = 0;		// Default to no match
  for (const auto &n : m_syscmos->pv_data_list )
    {
      if (cmd_num == n.param_num) // We match on a setter/getter
	{
	  strcpy(cmdName, n.command_string);
	  ret = 1;		// Note that we are a settter/getter
	}
      else if (cmd_num == n.param_q_num)
	{
	  strcpy(cmdName, n.command_string);
	  ret = -1;		// Note we are a querier
	}
      if (ret)	// We found a match 
	{
	  if (data_type)	// Returning type as well
	    {
	      *data_type = n.pv_type;
	    }
	  return ret;
	}
    }
  ///TODO assert like in _FindMatchingPV()?
  
  return 0;			// Parameter not found
}


//
//  Helper Function
//
bool CPV_Interface_IOC::_FindMatchingPV( const char *pvName, char *acmdName)
{
    for (const auto &n : pv_dv_cmds)
    {
        // std template is garbage. 'n.first' is KEY in all normal languages
        if (strcmp(pvName, n.first.c_str()) == 0)
        {
            strcpy(acmdName, n.second.c_str());
            return true;
        }
    }

    //debug check
    
    assert ( acmdName[ kSizeOfCmdName - 1] == '!'); // overrun check
    //
    return false;
}

// Returns:
//    asynSucces (= 0 )
//    -1 Unknown PV or unhandled
//    asynError (= 3)
int CPV_Interface_IOC::GetPV(const char *cmdName, int paramNum)
{
  const char *paramName = NULL;		// The PV asyn name, looked up by number
  bool bMatch;
  char *paramType;
    
  m_syscmos->getParamName(0, paramNum, &paramName);

  ///FIXME TODO Handle the Get parameters
  //bMatch = _FindResponsePV(paramName, s_cmdName);
  bMatch = false;
  
  if (!bMatch)
    {
      ///printf(" :: unhandled command:%s\n", paramName);
      ///free(paramName);
      return -1;
    }
  else
    {
      ///printf(" cmdName = %s \n", s_cmdName);
    }
  ///free(paramName);

  if (m_syscmos->pv_dv_types[paramNum] == SD_INT32)
    {
      paramType = kSTR_INT32;
    }
  else if (m_syscmos->pv_dv_types[paramNum] == SD_DOUBLE)
    {
      paramType = kSTR_DOUBLE;
    }
  else
    {
      paramType = kSTR_STRING;
    }
  
  
  //TODO Handle type of PV below -- string in lookup table?
  snprintf(m_privateBuffer, kSizeOfPrivateBuffer-1,
	   "#%d:getpv<%s>:%s?\r\n", m_sendCommandCounter++, paramType, s_cmdName);

  return writeWithReply(m_privateBuffer);
}

//  PV           DV
//  NumImages    NImages
//  NumFrames    FramesPerTrigger
//  TriggerMode  Trigger
//  Shutter
//  DoTrigger
//  SensorPower
//
//  Returns:
//    asynSuccess (= 0 ) calling function should call
//   -1 = Unknown PV or unhandled.
//    asynError ( = 3 )
int CPV_Interface_IOC::SetPV(const int pvNum, epicsInt32 val)
{
  enum SD_Param_Type data_type;
  int cmdMatch = _FindMatchingCmd(pvNum, /* out */ s_cmdName, &data_type);

    if (cmdMatch == 0)
    {
      //TODO Put in lookup of command name here?
        printf(" :: unhandled command:%i\n", pvNum);
        return (-1);
    }
    ///TODO Handle commands properly
    
    bool bSpecialCommand = false;  (void)bSpecialCommand; // SUC
    if (s_cmdName[0] == kSPECIALCHAR_FLAG) // TODO Add query support
    {
      bSpecialCommand = _HandleSpecialCommands(s_cmdName, cmdMatch);
        // ^ returns long string in m_privateBuffer
    }
    else if (cmdMatch < 0)		// We are doing a get
      {
	char * type_string;
	if (data_type == SD_INT32)
	  {
	    type_string = kSTR_INT32;
	  }
	else if (data_type == SD_DOUBLE)
	  {
	    type_string = kSTR_DOUBLE;
	  }
	else if (data_type == SD_STRING)
	  {
	    type_string = kSTR_STRING;
	  }
	else			// Shouldn't be here
	  {
	    printf("Unknown query data type: %i\n", data_type);
	    return -1;
	  }
	snprintf(m_privateBuffer, kSizeOfPrivateBuffer-1,
		 "#%d:getpv<%s>:%s?\r\n", m_sendCommandCounter++, type_string, s_cmdName);
      }
    else 			// We are doing a set -- cmdMatch > 0
    {
        //
        // SEND Request to change
        //
      /// TODO Perhaps look up data type?  May not be necessary for a set
        snprintf(m_privateBuffer, kSizeOfPrivateBuffer-1,
        "#%d:setpv<i32>:%s:%d\r\n", m_sendCommandCounter++, s_cmdName, val);
    }
    return writeWithReply(m_privateBuffer); 

}




//
//  Returns:
//    asynSuccess (= 0 ) calling function should call
//   -1 = Unknown PV or unhandled.
//    asynError ( = 3 )
int CPV_Interface_IOC::SetPV(const int pvNum, epicsFloat64 val)
{
  int cmdMatch = _FindMatchingCmd(pvNum, /*out */ s_cmdName);

   
    // can decide HERE if PV is handled - or can forward to socket and look for ? response.
    // It is probably more efficient to check here first


    if (cmdMatch == 0)
    {
      ///TODO Put in a lookup of command name?
        printf(" :: unhandled command num:%i\n", pvNum);
        return (-1);
    }
    else
    {
      ///printf(" cmdName = %s \n", s_cmdName);
    }

    /// TODO Handle return value and commands properly
    //
    // SEND Request to change
    //
    snprintf(m_privateBuffer, kSizeOfPrivateBuffer-1,
       "#%d:setpv<d>:%s:%f\r\n", m_sendCommandCounter++, s_cmdName, (double)val);
    return writeWithReply(m_privateBuffer); 
    
};

int CPV_Interface_IOC::SetPV(const int pvNum, const char *pval)
{
  ///printf("F:%s pvName:%s, val:%s", __func__, pvName, pval); // DEBUG

    // can decide HERE if PV is handled - or can forward to socket and look for ? response.
    // It is probably more efficient to check here first

  int cmdMatch = _FindMatchingCmd(pvNum, /*out*/ s_cmdName);

  if (cmdMatch == 4)
    {
      ///TODO Put in a looklup of command name?
        printf(" :: unhandled command number:%i\n", pvNum);
        return (-1);
    }
    else
    {
      ///printf(" cmdName = %s \n", s_cmdName);
    }


  ///TODO Handle results properly

    // SEND Request to change
    //
    snprintf(m_privateBuffer, kSizeOfPrivateBuffer-1,
       "#%d:setpv<d>:%s:%s\r\n", m_sendCommandCounter++, s_cmdName, pval);
    return writeWithReply(m_privateBuffer); 

     
};



//  Send a string to the detector
// Response goes in global m_inBuffer
//
//  Return codes
//   asynSuccess, asynError;
//  
asynStatus CPV_Interface_IOC::writeWithReply(char *pstr)
{
    size_t nwrote = 0;
    int eventStatus = 0;
    // static char s_outBuffer[128]; // outgoing buffer

    asynStatus status = asynSuccess;
    bool bBreak = false;
    int nCycle = 0;
    
    const int kWRITESIZE =  128 ;

    // need to handle two cases:
    // (1) len(pstr) <  sizeof(s_outBuffer);
    // (2) len(pstr) >=  sizeof(s_outBuffer);

    while (true)
    {

        if (strlen(pstr) < kWRITESIZE)
        {
            bBreak = true;
            char *pEndOfString = pstr + strlen(pstr) + 1;
            // Nice to do - not required::
            for (int i = 1 + strlen(pstr); i < kWRITESIZE; i++)
            {
                *pEndOfString++ = 0; // 0 // zero out all the unused buffer to end, since
                                     // we send a fixed length output.
        
            }
        }
        else
        {
            printf("DEBUG: writeWithReply(). Break up long string. Cycle:%d\n", nCycle);
            nCycle++;

        }

        status = m_pasynOctetSyncIO->write(m_pasynUserMeter, pstr, kWRITESIZE,
                                           1 /*M1K_TIMEOUT*/, &nwrote);

        ///printf(" F:%s >%s Stat:%d. Wrote:%d \n", __func__, pstr, status, (int)nwrote); // DEBUG
        if (bBreak)
            break;
        else
        {
            // loop again
            pstr += kWRITESIZE;
        }

    }

    eventStatus = epicsEventWaitWithTimeout(this->cmdEventId, 1);

    ///printf("  _returned from epicsEventWait %d\n", eventStatus );
    ///printf("%s: returning %i.\n", __FUNCTION__, status);
    return status;
}

//
//  std::string has no split method FGS ?
//

void tokenize(std::string const &str, const char delim,
              std::vector<std::string> &out)
{
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}

//
// std::string has no built-in find and replace - WTF?
//
void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

//
// TODO
// Returns
//  (-1) = bad format. setpv expected - not found.
//  (-2) = OK nor ERR not found.
//  (-3) = bad format. async expected - not found.
//  (-100) = got '?'.  Presume the PV in not known to plugin
//  (<= -1000) got ERR. errcode is -(returnval)
//     err:1000 = didn't get four colon separated tokens.
//     err:1010 = float parse error
//     err:1020 = over range or out of max bound
//
int CPV_Interface_IOC::ParseResponse(const char *strResponse, int *nFunction, PRType *prt)
{
    int ret = 0;
    // Expect  one of:
    // #5:setpv<i32>:TriggerMode:1:OK \r\n   -- or --
    // #5:async<i32>:TriggerMode:1:ERRnnnn \r\n      -- or --
    // #5:async<i32>:TriggerMode:1 \r\n      -- or --
    //  ?\r\n
    
    std::string sstr(strResponse);
    const char delim = ':';
    std::vector<std::string> out;
    tokenize(sstr, delim, out);

    
    std::string pvname;
    std::string strval;
    bool bOK = false;
    std::unordered_map<std::string, std::string>::const_iterator got_cmd;

    if (5 == out.size())
    {
        // #5:setpv<i32>:TriggerMode:1:OK \r\n          -- or --
        // #5:async<i32>:TriggerMode:1:ERRnnnn \r\n
      if ((0 != out[1].find(kSTR_SETPV))
	  && (0 != out[1].find(kSTR_GETPV)))
        {
            // Expected 'setpv' or 'getpv' - not found
            return (-1); //  **** EXIT
        }
        if (0 == out[4].find("OK"))
        {
            bOK = true;
            pvname = out[2];
            strval = out[3];
        }
        else if ( 0 == out[4].find("ERR") )
        {
	  int errcode = atoi(out[4].substr(3,std::string::npos).c_str() );
            return (-errcode); //  **** EXIT
        }
        else
        {
            // Expected 'OK' or ERRnnnn - not found
            return (-2);        //  **** EXIT
        }
       
    }

    else if (4 == out.size())
    {
        // #5:async<i32>:TriggerMode:1 \r\n
        if (0 != out[1].find(kSTR_ASYNC))
        {
            // Expected 'async' - not found
            return (-3); //  **** EXIT
        }

	///ICM TODO Should probably set bOK true here.
        pvname = out[2];
        strval = out[3];
    }

    else if (1 == out.size())
    {
        // un-recognized command
        return (-100);      //  **** EXIT
    }

    ///DEBUGGING
    if (!bOK)
      {
	return -1;
      }
    
    ///printf("Debug F:%s %s %s OK:%d \n", __func__, 
    ///    strResponse, pvname.c_str(), bOK);

    if (_FindResponsePV(pvname.c_str(), nFunction))
      {
	///printf("Found command %s in map search.\n", pvname.c_str());
	///printf("asyn param name: %s\n", s_pvName);
	///XXX Need to handle based on type
	//m_syscmos->findParam(s_pvName, nFunction);
	///TODO Need to get response code correct
	///FIXME type parsing is temporarily disabled
	if (out[1].find("<i32>") != std::string::npos) // Integer type
	  {
	    prt->nType = 1;
	    prt->ival = atoi(strval.c_str());
	  }
	else			// FIXME XXX handle string as well as double
	  {
	    ///
	    printf("Handling double param.\n");
	    fflush(stdout);
	    prt->nType = 2; // float64
	    prt->fval = atof(strval.c_str());
	  }
      }
    else if (_FindSpecialResponse(pvname)) // Could be a special command
      {
	ret = 1;		// A success, but we will not handle after return
	/// TODO ICM -- Maybe break this out to a separate function
	if (pvname == "Region")	// A region lookup
	  {
	    int enable_roi, min_x, min_y, size_x, size_y;
	    sscanf(strval.c_str(), " %i , %i , %i , %i , %i ", &enable_roi, &min_x, &min_y,
		   &size_x, &size_y);
	    m_syscmos->setIntegerParam(m_syscmos->SDEnableROI, enable_roi);
	    m_syscmos->setIntegerParam(m_syscmos->ADMinX, min_x);
	    m_syscmos->setIntegerParam(m_syscmos->ADMinY, min_y);
	    m_syscmos->setIntegerParam(m_syscmos->ADSizeX, size_x);
	    m_syscmos->setIntegerParam(m_syscmos->ADSizeY, size_x);
	  }
      }

    // TODO: create MapToDV( ADAcquireTimeString) - returns IntegTime
    
      /// TODO May want to return a different code if _FindResponsePV and 
      /// _FindSpecialResponsePV are false
    return (ret);
}

bool CPV_Interface_IOC::_FindSpecialResponse(const std::string &cmd_str)
{
  if (cmd_str == "Region")
    {
      return true;
    }
  return false;
}


bool CPV_Interface_IOC::_FindResponsePV(const char *cmd_str, int *pv_num)
{
  for (auto n: m_syscmos->pv_data_list)
    {
      if (strcmp(cmd_str, n.command_string) == 0)
	{
	  *pv_num = n.param_num; // XXX Never need to return the Query reference
	  return true;
	}
    }
  *pv_num = -1;
  return false;
}
