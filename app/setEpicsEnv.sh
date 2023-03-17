export EPICS_DIR=/home/ctrluser/epics/R3.14.12.5

export EPICS_HOST_ARCH=linux-x86_64
export EPICS_PATH=${EPICS_DIR}
export EPICS_BASE=${EPICS_DIR}/base
export EPICS_EXTENSIONS=${EPICS_DIR}/extensions
export EPICS_SYNAPPS=${EPICS_DIR}/epicsLibs/synApps_5_8/support
export RAON_SITELIBS=${EPICS_DIR}/siteLibs
export PATH=${EPICS_DIR}/base/bin/linux-x86_64:${EPICS_DIR}/extensions/bin/linux-x86_64:${EPICS_DIR}/siteLibs/bin/linux-x86_64:/usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games
export LD_LIBRARY_PATH=${EPICS_DIR}/base/lib/linux-x86_64:${EPICS_DIR}/extensions/lib/linux-x86_64:${EPICS_DIR}/siteLibs/lib/linux-x86_64
