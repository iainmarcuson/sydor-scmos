/* syscmos.cpp
 *
 * v 1.0 YF MMPAD to Dataviewer Epics Plugin 22APR2021
 * 
 */

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

#include <dbAccess.h>

#include "sydor_scmos_params.h"

#include "cpv_interface_ioc.h"
#include "syscmos.h"

static const char *driverName = "syscmos";

// YF
//epicsMutex *plock_ControlPortIO; //  = new epicsMutex(__FILE__,__LINE__) ;

#define NUM_SD_PARAMS (&LAST_SD_PARAM - &FIRST_SD_PARAM + 1)

void syscmos::swap4(char *value)
{
  char temp;
  temp = value[0];
  value[0] = value[3];
  value[3] = temp;
  temp = value[1];
  value[1] = value[2];
  value[2] = temp;
}

void syscmos::swap8(char *value)
{
  char temp;
  temp = value[0];
  value[0] = value[7];
  value[7] = temp;
  temp = value[1];
  value[1] = value[6];
  value[6] = temp;
  temp = value[2];
  value[2] = value[5];
  value[5] = temp;
  temp = value[3];
  value[3] = value[4];
  value[4] = temp;
}

epicsInt32 syscmos::stringToInt32(char *str)
{
  epicsInt32 value = *reinterpret_cast<epicsInt32 *>(str);
  if (isBigEndian_)
    swap4((char *)&value);
  return value;
}

long long syscmos::stringToInt64(char *str)
{
  long long value = *reinterpret_cast<long long *>(str);
  if (isBigEndian_)
    swap8((char *)&value);
  return value;
}

epicsFloat32 syscmos::stringToFloat32(char *str)
{
  epicsFloat32 value = *reinterpret_cast<epicsFloat32 *>(str);
  if (isBigEndian_)
    swap4((char *)&value);
  return value;
}

void syscmos::template_replace(std::string &haystack, const std::string &template_pattern, const std::string &new_str)
{
  ssize_t template_pos = 0;		// XXX Not worrying about repeated substitutions
  while ((template_pos = haystack.find(template_pattern)) != std::string::npos)
    {
      ///
      printf("Found match %s at position %i\n", template_pattern.c_str(), (int) template_pos);
      haystack.replace(template_pos, template_pattern.size(), new_str);
    }
  ///
  printf("Last match at position %i.\n", template_pos);
  return;
}

#if 0
// THIS NEEDS CLEANING UP! Leftover baggage from Mythen - TODO:

/** Sends a command to the detector and reads the response.**/
asynStatus syscmos::sendCommand()
{

  asynStatus status = asynSuccess;

  const char *functionName = "sendCommand";
  int aux;

  status = writeReadMeter(); // TODO: verboten.
  aux = stringToInt32(this->inString_);
  //check for errors
  if (aux < 0)
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
              "%s:%s: error, expected 0, received %d\n",
              driverName, functionName, aux);
  }

  return status;
}

#endif
/** Send a string to the detector and reads the response.**/
asynStatus syscmos::writeReadMeter()
{

  size_t nread;
  size_t nwrite;
  asynStatus status = asynSuccess;

  int eomReason;
  const char *functionName = __func__;

#if 0
    status = pasynOctetSyncIO->writeRead(pasynUserMeter_, outString_, sizeof(outString_),
					 inString_, sizeof(inString_), M1K_TIMEOUT, &nwrite, &nread, &eomReason);
    
    if(status != asynSuccess)
          asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
              "%s:%s: error!\n",driverName, functionName);
    ///DEBUGGING
    if(status != asynSuccess)
      {
	printf("writeReadMeter status: %i\n", status);
	fflush(stdout);
      }

#else
  asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
            "%s:%s: deprecated. Do not call! \n", driverName, functionName);
#endif
  return status;
}


#if 0

/** Starts and stops the acquisition. **/
asynStatus syscmos::setAcquire(epicsInt32 value)
{
  size_t nread;
  size_t nwrite;
  asynStatus status = asynSuccess;
  epicsInt32 eomReason;
  static const char *functionName = "setAcquire";
  // printf("setAcquire %d\n",value);
  if (value == 0)
  {
    //        while (1) {
    //            // Repeat sending STOP until we get only a 0 back
    //            status = pasynOctetSyncIO->setInputEos(pasynUserMeter_, "", 0);
    //            if (status) break;
    //status = pasynOctetSyncIO->writeRead(pasynUserMeter_, "-stop", strlen(outString_),
    //                 inString_, sizeof(epicsInt32), M1K_TIMEOUT, &nwrite, &nread, &eomReason);
    //      setIntegerParam(ADStatus, getStatus());
    //            if (status == asynSuccess || status == asynTimeout) break;
    //        }
    //acquiring_ = 0;
  }
  else
  {
    if (!(acquiring_))
    {
      printf("Sending start command.\n");
      strcpy(outString_, "start");
      this->sendCommand();
      // Notify the read thread that acquisition has started
      epicsEventSignal(startEventId_);

      acquiring_ = 1;
    }
  }
  if (status)
  {
    acquiring_ = 0;
  }
  if (status != asynSuccess)
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
              "%s:%s: error!\n", driverName, functionName);
  return status;
}


/** Sets the number of frames
 * with in an acquisition**/
asynStatus syscmos::setFrames(epicsInt32 value)
{
  
  asynStatus status;

  epicsInt32 imageMode;
  getIntegerParam(ADImageMode, &imageMode);
  if (imageMode == 0) //checks if it is in single acquire mode
  {
    epicsSnprintf(outString_, sizeof(outString_), "-frames %d", 1);
  }
  else
  {
    epicsSnprintf(outString_, sizeof(outString_), "-frames %d", value);
  }
  status = sendCommand();

  // Save frame count for acquition
  frames_ = value;

  return status;
}

/** Sets the Trigger Mode
 *      0 = None, 1 = Single, 2=Continuous */
asynStatus syscmos::setTrigger(epicsInt32 value)
{
  asynStatus status = asynSuccess;
  ;

  if (value == 0)
  {
    // Clearing trigen clears conttrig also
    epicsSnprintf(outString_, sizeof(outString_), "-trigen 0");
    status = sendCommand();
  }
  else if (value == 1)
  {
    epicsSnprintf(outString_, sizeof(outString_), "-trigen 1");
    status = sendCommand();
  }
  else if (value == 2)
  {
    epicsSnprintf(outString_, sizeof(outString_), "-conttrigen 1");
    status = sendCommand();
  }

  return status;
}

/** Sets the dead time constant for the rate correction**/
asynStatus syscmos::setTau(epicsFloat64 value)
{
  asynStatus status;

  if (value == -1 || value > 0)
  {
    epicsSnprintf(outString_, sizeof(outString_), "-tau %f", value);
    status = sendCommand();
  }
  else
  {
    setDoubleParam(SDTau, 0);
    callParamCallbacks();

    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
              "error check if tau -1 or >0 (outString = %s)", outString_);
    return asynError;
  }

  return status;
}

/** Sets the energy threshold for the module **/
asynStatus syscmos::setKthresh(epicsFloat64 value)
{
  epicsInt32 i;
  asynStatus status = asynSuccess;

  for (i = 0; i < this->nmodules; i++)
  {
    epicsSnprintf(outString_, sizeof(outString_), "-module %d", i);
    status = sendCommand();
    epicsSnprintf(outString_, sizeof(outString_), "-kthresh %f", value);
    status = sendCommand();
  }
  if (status == asynSuccess)
  {
    setDoubleParam(SDThreshold, value);
    callParamCallbacks();
  }
  else
  {
    setDoubleParam(SDThreshold, 0);
    callParamCallbacks();

    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
              "error check if value > 0 (outString = %s)", outString_);
    return asynError;
  }

  return status;
}

