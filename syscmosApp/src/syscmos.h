#ifndef SYSCMOS_H
#define SYSCMOS_H

#include <list>

#include "cpv_interface_ioc.h"

#define MAX_FILENAME_LEN 256
#define MAX_DIMS      4096
#define MAX_COMMAND_LEN 128
#define MAX_NMODULES 2
#define M1K_TIMEOUT 5.0
#define MAX_FRAMES 500
#define MAX_TRIGGER_TIMEOUT_COUNT 50

//Definitions for GenCam
#define SYSCMOS_MAX_DIM MAX_DIMS	// Maximum size of a dimension of an image
#define SYSCMOS_MAX_SIZE 8	// Maximum member size -- double or 64-bit int

enum eImageType
  {
   kUNDEF = 0,
   kUINT16 = 1,
   kINT16  = 2,
   // Only FmB buffer can be assigned these: [0]
   kINT17 = 3,   // This is F-B with KUINT16. It is +/- 2^16
   kINT33 = 4,   // This is F-B with KUINT32. It is +/- 2^32
   // Only FmB buffer can be assigned these: [0]
   
   kUINT32  = 5,
   kINT32  = 6,
   kFLT32  = 7,
   kDBL64 = 8
  };



struct _structTelemetry
{
    uint32_t nFrameNumber;
    // TODO - generic array of ints?
    // key value pairs as strings?
};


struct _structImageFrame
{
    uint16_t UID;
    char     *image = nullptr;        // can point to anything. Use nImageType to interpret
    int flag = 0;                     // lock count.
    bool        bMarkForDelete;
    uint32_t timeStamp;             // time acquired
    uint32_t    nFrame=0;              // increments from 0 to nFramesPerSequence-1
    uint32_t    nFramesPerSequence=0;  // constant over the sequence

    struct  _structTelemetry telemetry;
    int  meta1 = 0;     // rsvd special use.

};

typedef struct _structImageFrame ImageFrame;

struct _structDoc
{

    // structs can have constructors too!
    _structDoc( ) // Create empty one.
    {
       width = 0; height = 0; pFrame = nullptr; enImageType = kUNDEF;
    }

    _structDoc( uint16_t w, uint16_t h, _structImageFrame *apFrame)
    {
        width = w;
        height = h;
        pFrame = apFrame;
    }
    uint16_t width=0;
    uint16_t height=0;
    char bpp=0; // Bytes per pixel  (2 for uint16, 4 for int32)
    struct _structImageFrame *pFrame = nullptr;
    eImageType enImageType = kUNDEF;
    bool bStream = false;
};

typedef struct _structDoc SydorDocStruct;



class syscmos : public ADDriver {
public:
    syscmos(const char *portName, const char *CtrlPortName,
	   const char *DataPortName,
	   const char *prefix_name,
	   const char *record_name,
                   int maxBuffers, size_t maxMemory,
                   int priority, int stackSize);

    // ***************************************
    // *** Create interface member
    // ***************************************	
	CPV_Interface_IOC *m_cpv_interface = NULL;

    friend class CPV_Interface_IOC;
	
    /* These are the methods that we override from ADDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, 
                                    size_t nChars, size_t *nActual);
    virtual void report(FILE *fp, int details); 
    
    int HandleResponse( int nFunction, CPV_Interface_IOC::PRType *prt);
    
    // YF [0]
    //virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
    // [0]

  epicsInt32 dataCallback(epicsInt32 *pData, SydorDocStruct *doc_data, int image_size); /* This should be private but is called from C so must be public */
    void pollTask(); 
    void acquisitionTask(); 
    void shutdown(); 
    void controlSocketListenTask();

 public:

    int SDSetting;
    #define FIRST_SD_PARAM SDSetting

#include "gc_class_params.h"
  
    int SDDoTrigger;
  ///int SDRunName;
  ///int SDSetName;
    int SDSetDescription;
    
  ///int SDSensorPower;
    int SDStartRun;
    //int SDTrigger;
    int SDTau;
    int SDFirmwareVersion;
    int SDReadMode;
    int SDCommandOut;
  ///int SDLinkStatus;
  int SDEPICSLinkStatus;

  ///int SDDSNUMode;
  ///int SDPRNUMode;
  
  int SDRunStart;
  int SDSelectRun;
  int SDLoadNumFrames;
  
  int SDOutMux;
  
    int SDNModules;
    #define LAST_SD_PARAM SDNModules

  std::list<CPV_Data_t> pv_data_list;
  std::unordered_map<int, SD_Param_Type> pv_dv_types;
    /* These are the methods we implement from Mythen */
    #if 0
    virtual asynStatus setAcquire(epicsInt32 value);
    virtual asynStatus setFCorrection(epicsInt32 value);
    virtual asynStatus setRCorrection(epicsInt32 value);
    virtual asynStatus setExposureTime(epicsFloat64 value);
    virtual asynStatus setDelayAfterTrigger(epicsFloat64 value);
    virtual asynStatus setBitDepth(epicsInt32 value);
    virtual asynStatus setBadChanIntrpl(epicsInt32 value);
    virtual asynStatus setUseGates(epicsInt32 value);
    virtual asynStatus setNumGates(epicsInt32 value);
    virtual asynStatus setKthresh(epicsFloat64 value);
    virtual asynStatus setEnergy(epicsFloat64 value);
    virtual asynStatus setTau(epicsFloat64 value);
    virtual asynStatus setFrames(epicsInt32 value);
    virtual asynStatus setFlip(epicsInt32 value);
    virtual asynStatus setTrigger(epicsInt32 value);
    virtual asynStatus loadSettings(epicsInt32 value);
    virtual asynStatus getSettings();
    virtual epicsInt32  getStatus();
#endif    
    virtual asynStatus getFirmware();

  // Functions for query setup
  int init_query_pv();

    // void decodeRawReadout(int nmods, int nbits, int *data, int *result);

 private:                                       
    /* These are the methods that are new to this class */

    /* Our data */
    epicsEventId startEventId_;
	asynUser *pasynUserMeter_;
  	asynUser *pasynDataMeter_;
    epicsInt32 acquiring_;
    epicsInt32 frames_;
    epicsInt32 chanperline_, nbits_;
    epicsInt32 nmodules, readmode_;
    char *IPPortName_;
    char firmwareVersion_[7];
    //char outString_[MAX_COMMAND_LEN];
    //char inString_[MAX_COMMAND_LEN];
    char dataOutString_[MAX_COMMAND_LEN];
  std::string m_prefix_name;
  std::string m_record_name;
    bool isBigEndian_;
    epicsInt32 *detArray_;
    epicsUInt32 *tmpArray_;

  
    asynStatus sendCommand();
    asynStatus writeReadMeter(); // TODO - cleanup and remove this
    
    epicsInt32 stringToInt32(char *str);
    long long stringToInt64(char *str);
    epicsFloat32 stringToFloat32(char *str);
    void swap4(char* value);
    void swap8(char* value);
  void template_replace(std::string &haystack, const std::string &template_pattern, const std::string &new_str);
};


#endif


