< envPaths

# Specify largest array CA will transport
# Note for N sscanRecord data points, need (N+1)*8 bytes, else MEDM
# plot doesn't display
epicsEnvSet EPICS_CA_MAX_ARRAY_BYTES 32000000

dbLoadDatabase("$(TOP)/dbd/syscmosApp.dbd")

gencamApp_registerRecordDeviceDriver(pdbbase)

#drvAsynIPPortConfigure("portName","hostInfo",priority,noAutoConnect,
#                        noProcessEos)
drvAsynIPPortConfigure("IP_Ctrl", "192.168.11.177:10030", 0, 0, 0)
drvAsynIPPortConfigure("IP_Data", "192.168.11.177:10031", 0, 0, 0)

## Testing at YF Home
#drvAsynIPPortConfigure("IP_Ctrl", "YoramSoftDevLT:10030", 0, 0, 0)
#drvAsynIPPortConfigure("IP_Data", "YoramSoftDevLT:10031", 0, 0, 0)

### TESTING WITH PYTHON LOCAL SIMULATOR ##
#drvAsynIPPortConfigure("IP_Ctrl", "127.0.0.1:10030", 0, 0, 0)
#drvAsynIPPortConfigure("IP_Data", "127.0.0.1:10031", 0, 0, 0)


#drvAsynIPPortConfigure("IP_M1K", "127.168.0.90:1031", 0, 0, 1)
#drvAsynIPPortConfigure("IP_M1K", "164.54.109.66:1031", 0, 0, 0)

#asynOctetSetInputEos("IP_M1K",0,"\r\n")
#asynOctetSetOutputEos("IP_M1K",0,"\r")

#asynSetTraceIOMask("IP_M1K",0,6)
#asynSetTraceMask("IP_M1K",0,3)

#epicsEnvSet("PREFIX","dp_mythen1K:")
epicsEnvSet("PREFIX","dp:")
epicsEnvSet("PORT",   "SD1")

# YF less typing. v v v
#epicsEnvSet("PREFIX","")
#epicsEnvSet("PORT",  "")


epicsEnvSet("QSIZE",  "20")
epicsEnvSet("XSIZE",  "1280")
epicsEnvSet("YSIZE",  "1")
epicsEnvSet("NCHANS", "1280")
epicsEnvSet("CBUFFS", "100")
epicsEnvSet("EPICS_DB_INCLUDE_PATH","$(ADCORE)/db:$(ADSYSCMOS)/db")

# mythenConfig (
#               portName,       # The name of the asyn port driver to be created.
#               IPPortName,     # The network port connection to the Mythen
#               maxBuffers,     # The maximum number of NDArray buffers that the NDArrayPool for this driver is 
#                                 allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
#               maxMemory)      # The maximum amount of memory that the NDArrayPool for this driver is 
#                                 allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
gencamConfig("SD1", "IP_Ctrl", "IP_Data", "$(PREFIX)", "cam1:", -1,-1)

#asynSetTraceMask("IP_M1K",0,1)
#YF Try
#dbLoadRecords("$(ADGENCAM)/gencamApp/Db/ADBase.template", "P=$(PREFIX),R=cam1:,PORT=$(PORT),ADDR=0,TIMEOUT=1")
dbLoadRecords("$(SYSCMOS)/gencamApp/Db/gencam.template", "P=$(PREFIX),R=cam1:,PORT=$(PORT),ADDR=0,TIMEOUT=1")


# Create a standard arrays plugin
NDStdArraysConfigure("Image1", 3, 0, "$(PORT)", 0, 0)
dbLoadRecords("$(ADCORE)/db/NDStdArrays.template", "P=$(PREFIX),R=image1:,PORT=Image1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT),NDARRAY_ADDR=0,TYPE=Float64,FTVL=DOUBLE,NELEMENTS=1048576")

# Load all other plugins using commonPlugins.cmd
#///< $(ADCORE)/iocBoot/commonPlugins.cmd

# Load asynRecord records on Mythen communication
dbLoadRecords("$(ASYN)/db/asynRecord.db", "P=$(PREFIX),R=asyn_1,PORT=IP_M1K,ADDR=0,OMAX=256,IMAX=256")

set_requestfile_path("$(TOP)/gencamApp/Db")

iocInit()

# save things every thirty seconds
# YF create_monitor_set("auto_settings.req", 30,"P=$(PREFIX),D=cam1:")
