TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

DBD += timingAsynEpics.dbd

LIBRARY_IOC += timingAsynEpicsSupport 

USR_DBDFLAGS  += -I$(RAON_SITELIBS)/dbd
CXXFLAGS += -std=c++11 -Wno-unused-but-set-variable -Wno-format

timeStamp_DIR += $(RAON_SITELIBS)/lib/$(T_A)

timingAsynEpicsSupport_SRCS += xadc_core.c drxiic.cpp
timingAsynEpicsSupport_SRCS += ts2ipDrv.cpp ts2ipEvsys.cpp ts2ipZqsys.cpp ts2ipEvg.cpp ts2ipEvr.cpp ts2ipGtp.cpp drMpsPico.cpp tsDev.cpp tsDev_evx.cpp tsDev_zq9r.cpp verMan.cpp
# timingAsynEpicsSupport_SRCS += ts2ipDrv.cpp ts2ipZqsys.cpp ts2ipGtp.cpp ts2ipEvr.cpp
timingAsynEpicsSupport_SRCS += timingAsynEpics.cpp
timingAsynEpicsSupport_LIBS += asyn 
timingAsynEpicsSupport_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
# Build the IOC application

PROD_IOC = timing
# timing.dbd will be created and installed
DBD += timing.dbd

# timing.dbd will be made up from these files:
timing_DBD += base.dbd
timing_DBD += timingAsynEpics.dbd timestampRecord.dbd
timing_DBD += timestampDevSoft.dbd strToValue.dbd

# Include dbd files from all support applications:
#timing_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#timing_LIBS += xxx

# timing_registerRecordDeviceDriver.cpp derives from timing.dbd
timing_SRCS += timing_registerRecordDeviceDriver.cpp
timing_SRCS += strToValue.cpp
timing_LIBS += timingAsynEpicsSupport 
timing_LIBS += asyn timeStamp

# Build the main IOC entry point on workstation OSs.
timing_SRCS_DEFAULT += timingMain.cpp
timing_SRCS_vxWorks += -nil-

# Finally link to the EPICS Base libraries
timing_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

