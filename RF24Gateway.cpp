/**
 * @file RF24Gateway.cpp
 *
 * Class definitions for RF24Gateway
 */

#include "RF24Gateway.h"
#include "RF24Mesh/RF24Mesh_config.h"

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
ESBGateway<mesh_t, network_t, radio_t>::ESBGateway(radio_t& _radio, network_t& _network, mesh_t& _mesh) : radio(_radio), network(_network), mesh(_mesh)
{
    gotInterrupt = false;
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::begin(uint8_t nodeID, uint8_t _channel, rf24_datarate_e data_rate)
{
    mesh_enabled = true;
    begin(true, mesh_enabled, 0, nodeID, data_rate, _channel);
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::begin(uint16_t address, uint8_t _channel, rf24_datarate_e data_rate, bool meshEnable, uint8_t nodeID)
{
    begin(0, mesh_enabled, address, nodeID, data_rate, _channel);
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
bool ESBGateway<mesh_t, network_t, radio_t>::begin(bool configTUN, bool meshEnable, uint16_t address, uint8_t mesh_nodeID, rf24_datarate_e data_rate, uint8_t _channel)
{
#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
    printf("GW Begin\n");
    printf("Config Device address 0%o nodeID %d\n", address, mesh_nodeID);
#endif
    config_TUN = configTUN;

    /// FIX

    channel = _channel; // 97;

    dataRate = data_rate;

    configDevice(address);
    mesh_enabled = meshEnable;
    thisNodeID = mesh_nodeID;
    thisNodeAddress = address;

    if (meshEnable) {
        // GW radio channel setting takes precedence over mesh_default_channel
        if (channel == 97 && MESH_DEFAULT_CHANNEL != 97) {
            channel = MESH_DEFAULT_CHANNEL;
        }

        if (!thisNodeAddress && !mesh_nodeID) {
            mesh.setNodeID(0);
        }
        else {
            if (!mesh_nodeID) {
                mesh_nodeID = 253;
            }
            mesh.setNodeID(mesh_nodeID); // Try not to conflict with any low-numbered node-ids
        }
        mesh.begin(channel, data_rate);
        thisNodeAddress = mesh.mesh_address;
    }
    else {
        radio.begin();
        delay(5);
        const uint16_t this_node = address;
        radio.setDataRate(dataRate);
        radio.setChannel(channel);

        network.begin(/*node address*/ this_node);
        thisNodeAddress = this_node;
    }
    network.multicastRelay = 1;

    //#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
    radio.printDetails();
    //#endif

    setupSocket();
    loadRoutingTable();

    return true;
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::loadRoutingTable()
{
    std::ifstream infile("routing.txt", std::ifstream::in);
    if (!infile) {
        return;
    }

    std::string str;
    std::string ip, mask, gw;
    uint16_t count = 0;
    std::string space = " ";

    while (std::getline(infile, str)) {
        size_t startLen = 0;
        size_t subLen = str.find(space);
        if (subLen != std::string::npos) {
            ip = str.substr(0, subLen);
        }
        else {
            continue;
        }
        startLen = subLen + 1;
        subLen = str.find(space, startLen);
        if (subLen != std::string::npos) {
            subLen -= (startLen);
            mask = str.substr(startLen, subLen);
        }
        else {
            continue;
        }
        startLen = startLen + subLen + 1;
        subLen = str.length() - (startLen);
        gw = str.substr(startLen, subLen);

        routingStruct[count].ip.s_addr = ntohl(inet_network(ip.c_str()));
        routingStruct[count].mask.s_addr = ntohl(inet_network(mask.c_str()));
        routingStruct[count].gw.s_addr = ntohl(inet_network(gw.c_str()));

        count++;
        if (count >= 256) {
            break;
        }
    }
    routingTableSize = count;

    // for(int i=0; i<count; i++){
    //     std::cout << inet_ntoa(routingStruct[i].ip) << ";" << inet_ntoa(routingStruct[i].mask) << ";" << inet_ntoa(routingStruct[i].gw) << std::endl;
    // }
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
bool ESBGateway<mesh_t, network_t, radio_t>::meshEnabled()
{
    return mesh_enabled;
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
int ESBGateway<mesh_t, network_t, radio_t>::configDevice(uint16_t address)
{
    std::string tunTapDevice = "tun_nrf24";
    strcpy(tunName, tunTapDevice.c_str());

    int flags;
    if (config_TUN) {
        flags = IFF_TUN | IFF_NO_PI | IFF_MULTI_QUEUE;
    }
    else {
        flags = IFF_TAP | IFF_NO_PI | IFF_MULTI_QUEUE;
    }
    tunFd = allocateTunDevice(tunName, flags, address);
#if RF24GATEWAY_DEBUG_LEVEL >= 1
    if (tunFd >= 0) {
        std::cout << "RF24Gw: Successfully attached to tun/tap device " << tunTapDevice << std::endl;
    }
    else {
        std::cerr << "RF24Gw: Error allocating tun/tap interface: " << tunFd << std::endl;
        exit(1);
    }
#endif
    return tunFd;
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
int ESBGateway<mesh_t, network_t, radio_t>::allocateTunDevice(char* dev, int flags, uint16_t address)
{
    struct ifreq ifr;
    int fd;

    // open the device
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags; // IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI

    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    // Create device
    if (ioctl(fd, TUNSETIFF, (void*)&ifr) < 0) {
        // close(fd);
        //#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
        std::cerr << "RF24Gw: Error: enabling TUNSETIFF" << std::endl;
        uint32_t UID = getuid();
        if (UID) {
            std::cout << "Not running as root, preconfigure the interface as follows" << std::endl;
            std::cout << "sudo ip tuntap add dev tun_nrf24 mode tun user " << getlogin() << " multi_queue" << std::endl;
            std::cout << "sudo ifconfig tun_nrf24 10.10.2.2/24" << std::endl;
        }
        return -1;
        //#endif
    }

    // Make persistent
    if (ioctl(fd, TUNSETPERSIST, 1) < 0) {
#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
        std::cerr << "RF24Gw: Error: enabling TUNSETPERSIST" << std::endl;
#endif
        return -1;
    }

    if (!config_TUN) {
        struct sockaddr sap;
        sap.sa_family = ARPHRD_ETHER;
        ((char*)sap.sa_data)[4] = address;
        ((char*)sap.sa_data)[5] = address >> 8;
        ((char*)sap.sa_data)[0] = 0x52;
        ((char*)sap.sa_data)[1] = 0x46;
        ((char*)sap.sa_data)[2] = 0x32;
        ((char*)sap.sa_data)[3] = 0x34;

        // printf("Address 0%o first %u last %u\n",address,sap.sa_data[0],sap.sa_data[1]);
        memcpy((char*)&ifr.ifr_hwaddr, (char*)&sap, sizeof(struct sockaddr));

        if (ioctl(fd, SIOCSIFHWADDR, &ifr) < 0) {
#if RF24GATEWAY_DEBUG_LEVEL >= 1
            fprintf(stderr, "RF24Gw: Failed to set MAC address\n");
#endif
        }
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
int ESBGateway<mesh_t, network_t, radio_t>::setIP(char* ip_addr, char* mask)
{
    struct ifreq ifr;
    struct sockaddr_in sin;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Could not get socket.\n");
        return -1;
    }

    sin.sin_family = AF_INET;
    // inet_aton(ip_addr,&sin.sin_addr.s_addr);
    inet_aton(ip_addr, &sin.sin_addr);
    strncpy(ifr.ifr_name, tunName, IFNAMSIZ);

    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        fprintf(stderr, "ifdown: shutdown ");
        perror(ifr.ifr_name);
        return -1;
    }

#ifdef ifr_flags
    #define IRFFLAGS ifr_flags
#else /* Present on kFreeBSD */
    #define IRFFLAGS ifr_flagshigh
#endif

    if (!(ifr.IRFFLAGS & IFF_UP)) {
        // fprintf(stdout, "Device is currently down..setting up.-- %u\n", ifr.IRFFLAGS);
        ifr.IRFFLAGS |= IFF_UP;
        if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
            fprintf(stderr, "ifup: failed ");
            perror(ifr.ifr_name);
            return -1;
        }
    }

    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

    // Set interface address
    if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
        fprintf(stderr, "Cannot set IP address. ");
        perror(ifr.ifr_name);
        return -1;
    }

    inet_aton(mask, &sin.sin_addr);
    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

    if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
        fprintf(stderr, "Cannot define subnet mask for this device");
        perror(ifr.ifr_name);
        return -1;
    }

#undef IRFFLAGS
    return 0;
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::interrupts(bool enable)
{
    // No longer required
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::update(bool interrupts)
{

    if (interrupts) {
        gotInterrupt = true;
    }
    else {
        handleRadioIn();
        handleTX();
        handleRX();
        handleRadioOut();
    }
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::poll(uint32_t waitDelay)
{
    if (!gotInterrupt && radio.available()) {
        fifoCleared = true;
    }
    else {
        if (!gotInterrupt) {
            delay(waitDelay);
        }
    }
    gotInterrupt = false;
    handleRadioIn();
    handleTX();
    handleRX();
    handleRadioOut();
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::handleRadioIn()
{

    uint8_t returnVal = 0;

    if (mesh_enabled) {
        while ((returnVal = mesh.update())) {
            if (returnVal == NETWORK_OVERRUN) {
                ++networkOverruns;
            }
            else if (returnVal == NETWORK_CORRUPTION) {
                ++networkCorruption;
            }
            if (!thisNodeAddress) {
                mesh.DHCP();
            }
        }
    }
    else {
        while ((returnVal = network.update())) {
            if (returnVal == NETWORK_OVERRUN) {
                ++networkOverruns;
            }
            else if (returnVal == NETWORK_CORRUPTION) {
                ++networkCorruption;
            }
        }
    }

    RF24NetworkFrame f;
    while (network.external_queue.size() > 0) {
        f = network.external_queue.front();

        msgStruct msg;

        if (f.message_size > 0) {
            memcpy(&msg.message, &f.message_buffer, f.message_size);
            msg.size = f.message_size;

#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
            std::cout << "Radio: Received " << f.message_size << " bytes ... " << std::endl;
#endif
#if (RF24GATEWAY_DEBUG_LEVEL >= 3)
            // printPayload(msg.getPayloadStr(),"radio RX");
            std::cout << "TunRead: " << std::endl;
            for (size_t i = 0; i < msg.size; i++) {
                // std::cout << std::hex << buffer[i];
                printf(":%0x :", msg.message[i]);
            }
            std::cout << std::endl;

#endif

            rxQueue.push(msg);
        }
        else {
            // std::cerr << "Radio: Error reading data from radio. Read '" << f.message_size << "' Bytes." << std::endl;
        }
        network.external_queue.pop();
    }
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
struct in_addr ESBGateway<mesh_t, network_t, radio_t>::getLocalIP()
{
    struct ifaddrs *ifap, *ifa;
    int family, s, n;
    char host[NI_MAXHOST];
    struct in_addr myNet;
    memset(&myNet, 0, sizeof(myNet));

    getifaddrs(&ifap);
    for (ifa = ifap, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
        if (std::string("tun_nrf24").compare(ifa->ifa_name) != 0 || ifa->ifa_addr == NULL) {
            if (ifa->ifa_next == NULL) {
                break;
            }
            else {
                continue;
            }
        }

        family = ifa->ifa_addr->sa_family;

        // This is an IPv4 interface, get the IP
        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s == 0) {
                myNet.s_addr = ntohl(inet_network(host));
                freeifaddrs(ifap);
                return myNet;
            }
        }
    }
    freeifaddrs(ifap);
    return myNet;
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::handleRadioOut()
{
    bool ok = 0;

    uint32_t txQueueTimer = millis() + 750;

    while (!txQueue.empty() && network.external_queue.size() == 0) {

        uint32_t queueSize = txQueue.size();
        if (millis() > txQueueTimer && queueSize >= 10) {
            for (uint32_t i = 0; i < queueSize; i++) {
                droppedIncoming++;
                txQueue.pop();
            }
            return;
        }
        msgStruct* msgTx = &txQueue.front();

#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
        std::cout << "Radio: Sending " << msgTx->size << " bytes ... ";
        std::cout << std::endl;
#endif
#if (RF24GATEWAY_DEBUG_LEVEL >= 3)

        // PrintDebug == 1 does not have an endline.
        // printPayload(msg.getPayloadStr(),"radio TX");
#endif

        std::uint8_t* tmp = msgTx->message;

        if (!config_TUN) { // TAP can use RF24Mesh for address assignment, but will still use ARP for address resolution

            uint32_t RF24_STR = 0x34324652; // Identifies the mac as an RF24 mac
            uint32_t ARP_BC = 0xFFFFFFFF;   // Broadcast address
            struct macStruct
            {
                uint32_t rf24_Verification;
                uint16_t rf24_Addr;
            };

            macStruct macData;
            memcpy(&macData.rf24_Addr, tmp + 4, 2);
            memcpy(&macData.rf24_Verification, tmp, 4);

            if (macData.rf24_Verification == RF24_STR) {
                const uint16_t other_node = macData.rf24_Addr;
                RF24NetworkHeader header(/*to node*/ other_node, EXTERNAL_DATA_TYPE);
                ok = network.write(header, &msgTx->message, msgTx->size);
            }
            else if (macData.rf24_Verification == ARP_BC) {
                RF24NetworkHeader header(/*to node*/ 00, EXTERNAL_DATA_TYPE); // Set to master node, will be modified by RF24Network if multi-casting
                if (msgTx->size <= 42) {
                    if (thisNodeAddress == 00) { // Master Node

                        uint32_t arp_timeout = millis();

                        ok = network.multicast(header, &msgTx->message, msgTx->size, 1); // Send to Level 1
                        while (millis() - arp_timeout < 5) {
                            uint8_t returnVal = network.update();
                            if (returnVal == NETWORK_OVERRUN) {
                                ++networkOverruns;
                            }
                            else if (returnVal == NETWORK_CORRUPTION) {
                                ++networkCorruption;
                            }
                        }
                        network.multicast(header, &msgTx->message, msgTx->size, 1); // Send to Level 1
                        arp_timeout = millis();
                        while (millis() - arp_timeout < 15) {
                            uint8_t returnVal = network.update();
                            if (returnVal == NETWORK_OVERRUN) {
                                ++networkOverruns;
                            }
                            else if (returnVal == NETWORK_CORRUPTION) {
                                ++networkCorruption;
                            }
                        }
                        network.multicast(header, &msgTx->message, msgTx->size, 1); // Send to Level 1
                    }
                    else {
                        ok = network.write(header, &msgTx->message, msgTx->size);
                    }
                }
            }
        }
        else { // TUN always needs to use RF24Mesh for address assignment AND resolution

            uint8_t lastOctet = tmp[19];
            int16_t meshAddr;

            RF24NetworkHeader header(00, EXTERNAL_DATA_TYPE);
            bool sendData = false;

            struct in_addr ipDestination;
            memcpy(&ipDestination.s_addr, &tmp[16], 4);

            if ((getLocalIP().s_addr & 0x00FFFFFF) == (ipDestination.s_addr & 0x00FFFFFF)) { // Is inside the RF24Mesh network
                if ((meshAddr = mesh.getAddress(lastOctet)) > 0) {
                    header.to_node = meshAddr;
                    sendData = true;
                }
                else {
                    if (thisNodeID > 0) { // If IP is in mesh range, address lookup fails, and this is not master,
                        sendData = true;  // send to 00 anyway in case destination is master, or the lookup just failed
                    }
                    // printf("Could not find matching mesh nodeID for IP ending in %d\n",lastOctet);
                }
            }
            else if (thisNodeID > 0) { // If not master, send to master for routing etc. if target not within mesh
                sendData = true;
            }
            else if (routingTableSize > 0) {
                for (int i = 0; i < routingTableSize; i++) {
                    struct in_addr network;
                    network.s_addr = routingStruct[i].ip.s_addr & routingStruct[i].mask.s_addr;
                    struct in_addr destNet;
                    destNet.s_addr = ipDestination.s_addr & routingStruct[i].mask.s_addr;
                    // printf("network %s destNet: %s\n",inet_ntoa(network),inet_ntoa(destNet));
                    if (destNet.s_addr == network.s_addr) {
                        uint8_t toNode = routingStruct[i].gw.s_addr >> 24;
                        int16_t netAddr = 0;
                        if ((netAddr = mesh.getAddress(toNode)) > 0) {
                            header.to_node = netAddr;
                            sendData = true;
                            break;
                        }
                    }
                }
            }

            if (sendData) {
                ok = network.write(header, msgTx->message, msgTx->size);
                // std::cout << "SendData " << header.to_node << std::endl;
            }
        }
        // delay( rf24_min(msgTx->size/48,20));
        txQueue.pop();

        // printf("Addr: 0%#x\n",macData.rf24_Addr);
        // printf("Verif: 0%#x\n",macData.rf24_Verification);
        if (ok) {
            // std::cout << "ok." << std::endl;
        }
        else {
            // std::cerr << "failed." << std::endl;
        }

    } // End Tx
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::handleRX(uint32_t waitDelay)
{
    fd_set socketSet;
    struct timeval selectTimeout;
    uint8_t buffer[MAX_PAYLOAD_SIZE];
    int nread;

    FD_ZERO(&socketSet);
    FD_SET(tunFd, &socketSet);

    selectTimeout.tv_sec = 0;
    selectTimeout.tv_usec = waitDelay * 1000;

    uint32_t tunTimeout = millis();

    while (select(tunFd + 1, &socketSet, NULL, NULL, &selectTimeout) != 0) {
        if (FD_ISSET(tunFd, &socketSet)) {
            if ((nread = read(tunFd, buffer, MAX_PAYLOAD_SIZE)) >= 0) {
#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
                std::cout << "Tun: Successfully read " << nread << " bytes from tun device" << std::endl;
#endif
#if (RF24GATEWAY_DEBUG_LEVEL >= 3)
                std::cout << "TunRead: " << std::endl;
                for (int i = 0; i < nread; i++)
                {
                    printf(":%0x :", buffer[i]);
                }
                std::cout << std::endl;
#endif
                msgStruct msg;
                memcpy(&msg.message, &buffer, nread);
                msg.size = nread;
                txQueue.push(msg);
            }
            else {
#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
                std::cerr << "Tun: Error while reading from tun/tap interface." << std::endl;
#endif
            }
        }
        if (millis() - tunTimeout > 750) {
            return;
        }
    }
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::handleTX()
{

    if (rxQueue.size() < 1)
    {
        return;
    }
    msgStruct* msg = &rxQueue.front();

    if (msg->size > MAX_PAYLOAD_SIZE)
    {
        // printf("*****WTF OVER *****");
        rxQueue.pop();
        return;
    }

    if (msg->size > 0)
    {

        size_t writtenBytes = write(tunFd, &msg->message, msg->size);
        if (writtenBytes != msg->size)
        {
// std::cerr << "Tun: Less bytes written to tun/tap device then requested." << std::endl;
#if RF24GATEWAY_DEBUG_LEVEL >= 1
            printf("Tun: Less bytes written %d to tun/tap device then requested %d.", writtenBytes, msg->size);
#endif
        }
        else
        {
#if (RF24GATEWAY_DEBUG_LEVEL >= 1)
            std::cout << "Tun: Successfully wrote " << writtenBytes << " bytes to tun device" << std::endl;
#endif
        }

#if (RF24GATEWAY_DEBUG_LEVEL >= 3)
        // printPayload(msg.message,"tun write");
        std::cout << "TunRead: " << std::endl;
        for (size_t i = 0; i < msg->size; i++) {
            // printf(":%0x :",msg->message[i]);
        }
        std::cout << std::endl;
#endif
    }

    rxQueue.pop();
}

/***************************************************************************************

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::printPayload(std::string buffer, std::string debugMsg)
{
}
*/
/***************************************************************************************

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::printPayload(char* buffer, int nread, std::string debugMsg)
{
}
*/
/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::setupSocket()
{
    int ret;
    const char* myAddr = "127.0.0.1";

    addr.sin_family = AF_INET;
    ret = inet_aton(myAddr, &addr.sin_addr);
    if (ret == 0) {
        perror("inet_aton");
        exit(1);
    }
    addr.sin_port = htons(32001);
    // buf = "Hello UDP";
    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s == -1) {
        perror("socket");
        exit(1);
    }
}

/***************************************************************************************/

template<class mesh_t, class network_t, class radio_t>
void ESBGateway<mesh_t, network_t, radio_t>::sendUDP(uint8_t nodeID, RF24NetworkFrame frame)
{

    uint8_t buffer[MAX_PAYLOAD_SIZE + 11];

    memcpy(&buffer[0], &nodeID, 1);
    memcpy(&buffer[1], &frame.header, 8);
    memcpy(&buffer[9], &frame.message_size, 2);
    memcpy(&buffer[11], &frame.message_buffer, frame.message_size);

    int ret = sendto(s, &buffer, frame.message_size + 11, 0, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("sendto");
        exit(1);
    }
}

// ensure the compiler is aware of the possible datatype for the template class
template class ESBGateway<ESBMesh<ESBNetwork<RF24>, RF24>, ESBNetwork<RF24>, RF24>;
