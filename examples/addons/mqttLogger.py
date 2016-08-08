# Note: This code requires paho-mqtt
# sudo pip-install paho-mqtt

# Simple MQTT data logger, listens for UDP output from RF24Gateway & logs to an MQTT topic
# For use with RF24Mesh nodes that are not able to communicate directly via TCP/IP etc
# See RF24Gateway examples: 
#                            RF24NetworkFrame frame = RF24NetworkFrame(header,buf,size); 
#                            gw.sendUDP(mesh.getNodeID(header.from_node),frame);
#

# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import socket
import sys

reload(sys) 
sys.setdefaultencoding('utf8')

### Setup the MQTT host IP & topic to publish to
mqttHost = "10.10.2.2"
topic = 'data/RF24'

### Bind to UDP port 32001 (default port of RF24Gateway UDP output)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = ('localhost', 32001)
print >>sys.stderr, 'Waiting for UDP message on %s port %s\n' % server_address
sock.bind(server_address)

while True:

    data, address = sock.recvfrom(2048)
    
    print >>sys.stderr, 'received %s bytes from %s' % (len(data), address)
    print >>sys.stderr, data
    
    if data:
        ## Just log the entire chunk of data to MQTT for now
        ## Todo: Sort, Display and Analyze the data
        mqttc = mqtt.Client()
        mqttc.connect(mqttHost, 1883)
        data = unicode(data, errors='replace')
        mqttc.publish(topic, data)
        mqttc.loop(2)