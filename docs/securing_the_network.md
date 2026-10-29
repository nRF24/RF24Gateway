# Securing the Network

@tableofcontents

RF24Gateway bridges a wireless RF24Mesh network onto a Linux TUN/TAP interface (`tun_nrf24` by default). Anything that can transmit
on the same channel with compatible radio settings can try to join the mesh and send traffic into the Linux host, and from there
onto your LAN or the internet. By default nothing on the Linux side limits what that traffic can do.

This document describes a layered approach to locking the gateway down:

1. A basic firewall that only allows MQTT and ICMP traffic across the RF24 interface
2. Blocking IPv6 on the RF24 interface
3. Enabling IPv4 reverse path filtering (`rp_filter`) to reject spoofed source addresses
4. Restricting which RF24Mesh nodeIDs are allowed to hold an address, using the commented-out code in the examples
5. Encrypting and authenticating all radio traffic with AES-CCM

These steps are independent; apply as many as suit your deployment. The commands below assume the default interface name
`tun_nrf24` and a Debian-based OS such as Raspberry Pi OS. Run them on the device running RF24Gateway.

@warning Test these settings from a local console, or make sure your SSH session does not depend on the `tun_nrf24` interface,
before making them permanent. A firewall mistake can lock you out of the device.

## Basic Firewall: Allow Only MQTT and ICMP

Most RF24Ethernet sensor nodes only need to publish and subscribe to an MQTT broker, and answer pings for diagnostics. The rules below
drop all other traffic entering or leaving the mesh through `tun_nrf24`. Traffic on other interfaces (eth0, wlan0, loopback) is not affected.

The rules are kept in a dedicated chain, `RF24-FW`, so they are easy to inspect and remove:

```shell
# Create a chain for traffic that crosses the RF24 interface
sudo iptables -N RF24-FW

# Allow replies to connections that were already accepted
sudo iptables -A RF24-FW -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT

# Allow ICMP (ping, and the "fragmentation needed" messages that path MTU discovery relies on)
sudo iptables -A RF24-FW -p icmp -j ACCEPT

# Allow new MQTT connections (TCP port 1883)
sudo iptables -A RF24-FW -p tcp --dport 1883 -m conntrack --ctstate NEW -j ACCEPT

# Drop everything else
sudo iptables -A RF24-FW -j DROP

# Send all traffic to, from, and through tun_nrf24 into the chain
sudo iptables -I INPUT 1 -i tun_nrf24 -j RF24-FW
sudo iptables -I OUTPUT 1 -o tun_nrf24 -j RF24-FW
sudo iptables -I FORWARD 1 -i tun_nrf24 -j RF24-FW
sudo iptables -I FORWARD 1 -o tun_nrf24 -j RF24-FW
```

What these rules do:

- **INPUT** covers traffic from mesh nodes to the gateway itself, such as nodes connecting to a Mosquitto broker running on the gateway.
- **FORWARD** covers traffic routed through the gateway, such as nodes connecting to a broker elsewhere on the LAN, or LAN hosts pinging a node.
- **OUTPUT** covers traffic the gateway itself sends into the mesh.
- The `ESTABLISHED,RELATED` rule lets replies flow back without opening any extra ports.
- `DROP` is used instead of `REJECT` so the gateway does not spend radio airtime sending rejection packets back over the mesh.

