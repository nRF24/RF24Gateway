nodeClient.js example for use with RF24Ethernet/RF24Gateway

Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
the sensor nodes.

This is a simple example demonstrating how to connect directly to a sensor node running one of the 
included server examples via a NodeJS script and control LEDs/lighting.

Usage:
1. a: Edit nodeClient.js and configure the URL( Sensor IP, page to request (OFF,ON)
   b: From the example directory, run ```npm install request```
2. Run ```node nodeClient.js```
3. Status code: 200 indicates success

