#! /bin/sh
source /git/envDev.sh
PROJECT_TOP=".."

make

gitPush()
{
expect <<EOF
set timeout 180
spawn git push $gitServerName $gitBranchName
expect {
  "(y/n)" {send "y\r";exp_continue}
  "password:" {send "$gitServerPD\n"}
  "root@" {send "\r"}
  "Already up-to-date." {send "\r"}
}
expect eof
EOF
}

_common ()
{
  echo "$1"
  git commit -m "$1"
  gitPush
  echo ======================================================

  rm -f vers_${PROJECT_NAME}.sh
  perl ${PROJECT_TOP}/genVersionHeader.pl -v -t ${PROJECT_TOP}/ -N ver_${PROJECT_NAME} vers_${PROJECT_NAME}.sh

  echo =========== git commit version infomation  ===========
  git add vers_${PROJECT_NAME}.sh
  git commit -m "update ${PROJECT_NAME} version info"
  gitPush
  echo ======================================================
}

_update ()
{
  echo ================== git commit start ==================
  PROJECT_NAME="sw"
  git add ../app/*
  _common "$1"
}

case "$1" in
  -m)
          _update "$2"; ;;
  "")
          echo "======================================="
          echo "[err] missing option : 'commit comment'"
          exit 1
esac

exit $?


