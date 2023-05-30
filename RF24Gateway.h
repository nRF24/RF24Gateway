

#ifndef __RF24GATEWAY_H__
#define __RF24GATEWAY_H__

/**
 * @file RF24Gateway.h
 *
 * Class declaration for RF24Gateway
 */
#include "net/if_arp.h"
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <fstream>

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <RF24Mesh/RF24Mesh.h>

#ifndef IFF_MULTI_QUEUE
    #define IFF_MULTI_QUEUE 0x0100
#endif

#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL 0
#endif // DEBUG_LEVEL

#define BACKLOG 10 /* Passed to listen() */

class RF24;

template<class radio_t>
class ESBNetwork;

template<class network_t, class radio_t>
class ESBMesh;

/**
 * @tparam mesh_t The `mesh` object's type. Defaults to `RF24Mesh` for legacy behavior.
 * This new abstraction is really meant for using the nRF52840 SoC as a drop-in replacement
 * for the nRF24L01 radio. For more detail, see the
 * [nrf_to_nrf Arduino library](https://github.com/TMRh20/nrf_to_nrf).
 * @tparam network_t The `network` object's type. Defaults to `RF24Network` for legacy behavior.
 * This new abstraction is really meant for using the nRF52840 SoC as a drop-in replacement
 * for the nRF24L01 radio. For more detail, see the
 * [nrf_to_nrf Arduino library](https://github.com/TMRh20/nrf_to_nrf).
 * @tparam radio_t The `radio` object's type. Defaults to `RF24` for legacy behavior.
 * This new abstraction is really meant for using the nRF52840 SoC as a drop-in replacement
 * for the nRF24L01 radio. For more detail, see the
 * [nrf_to_nrf Arduino library](https://github.com/TMRh20/nrf_to_nrf).
 */
template<class mesh_t = ESBMesh<ESBNetwork<RF24>, RF24>, class network_t = ESBNetwork<RF24>, class radio_t = RF24>
class ESBGateway
{

    /**
     * @name RF24Gateway (RPi/Linux)
     *
     *  RF24Gateway library for devices with an IP stack
     */
    /**@{*/

public:
    /**
     * ESBGateway constructor.
     */
    ESBGateway(radio_t& _radio, network_t& _network, mesh_t& _mesh);

    /**
     * Begin function for use with RF24Mesh (TUN interface)
     *
     * @param nodeID The RF24Mesh nodeID to use
     * @param channel The radio channel to use (1-127)
     * @param data_rate The RF24 datarate to use (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS)
     *
     * @code gw.begin(); //Start the gateway using RF24Mesh, with nodeID 0 (Master) @endcode
     * @code uint8_t nodeID; gw.begin(nodeID); //Start the gateway using RF24Mesh, with nodeID 1 (Child node) @endcode
     */
    void begin(uint8_t nodeID = 0, uint8_t channel = 97, rf24_datarate_e data_rate = RF24_1MBPS);

    /**
     * Begin function for use with a TAP (Ethernet) interface. RF24Mesh can be used for address assignment, but
     * ARP will be used to perform the lookups.
     *
     * @param address The RF24Network address to use
     * @param channel The radio channel (0-127) to use
     * @param data_rate The RF24 datarate to use (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS)
     * @param meshEnable Whether to use RF24Mesh for address assignment
     * @param nodeID The RF24Mesh nodeID to use **if** meshEnable has been set to true
     *
     * @code uint16_t address=00; gw.begin(address); //Start the gateway without using RF24Mesh, with RF24Network address 00 (Master) @endcode
     * @code uint16_t address=01; gw.begin(address); //Start the gateway without using RF24Mesh, with RF24Network address 01 (Child) @endcode
     */
    void begin(uint16_t address, uint8_t channel = 97, rf24_datarate_e data_rate = RF24_1MBPS, bool meshEnable = 0, uint8_t nodeID = 0);

