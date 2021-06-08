TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

LIBRARY_IOC += syscmos cpv_interface_ioc
LIB_SRCS += syscmos.cpp cpv_interface_ioc.cpp

# <name>.dbd will be created from <name>Include.dbd
DBD += syscmosSupport.dbd

include $(ADCORE)/ADApp/commonLibraryMakefile

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

