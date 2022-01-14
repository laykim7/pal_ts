#!/bin/sh
export L_DEV_PN="evx"

export L_SYS="SCL3"
export L_SUB_SYS="TS"
export L_DEV="EVS"
export L_DEV_NUM="001"

export L_OP_MODE="0"
export L_TZONE="32400"
export L_TICKPERIOD="80000000"
export L_RSRVD3="0"
export L_RSRVD4="0"
export L_RSRVD5="0"


### auto Start 
#export autoStart="0"
export autoStart="1"

export ipAddr="192.168.0.162"
export serverIp="192.168.0.182"

export gitServerName="syn"
export gitServerPD="duruA103\n"
export gitServerPATH="http://192.168.0.20:30000/laykim/pal_ts.git"
export gitBranchName="app_evx1.4"
export gitProjName="pal_ts"

###==========================================================================###
### Device Configuration for EPICS ###

### The name "xIPx_" will be deleted.
### example : export xIPx_SYS  = "xIPx_SYS" -> will be deleted.
export xIPx_SYS="SYS"
export xIPx_GTP="GTP"
export xIPx_EV="EV"
export xIPx_ZQ="xIPx_ZQ"
export xIPx_EVR0="EVR0"
export xIPx_EVR1="EVR1"
export xIPx_EVG0="EVG0"
export xIPx_EVG1="EVG1"
export xIPx_MP0="xIPx_MP0"
export xIPx_MP1="xIPx_MP1"
export xIPx_MP2="xIPx_MP2"
export xIPx_MP3="xIPx_MP3"
export xIPx_MP4="xIPx_MP4"

### target project folder [petalinux, epics]
export epicsF="/home/ctrluser/epics/R3.14.12.5"

#=================================================================#

##-- ibs -------------------------------------------------------###
#export wspace="/home/ctrluser/epics/R3.14.12.5/siteApps/Ctrl_IOC/timing-1-1"
#export src_wspace="/mnt/sdcard/ts_pal_sw"

##-- nfri ------------------------------------------------------###
# export gitBranchName="ts_nfri_sw"
# export src_wspace="/mnt/sdcard/${gitBranchName}"
# export wspace="/git/${gitBranchName}"


##-- pal -------------------------------------------------------###
export src_wspace="/mnt/sdcard/${gitProjName}"
export wspace="/git/${gitProjName}"

#=================================================================#

export localEnv="${src_wspace}/${gitBranchName}/local-env"
export iocF="${wspace}/${gitBranchName}/app/iocBoot/ioctiming"

export insDrv0="1"
export insDrv1="0"
export insDrv2="0"
export insDrv3="0"
export insDrv4="0"
export insDrv5="0"
export insDrv6="0"
export insDrv7="0"
export insDrv8="0"
export insDrv9="0"

export drv0="${wspace}/${gitBranchName}/driver/ts2ip/ts2ip.ko"




