#!/bin/bash

kill_proc()
{
	target=$1
	echo "stop $target"
	pids=`ps -ef | grep $target | grep -w $USER | grep -v grep | awk '{print $2}'` 
	for pid  in $pids 
	do 
		echo -n "$pid "
		kill -9 $pid 
	done 
	echo
}

kill_proc routersvrd
kill_proc agentsvrd

#echo "delete share memory" 
#shmid=`ipcs -m | grep -w $USER | awk '$6==0{printf " -m  " $2  " "}'` 
#echo "$shmid" 
#ipcrm $shmid
