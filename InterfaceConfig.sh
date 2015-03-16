#!/bin/bash

USAGE="Usage:
Set the IP for this node and ping the other node.
  $0 \$nodeIP \$nodeIP

This is node1 and it will ping node2 three times
  $0 1 2
  
Optionally set the octal RF24Network address for non-mesh
nodes.
  $0 1 2 01  
"

if [[ -z "${1##*[!0-9]*}" ]] || [[ -z "${2##*[!0-9]*}" ]]; then
        echo -e "$USAGE"
		exit
fi

ADDRESS=${3}
if [[ -z "${ADDRESS##*[!0-9]*}" ]]; then
	ADDRESS=0
fi

###### CONFIGURE the IP/Subnet here. The last octet will be set when running the script #######
INTERFACE="tun_nrf24"
MYIP="10.10.2.${1}/24"
#MYIP="192.168.1.${1}/24"
MTU=1500


function setIP() {
	sleep 4s && \
	if [ -d /proc/sys/net/ipv4/conf/${INTERFACE} ]; then
		ip link set ${INTERFACE} up  > /dev/null 2>&1
		ip addr add ${MYIP} dev ${INTERFACE}  > /dev/null 2>&1
		ip link set dev ${INTERFACE} mtu ${MTU} > /dev/null 2>&1
		ip addr show dev ${INTERFACE} > /dev/null 2>&1
		return 0
	else
		return 1
	fi
}

setIP
