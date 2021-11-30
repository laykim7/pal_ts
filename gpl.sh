#! /bin/sh

PROJECT_TOP=".."
source ./gitBranchInfo.sh

echo '  remote: ' $gitServerName
echo '  branch: ' $gitBranchName

gitPull()
{
  spawn git pull $gitServerName $gitBranchName
}

case "$1" in
  "")
          gitPull 
          exit 1
esac

exit $?


