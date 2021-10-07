#! /bin/sh

PROJECT_TOP=".."
source $HOME/envDev.sh
source ./gitBranchInfo.sh

echo '  remote: NAS'
echo '  branch: ' $gitBranchName

gitPush()
{
expect <<EOF
set timeout 180
spawn git push nas $gitBranchName
expect {
  "(y/n)" {send "y\r";exp_continue}
  "password:" {send "$gitServerPD\n"}
  "root@" {send "\r"}
  "Already up-to-date." {send "\r"}
}
expect eof
EOF
}

_update ()
{
  echo ================== git commit start ==================
  git add -A
  git commit -m "$1"
  gitPush
  echo ======================================================

  # rm -f vers_${PROJECT_NAME}.sh
  # perl ${PROJECT_TOP}/genVersionHeader.pl -v -t ${PROJECT_TOP}/ -N ver_${PROJECT_NAME} vers_${PROJECT_NAME}.sh

  # echo =========== git commit version infomation  ===========
  # git add vers_${PROJECT_NAME}.sh
  # git commit -m "update ${PROJECT_NAME} version info"
  # gitPush
  echo ======================================================
}

case "$1" in
  -m)
          _update "$2"; ;;
  "")
          echo "======================================="
          echo "[err] missing option (-m): 'commit comment'"
          exit 1
esac

exit $?


