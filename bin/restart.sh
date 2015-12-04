#!/bin/bash

export SERVER_HOME=~/server

./stop.sh

gatetarget='gatesvrd -D'

target='dbsvrd -D'
echo $target
cd $SERVER_HOME/dbserver/bin
rm ../log/* -f
./$target
sleep 2

target='loginsvrd -D'
echo $target
cd $SERVER_HOME/loginserver/bin
rm ../log/* -f
./$gatetarget
sleep 1
./$target
sleep 1

target='scenesvrd -D'
echo $target
cd $SERVER_HOME/sceneserver/bin
rm ../log/* -f
./$gatetarget
sleep 1
./$target
sleep 1

ps aux | grep $USER | grep -v grep | grep svrd
