#! /bin/sh
PROJECT_TOP=".."
source $PROJECT_TOP/gitBranchInfo.sh

gitPush()
{
  git push $gitServerName $gitBranchName
}


_common ()
{
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
  PROJECT_NAME="sm"
  git add ./sysManage.sh
  _common "$1"
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


