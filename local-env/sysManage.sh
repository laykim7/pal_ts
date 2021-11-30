#!/bin/sh

echo "======================================="
echo -n "$0 "
for VAR in "$@"
do
echo "$VAR "
done

echo -E

if [ -f /mnt/sdcard/envDev.sh ]; then
  source /mnt/sdcard/envDev.sh
else
  echo There is no device env files.
  echo Exit...
  echo -E
  exit 1
fi



_exit ()
{
  echo Exit...
  echo -E
  reboot
  exit 1
}


gitPullNas ()
{
expect <<EOF
set timeout 180
spawn git pull $gitServerName $gitBranchName
expect {
  "(y/n)" {send "y\r";exp_continue}
  "password:" {send "$gitServerPD\n"}
  "root@" {send "\r"}
  "Already up-to-date." {send "\r"}
}
expect eof   
EOF
}


chkVer ()
{
  local REBOOT_CHECK=0

  cd ${localEnv}
  gitPullNas
  sync

  #vers(source)  verl(local)

  #=======================================================================
  f_vers_sm="${localEnv}/vers_sm.sh"
  f_verl_sm="/mnt/sdcard/verl_sm.sh"
  
  source ${f_vers_sm}

  if [ -f ${f_verl_sm} ]; then
    source ${f_verl_sm}
  else
    echo There is no sysmanage version file.
    echo copy files..
    cp -f ${f_vers_sm} ${f_verl_sm}
    cp -f ${localEnv}/sysManage.sh /mnt/sdcard/sysManage.sh
    sync
    REBOOT_CHECK=1
  fi

  if [[ $ver_sm == $ver_sm_old ]]; then
    echo ver_sm is recent.
  else
    echo sysmanage.sh file update process......
    cp -f ${f_vers_sm} ${f_verl_sm}
    find /mnt/sdcard -iname 'verl_sm.sh' -exec sed -i 's/ver_sm/ver_sm_old/gi;' {} \;
    cp -f ${localEnv}/sysManage.sh /mnt/sdcard/sysManage.sh
    sync
    echo ver_sm update done.
    REBOOT_CHECK=1
  fi

  #=======================================================================
  f_vers_tspv="${localEnv}/pv/vers_tspv.sh"
  f_verl_tspv="/mnt/sdcard/verl_tspv.sh"
  
  source ${f_vers_tspv}

  if [ -f ${f_verl_tspv} ]; then
    source ${f_verl_tspv}
  else
    echo There is no tspv version file.
    echo copy files..
    cp -f ${f_vers_tspv} ${f_verl_tspv}
    upEpics
    sync
  fi

  if [[ $ver_tspv == $ver_tspv_old ]]; then
    echo ver_tspv is recent.
  else
    echo PV file update process......
    cp -f ${f_vers_tspv} ${f_verl_tspv}
    find /mnt/sdcard -iname 'verl_tspv.sh' -exec sed -i 's/ver_tspv/ver_tspv_old/gi;' {} \;
    upEpics
    sync
    echo ver_tspv update done.
  fi

  #=======================================================================
  f_vers_boot="${localEnv}/dev.${L_DEV_PN}/vers_boot.sh"
  f_verl_boot="/mnt/sdcard/verl_boot.sh"
  
  source ${f_vers_boot}

  if [ -f ${f_verl_boot} ]; then
    source ${f_verl_boot}
  else
    echo There is no boot version file.
    echo copy files..
    echo BOOT.BIN file update process......
    cp -f ${f_vers_boot} ${f_verl_boot}
    find /mnt/sdcard -iname 'verl_boot.sh' -exec sed -i 's/ver_boot/ver_boot_old/gi;' {} \;
    cp -f ${localEnv}/dev.${L_DEV_PN}/BOOT.BIN /mnt/sdcard/BOOT.BIN
    sync
    echo "BOOT.BIN copy - done."
    REBOOT_CHECK=1
  fi
  
  if [[ $ver_boot == $ver_boot_old ]]; then
    echo ver_boot is recent.
  else
    echo BOOT.BIN file update process......
    cp -f ${f_vers_boot} ${f_verl_boot}
    find /mnt/sdcard -iname 'verl_boot.sh' -exec sed -i 's/ver_boot/ver_boot_old/gi;' {} \;
    cp -f ${localEnv}/dev.${L_DEV_PN}/BOOT.BIN /mnt/sdcard/BOOT.BIN
    sync
    echo "BOOT.BIN update - done."
    REBOOT_CHECK=1
  fi

  #=======================================================================
  f_vers_image="${localEnv}/dev.${L_DEV_PN}/vers_image.sh"
  f_verl_image="/mnt/sdcard/verl_image.sh"
  
  source ${f_vers_image}

  if [ -f ${f_verl_image} ]; then
    source ${f_verl_image}
  else
    echo There is no image version file.
    echo copy files..
    cp -f ${f_vers_image} ${f_verl_image}
    find /mnt/sdcard -iname 'verl_image.sh' -exec sed -i 's/ver_image/ver_image_old/gi;' {} \;
    cp -f ${localEnv}/dev.${L_DEV_PN}/image.ub /mnt/sdcard/image.ub
    sync
    echo "image.ub update - done."
    REBOOT_CHECK=1
  fi
  
  if [[ $ver_image == $ver_image_old ]]; then
    echo ver_image is recent.
  else
    echo image.ub file update process......
    cp -f ${f_vers_image} ${f_verl_image}
    find /mnt/sdcard -iname 'verl_image.sh' -exec sed -i 's/ver_image/ver_image_old/gi;' {} \;
    cp -f ${localEnv}/dev.${L_DEV_PN}/image.ub /mnt/sdcard/image.ub
    sync
    echo "image.ub update - done."
    REBOOT_CHECK=1
  fi

  echo REBOOT_CHECK $REBOOT_CHECK

  if [ $REBOOT_CHECK == "1" ]; then
    _exit
  fi
}

