#! /bin/sh
PROJECT_NAME="ts2ip"
PROJECT_TOP=${PWD}/timingApp/src

svn update
rm -f ${PROJECT_TOP}/${PROJECT_NAME}_version.h
perl genVersionHeader.pl -v -t . -N ${PROJECT_NAME}_version ${PROJECT_TOP}/${PROJECT_NAME}_version.h

make

if [ "" = "$1" ]
then
  svn ci -m "-"
else
  svn ci -m $1
fi


