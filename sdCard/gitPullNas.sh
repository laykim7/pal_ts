#!/usr/bin/expect -f

set timeout 180

spawn git pull $::env(gitServerName) $::env(gitBranchName)

expect {
  "(y/n)" {send "y\r";exp_continue}
  "password:" {send "$::env(gitServerPD)\n"}
  "root@" {send "\r"}
  "Already up-to-date." {send "\r"}
}

expect eof   
