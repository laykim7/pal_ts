#! /bin/sh
PROJECT_TOP=".."
source $HOME/envDev.sh
source ./gitBranchInfo.sh

echo '  remote: NAS'
echo '  branch: ' $gitBranchName

gitPull()
{
expect <<EOF
set timeout 180
spawn git pull nas $gitBranchName
expect {
  "password:" {send "$gitServerPD\n"}
}
expect eof
EOF
}

case "$1" in
  "")
          gitPull 
          exit 1
esac

exit $?