    /**
     * Once the Gateway has been started via begin() , call setIP to configure the IP and
     * subnet mask.
     *
     * @param ip_addr A character array containing the numeric IP address ie: 192.168.1.1
     * @param mask A character array containing the subnet mask ie: 255.255.255.0
     * @return -1 if failed, 0 on success
     */
    int setIP(char* ip_addr, char* mask);

    /**
     * Calling update() keeps the network and mesh layers active and processing data. This needs to be called regularly.
     * @code
     * gw.update();
     * if(network.available()){
     *  ...do something
     * }
     * @endcode
     * @param interrupts Set true if called from an interrupt handler & call poll() from the main loop or a thread.
     */
    void update(bool interrupts = 0);

    /**
     * gw.poll(); needs to be called to handle incoming data from the network interface.
     * The function will perform a delayed wait of max 3ms unless otherwise specified.
     * @param waitDelay How long in milliseconds this function will wait for incoming data.
     */
    void poll(uint32_t waitDelay = 3);

    /**
     * When using interrupts (gwNodeInt, ncursesInt examples) users need to call
     * this function to disable interrupts before accessing the radio and again to
     * re-enable interrupts when complete
     * @param enable 0 to disable interrupts and access the radio, 1 to re-enable
     */
    void interrupts(bool enable = 1);

    /**@}*/
    /**
     * @name Advanced Operation
     *
     *  More advanced methods and options
     */
    /**@{*/

    uint16_t thisNodeAddress; /**< Address of our node in Octal format (01,021, etc) */
    uint8_t thisNodeID;       /**< NodeID (0-255) */

    bool meshEnabled(); /**< Is RF24Mesh enabled? */
    bool config_TUN;    /**< Using a TAP(false) or TUN(true) interface */
    bool fifoCleared;

    uint32_t ifDropped()
    {
        return droppedIncoming;
    }

    void sendUDP(uint8_t nodeID, RF24NetworkFrame frame);

    /**@}*/
    /**
     * @name Routing Table
     *
     * Utilizing a routing table to provide complete connectivity
     */
    /**@{*/

    /**
     * If a user has created a file "routing.txt" in the RF24Gateway working directory, it will be loaded <br>
     * at startup into the routing table. The file should contain standard routing table entries as follows: <br>
     * IP<space>NetMask<space>Gateway <br>
     * ie: routing.txt
     * @code
     * 10.1.4.0 255.255.255.0 10.1.3.33
     * 0.0.0.0 0.0.0.0 10.1.3.34 @endcode
     *
     * The first example entry would route all traffic to the 10.1.4.x subnet to 10.1.3.33 <br>
     * All other traffic not destined for the RF24 nodes will use 10.1.3.34 as the gateway
     *
     * Data can be accessed using standard linux Internet address manipulation routines as follows:
     * @code
     * printf("**IP\t\tMask\t\tGateway**\n");
     * for(int i=0; i<gw.routingTableSize; i++){
     *   printf("%s \t",inet_ntoa(gw.routingStruct[i].ip));//inet_ntoa uses a statically assigned buffer, so the printf calls need to be done separately
     *   printf("%s \t",inet_ntoa(gw.routingStruct[i].mask));
     *   printf("%s\n", inet_ntoa(gw.routingStruct[i].gw));
     *   //std::cout << inet_ntoa(gw.routingStruct[i].ip) << " \t" << inet_ntoa(gw.routingStruct[i].mask) << " \t" << inet_ntoa(gw.routingStruct[i].gw) << std::endl;
     * }
     * printf("*****\n");
     *
     * Users can modify the routing table as desired, but changes made in code will not currently be saved to file
     *
     * @endcode
     *
     */
    struct routeStruct
    {
        struct in_addr ip;
        struct in_addr mask;
        struct in_addr gw;
    };

    /**
     * The array that holds the routing structure data. See routeStruct
     */
    routeStruct routingStruct[256];