The default port 1883 is plain-text MQTT. If your broker uses MQTT over TLS, allow port 8883 instead of (or as well as) 1883.
If your nodes need other services (DNS on UDP port 53, NTP on UDP port 123, HTTP to a node's web server, etc.), add an `ACCEPT` rule for
each one above the final `DROP` rule, for example:

```shell
sudo iptables -I RF24-FW 4 -p udp --dport 53 -j ACCEPT
```

@note The RF24Network UDP messages used by `gw.sendUDP()` in the examples go over the loopback interface, not `tun_nrf24`, so these rules do not block them.

### Checking the Firewall

```shell
sudo iptables -L RF24-FW -v -n
```

The packet counters on each rule show what is being accepted and dropped. From a mesh node, pinging the gateway and connecting to the MQTT broker
should work, while connections to other ports (for example SSH on port 22 at the gateway's mesh IP) should time out.

### Making the Firewall Permanent

iptables rules are lost on reboot. To save them:

```shell
sudo apt install iptables-persistent
sudo netfilter-persistent save
```

Rules are saved to `/etc/iptables/rules.v4` (and `rules.v6` for the IPv6 rules below) and restored at boot. Because the rules match
the interface by name, they take effect as soon as RF24Gateway creates `tun_nrf24`, even though the interface does not exist when the rules are loaded.

To remove the firewall:

```shell
sudo iptables -D INPUT -i tun_nrf24 -j RF24-FW
sudo iptables -D OUTPUT -o tun_nrf24 -j RF24-FW
sudo iptables -D FORWARD -i tun_nrf24 -j RF24-FW
sudo iptables -D FORWARD -o tun_nrf24 -j RF24-FW
sudo iptables -F RF24-FW
sudo iptables -X RF24-FW
```

## Blocking IPv6

RF24Gateway and RF24Ethernet only use IPv4, but Linux may still enable IPv6 on the `tun_nrf24` interface, and the IPv4 firewall above does not
apply to IPv6 traffic at all. Block it in two ways.

First, drop all IPv6 traffic on the interface with ip6tables:

```shell
sudo ip6tables -I INPUT 1 -i tun_nrf24 -j DROP
sudo ip6tables -I OUTPUT 1 -o tun_nrf24 -j DROP
sudo ip6tables -I FORWARD 1 -i tun_nrf24 -j DROP
sudo ip6tables -I FORWARD 1 -o tun_nrf24 -j DROP
sudo netfilter-persistent save
```

Second, disable IPv6 on the interface itself so it never gets an IPv6 address:

```shell
# For the interface while it exists
sudo sysctl -w net.ipv6.conf.tun_nrf24.disable_ipv6=1
```

Since RF24Gateway creates `tun_nrf24` after boot, a per-interface setting in a config file is not applied at boot. To have it applied
automatically, disable IPv6 by default for newly created interfaces (see [Making the sysctl Settings Permanent](#rf24gw_sysctl_permanent)):

```text
net.ipv6.conf.default.disable_ipv6 = 1
```

@note `net.ipv6.conf.default` affects every interface created after it is set, including USB network adapters, VPN interfaces, and Docker interfaces.
If you do not use IPv6 anywhere on the device, you can disable it completely with `net.ipv6.conf.all.disable_ipv6 = 1`. Make sure you are
not connected over IPv6 before doing so.

Check the result with:

```shell
ip -6 addr show dev tun_nrf24
```

which should print nothing.

## Enabling IPv4 Reverse Path Filtering {#rf24gw_rp_filter}

Reverse path filtering makes the kernel check the source address of every incoming packet: if the reply to that packet would not be sent
back out the same interface it arrived on, the packet is dropped. This stops a mesh node from sending packets that pretend to come from your LAN
(or the internet), and stops LAN hosts from pretending to be mesh nodes.

Enable strict mode (`1`) for all interfaces:

```shell
sudo sysctl -w net.ipv4.conf.all.rp_filter=1
sudo sysctl -w net.ipv4.conf.default.rp_filter=1
```

The kernel uses the higher of the `all` value and the per-interface value, so setting `all` to `1` applies strict mode to `tun_nrf24` even though
it is created later.

On the master gateway, RF24Gateway also checks traffic arriving from the radio side: an IPv4 packet is only accepted if the sender currently holds an
address assigned by the master, and its source IP ends in the sender's own nodeID (or, for addresses outside the mesh subnet, the sender is listed as the
gateway for that subnet in `routing.txt`). `rp_filter` adds the same kind of protection at the Linux level, for every interface.

### rp_filter and the Advanced Configuration

Strict mode requires the kernel's routing table to know about every network that should arrive on `tun_nrf24`. In the simple configuration
(everything in one subnet such as 10.1.3.0/24) this is automatic.

In the Advanced Configuration described on the main page, where child RF24Gateway nodes route extra subnets (for example 10.1.5.0/24 behind 10.1.3.33):

- On the **master**, the kernel needs a route to each of those subnets via `tun_nrf24`. If the interface netmask already covers them
  (for example 10.1.3.1/16 covers 10.1.5.x), nothing else is needed. Otherwise add a route, for example:

  ```shell
  sudo ip route add 10.1.5.0/24 dev tun_nrf24
  ```

- On **child** gateways, traffic from your LAN or the internet arrives on `tun_nrf24`, so the child's route back to those networks (usually its default
  route) must also go via `tun_nrf24`.

If a correct route is not practical, use loose mode (`2`) instead. Loose mode only drops packets whose source address is not reachable through
any interface, which still blocks obviously bogus source addresses:

```shell
sudo sysctl -w net.ipv4.conf.all.rp_filter=2
```

To see packets that are being dropped by `rp_filter`, temporarily enable logging and watch the kernel log:

```shell
sudo sysctl -w net.ipv4.conf.all.log_martians=1
sudo dmesg -w | grep -i martian
```

### Making the sysctl Settings Permanent {#rf24gw_sysctl_permanent}

`sysctl -w` changes are lost on reboot. To keep them, create `/etc/sysctl.d/90-rf24gateway.conf` containing:

```text
# Reverse path filtering (1 = strict, 2 = loose)
net.ipv4.conf.all.rp_filter = 1
net.ipv4.conf.default.rp_filter = 1

# No IPv6 on newly created interfaces, including tun_nrf24
net.ipv6.conf.default.disable_ipv6 = 1
```

then load it with:

```shell
sudo sysctl --system
```

Confirm the values with `sysctl net.ipv4.conf.all.rp_filter net.ipv6.conf.tun_nrf24.disable_ipv6`.

## Only Allowing Specific Mesh nodeIDs

The firewall controls *what* traffic can cross the gateway. The gateway examples also include a commented-out block that controls *which nodes*
are allowed to hold an address on the mesh. It can be found in:

- `examples/RF24GatewayNode.cpp`
- `examples/gwNodeInt/RF24GatewayNodeInt.cpp`
- `examples/ncurses/RF24Gateway_ncurses.cpp`
- `examples/ncursesInt/RF24Gateway_ncursesInt.cpp`

```cpp
// The following code can be un-commented in a production environment to limit the authorized nodes
// to a specified list of nodeIDs. Any nodes not in this list will be de-allocated. Can be run on a timer.
// AUTHORIZED NODES LIST:
/*for(int i=0; i<mesh.addrListTop; i++){
    if(mesh.addrList[i].nodeID != 3 && mesh.addrList[i].nodeID != 4 && mesh.addrList[i].nodeID != 5){
        mesh.addrList[i].address = 0;
    }
}*/
```

### How It Works

The master node (nodeID 0) acts as the DHCP server for the mesh. Every time a node joins, it requests an address and the master records the pair in
`mesh.addrList`, an array of `{nodeID, address}` entries. `mesh.addrListTop` is the number of entries in the list.

The block above runs through the whole list on every pass of the main loop. Any entry whose nodeID is not 3, 4 or 5 has its address set to `0`, which is
the same thing `mesh.releaseAddress()` does: it marks the entry as no longer holding an address. Once released:

- `mesh.getAddress(nodeID)` no longer returns a valid address for that node, so RF24Gateway does not forward any TCP/IP traffic to it.
- `mesh.getNodeID(address)` no longer finds the node, so the master gateway drops IP traffic arriving from it (see [rp_filter](#rf24gw_rp_filter) above).
- The node's own connectivity check (`mesh.checkConnection()`) eventually fails. When it asks for a new address, the master assigns one, and the
  allowlist releases it again on the next pass.

The result is that only nodes 3, 4 and 5 can exchange TCP/IP traffic through the gateway.

### Enabling It

1. The code only has an effect on the **master** gateway, since only the master holds the address list.
2. Remove the `/*` and `*/` around the loop.
3. Replace `3`, `4` and `5` with the nodeIDs you use. With RF24Ethernet nodes, the nodeID is the last octet of the node's IP address, so node 10.1.3.4 is nodeID 4.
4. Rebuild and restart the gateway.

For more than a few nodes, a list is easier to maintain than a chain of `!=` comparisons:

```cpp
// Before main()
const uint8_t authorizedNodes[] = { 3, 4, 5, 22 };

// In the main loop, after gw.update() / gw.poll()
for (int i = 0; i < mesh.addrListTop; i++) {
    bool authorized = false;
    for (uint8_t id : authorizedNodes) {
        if (mesh.addrList[i].nodeID == id) {
            authorized = true;
            break;
        }
    }
    if (!authorized) {
        mesh.addrList[i].address = 0;
    }
}
```

The comment says the check "can be run on a timer" to save CPU time. Keep the interval short: an unauthorized node holds a valid address from the
moment the master assigns it until the next time the check runs. Running it on every loop, right after the gateway processes DHCP requests, gives the smallest window.

### Limitations

The nodeID is chosen by the node itself and sent in its address request; nothing proves a node is who it says it is. Anyone who knows (or guesses) an
authorized nodeID can configure a device with that ID and be treated as authorized. The allowlist is useful for keeping stray or misconfigured nodes off the
network, and limits which IP addresses can be used, but it is not authentication. Use it together with the firewall and `rp_filter` settings above,
and enable [encryption](#rf24gw_encryption) so that only nodes holding the network key can join at all.

## Encrypting the Radio Link {#rf24gw_encryption}

By default, everything sent over the radio is plain text: anyone with a compatible radio on the same channel can read the traffic, and
can inject frames of their own. RF24Gateway can encrypt and authenticate every radio frame with AES-128 in CCM mode, using the same over-the-air
format as the encryption built into the nRF52/nRF54 radio peripheral (and the nrf_to_nrf library).

Each frame carries a random IV, a packet counter, and a 4-byte message integrity code (MIC). A receiver drops any frame that was not encrypted with
the shared key or was modified in transit. A node that does not have the key cannot read traffic, join the mesh, or get an address.

### Supported Devices

| Device                                                        | Radio driver                            | Encryption support                                               |
|---------------------------------------------------------------|-----------------------------------------|------------------------------------------------------------------|
| Linux (Raspberry Pi, etc.) with an nRF24L01(+) module         | RF24, via the `RF24CCM` wrapper         | Yes: software AES. This is how RF24Gateway itself is encrypted   |
| nRF52840, nRF52833, nRF52832                                  | nrf_to_nrf (v1.5.1 or newer)            | Yes: uses the chip's hardware CCM peripheral                      |
| nRF54L15                                                      | nrf_to_nrf, with the [nrf54l15clean core](https://github.com/lolren/nrf54-arduino-core) | Yes: uses the core's `Ccm` and `CracenRng` classes |
| Arduino (AVR, ESP8266, ESP32, etc.) with an nRF24L01(+) module | RF24                                    | **No**                                                           |

The key points:

- **All nodes on an encrypted network must use encryption with the same key.** Encryption cannot be enabled for some nodes and not others on
  the same channel. Unencrypted nodes, such as an Arduino Uno with an nRF24L01, cannot communicate with an encrypting gateway. If you need both,
  run them as two separate networks on different channels, each with its own gateway.
- A Linux node running RF24Gateway (the master, or a child gateway in the advanced configuration) encrypts in software with `RF24CCM`.
- nRF52/nRF54 nodes encrypt in hardware with nrf_to_nrf. nrf_to_nrf versions before v1.5.1 only encrypted the first two bytes of each payload on nRF52
  devices, so update before enabling encryption.

### Building With Encryption Support

Encryption support is a compile-time option, `RF24NETWORK_CCM`, which must be enabled for RF24Network, RF24Mesh and RF24Gateway.

- **Using the installer:** answer `y` when `install.sh` asks "Build with CCM encryption support (RF24NETWORK_CCM)".
- **Building manually with CMake:** add `-DRF24NETWORK_CCM=ON` when configuring each of the three libraries, then build and install them in order:

  ```shell
  cmake -B build -S . -DRF24NETWORK_CCM=ON
  cmake --build build
  sudo cmake --install build
  ```

This installs `RF24Network/RF24CCM.h` and builds the library templates for the `RF24CCM` radio type. Libraries built without the option still work as
before; they just cannot be used with `RF24CCM`.

### Enabling Encryption on the Gateway

The gateway examples (for example `examples/RF24GatewayNode.cpp`) include the required code, commented out. Replace the usual radio, network,
mesh and gateway declarations with their `RF24CCM` versions, and set a key before calling `gw.begin()`:

```cpp
#include <RF24Network/RF24CCM.h>

RF24CCM radio(22, 0);
ESBNetwork<RF24CCM> network(radio);
ESBMesh<ESBNetwork<RF24CCM>, RF24CCM> mesh(radio, network);
ESBGateway<ESBMesh<ESBNetwork<RF24CCM>, RF24CCM>, ESBNetwork<RF24CCM>, RF24CCM> gw(radio, network, mesh);

// Replace with your own random key, e.g. from: head -c16 /dev/urandom | xxd -i
uint8_t encryptionKey[RF24CCM_KEY_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                           0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

int main(int argc, char** argv)
{
    // Set the key and enable encryption before gw.begin()
    radio.setKey(encryptionKey);
    radio.enableEncryption = true;

    gw.begin();
    // ...
}
```

`RF24CCM` is a drop-in replacement for `RF24`, so the rest of the program does not change. Encryption must be enabled before `gw.begin()` because
`begin()` sets the radio's retry settings, and because mesh address requests are encrypted too.

Generate a key with:

```shell
head -c16 /dev/urandom | xxd -i
```

and never use the example key. Anyone who has the key can read and inject traffic, so keep it out of public source repositories.

### Enabling Encryption on nRF52/nRF54 Nodes

On nrf_to_nrf nodes, set the same key and enable encryption before `mesh.begin()`:

```cpp
#include <nrf_to_nrf.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

uint8_t myKey[16] = { /* the same 16 bytes as the gateway's key */ };

nrf_to_nrf radio;
RF52Network network(radio);
RF52Mesh mesh(radio, network);

void setup() {
  radio.begin();
  radio.setKey(myKey);
  radio.enableEncryption = true;

  mesh.setNodeID(1);
  mesh.begin(90, RF24_1MBPS); // same channel and data rate as the gateway
}
```

@warning The nrf_to_nrf encryption examples call `radio.enableDynamicPayloads(254)` (or `123`) to use the nRF52's larger frames. **Do not do this
on nodes that talk to an RF24Gateway with an nRF24L01.** The nRF24L01 can only receive frames of up to 32 bytes, so the nodes must keep the default
32-byte frames.

If a node cannot connect (`mesh.begin()` keeps failing), check that the key, channel and data rate match the gateway exactly. A key mismatch looks the
same as being out of range.

### Performance

Every encrypted frame carries 12 bytes of overhead (5-byte IV, 3-byte packet counter, 4-byte MIC). With 32-byte frames and the 8-byte RF24Network
header, each frame holds 12 bytes of data instead of 24, so:

- Messages take about twice as many radio frames, and throughput is roughly halved.
- Longer messages are more likely to need fragmentation.
- While encryption is enabled, `RF24CCM` raises the radio's automatic retry delay and count to at least 15 (`radio.minRetryDelay` and `radio.minRetryCount`),
  since nrf_to_nrf nodes only acknowledge a frame when the sketch polls the radio. Without this, multi-frame exchanges such as connecting to an MQTT broker
  often fail. Change these values before `gw.begin()` if needed.

Frames that are too short or fail authentication are dropped and counted in `radio.decryptFailures`. A steadily rising count means there is a node with
the wrong key (or no key) on the channel, or someone is sending invalid frames.

### What Encryption Does Not Cover

- **Replay:** receivers do not track packet counters, so an attacker can record a valid encrypted frame and send it again later. It will decrypt and be
  accepted. Use application-level protection (for example MQTT over TLS, or message sequence numbers) for commands where a replay could do harm.
- **Traffic after the gateway:** encryption only covers the radio link. Data is decrypted by the gateway and is plain text on `tun_nrf24`, your LAN, and
  the internet. The firewall and `rp_filter` settings above still apply.
- **A leaked key:** every node shares one key, so a key read out of one device's flash compromises the whole network. To change it, update every node and
  the gateway.