/** Sets the energy threshold for the module **/
asynStatus syscmos::setEnergy(epicsFloat64 value)
{

  epicsInt32 i;
  asynStatus status = asynSuccess;

  if ((int)firmwareVersion_[1] % 48 >= 3)
  {
    for (i = 0; i < this->nmodules; i++)
    {
      epicsSnprintf(outString_, sizeof(outString_), "-module %d", i);
      status = sendCommand();
      epicsSnprintf(outString_, sizeof(outString_), "-energy %f", value);
      status = sendCommand();
    }
    if (status != asynSuccess)
    {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
                "error check if value > 0 (outString = %s)", outString_);
      return asynError;
    }
  }

  return status;
}

/** Sets the exposure time of one frame. (units of 100ns) **/
asynStatus syscmos::setExposureTime(epicsFloat64 value)
{
  asynStatus status;

  int hns = (int)(value * (1E+7));
  epicsSnprintf(outString_, sizeof(outString_), "-time %d", hns);
  status = sendCommand();

  return status;
}

/** Sets the exposure time of one frame. (units of 100ns) **/
asynStatus syscmos::setDelayAfterTrigger(epicsFloat64 value)
{

  asynStatus status;
  int hns = (int)(value * (1E+7));
  epicsSnprintf(outString_, sizeof(outString_), "-delafter %d", hns);
  status = sendCommand();

  return status;
}


/** Enables or disables the flatfield correction. 
    After initialisation the flatfield correction is enabled. **/
asynStatus syscmos::setFCorrection(epicsInt32 value)
{
  asynStatus status;

  epicsSnprintf(outString_, sizeof(outString_), "-flatfieldcorrection %d", value);
  status = sendCommand();
  return status;
}

/** Enables or disables the bad channel interpolation **/
asynStatus syscmos::setBadChanIntrpl(epicsInt32 value)
{
  asynStatus status;

  epicsSnprintf(outString_, sizeof(outString_), "-badchannelinterpolation %d", value);
  status = sendCommand();
  return status;
}

/** Enables or disables the rate correction. 
    After initialisation the rate correction is disabled. **/
asynStatus syscmos::setRCorrection(epicsInt32 value)
{
  asynStatus status;

  epicsSnprintf(outString_, sizeof(outString_), "-ratecorrection %d", value);
  status = sendCommand();
  return status;
}

/** Enables or disables the gates. 
    After initialisation the gates are disabled. **/
asynStatus syscmos::setUseGates(epicsInt32 value)
{
  asynStatus status;

  epicsSnprintf(outString_, sizeof(outString_), "-gateen %d", value);
  status = sendCommand();
  return status;
}

/** Number of gates. **/
asynStatus syscmos::setNumGates(epicsInt32 value)
{
  asynStatus status;

  epicsSnprintf(outString_, sizeof(outString_), "-gates %d", value);
  status = sendCommand();
  return status;
}

#endif




/** Get Firmware Version **/
asynStatus syscmos::getFirmware()
{
  asynStatus status = asynSuccess;

  // epicsSnprintf(outString_, sizeof(outString_), "-get version");
  // ///writeReadMeter();
  // //epicsSnprintf(inString_, sizeof(inString_), "");

  return status;
}

#if 0
/** Get Acquition Status */
epicsInt32 syscmos::getStatus()
{
  epicsInt32 detStatus;
  int aux;

  strcpy(outString_, "-get status");
  ///writeReadMeter();
  aux = stringToInt32(this->inString_);
  int m_status = aux & 1;         // Acquire running status (non-zero)
  int t_status = aux & (1 << 3);  // Waiting for trigger (non-zero)
  int d_status = aux & (1 << 16); // No Data Available when not zero
  int triggerWaitCnt = 0;
  double triggerWait;

  //printf("Mythen Status - M:%d\tT:%d\tD:%d\n",m_status,t_status, d_status);

  if (m_status || !d_status)
  {
    detStatus = ADStatusAcquire;

    triggerWaitCnt = 0;
    //Waits for Trigger for increaseing amount of time for a total of almost 1 minute
    while ((t_status) && (triggerWaitCnt < MAX_TRIGGER_TIMEOUT_COUNT))
    {
      triggerWait = 0.0001 * pow(10.0, ((double)(triggerWaitCnt / 10) + 1));
      //Wait
      epicsThreadSleep(triggerWait);
      //Check again
      strcpy(outString_, "-get status");
      ///writeReadMeter();
      aux = stringToInt32(this->inString_);
      t_status = aux & (1 << 3);
      d_status = aux & (1 << 16);
      triggerWaitCnt++;
    }

    if (!d_status)
      detStatus = ADStatusReadout;
    if (triggerWaitCnt == MAX_TRIGGER_TIMEOUT_COUNT)
      detStatus = ADStatusError;
  }
  else
    detStatus = ADStatusIdle;

  return detStatus;
}


/**Enables or disables the flipping of the channel numbering. **/
asynStatus syscmos::setFlip(epicsInt32 value)
{
  asynStatus status;

  epicsSnprintf(outString_, sizeof(outString_), "-flipchannels %d", value);
  status = sendCommand();
  return status;
}

/**Enables or disables the flipping of the channel numbering. **/
asynStatus syscmos::setBitDepth(epicsInt32 value)
{
  asynStatus status;
  epicsInt32 nbits;
  switch (value)
  {
  case 0:
    nbits = 24;
    break;
  case 1:
    nbits = 16;
    break;
  case 2:
    nbits = 8;
    break;
  case 3:
    nbits = 4;
    break;
  default:
    nbits = 24;
    break;
  }

  epicsSnprintf(outString_, sizeof(outString_), "-nbits %d", nbits);
  status = sendCommand();
  return status;
}


/**Loads predefined settings for the current module to measure
some common x-ray radiation. The command loads the energy calibration,
the bad channels file, the flatfield correction, and the trimbits
for the corresponding settings, and sets the energy threshold to a
suitable value.**/
asynStatus syscmos::loadSettings(epicsInt32 value)
{
  asynStatus status = asynSuccess;
  epicsInt32 i = 0;

  for (i = 0; i < this->nmodules; i++)
  {
    epicsSnprintf(outString_, sizeof(outString_), "-module %d", i);
    status = sendCommand();

    switch (value)
    {
    //Cu
    case 0:
      if ((int)firmwareVersion_[1] % 48 >= 3)
      {
        epicsSnprintf(outString_, sizeof(outString_), "-settings Cu");
      }
      else
      {
        epicsSnprintf(outString_, sizeof(outString_), "-settings StdCu");
      }
      break;

    //Mo
    case 1:
      if ((int)firmwareVersion_[1] % 48 >= 3)
      {
        epicsSnprintf(outString_, sizeof(outString_), "-settings Mo");
      }
      else
      {
        epicsSnprintf(outString_, sizeof(outString_), "-settings StdMo");
      }
      break;

    //Ag
    //Not supported on firmware verions less than 3
    case 2:
      epicsSnprintf(outString_, sizeof(outString_), "-settings Ag");
      break;

    // Cr
    case 3:
      if ((int)firmwareVersion_[1] % 48 >= 3)
      {
        epicsSnprintf(outString_, sizeof(outString_), "-settings Cr");
      }
      else
      {
        epicsSnprintf(outString_, sizeof(outString_), "-settings HgCr");
      }
      break;

    default:
      epicsSnprintf(outString_, sizeof(outString_), "-reset");
      break;
    }
    status = sendCommand();
  }
  setIntegerParam(SDSetting, value);

  callParamCallbacks();

  return status;
}