start ()
{

# add your app
  ifconfig eth0 down 
  ifconfig eth0 ${ipAddr} up 

  /etc/init.d/ntpd stop
  rm /etc/ntp.conf
  ln -s /mnt/sdcard/ntp.conf /etc/ntp.conf
  /etc/init.d/ntpd start

  mkdir /home/ctrluser
  ln -s /mnt/sdcard/epics /home/ctrluser/epics
  
  ln -s ${src_wspace} /gitWspace

  mkdir /git
  ln -s ${src_wspace} /git/${gitProjName}

  mkdir /mnt/nfs
  # mount -o nolock ${serverIp}:${wspace} /mnt/nfs

  chkVer

  if [ ${insDrv0} = 1 ]; then insmod ${drv0}; fi
  if [ ${insDrv1} = 1 ]; then insmod ${drv1}; fi
  if [ ${insDrv2} = 1 ]; then insmod ${drv2}; fi
  if [ ${insDrv3} = 1 ]; then insmod ${drv3}; fi
  if [ ${insDrv4} = 1 ]; then insmod ${drv4}; fi
  if [ ${insDrv5} = 1 ]; then insmod ${drv5}; fi
  if [ ${insDrv6} = 1 ]; then insmod ${drv6}; fi
  if [ ${insDrv7} = 1 ]; then insmod ${drv7}; fi
  if [ ${insDrv8} = 1 ]; then insmod ${drv8}; fi
  if [ ${insDrv9} = 1 ]; then insmod ${drv9}; fi

  ln -s /mnt/sdcard/sysManage.sh /home/root/sm

  export PS1=\$

  echo "init done."

  if [ $autoStart = 1 ]; then
    echo "auto start!!!"
    screen -S epics -m /home/root/./sm run
  fi
}

stop ()
{
  rmmod ${drv0}
  rmmod ${drv1}
  rmmod ${drv2}
  rmmod ${drv3}
  rmmod ${drv4}
  rmmod ${drv5}
  rmmod ${drv6}
  rmmod ${drv7}
  rmmod ${drv8}
  rmmod ${drv9}
  echo "stop - done."
}

epicsRun ()
{
# add your app
  cd ${iocF}
  ./st.cmd
}

epicsUpdateRun ()
{
# add your app
  chkVer
  
  cd ${iocF}
  ./st.cmd
}

upSys()
{
    echo "update sysManage.sh"
    cp -f ${localEnv}/sysManage.sh /mnt/sdcard/sysManage.sh
    sync
    echo "update - done."
}

gitRecovery()
{
  rm $src_wspace -rf
  mkdir $src_wspace
  cd $src_wspace
  git init
  sync
  git config user.name $L_DEV_PN
  git config user.email 'info@dandansys.com'
  git remote add nas $gitServerPATH
  sync
  gitPullNas
  sync
  echo "gitRecovery - done."
}

