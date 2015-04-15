#!/bin/bash


if (( $# != 2 ))
then
  echo "Usage: ... ./bClient.sh <IP> <ON/OFF>"
  exit 1
fi

IP=$1
PAGE=$2

URL=":1000/"
SUFFIX="</td*"

#Build a request ( http://10.10.2.4:1000/ON )
REQUEST="http://"$IP$URL$PAGE

#Make the request, with 1 retry and a timeout of 10 seconds per retry (max 20 seconds per request)
RESULT=$(wget -q --tries=1 --timeout=10 -O- $REQUEST | grep 'LED is')

#Trim the HTML from the result
RESULT=${RESULT%$SUFFIX}

#If succesful, print status, else indicate failure
if [ ${#RESULT} -ne 0 ]
then
  echo $RESULT
else
  echo "Request Failed"
fi