/**Sets the detector back to default settings. This command takes
about two seconds per module.**/
asynStatus syscmos::setReset()
{
  asynStatus status = asynSuccess;
  epicsInt32 i = 0;

  setIntegerParam(SDReset, 1);
  for (i = 0; i < this->nmodules; i++)
  {
    epicsSnprintf(outString_, sizeof(outString_), "-module %d", i);
    status = sendCommand();

    strcpy(outString_, "-reset");
    status = sendCommand();
  }
  setIntegerParam(SDReset, 0);
  callParamCallbacks();
  return status;
}

/** Reads the values of all the modules parameters, sets them in the parameter library**/
asynStatus syscmos::getSettings()
{
  int aux = 0;
  epicsFloat32 faux;
  long long laux;
  epicsFloat64 DetTime;

  static const char *functionName = "getSettings";

  if (acquiring_)
  {
    strcpy(outString_, "Called during Acquire");
    inString_[0] = 0;
    goto error;
  }

  strcpy(outString_, "-get flatfieldcorrection");
  ///writeReadMeter();
  ///aux = stringToInt32(this->inString_);
  if (aux != 0 && aux != 1)
    goto error;
  setIntegerParam(SDUseFlatField, aux);

  strcpy(outString_, "-get badchannelinterpolation");
  ///writeReadMeter();
  aux = stringToInt32(this->inString_);
  if (aux != 0 && aux != 1)
    goto error;
  setIntegerParam(SDUseBadChanIntrpl, aux);

  strcpy(outString_, "-get ratecorrection");
  ///writeReadMeter();
  aux = stringToInt32(this->inString_);
  if (aux != 0 && aux != 1)
    goto error;
  setIntegerParam(SDUseCountRate, aux);

  strcpy(outString_, "-get nbits");
  // YF what is this writeReadMeter();
  aux = stringToInt32(this->inString_);
  if (aux < 0)
    goto error;
  nbits_ = aux;
  chanperline_ = 32 / aux;
  setIntegerParam(SDBitDepth, aux);

  strcpy(outString_, "-get time");
  ///writeReadMeter();
  aux = stringToInt32(this->inString_);
  if (aux >= 0)
    DetTime = (aux * (1E-7));
  else
    goto error;
  setDoubleParam(ADAcquireTime, DetTime);

  strcpy(outString_, "-get frames");
  ///writeReadMeter();
  aux = stringToInt32(this->inString_);
  if (aux >= 0)
    setIntegerParam(SDNumFrames, aux);

  strcpy(outString_, "-get tau");
  ///writeReadMeter();
  //faux = stringToFloat32(this->inString_);
  faux = 0;
  if (faux == -1 || faux > 0)
    setDoubleParam(SDTau, faux);
  else
    goto error;

  strcpy(outString_, "-get kthresh");
  ///writeReadMeter();
  ///faux = stringToFloat32(this->inString_);
  faux = 0;
  if (faux < 0)
    goto error;
  else
    setDoubleParam(SDThreshold, faux);

  //Firmware greater than 3 commands
  if ((int)firmwareVersion_[1] % 48 >= 3)
  {
    strcpy(outString_, "-get energy");
    ///writeReadMeter();
    //faux = stringToFloat32(this->inString_);
    faux = 0;
    if (faux < 0)
      goto error;
    else
      setDoubleParam(SDEnergy, faux);

    strcpy(outString_, "-get delafter");
    ///writeReadMeter();
    //laux = stringToInt64(this->inString_);
    laux = 0;
    if (laux >= 0)
      DetTime = (laux * (1E-7));
    else
      goto error;
    setDoubleParam(SDDelayTime, DetTime);

    /* Get trigger modes */
    strcpy(outString_, "-get conttrig");
    ///writeReadMeter();
    aux = stringToInt32(this->inString_);
    if (aux < 0)
      goto error;

    // if (aux == 1)
    //   setIntegerParam(SDTrigger, 2);
    // else
    // {
    //   strcpy(outString_, "-get trig");
    //   ///writeReadMeter();
    //   aux = stringToInt32(this->inString_);
    //   if (aux < 0)
    //     goto error;
    //   // if (aux == 1)
    //   //   setIntegerParam(SDTrigger, 1);
    //   // else
    //   //   setIntegerParam(SDTrigger, 0);
    // }
  }

  callParamCallbacks();
  // if (prevAcquiring) setAcquire(1);

  return asynSuccess;

error:
  asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
            "%s:%s: error, outString=%s, inString=%s\n",
            driverName, functionName, outString_, inString_);
  return asynError;
}


#endif
//static void c_shutdown(void* arg) {
//    syscmos *p = (syscmos*)arg;
//    p->shutdown();
//}

// YF
void controlSocketListenTaskC(void *drvPvt)
{
  syscmos *pPvt = (syscmos *)drvPvt;
  pPvt->controlSocketListenTask();
}

void acquisitionTaskC(void *drvPvt)
{
  syscmos *pPvt = (syscmos *)drvPvt;
  pPvt->acquisitionTask();
}

void initCompanionTaskC(void *drvPvt)
{
  syscmos *pPvt = (syscmos *)drvPvt;
  pPvt->init_query_pv();
}

void pollTaskC(void *drvPvt)
{
  syscmos *pPvt = (syscmos *)drvPvt;
  pPvt->pollTask();
}

void syscmos::shutdown()
{
  //    if (pDetector)
  //        delete pDetector;
}

void syscmos::pollTask()
{
  // int acquire;
  /* Poll detector running status every second*/
  while (1)
  {
    epicsThreadSleep(1);

    /* Update detector status */
    this->lock();
    // int detStatus = pDetector->getDetectorStatus();
    // setIntegerParam(ADStatus, detStatus);
    callParamCallbacks();
    this->unlock();
  }
}

void dobreak()
{
  printf("BREAK!");
}

// fcn proto
//int raise(int sig);

// public
//
// Return codes
//  0
int syscmos::HandleResponse(int nFunction, CPV_Interface_IOC::PRType *prt)
{
  this->lock();

  if (1 == prt->nType)
  {
    printf("F:%s: setIntegerParam (%d , %d);  \n", __func__, nFunction, prt->ival);
    setIntegerParam(nFunction, prt->ival);
  }
  else if (2 == prt->nType)
  {
    printf("F:%s: setDoubleParam (%d , %f); \n", 
        __func__, nFunction, prt->fval);
    setDoubleParam(nFunction, prt->fval );
  }
  else if (3 == prt->nType)
  {
    printf("F:%s: setStringParam (%d , %s); \n", 
        __func__, nFunction, prt->sval);

    setStringParam(nFunction, prt->sval);
  }
  else
  {
    printf("Error. HandleResponse(). ParseResponse broke.\n");
  }

  // int detStatus = pDetector->getDetectorStatus();
  // setIntegerParam(ADStatus, detStatus);
  callParamCallbacks();

  this->unlock();

  return (0);
}

