#!/bin/bash

export SERVER_HOME=~/disvr

./stop.sh

target='routersvrd -D'
echo $target
cd $SERVER_HOME/routerserver/bin
rm ../log/* -f
./$target
sleep 2

target='agentsvrd -D'
echo $target
cd $SERVER_HOME/agentserver/bin
rm ../log/* -f
./$target
sleep 2

ps aux | grep $USER | grep -v grep | grep svrd
