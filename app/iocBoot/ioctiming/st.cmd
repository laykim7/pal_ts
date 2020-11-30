#!../../bin/linux-arm/timing

## You may have to change timing to something else
## everywhere it appears in this file!

< envPaths
< /mnt/sdcard/envEpics

epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES","6553500")
epicsEnvSet("ARCH","linux-arm")
epicsEnvSet("TS_LOCAL","/mnt/sdcard")
epicsEnvSet("TS_SERVER","/gitWspace")

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/timing.dbd"
timing_registerRecordDeviceDriver pdbbase
timingAsynEpicsConfigure("timing", 1000, 0, 10, "${TS_SERVER}", "${TS_LOCAL}", "${DEV_SYS}", "${DEV_SUBSYS}", "${DEV_NAME}", "${DEV_NUM}", ${OP_MODE}, ${TZONE}, ${TICKPERIOD}, ${TS_RESERVED_3}, ${TS_RESERVED_4}, ${TS_RESERVED_5})
epicsThreadSleep(1)

## Load record instances
dbLoadRecords("db/common.db","SYS=${DEV_SYS},SUBSYS=${DEV_SUBSYS},DEV=${DEV_NAME}${DEV_NUM}")
dbLoadTemplate("${TS_LOCAL}/tspv.sub")
asynSetTraceIOMask("timing", 0, 0x2)

iocInit
dbpf ${DEV_SYS}:${DEV_SUBSYS}:${DEV_NAME}${DEV_NUM}:SYS_P_RUN "RUN"










## Start any sequence programs
#seq sncxxx,"user=ctrluserHost"


## For Setup Clock
#epicsThreadSleep(5)
#dbpf SCL3:TS:EVG:EVGSet 1