//
//  Manually need to set this up
//
void syscmos::controlSocketListenTask()
{
  static char inBuf[128];
  size_t nread;
  epicsInt32 eomReason;
  asynStatus status = asynSuccess;
  bool bWriteReadBusy;

  memset(inBuf, 0, sizeof(inBuf));

  while (1)
  {
    status = pasynOctetSyncIO->read(pasynUserMeter_, (char *)inBuf, sizeof(inBuf), .5, &nread, &eomReason);

    if (0 == status)
    {
      printf("Received cSLT:%s, status:%i, eomReason:%i, nread:%d \n", inBuf, status, eomReason, nread);

      // TODO parse text and set correct PV to correct value
      if (nread > 0)
      {
        int nFunction;
        CPV_Interface_IOC::PRType prt;

        int ret = m_cpv_interface->ParseResponse(inBuf, &nFunction, &prt);

        
        if (0 == ret)
        {
          HandleResponse(nFunction, &prt);
          epicsEventSignal(m_cpv_interface->cmdEventId);
          // It appears this does not work - IDK why.

        }
        else
        {
            printf("   ParseResponse() with error %d\n", ret);

        }

        // clear out
        memset(inBuf, 0, sizeof(inBuf));

      }
    }
   
  }  // while 
}

void syscmos::acquisitionTask()
{
  size_t nread, nread_expect;
  size_t nwrite;
  int eventStatus = 0;
  int imageMode;
  epicsInt32 acquire, eomReason;
  double acquireTime;
  asynStatus status = asynSuccess;
  int dataOK;
  struct _structImageFrame frame_data; // Information about a frame
  struct _structDoc doc_data;          // Information that is constant among frames
  int pixel_size;                      // Number of bytes in a pixel
  struct timeval start_time, end_time;
  double start_usec, end_usec;
  int img_count;
  int miss_count;
  int miss_count_hdr;
  ///Store images for viewing
  FILE *img_file;

  ///Initialize Image FILE
  img_file = fopen("raw_img.dat", "w"); // YF WTF is this?

  static const char *functionName = "acquisitionTask";
  this->lock();

  while (1)
  {
    /* Is acquisition active? */
    getIntegerParam(ADAcquire, &acquire);

    /* If we are not acquiring then wait for a semaphore that is given when acquisition is started */
    if (!acquire || !acquiring_)
    {
      setIntegerParam(ADStatus, ADStatusIdle);
      callParamCallbacks();
      /* Release the lock while we wait for an event that says acquire has started, then lock again */
      asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
                "%s:%s: waiting for acquire to start\n", driverName, functionName);
      this->unlock();
      eventStatus = epicsEventWait(this->startEventId_);

      ///
      printf("EPICS Acquisition Task Released.\n");
      fflush(stdout);
      
      nread_expect = sizeof(doc_data);

      dataOK = 1;

      gettimeofday(&start_time, NULL);
      img_count = 0;
      miss_count = 0;
      miss_count_hdr = 0;
      strcpy(dataOutString_, "read");
      do
      {
      acq_loop_head_:
	(void) nread;
        nread = 0;
        nread_expect = sizeof(doc_data);
        ///Point to the correct socket+
        ///printf("Starting new acq cycle.\n");
        ///status = pasynOctetSyncIO->writeRead(pasynDataMeter_, dataOutString_, strlen(dataOutString_), (char *)&doc_data,
        ///				     sizeof(doc_data), 600.5, &nwrite, &nread, &eomReason);  //Timeout is 1.5 seconds, appropriate for 1 second send interval
        status = pasynOctetSyncIO->read(pasynDataMeter_, (char *)&doc_data, sizeof(doc_data), 600.5, &nread, &eomReason); ///Reduced timeout from 600.5 to 10.5
        printf("Received %li of %li bytes for doc_data.\n", nread, nread_expect);
        fflush(stdout);
        ///TODO May want to do proper timeout checking
        if (nread != nread_expect) // Failed to get proper header bytes
        {
          miss_count_hdr++;
	  goto acq_loop_head_; /// Really try again; delete below line if this works
          continue; // Try again
        }

        // Now get the frame data
        nread_expect = sizeof(frame_data);
        ///status = pasynOctetSyncIO->writeRead(pasynDataMeter_, dataOutString_, strlen(dataOutString_), (char *)&frame_data,
        ///				     sizeof(frame_data), 600.5, &nwrite, &nread, &eomReason);  //Timeout is 1.5 seconds, appropriate for 1 second send interval

        status = pasynOctetSyncIO->read(pasynDataMeter_, (char *)&frame_data, sizeof(frame_data), 10.5, &nread, &eomReason); ///Reduced timeout from 600.5 to 10.5
        ///
        printf("Received %li of %li bytes for frame_data.\n", nread, nread_expect);
        printf("Image Type is %i.\n", doc_data.enImageType);
        fflush(stdout);
        // XXX TODO Handle case of error in frame data

        // TODO Handle exotic types such of 17- or 33-bit
        if ((doc_data.enImageType >= 1) && (doc_data.enImageType <= 2))
        {
          pixel_size = 2;
        }
        else if ((doc_data.enImageType >= 5) && (doc_data.enImageType <= 7))
        {
          pixel_size = 4;
        }
        else if (doc_data.enImageType == 8)
        {
          pixel_size = 8;
        }

        nread_expect = pixel_size * doc_data.width * doc_data.height;

        ///
        printf("Expecting image size %ix%i and %li bytes.\n", doc_data.width, doc_data.height, nread_expect);
        fflush(stdout);
        //Assign pointers to point to data and metadata
        doc_data.pFrame = &frame_data;
        frame_data.image = (char *)detArray_;

        status = pasynOctetSyncIO->read(pasynDataMeter_, (char *)detArray_, nread_expect, 4.5, &nread, &eomReason); ///Reduced timeout from 600.5 to 10.5
        printf("Data: Received %li of %li bytes.\n", nread, nread_expect);
        printf("First bytes: %c%c%c%c\n", detArray_[0], detArray_[1], detArray_[2], detArray_[3]);
        printf("Data: eomReason: %i\tstatus: %i\n", eomReason, status);
        fflush(stdout);
        if (nread == nread_expect)
        {
          ///printf("Received correct amount of image data.\n");
          ///fflush(stdout);
          ///Store image in file

          if (img_file)
          {
            fwrite(detArray_, 512 * 512, sizeof(float), img_file);
          }
          img_count++;
          /*
            if ((img_count % 25)==0)
              {
          printf("Received %i images successfully\n", img_count);
          fflush(stdout);
              }
            
            if (img_count == 500)
              {
          gettimeofday(&end_time, NULL);
          start_usec = start_time.tv_sec*1000000+start_time.tv_usec;
          end_usec = end_time.tv_sec*1000000+end_time.tv_usec;
          printf("Elapsed time for 500 images: %f\n", (end_usec-start_usec)/1000000);
          fflush(stdout);
          fclose(img_file);
          img_file = NULL;
              }
		    */
          this->lock();
          dataOK = dataCallback(detArray_, &doc_data, nread_expect);
          this->unlock();
          ///printf("Ran data callback.\n");
          if (!dataOK)
          {
            //eventStatus = getStatus();
            setIntegerParam(ADStatus, eventStatus);
            printf("Data callback not okay.\n");
          }
          ///printf("Value 0 is %i.\n", detArray_[0]);
          ///fflush(stdout);
        }
        else
        {
          miss_count++;
          //eventStatus = getStatus();
          setIntegerParam(ADStatus, eventStatus);
          printf("Data not size expected: count %i\nADStatus: %d\n", miss_count, eventStatus);
	  printf("About to restart acq loop.\n");
	  fflush(stdout);
	  ///DEBUGGING
	  pasynOctetSyncIO->flush(pasynDataMeter_);
	  status = asynSuccess; ///XXX Force to be correct for loop again, instead of goto
	  ///XXX
	  goto acq_loop_head_;
	  continue;
        }
        if (status != asynSuccess)
        {
          asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
                    "%s:%s: error using readout command status=%d, nRead=%d, eomReason=%d\n",
                    driverName, functionName, status, (int)nread, eomReason);
        }
        printf("End of acq do while.\n");
        fflush(stdout);
      } while (status == asynSuccess && /*(eventStatus==ADStatusAcquire||eventStatus==ADStatusReadout) && */ acquiring_);
      printf("Left acq do while.\n");
      printf("Image count: %i\tMiss count: %i\n", img_count, miss_count);
      printf("Header misses: %i\n", miss_count_hdr);
      if (img_file)
      {
        fflush(img_file);
        fclose(img_file);
      }
      fflush(stdout);
    }
    this->lock();

    /*
	if (eventStatus!=ADStatusError ) {
          printf("Acquisition finish\n");
          getIntegerParam(ADImageMode, &imageMode);
          if (imageMode == ADImageSingle || imageMode == ADImageMultiple) {
            printf("ADAcquire Callback\n");
            acquiring_ = 0;
            setIntegerParam(ADAcquire,  0); 
            callParamCallbacks(); 
          }
        }
        else {
          //Abort read 
          asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s: error timed out waiting for data\n",
                driverName, functionName);
          acquiring_ = 0;
            setAcquire(0);
          setIntegerParam(ADAcquire,  0); 
          callParamCallbacks(); 
        }
	*/
  }
}

