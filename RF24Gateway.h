

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
  #include <linux/if.h>
  #include <linux/if_tun.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/ioctl.h>
  
  #include <RF24/RF24.h>
  #include <RF24Network/RF24Network.h>
  #include <RF24Mesh/RF24Mesh.h>
  
#ifndef IFF_MULTI_QUEUE
	#define IFF_MULTI_QUEUE 0x0100
#endif  

#define DEBUG 0
  
  class RF24;
  class RF24Network;
  class RF24Mesh;
  
class RF24Gateway {

  /**@}*/
  /**
   * @name RF24Gateway (RPi/Linux)
   *
   *  RF24Gateway library for devices with an IP stack
   */
  /**@{*/

public:

  /**
   * RF24Gateway constructor.
   */
  RF24Gateway(RF24& _radio,RF24Network& _network, RF24Mesh& _mesh);
  
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
  void begin(uint8_t nodeID=0, uint8_t channel=97,rf24_datarate_e data_rate=RF24_1MBPS);
  
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
  void begin(uint16_t address, uint8_t channel=97, rf24_datarate_e data_rate=RF24_1MBPS, bool meshEnable=0, uint8_t nodeID=0 );
  
  /**
   * Once the Gateway has been started via begin() , call setIP to configure the IP and
   * subnet mask.
   *
   * @param ip A character array containing the numeric IP address ie: 192.168.1.1
   * @param mask A character array containing the subnet mask ie: 255.255.255.0
   * @return -1 if failed, 0 on success
   */
  int setIP( char *ip_addr, char *mask);
  
  /**
   * Calling update() keeps the network and mesh layers active and processing data. This needs to be called regularly.
   * @code
   * gw.update();
   * if(network.available()){
   *  ...do something
   * }
   * @endcode
   */
  void update(); 
  
  uint16_t thisNodeAddress; /**< Address of our node in Octal format (01,021, etc) */
  uint8_t thisNodeID;  /**< NodeID (0-255) */
  
  
  bool meshEnabled(); /**< Is RF24Mesh enabled? */
  bool config_TUN; /**< Using a TAP(false) or TUN(true) interface */
  
  uint32_t ifDropped(){
    return droppedIncoming;
  }
  
private:
  RF24& radio;
  RF24Network& network;
  RF24Mesh& mesh;
  
  bool begin(bool configTUN, bool meshEnable, uint16_t address, uint8_t mesh_nodeID, rf24_datarate_e data_rate, uint8_t _channel);
  bool mesh_enabled;
  
  uint32_t droppedIncoming;
  
  uint8_t channel;
  rf24_datarate_e dataRate;
  char tunName[IFNAMSIZ];
  int tunFd;
  
  unsigned long packets_sent;  /**< How many have we sent already */
  uint32_t interfaceInTimer;
  
  void handleRadio();
  void handleRX();
  void handleTX();
   
  int configDevice(uint16_t address);  
  int allocateTunDevice(char *dev, int flags, uint16_t address);
  
  struct msgStruct{
    std::uint8_t message[MAX_PAYLOAD_SIZE];
	std::size_t size;
  };
  
  std::queue<msgStruct>rxQueue;
  std::queue<msgStruct>txQueue;
  
  void printPayload(std::string buffer, std::string debugMsg = "");
  void printPayload(char *buffer, int nread, std::string debugMsg = "");
};
  
  
  

/**
 * @example RF24GatewayNode.cpp
 *
 * A simple example of using RF24Gateway node to forward IP traffic automatically to a network interface, while
 * managing standard RF24Network user payloads independently.
 */
 
 /**
 * @example RF24Gateway_ncurses.cpp
 *
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
 *
 * @image html ncurses.JPG
 */
 
 /** @example bClient.sh
 *
 * Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
 * the sensor nodes. <br>Example of on demand LED/Lighting control using a Bash script.
 */
 
 /** @example nodeClient.js
 *
 * Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
 * the sensor nodes. <br> Example of on demand LED/Lighting control using a NodeJS script.
 */

 /** @example pyClient.py
 *
 * Once RF24Gateway and RF24Ethernet are configured, standard tools can be used to interact with
 * the sensor nodes. <br> Example of scheduled LED/Lighting control using a Python script.
 */
 
/**
 * @mainpage RF24Gateway
 *
 * RF24Gateway is a library style replacement for the standalone RF24toTUN application, used with RF24Ethernet.
 *
 * In addition to passing TCP/IP payloads to a network interface, RF24Gateway also allows users to send and receive
 * standard RF24Network payloads.
 *
 * This allows users to deploy and manage hybrid networks, consisting of nodes communicating via TCP/IP and RF24Network Messages
 *
 * @section Installation Installation
 *
 * Installation instructions are included with the RF24Ethernet documentation at http://tmrh20.github.io/RF24Ethernet 
 *
 *
 * Note: Forwarding/NAT may need to be configured<br>
 * Note: RF24Gateway is tested with and defaults to using RF24Mesh. Sensor nodes must also use RF24Mesh with the default configuration.
 *
 * 
 */
 
 
 #endif