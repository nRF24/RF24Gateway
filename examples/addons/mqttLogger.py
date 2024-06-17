"""
.. note::
    This code requires paho-mqtt. Install this library with

    sudo pip3 install paho-mqtt

Simple MQTT data logger, listens for UDP output from RF24Gateway & logs to an MQTT topic
For use with RF24Mesh nodes that are not able to communicate directly via TCP/IP etc
See RF24Gateway examples:
                           RF24NetworkFrame frame = RF24NetworkFrame(header, buf, size);
                           gw.sendUDP(mesh.getNodeID(header.from_node), frame);
"""

import socket
import paho.mqtt.client as mqtt

### Setup the MQTT host IP & topic to publish to
mqttHost = "10.10.2.2"
topic = "data/RF24"

### Bind to UDP port 32001 (default port of RF24Gateway UDP output)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = ("localhost", 32001)
print("Waiting for UDP message on {} port {}" % server_address)
sock.bind(server_address)

while True:
    data, address = sock.recvfrom(2048)

    print("received {} bytes from {}".format(len(data), address))
    print(data)

    if data:
        # Just log the entire chunk of data to MQTT for now
        # TODO: Sort, Display and Analyze the data
        mqttc = mqtt.Client()
        mqttc.connect(mqttHost, 1883)
        data = data.decode(errors="replace")
        mqttc.publish(topic, data)
        mqttc.loop(2)