epicsInt32 syscmos::dataCallback(epicsInt32 *pData, SydorDocStruct *doc_data, int image_bytes)
{
  NDArray *pImage;
  int ndims = 2;
  size_t dims[2];
  int totalBytes;
  int arrayCallbacks;
  int imageCounter;
  epicsTimeStamp timeStamp;
  epicsInt32 colorMode = NDColorModeMono;
  epicsInt32 callback_datatype;

  ///
  int i, j;
  int pixel_size;
  int width, height;
  const int FORCE_FLOAT = 1;
  const int CLIP_16 = 1; // Used to set for 16 bit output

  // printf ("pData[0] = %X\n",pData[0]);

  if (pData == NULL)
    return (0);

  ///dims[0] = MAX_DIMS;
  ///dims[1] = 1;
  dims[0] = doc_data->width;
  dims[1] = doc_data->height;
  ///XXX
  totalBytes = doc_data->width * doc_data->height * 4; // Always for 32-bit pixels
  if (FORCE_FLOAT)
  {
    //nop
  }
  else if (CLIP_16)
  {
    totalBytes = totalBytes / 2;
  }

  width = doc_data->width;
  height = doc_data->height;

  getIntegerParam(NDDataType, &callback_datatype);
  printf("Callback data type: %i\n", callback_datatype);

  printf("Callback image size %iWx%iH with %i bytes.\n", width, height, totalBytes);
  fflush(stdout);

  /* get the current time */
  epicsTimeGetCurrent(&timeStamp);

  /* Allocate a new image buffer */
  // The image buffer will always be for 32-bit data; either the short ints are cast
  // I will have to distinguish between signed and unsigned
  if ((doc_data->enImageType == kINT16) || (doc_data->enImageType == kINT32)) // Signed int
  {

    if (FORCE_FLOAT)
    {
      pImage = this->pNDArrayPool->alloc(ndims, dims, NDFloat32, totalBytes, NULL);
      printf("Assigneeeed float   in int.\n");
    }
    else if (CLIP_16)
    {
      pImage = this->pNDArrayPool->alloc(ndims, dims, NDInt16, totalBytes, NULL);
      printf("Assigned short signed in alloc.\n");
    }
    else
    {
      pImage = this->pNDArrayPool->alloc(ndims, dims, NDInt32, totalBytes, NULL);
    }

    ///

    for (i = 0; i < (width * height); i++)
    {
      ///((int *)pImage->pData)[i] = 0xaaaaaaaa;
    }
  }
  else if ((doc_data->enImageType == kUINT16) || (doc_data->enImageType == kUINT32)) // Unsigned Int
  {
    if (FORCE_FLOAT)
    {
      pImage = this->pNDArrayPool->alloc(ndims, dims, NDFloat32, totalBytes, NULL);
    }
    else if (CLIP_16)
    {
      pImage = this->pNDArrayPool->alloc(ndims, dims, NDUInt16, totalBytes, NULL);
      printf("Assigned short unsigned in alloc.\n");
    }
    else
    {
      pImage = this->pNDArrayPool->alloc(ndims, dims, NDUInt32, totalBytes, NULL);
    }
    ///pImage = this->pNDArrayPool->alloc(ndims, dims, NDFloat32, totalBytes, NULL);
    ///
    for (i = 0; i < (width * height); i++)
    {
      ///((unsigned int *)pImage->pData)[i] = 0x11111111;
    }
  }
  ///XXX TODO Handle more exotic types
  else // Float
  {
    pImage = this->pNDArrayPool->alloc(ndims, dims, NDFloat32, totalBytes, NULL);

    ///
    for (i = 0; i < (width * height); i++)
    {
      ((float *)pImage->pData)[i] = 567.8;
    }
    printf("Assigned float in alloc.\n");
  }

  fflush(stdout);
  ///
  ///XXX memset(pImage->pData, 0xaa, totalBytes);

  // Now copy the data.  The data are cast.

  // This could probably be made faster with loops on the inside, but I am lazy
  // Or perhaps a function pointer
  for (i = 0; i < (doc_data->height * doc_data->width); i++)
  {

    if (doc_data->enImageType == kINT16) // Signed short pixel data
    {
      if (FORCE_FLOAT)
      {
        ((float *)pImage->pData)[i] = ((short int *)pData)[i];
        pImage->dataType = NDFloat32;
      }
      else if (CLIP_16)
      {
        ((short int *)pImage->pData)[i] = ((short int *)pData)[i];
        pImage->dataType = NDInt16;
      }
      else
      {
        ((int *)pImage->pData)[i] = ((short int *)pData)[i];
        pImage->dataType = NDInt32;
      }
    }
    else if (doc_data->enImageType == kINT32) // Signed int
    {
      if (FORCE_FLOAT)
      {
        ((float *)pImage->pData)[i] = ((int *)pData)[i];
        pImage->dataType = NDFloat32;
      }
      else if (CLIP_16)
      {
        ((short int *)pImage->pData)[i] = (((int *)pData)[i]) / 65536;
        pImage->dataType = NDInt16;
      }
      else
      {
        ((int *)pImage->pData)[i] = ((int *)pData)[i];
        pImage->dataType = NDFloat32;
      }
    }
    else if (doc_data->enImageType == kUINT16) // Unsigned short
    {
      if (FORCE_FLOAT)
      {
        ((float *)pImage->pData)[i] = ((unsigned short int *)pData)[i];
        pImage->dataType = NDFloat32;
      }
      else if (CLIP_16)
      {
        ((unsigned short int *)pImage->pData)[i] = ((unsigned short int *)pData)[i];
        pImage->dataType = NDUInt16;
      }
      else
      {
        ((unsigned int *)pImage->pData)[i] = ((unsigned short int *)pData)[i];
        pImage->dataType = NDUInt32;
      }
    }
    else if (doc_data->enImageType == kUINT32) // Unsigned int
    {
      if (FORCE_FLOAT)
      {
        ((float *)pImage->pData)[i] = (float)(((unsigned int *)pData)[i]);
        pImage->dataType = NDFloat32;
        if (i == 0)
        {
          printf("Forcing float for UINT32.\n");
        }
        if ((i % (512 * 32)) == 0)
        {
          printf("Row %i first pixel is %u\t%f.\n", i / 512, ((unsigned int *)pData)[i], (double)(((float *)pImage->pData)[i]));
        }
      }
      else if (CLIP_16)
      {
        ((unsigned short int *)pImage->pData)[i] = (((unsigned int *)pData)[i]) / 65536;
        pImage->dataType = NDInt16;
      }
      else
      {
        ((unsigned int *)pImage->pData)[i] = ((unsigned int *)pData)[i];
        pImage->dataType = NDUInt32;
      }
    }
    else // XXX Probably Float, but there are other exotic types
    {
      ///FIXME Incompatible? with CLIP_16
      ((float *)pImage->pData)[i] = ((float *)pData)[i];
      pImage->dataType = NDFloat32;
    }
  }

  ///
  if (0)
  {
    for (i = 0; i < (width * height); i++)
    {
      ((float *)pImage->pData)[i] = i;
    }
    printf("Assigned %i pixels.\n", i);
  }

  pImage->ndims = ndims;
  pImage->dims[0].size = dims[0];
  pImage->dims[0].offset = 0;
  pImage->dims[0].binning = 1;
  pImage->dims[1].size = dims[1];
  pImage->dims[1].offset = 0;
  pImage->dims[1].binning = 1;

  pImage->pAttributeList->add("ColorMode", "Color Mode", NDAttrInt32, &colorMode);

  /* Increase image counter */
  getIntegerParam(NDArrayCounter, &imageCounter);
  imageCounter++;
  setIntegerParam(NDArrayCounter, imageCounter);

  /* Set the uniqueId and time stamp */
  pImage->uniqueId = imageCounter;
  pImage->timeStamp = timeStamp.secPastEpoch + timeStamp.nsec / 1e9;

  /* Get any attributes that have been defined for this driver */
  this->getAttributes(pImage->pAttributeList);

  getIntegerParam(NDArrayCallbacks, &arrayCallbacks);
  if (arrayCallbacks)
  {
    /* Call the NDArray callback */
    doCallbacksGenericPointer(pImage, NDArrayData, 0);
  }

  /* We save the most recent good image buffer so it can be used in the
     * readADImage function.  Now release it. */
  if (this->pArrays[0])
    this->pArrays[0]->release();
  this->pArrays[0] = pImage;

  /* Update any changed parameters */
  callParamCallbacks();

  return (1);
}

