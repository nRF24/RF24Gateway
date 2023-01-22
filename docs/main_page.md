# RF24Gateway

@tableofcontents

RF24Gateway is a library style replacement for the standalone RF24toTUN application, used with RF24Ethernet.

In addition to passing TCP/IP payloads to a network interface, RF24Gateway also allows users to send and receive
standard RF24Network payloads.

This allows users to deploy and manage hybrid networks, consisting of nodes communicating via TCP/IP and RF24Network Messages

## Installation

See http://nRF24.github.io/RF24 for installation instructions using the installer, or clone the RF24 libs and run 'make install' for each one.

After running the installer, RF24Gateway will be installed at rf24libs/RF24Gateway. [**Examples**](examples.html) are included for operating the gateway
as well as client scripts & programs to control nodes via http requests.

Note: RF24Gateway is tested with and defaults to using RF24Mesh. Sensor nodes must also use RF24Mesh or be statically assigned within RF24Mesh.

See http://nRF24.github.io/RF24Ethernet/ for related documentation for use with RF24Gateway.

## How It Works

RF24Gateway is relatively simple in design, and uses the [Universal TUN/TAP driver for Linux](https://www.kernel.org/doc/Documentation/networking/tuntap.txt)

Data incoming from the RF24 communication stack (designated as external data) is passed to the interface without modification. Conversely, incoming TCP/IP or other data
received on the TUN/TAP interface is passed directly into the radio network and designated as external data.

Linux devices running RF24Gateway can establish network links using the built-in TCP/IP stack, and users can include Arduino/AVR devices or additional RPi devices to automatically
extend the wireless range.

Arduino/AVR devices, etc must utilize a software IP stack such as uIP.

## Simple Configuration

In the standard configuration, a single Linux/RF24Gateway node is used to provide connectivity to one or more
Arduino nodes running RF24Ethernet.
In this case the master node could be configured with IP 10.1.3.1 netmask 255.255.255.0 and nodeID 0
The remaining Arduino nodes can used nodeIDs 2-253, and would be assigned IP addresses 10.1.3.2-253 and netmask 255.255.255.0

If there are problems starting the examples, the following commands can be run to configure the interface:

```shell
ip tuntap add dev tun_nrf24 mode tun user pi multi_queue
ifconfig tun_nrf24 10.1.3.1/24
```

To enable nat and routing, the following commands can also be run:

```shell
sudo sysctl -w net.ipv4.ip_forward=1
sudo iptables -t nat -A POSTROUTING -j MASQUERADE
```

See http://nRF24.github.io/RF24Ethernet/ConfigAndSetup.html for more info

## Advanced Configuration

In a more advanced configuration, users can use two or more nodes running RF24Gateway together, along with Arduino Nodes
running RF24Ethernet
In this case the master node could be configured differently with IP 10.1.3.1 netmask 255.255.0.0 and nodeID 0
The child RPi2 node could be assigned IP 10.1.3.33 netmask 255.255.0.0 and nodeID 33
Another device, lets say usb0 connected to the child RPi2 node is assigned IP 10.1.5.1 netmask 255.255.255.0

In this configuration, connectivity between all nodes in the mesh would be established, but a routing table is
needed to include attached and internet devices.

To fully enable routing beyond the RPi devices, a routing table needs to be added to the Master node:

1. Rename the file RF24Gateway/examples/routing.txx to routing.txt
2. Edit the file accordingly
3. Restart the gateway example to reload routing info

## Network Configuration Example

Local WiFi/Ethernet Network (Windows,Linux,RPi devices etc) - 10.10.1.0 - 255.255.255.0 ( 10.10.1.x network ) <br>
RPi devices (tun_nrf24 interfaces & all connected devices) - 10.1.0.0 - 255.255.0.0 ( 10.1.x.x network )

### Explanation

The RF24Gateway/RF24Mesh network supports about 253 hosts. I setup the RF24Gateway and all associated
devices with a much larger subnet in this config to encompass all devices directly accessible via the RF24Gateway network.

ie: RPi master 10.1.3.1/16, RPi2 child 10.1.3.33/16, RPi2 child usb0 10.1.5.1/24

### Routing Configuration

In the RF24Gateway example working directory, create a file "routing.txt".
Add routing entries in a standard kind of format:

```text
IP<space>NetMask<space>Gateway
```

Example routing.txt file:

- Traffic for 10.1.5.x will use 10.1.3.33 as the gateway

   ```text
   10.1.5.0 255.255.255.0 10.1.3.33
   ```

- Traffic for 10.1.4.x will use 10.1.3.34 as the gateway

   ```text
   10.1.4.0 255.255.255.0 10.1.3.34
   ```

- Traffic not in the RF24Gateway network will be sent via 10.10.3.33 by default

   ```text
   0.0.0.0 0.0.0.0 10.10.3.33
   ```

#### Additional Routing Configuration

You can specify any combination of IP/Netmask in the routing.txt file to create unique subnets for routing traffic.

Examples are:

- Traffic for 10.1.5.(1-30) will use 10.1.3.33 as the gateway

   ```text
   10.1.5.0 255.255.255.224 10.1.3.33
   ```

- Traffic for 10.1.5.(33-62) will use 10.1.3.34 as the gateway

   ```text
   10.1.5.32 255.255.255.224 10.1.3.34
   ```

- Traffic for 10.1.5.(65-94) will use 10.1.3.35 as the gateway

   ```text
   10.1.5.64 255.255.255.224 10.1.3.35
   ```

- Traffic not in the RF24Gateway network will be sent via 10.10.3.32 by default

   ```text
   0.0.0.0 0.0.0.0 10.10.3.32
   ```