upEpics()
{
  echo ${L_SYS}, ${L_SUB_SYS}, ${L_DEV}, ${L_DEV_NUM}

  cp ${localEnv}/pv/tspv_template.reg /mnt/sdcard/tspv.reg
  cp ${localEnv}/pv/tspv_template.sub /mnt/sdcard/tspv.sub
  cp ${localEnv}/envEpics_Template    /mnt/sdcard/envEpics
  sync

  find /mnt/sdcard -iname 'envEpics' -exec sed -i 's/NEW_SYS/'${L_SYS}'/gi;s/NEW_SUB_SYS/'${L_SUB_SYS}'/gi;s/NEW_DEV_NUM/'${L_DEV_NUM}'/gi;s/NEW_DEV/'${L_DEV}'/gi;' {} \;
  find /mnt/sdcard -iname 'envEpics' -exec sed -i 's/NEW_OP_MODE/'${L_OP_MODE}'/gi;s/NEW_TZONE/'${L_TZONE}'/gi;s/NEW_TICKPERIOD/'${L_TICKPERIOD}'/gi;s/NEW_RSRVD3/'${L_RSRVD3}'/gi;s/NEW_RSRVD4/'${L_RSRVD4}'/gi;s/NEW_RSRVD5/'${L_RSRVD5}'/gi;' {} \;
  sync

  find /mnt/sdcard -iname 'tspv.sub' -exec sed -i 's/NEW_SYS/'${L_SYS}'/gi;s/NEW_SUB_SYS/'${L_SUB_SYS}'/gi;s/NEW_DEV_N/'${L_DEV}${L_DEV_NUM}'/gi;' {} \;
  sync

  find /mnt/sdcard -iname 'tspv.sub' -exec sed -i 's/xIPx_SYS/'${xIPx_SYS}'/gi;''s/xIPx_GTP/'${xIPx_GTP}'/gi;''s/xIPx_EV/'${xIPx_EV}'/gi;''s/xIPx_ZQ/'${xIPx_ZQ}'/gi;' {} \;
  find /mnt/sdcard -iname 'tspv.sub' -exec sed -i 's/xIPx_EVR0/'${xIPx_EVR0}'/gi;''s/xIPx_EVR1/'${xIPx_EVR1}'/gi;''s/xIPx_EVG0/'${xIPx_EVG0}'/gi;''s/xIPx_EVG1/'${xIPx_EVG1}'/gi;' {} \;
  find /mnt/sdcard -iname 'tspv.sub' -exec sed -i 's/xIPx_MP0/'${xIPx_MP0}'/gi;''s/xIPx_MP1/'${xIPx_MP1}'/gi;''s/xIPx_MP2/'${xIPx_MP2}'/gi;''s/xIPx_MP3/'${xIPx_MP3}'/gi;''s/xIPx_MP4/'${xIPx_MP4}'/gi;' {} \;
  sync

  find /mnt/sdcard -iname 'tspv.sub' -exec sed -i '/xIPx_/d' {} \;
  sync

  find /mnt/sdcard -iname 'tspv.reg' -exec sed -i 's/xIPx_SYS/'${xIPx_SYS}'/gi;''s/xIPx_GTP/'${xIPx_GTP}'/gi;''s/xIPx_EV/'${xIPx_EV}'/gi;''s/xIPx_ZQ/'${xIPx_ZQ}'/gi;' {} \;
  find /mnt/sdcard -iname 'tspv.reg' -exec sed -i 's/xIPx_EVR0/'${xIPx_EVR0}'/gi;''s/xIPx_EVR1/'${xIPx_EVR1}'/gi;''s/xIPx_EVG0/'${xIPx_EVG0}'/gi;''s/xIPx_EVG1/'${xIPx_EVG1}'/gi;' {} \;
  find /mnt/sdcard -iname 'tspv.reg' -exec sed -i 's/xIPx_MP0/'${xIPx_MP0}'/gi;''s/xIPx_MP1/'${xIPx_MP1}'/gi;''s/xIPx_MP2/'${xIPx_MP2}'/gi;''s/xIPx_MP3/'${xIPx_MP3}'/gi;''s/xIPx_MP4/'${xIPx_MP4}'/gi;' {} \;
  sync

  find /mnt/sdcard -iname 'tspv.reg' -exec sed -i '/xIPx_/d' {} \;
  sync

  echo "update done ok: 'tspv.sub', 'tspv.reg', 'envEpics'"
}


case "$1" in
  start)
          start; ;;
  stop)
          stop; ;;
  run)
          epicsRun $2; ;;
  urun)
          epicsUpdateRun $2; ;;              
  upSys)
          upSys; ;;
  upEpics)
          upEpics; ;;
  chkVer)
          chkVer; ;;
  gitRecovery)
          gitRecovery; ;;
  *)
          echo "======================================="
          echo "Usage: $0 {start|stop|run|urun|upSys|upLinux|upFpga|upEpics|gitRecovery}"
          exit 1
esac

exit $?