#if 0
// Input
// nmods: number of active modules
// nbits: number of bits, which were read out
// data: response of the -readoutraw command
//
// Output
// result: array of size 1280*nmods with the number of counts of all // channels
void syscmos::decodeRawReadout(int nmods, int nbits, int *data, int *result)
{
  // default for 24 bits
  int chanperline = 1; // default for 24 bits
  int mask = 0xffffff; // default for 24 bits
  if (nbits == 16)
  {
    chanperline = 2;
    mask = 0xffff;
  }
  if (nbits == 8)
  {
    chanperline = 4;
    mask = 0xff;
  }
  if (nbits == 4)
  {
    chanperline = 8;
    mask = 0xf;
  }

  int size = 1280 / chanperline * nmods;
  memcpy(tmpArray_, data, size * sizeof(int)); // unsigned int32
  for (int j = 0; j < chanperline; j++)
  {
    int shift = nbits * j;
    int shiftedMask = mask << shift;
    for (int i = 0; i < size; i++)
    {
      result[i * chanperline + j] = ((tmpArray_[i] & shiftedMask) >> shift) & mask;
    }
  }
}

#endif



/** Called when asyn clients call pasynOctet->write().
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. 
  * \param[in] nChars Number of characters to write 
  * \param[out] nActual Number of characters actually written */
asynStatus syscmos::writeOctet(asynUser *pasynUser, const char *value,
                              size_t nChars, size_t *nActual)
{
  int function = pasynUser->reason;
  int status = asynSuccess;
  const char *functionName = "writeOctet";

  const char *pvName = NULL;
  getParamName(0 /*int list*/, function, &pvName);
  int ret = m_cpv_interface->SetPV(pvName, value);

  if (0 == ret)
  {
    // was handled by interface - nothing to do here.
  }

  else if (-1 == ret)
  {
    /* Reject any call to the detector if it is running */
    //    if (acquiring_) {
    //        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
    //        "%s:%s: detector is busy\n", driverName, functionName);
    //        return asynError;
    //    }

    /* Set the parameter in the parameter library. */
    status |= (asynStatus)setStringParam(function, (char *)value);

    // YF - seems like a good place to create a query command
    if (function == SDCommandOut)
    {
      printf("DEBUG ! hello From writeOctet()\n"); // TODO  -
    }

    /* If this is not a parameter we have handled call the base class */
    if (function < FIRST_SD_PARAM)
      status = ADDriver::writeOctet(pasynUser, value, nChars, nActual);

    /* Update any changed parameters */
    callParamCallbacks();

    if (status)
      asynPrint(pasynUser, ASYN_TRACE_ERROR,
                "%s:%s: error, status=%d function=%d, value=%s\n",
                driverName, functionName, status, function, value);
    else
      asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
                "%s:%s: function=%d, value=%s\n",
                driverName, functionName, function, value);
  }
  else
  {
    status = ret;
  }

  printf("functionID = %d, writeOctet() returned:%d\n", function, status); // DEBUG
   *nActual = nChars;
  return ((asynStatus)status);
}

/** Called when asyn clients call pasynInt32->write().
  * This function performs actions for some parameters, including ADAcquire, ADBinX, etc.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus syscmos::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int function = pasynUser->reason;
  int status = asynSuccess;
  static const char *functionName = "writeInt32";
  bool doSend = false;

  // TODO FIXME Will probably need to change this unless we have a way to abort acquisition.
  /* Reject any call to the detector if it is running */
  /// Disable busy check
  /*
  if ((function != ADAcquire) && acquiring_)
  {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
              "%s:%s: detector is busy\n", driverName, functionName);
    return asynError;
  }
  */
  
  const char *pvName = NULL;

  if (function == SDOutMux)
    {
      printf("OutMux called with value %i\n", value);
    }

  getParamName(0 /*int list*/, function, &pvName);
  int ret = m_cpv_interface->SetPV(pvName, value);

  printf("%s: SetPV returned %i.  Magic values are %i and %i.\n",
	 __FUNCTION__, ret, asynDisconnected, asynTimeout);
  if ((asynDisconnected == ret) || (asynTimeout == ret)) //Assume link down
    {
      setIntegerParam(SDEPICSLinkStatus, 0);
    }
  else if (ret != -1) //Assume link up, but don't change status if variable not found
    {
      setIntegerParam(SDEPICSLinkStatus, 1);
    }
  callParamCallbacks();
  
  if (0 == ret)
  {
    // was handled by interface - nothing to do here.
  }
  
  else if (-1 == ret)
  {
    // -1 = Not recognized by SetPV

    // All other (Int32) PV's not known to plugin can be handled here.

    /* Set the parameter and readback in the parameter library.
        * This may be overwritten when we read back the
        * status at the end, but that's OK */
    status |= setIntegerParam(function, value);

    ///Testing
    if ((function == ADAcquire) && value)
      {
	///
	printf("About to start acquiring.\n");
	acquiring_ = 1;
	epicsEventSignal(startEventId_);
      }

    /* If this is not a parameter we have handled call the base class */
    if (function < FIRST_SD_PARAM)
      status = ADDriver::writeInt32(pasynUser, value);

    ///TODO XXX Make this optional via a flag set/reset every time?
    ///TODO Also, how to parse response?
    //status |= getSettings();

    /* Update any changed parameters */
    status |= callParamCallbacks();

    if (status)
      asynPrint(pasynUser, ASYN_TRACE_ERROR,
                "%s:%s: error, status=%d function=%d, value=%d\n",
                driverName, functionName, status, function, value);
    else
      asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
                "%s:%s: function=%d, value=%d\n",
                driverName, functionName, function, value);
  }
  else
  {
    status = ret;
  }

  printf("functionID = %d, writeInt32() returned:%d\n", function, status); // DEBUG
  return ((asynStatus)status);
}

