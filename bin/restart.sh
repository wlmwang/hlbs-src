#!/bin/bash

export SERVER_HOME=/usr/local/hlbs

./stop.sh

target='routersvrd -d'
echo $target
cd $SERVER_HOME/routerserver/bin
rm ../log/* -f
./$target
sleep 2

target='agentsvrd -d'
echo $target
cd $SERVER_HOME/agentserver/bin
rm ../log/* -f
./$target
sleep 2

ps aux | grep $USER | grep -v grep | grep svrd
