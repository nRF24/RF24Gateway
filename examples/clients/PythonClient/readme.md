pyClient.py example for use with RF24Ethernet/RF24Gateway

Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
the sensor nodes.

This is a simple example demonstrating how to connect directly to a sensor node running one of the 
included server examples via python and control LEDs/lighting.

Usage:
1. Configure the schedule and IP of the sensor by editing pyClient.py
2. Run sudo ./pyClient.py
3. Run ' cat /var/log/syslog | grep pyClient.py ' to view the output
