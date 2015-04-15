bClient.sh example for use with RF24Ethernet/RF24Gateway

Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
the sensor nodes.

This is a simple example demonstrating how to connect directly to a sensor node running one of the 
included server examples via a bash script and control LEDs/lighting.

Usage:
1. Edit bClient.sh and configure the options
2. Run sudo ./bClient.sh <IP OF NODE> <ON or OFF>  ie: ./bClient.sh 10.10.2.4 ON