    /**
     * The size of the existing routing table loaded into memory. See routeStruct
     */
    uint8_t routingTableSize;

private:
    radio_t& radio;
    network_t& network;
    mesh_t& mesh;

    bool begin(bool configTUN, bool meshEnable, uint16_t address, uint8_t mesh_nodeID, rf24_datarate_e data_rate, uint8_t _channel);
    bool mesh_enabled;

    uint32_t droppedIncoming;

    uint8_t channel;
    rf24_datarate_e dataRate;
    char tunName[IFNAMSIZ];
    int tunFd;

    unsigned long packets_sent; /**< How many have we sent already */
    uint32_t interfaceInTimer;

    void handleRadioOut();
    void handleRadioIn();
    void handleRX(uint32_t waitDelay = 0);
    void handleTX();
    volatile bool interruptInProgress;
    volatile bool interruptsEnabled;

    int configDevice(uint16_t address);
    int allocateTunDevice(char* dev, int flags, uint16_t address);

    struct msgStruct
    {
        std::uint8_t message[MAX_PAYLOAD_SIZE];
        std::size_t size;
    };

    std::queue<msgStruct> rxQueue;
    std::queue<msgStruct> txQueue;

    // void printPayload(std::string buffer, std::string debugMsg = "");
    // void printPayload(char* buffer, int nread, std::string debugMsg = "");

    int s; //Socket variable for sending UDP
    void setupSocket();
    struct sockaddr_in addr;
    struct in_addr getLocalIP();

    void loadRoutingTable();
};

/**
 * A type definition of the template class `ESBGateway` to maintain backward compatibility.
 * 
 * ```.cpp
 * RF24 radio(7, 8);
 * RF24Network network(radio);
 * RF24Mesh mesh(radio, network);
 * 
 * RF24Gateway gateway(radio, network, mesh);
 * // is equivalent to
 * ESBGateway<ESBMesh<ESBNetwork<RF24>, RF24>, ESBNetwork<RF24>, RF24> gateway(radio, network, mesh);
 * ```
 */
typedef ESBGateway<ESBMesh<ESBNetwork<RF24>, RF24>, ESBNetwork<RF24>, RF24> RF24Gateway;

/**
 * @example RF24GatewayNode.cpp
 *
 * A simple example of using RF24Gateway node to forward IP traffic automatically to a network interface, while
 * managing standard RF24Network user payloads independently.
 */

/**
 * @example RF24Gateway_ncurses.cpp
 * RF24Gateway NCurses interface - TMRh20 2015 <br>
 * This is a generic tool for nodes supporting or combining with RF24Ethernet and/or RF24Network.
 *
 * The tool provides a simple interface for monitoring information and activity regarding the RF24Gateway: <br>
 * a: Interface statistics from /proc/net/dev <br>
 * b: RF24Mesh address/id assignments <br>
 * c: RF24Network/Radio information <br>
 * d: Active IP connections (optional) <br>
 *
 * **Requirements: NCurses** <br>
 * Install NCurses: apt-get install libncurses5-dev
 * Optional: Enable nf_conntrack: @code modprobe nf_conntrack_ipv4 @endcode
 *
 * @image html ncurses.JPG
 */

/**
 * @example RF24GatewayNodeInt.cpp
 * A copy of the RF24GatewayNode example using interrupts.
 */

/**
 * @example RF24Gateway_ncursesInt.cpp
 * A copy of the ncurses example using interrupts.
 */

/**
 * @example bClient.sh
 * Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
 * the sensor nodes. <br>Example of on demand LED/Lighting control using a Bash script.
 */

/**
 * @example nodeClient.js
 * Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
 * the sensor nodes. <br> Example of on demand LED/Lighting control using a NodeJS script.
 */

/**
 * @example pyClient.py
 * Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
 * the sensor nodes. <br> Example of scheduled LED/Lighting control using a Python script.
 */

#endif