

# longer range TCPIP via NRF24L01 for a pair of Raspberry PIs

## Intro

This tutorial is trying to use `NRF24L01` to create a TCP/IP link between two Raspberry Pi boards.
Due to long range of NRFs, some of those have 1800 meter wireless range, it would be good to have a TCPIP link between two RPi via NRFs so it would be possible to have a TCPIP connection between two RPi in longer range. The onboard wifi of RPi cannot do long ranges like 50 meter even in clear sight. 

## Hardware Configuration

1- wiring: Here is how i did connect the module to the RPi as described in the main RF24 documentation [here](https://nrf24.github.io/RF24/#autotoc_md227).
2- Noise on 3.3v on RPi: put some capacitor or L-C filter to reduce the noise on 3.3v supply from RPi.
3- Shielding NRF24 module: shield your NRF module, if it have not. (More info [here](http://embedblog.eu/?p=187) and [here](https://forum.mysensors.org/topic/3615/long-range-transmission-with-nrf24l01-pa-lna))

## Software Configuration

1- Enable SPI from `raspi-config': more detail [here](https://pimylifeup.com/raspberry-pi-spi/). then a reboot.
2- install RF24 drivers on RPi: 
more detail [here](https://nrf24.github.io/RF24/md_docs_2linux__install.html). 

```
sudo apt-get update
sudo apt-get upgrade
wget https://raw.githubusercontent.com/nRF24/.github/main/installer/install.sh
chmod +x install.sh
./install.sh
```
Installer will promp which modules you want to install. I did installed all modules: "RF24", "RF24Network" "RF24Mesh" "RF24Gateway". Also please select `SPIDEV` driver during installation.

after installation done, and if there are no errors in the process, there will be these directories inside the RPi:

```text
~/rf24libs/RF24
~/rf24libs/RF24Network
~/rf24libs/RF24Gateway
~/rf24libs/RF24Mesh
```

Next we need to choose a master/primary node (as discussed [here](https://github.com/nRF24/RF24Gateway/issues/41)). so one RPi will be primary, and another one will be secondary. we'll use the official example named `ncurses` in [RF24 repo](https://github.com/nRF24/RF24Gateway/tree/master/examples/ncurses) to establish the network.
this code is already cloned to local device in process of installation. so we need to have some edits on the code. On the Master/Primary machine no need to do edits, but on the secondary machine we need to edit `~/rf24libs/RF24Gateway/examples/ncurses/RF24Gateway_ncurses.cpp` file, first lines of method `main()`

Before edit (first lines)
```cpp
int main()
{

    gw.begin();
    //mesh.setStaticAddress(8, 1);

    //uint8_t nodeID = 22;
    //gw.begin(nodeID,3,RF24_2MBPS);

    //uint16_t address = 0;
    //gw.begin(address,3,RF24_2MBPS);
```
after edit:
```cpp
int main()
{

    //gw.begin();
    //mesh.setStaticAddress(8, 1);

    uint8_t nodeID = 3;
    gw.begin(nodeID);

    //uint16_t address = 0;
    //gw.begin(address,3,RF24_2MBPS);
```

Again, the above edit is only done in the secondary machine, the primary machine needs no edits.

then need to recompile the ncurses example, run in terminal:

```text
cd ~/rf24libs/RF24Gateway/examples/build
make
```

### Primary machine config

```text
sudo ip tuntap add dev tun_nrf24 mode tun user pi multi_queue
sudo ifconfig tun_nrf24 10.11.2.2/24
```

### Secondary machine config

```text
sudo ip tuntap add dev tun_nrf24 mode tun user pi multi_queue
sudo ifconfig tun_nrf24 10.11.2.3/24
```

### Run the ncurses example on both machines

```text
cd ~/rf24libs/RF24Gateway/examples/build/ncurses
./RF24Gateway_ncurses
```

Done. The primary machine IP is `10.11.2.2`, and the secondary machine IP is `10.11.2.3`.
One could ping machines from each other.

The resulting latency when pinging primary machine from secondary is about a few milliseconds (or even less than a millisecond), and the speed is about `10kB/s` (equal to 100K bits per second).