/** Called when asyn clients call pasynFloat64->write().
  * For all  parameters it  sets the value in the parameter library and calls any registered callbacks.
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */

// fcn proto
void *memset(void *ptr, int value, size_t num);

asynStatus syscmos::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
  static char wf64Buffer[128]; // YF

  // probably a good idea to clear out the buffer each time
  memset(wf64Buffer, 0, 128);

  int function = pasynUser->reason;
  int status = asynSuccess;
  int addr = 0;
  const char *functionName = "writeFloat64";
  const char *pvName = NULL;

  getParamName(0 /*int list*/, function, &pvName);
  int ret = m_cpv_interface->SetPV(pvName, value);

  if (0 == ret)
  {
    // was handled by interface - nothing to do here.
  }

  else if (-1 == ret)
  {

    status = getAddress(pasynUser, &addr);
    if (status != asynSuccess)
      return ((asynStatus)status);

    /* Reject any call to the detector if it is running */
    if (acquiring_)
    {
      asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s: detector is busy\n", driverName, functionName);
      return asynError;
    }

    /* Set the parameter in the parameter library. */
    status = (asynStatus)setDoubleParam(addr, function, value);

    /* If this is not a parameter we have handled call the base class */
    if (function < FIRST_SD_PARAM)
      status = ADDriver::writeFloat64(pasynUser, value);
  

    /* Update any changed parameters */
    status |= callParamCallbacks(); //YF Not entirely sure what this is doing - try to set breakpoint here.

    if (status)
      asynPrint(pasynUser, ASYN_TRACE_ERROR,
                "%s:%s: error, status=%d function=%d, value=%g\n",
                driverName, functionName, status, function, value);
    else
      asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
                "%s:%s: function=%d, value=%g\n",
                driverName, functionName, function, value);
  }
  else
  {
    status = ret;
  }
  printf("writeFloat64() returned:%d\n", status); // DEBUG
  return ((asynStatus)status);
}

// Initialize PVs for query
// Should return status of success or failure
int syscmos::init_query_pv()
{
  DBADDR curr_addr;
  long get_options;
  long n_request;
  double acq_time_val;
  long int acq_time_param_num;
  long out_val;
  long param_num;
  std::string pv_name;


  /// Wait for IOC init before manipulating database
  while (!interruptAccept)
    {
      epicsThreadSleep(0.1);
    }

  /*
  dbNameToAddr("dp:cam1:AcquireTime_Q", &curr_addr);
  acq_time_param_num = ADAcquireTime;

  printf("Acquire time param is %li\n", acq_time_param_num);
  fflush(stdout);
  dbPutField(&curr_addr, DBR_LONG, &acq_time_param_num, 1); // Put the parameter number in the VAL field for later processing.
  get_options = 0;
  n_request = 1;
  out_val = 1234;
  dbGetField(&curr_addr, DBR_LONG, &out_val, &get_options, &n_request, NULL);
  printf("Retrieve companion value is %li.\n", out_val);

  */

#include "query_code.c"
  
  dbNameToAddr("dp:cam1:OutMux.DISA", &curr_addr);
  out_val = 0;
  dbPutField(&curr_addr, DBR_LONG, &out_val, 1); // Now enable processing

  return 0;			// We are done
}
  


/** Report status of the driver.
  * Prints details about the driver if details>0.
  * It then calls the ADDriver::report() method.
  * \param[in] fp File pointed passed by caller where the output is written to.
  * \param[in] details If >0 then driver details are printed.
  */
void syscmos::report(FILE *fp, int details)
{
  fprintf(fp, "syscmos %s\n", this->portName);
  if (details > 0)
  {
    int nx, ny, dataType;
    getIntegerParam(ADSizeX, &nx);
    getIntegerParam(ADSizeY, &ny);
    getIntegerParam(NDDataType, &dataType);
    fprintf(fp, "  NX, NY:            %d  %d\n", nx, ny);
    fprintf(fp, "  Data type:         %d\n", dataType);
  }
  /* Invoke the base class method */
  ADDriver::report(fp, details);
}

extern "C" int syscmosConfig(const char *portName, const char *CtrlPortName,
                            const char *DataPortName,
			    const char *prefix_name,
			    const char *record_name,
                            int maxBuffers, size_t maxMemory,
                            int priority, int stackSize)
{
  new syscmos(portName, CtrlPortName, DataPortName,
	     prefix_name, record_name,
             maxBuffers, maxMemory, priority, stackSize);
  return (asynSuccess);
}

/** Constructor for syscmos driver; most parameters are simply passed to ADDriver::ADDriver.
  * After calling the base class constructor this method creates a thread to collect the detector data, 
  * and sets reasonable default values for the parameters defined in this class, asynNDArrayDriver, and ADDriver.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] IPPortName The asyn network port connection to the Mythen
  * \param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is 
  *            allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
  * \param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is 
  *            allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
  * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  */
syscmos::syscmos(const char *portName, const char *CtrlPortName,
               const char *DataPortName,
	       const char *prefix_name,
	       const char *record_name,
               int maxBuffers, size_t maxMemory,
               int priority, int stackSize)

    : ADDriver(portName, 1, NUM_SD_PARAMS, maxBuffers, maxMemory,
               0, 0,                                /* No interfaces beyond those set in ADDriver.cpp */
               ASYN_CANBLOCK | ASYN_MULTIDEVICE, 1, /* ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=1, autoConnect=1 */
               priority, stackSize),
      m_prefix_name(prefix_name),
      m_record_name(record_name)
// , pDetector(NULL)

