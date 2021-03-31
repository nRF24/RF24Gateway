nodeClient.js example for use with RF24Ethernet/RF24Gateway

Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
the sensor nodes.

This is a simple example demonstrating how to connect directly to a sensor node running one of the
included server examples via a NodeJS script and control LEDs/lighting.

Usage:
1. Edit nodeClient.js and configure the URL
   ```text
   <Sensor IP>:<port>/<page to request = OFF or ON>
   ```
2. From the example directory, run
   ```shell
   npm install request
   ```
3. Run
   ```shell
   node nodeClient.js
   ```

Note: The status code `200` indicates success