{
  int status = asynSuccess;
  // NDArray *pData;
  const char *functionName = "syscmos";

  // TRY here instead
  // plock_ControlPortIO = new epicsMutex(__FILE__, __LINE__);
  m_cpv_interface = new CPV_Interface_IOC(this);

  IPPortName_ = epicsStrDup(CtrlPortName);
  isBigEndian_ = EPICS_BYTE_ORDER == EPICS_ENDIAN_BIG;

  /* Create the epicsEvents for signaling to the syscmos task when acquisition starts and stops */
  this->startEventId_ = epicsEventCreate(epicsEventEmpty);
  if (!this->startEventId_)
  {
    printf("%s:%s epicsEventCreate failure for start event\n",
           driverName, functionName);
    return;
  }
  ///Start with acquiring in background
  /// OR NOT epicsEventSignal(startEventId_);

  // Connect to the server
  status = pasynOctetSyncIO->connect(CtrlPortName, 0, &pasynUserMeter_, NULL);
  if (status)
  {
    printf("%s:%s: error calling pasynOctetSyncIO->connect, status=%d, error=%s\n",
           driverName, functionName, status, pasynUserMeter_->errorMessage);
    return;
  }
  

  // *****************************************************
  // ** REQUIRED - call to init **
  // *****************************************************
  m_cpv_interface->Init(pasynOctetSyncIO, pasynUserMeter_);

  status = pasynOctetSyncIO->connect(DataPortName, 0, &pasynDataMeter_, NULL);
  if (status)
  {
    printf("%s:%s: error calling pasynOctetSyncIO->connect, status=%d, error=%s\n",
           driverName, functionName, status, pasynDataMeter_->errorMessage); // YF typo fix
    return;
  }

  createParam(SDSettingString, asynParamInt32, &SDSetting);
  createParam(SDDelayTimeString, asynParamFloat64, &SDDelayTime);
  createParam(SDThresholdString, asynParamFloat64, &SDThreshold);
  createParam(SDEnergyString, asynParamFloat64, &SDEnergy);
  createParam(SDUseFlatFieldString, asynParamInt32, &SDUseFlatField);
  createParam(SDUseCountRateString, asynParamInt32, &SDUseCountRate);
  createParam(SDUseBadChanIntrplString, asynParamInt32, &SDUseBadChanIntrpl);
  createParam(SDBitDepthString, asynParamInt32, &SDBitDepth);
  createParam(SDUseGatesString, asynParamInt32, &SDUseGates);
  createParam(SDNumGatesString, asynParamInt32, &SDNumGates);
  createParam(SDNumFramesString, asynParamInt32, &SDNumFrames);
  createParam(SDInterframeTimeString, asynParamFloat64, &SDInterframeTime);
  createParam(SDSensorPowerString,asynParamInt32, &SDSensorPower);
  createParam(SDDoTriggerString, asynParamInt32, &SDDoTrigger);

  createParam(SDLinkStatusString, asynParamInt32, &SDLinkStatus);
  createParam(SDEPICSLinkStatusString, asynParamInt32, &SDEPICSLinkStatus);

  createParam(SDDSNUString, asynParamInt32, &SDDSNUMode);
  createParam(SDPRNUString, asynParamInt32, &SDPRNUMode);
  
  createParam(SDRunStartString, asynParamInt32, &SDRunStart);
  createParam(SDSelectRunString, asynParamInt32, &SDSelectRun);
  createParam(SDLoadNumFramesString, asynParamInt32, &SDLoadNumFrames);
  ///
  printf("OutMux Before: %i\n", SDOutMux);
  createParam(SDOutMuxString, asynParamInt32, &SDOutMux);
  printf("OutMux After: %i\n", SDOutMux);
  fflush(stdout);
  
  // YF already in base createParam(SDTriggerString, asynParamInt32, &SDTrigger);
  createParam(SDResetString, asynParamInt32, &SDReset);
  // YF prob not relevant createParam(SDTauString, asynParamFloat64, &SDTau);
  createParam(SDNModulesString, asynParamInt32, &SDNModules);
  createParam(SDFirmwareVersionString, asynParamOctet, &SDFirmwareVersion);
  createParam(SDReadModeString, asynParamInt32, &SDReadMode);

  createParam(SDCommandOutString, asynParamOctet, &SDCommandOut);
  createParam(SDRunNameString, asynParamOctet, &SDRunName );
  createParam(SDSetNameString, asynParamOctet, &SDSetName );
  createParam(SDSetDescriptionString, asynParamOctet, &SDSetDescription );

  createParam(SDStartRunString, asynParamInt32, &SDStartRun);


  
  status = setStringParam(ADManufacturer, "Sydor");
  status |= setStringParam(ADModel, "MMPAD");

  //status |= getFirmware();
  //status |= setStringParam (SDFirmwareVersion, firmwareVersion_);

  int sensorSizeX = MAX_DIMS;
  int sensorSizeY = MAX_DIMS; ///FIXME Unhardcode
  status |= setIntegerParam(ADMaxSizeX, sensorSizeX);
  status |= setIntegerParam(ADMaxSizeY, sensorSizeY);

  int minX, minY, sizeX, sizeY;
  minX = 1;
  minY = 1;
  sizeX = MAX_DIMS;
  sizeY = MAX_DIMS;
  status |= setIntegerParam(ADMinX, minX);
  status |= setIntegerParam(ADMinY, minY);
  status |= setIntegerParam(ADSizeX, sizeX);
  status |= setIntegerParam(ADSizeY, sizeY);

  status |= setIntegerParam(NDArraySize, 0);
  //status |= setIntegerParam(NDDataType,  NDInt32);
  status |= setIntegerParam(NDDataType, NDUInt16);

  status |= setIntegerParam(ADImageMode, ADImageSingle);

  /* NOTE: these char type waveform record could not be initialized in iocInit 
     * Instead use autosave to restore their values.
     * It is left here only for references.
     * */
  //status |= setIntegerParam(ADStatus, getStatus());

  //Get Firmware version

  // Read the current settings from the device.  This will set parameters in the parameter library.
  //getSettings();

  detArray_ = (epicsInt32 *)malloc(SYSCMOS_MAX_SIZE *
                                   SYSCMOS_MAX_DIM * SYSCMOS_MAX_DIM);
  callParamCallbacks();

  if (status)
  {
    printf("%s: unable to read camera parameters\n", functionName);
    return;
  }

  /* Register the shutdown function for epicsAtExit */
  // epicsAtExit(c_shutdown, (void*)this);

  /* Create the thread that runs acquisition */
  status = (epicsThreadCreate("acquisitionTask",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)acquisitionTaskC,
                              this) == NULL);


  // Create the thread that initializes companion PVs for query
  status = (epicsThreadCreate("initQueryTask",
			      epicsThreadPriorityMedium,
			      epicsThreadGetStackSize(epicsThreadStackMedium),
			      (EPICSTHREADFUNC)initCompanionTaskC,
			      this) == NULL);
  // YF
  /* Create the thread that listens on control socket */
  status = (epicsThreadCreate("controlSocketListenTask",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)controlSocketListenTaskC,
                              this) == NULL);

  /* Create the thread that polls status */
  //    status = (epicsThreadCreate("pollTask",
  //                                epicsThreadPriorityMedium,
  //                                epicsThreadGetStackSize(epicsThreadStackMedium),
  //                                (EPICSTHREADFUNC)pollTaskC,
  //                                this) == NULL);
}

/* Code for iocsh registration */
static const iocshArg syscmosConfigArg0 = {"Port name", iocshArgString};
static const iocshArg syscmosConfigArg1 = {"Ctrl port name", iocshArgString};
static const iocshArg syscmosConfigArg2 = {"Data port name", iocshArgString};
static const iocshArg syscmosConfigArg3 = {"Prefix name", iocshArgString};
static const iocshArg syscmosConfigArg4 = {"Record name", iocshArgString};
static const iocshArg syscmosConfigArg5 = {"maxBuffers", iocshArgInt};
static const iocshArg syscmosConfigArg6 = {"maxMemory", iocshArgInt};
static const iocshArg syscmosConfigArg7 = {"priority", iocshArgInt};
static const iocshArg syscmosConfigArg8 = {"stackSize", iocshArgInt};
static const iocshArg *const syscmosConfigArgs[] = {&syscmosConfigArg0,
                                                   &syscmosConfigArg1,
                                                   &syscmosConfigArg2,
                                                   &syscmosConfigArg3,
                                                   &syscmosConfigArg4,
                                                   &syscmosConfigArg5,
                                                   &syscmosConfigArg6,
						   &syscmosConfigArg7,
						   &syscmosConfigArg8};
static const iocshFuncDef configsyscmos = {"syscmosConfig", 9, syscmosConfigArgs};
static void configsyscmosCallFunc(const iocshArgBuf *args)
{
  syscmosConfig(args[0].sval, args[1].sval, args[2].sval,
	       args[3].sval, args[4].sval,
               args[5].ival, args[6].ival, args[7].ival, args[8].ival);
}

static void syscmosRegister(void)
{

  iocshRegister(&configsyscmos, configsyscmosCallFunc);
}

extern "C"
{
  epicsExportRegistrar(syscmosRegister);
}
