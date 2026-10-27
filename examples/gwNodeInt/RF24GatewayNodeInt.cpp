

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <RF24Mesh/RF24Mesh.h>
#include <RF24Gateway/RF24Gateway.h>

//RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24 radio(22, 0);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
RF24Gateway gw(radio, network, mesh);

// AES-CCM encryption (compatible with nRF52/nRF54 nodes using nrf_to_nrf with radio.enableEncryption = true):
// Replace the four declarations above with the ones below, and uncomment the encryption lines in main().
// - RF24Network, RF24Mesh and RF24Gateway must be built with -DRF24NETWORK_CCM=ON (install.sh asks)
// - Every node must use the same key. Nodes without encryption, e.g. Arduino + nRF24L01, can't communicate
//   with an encrypting gateway, so run them on a different channel
// - Each radio frame carries 12 bytes of data instead of 24, so throughput is lower. nrf_to_nrf nodes must keep
//   32 byte frames: don't call radio.enableDynamicPayloads(123) on them
//#include <RF24Network/RF24CCM.h>
//RF24CCM radio(22, 0);
//ESBNetwork<RF24CCM> network(radio);
//ESBMesh<ESBNetwork<RF24CCM>, RF24CCM> mesh(radio, network);
//ESBGateway<ESBMesh<ESBNetwork<RF24CCM>, RF24CCM>, ESBNetwork<RF24CCM>, RF24CCM> gw(radio, network, mesh);
// Replace with your own random key, e.g. from: head -c16 /dev/urandom | xxd -i
//uint8_t encryptionKey[RF24CCM_KEY_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
//                                           0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

void intHandler()
{

    //Handle RF24Network reads/routing and read/writes of the tun_nrf24 TUN/TAP interface via interrupt
    gw.update(true);
}

uint32_t mesh_timer = 0;

int main(int argc, char** argv)
{

    //AES-CCM encryption (see RF24CCM above): set the key and enable it before gw.begin()
    //radio.setKey(encryptionKey);
    //radio.enableEncryption = true;

    //Config for use with RF24Mesh as Master Node
    //uint8_t nodeID=0;
    gw.begin();

    //Config for use with RF24Mesh as child Node
    // uint8_t nodeID = 1;
    // gw.begin(nodeID);

    //Config for use without RF24Mesh
    // uint16_t RF24NetworkAddress = 0;
    // gw.begin(RF24NetworkAddress);

    //Set this to your chosen IP/Subnet
    char ip[] = "10.10.2.2";
    char subnet[] = "255.255.255.0";

    gw.setIP(ip, subnet);
    radio.maskIRQ(1, 1, 0);
    attachInterrupt(23, INT_EDGE_FALLING, intHandler);

    uint32_t failCounter = 0;
    uint32_t failTimer = 0;

    while (1) {

        // The gateway handles all IP traffic (marked as EXTERNAL_DATA_TYPE) and passes it to the associated network interface
        // RF24Network user payloads are loaded into the user cache

        if (network.available()) {
            RF24NetworkHeader header;
            size_t size = network.peek(header);
            uint8_t buf[size];
            network.read(header, &buf, size);
            printf("Received Network Message, type: %d id %d from %d\n", header.type, header.id, mesh.getNodeID(header.from_node));
        }

        if (millis() - mesh_timer > 30000 && mesh.getNodeID() > 0) { //Every 30 seconds, test mesh connectivity
            mesh_timer = millis();
            if (!mesh.checkConnection()) {
                //refresh the network address
                mesh.renewAddress();
            }
        }
        // The following code can be un-commented in a production environment to limit the authorized nodes
        // to a specified list of nodeIDs. Any nodes not in this list will be de-allocated. Can be run on a timer.
        // AUTHORIZED NODES LIST:
        /*for(int i=0; i<mesh.addrListTop; i++){
            if(mesh.addrList[i].nodeID != 3 && mesh.addrList[i].nodeID != 4 && mesh.addrList[i].nodeID != 5){
                mesh.addrList[i].address = 0;
            }
        }*/

        //This section checks for failures detected by RF24 & RF24Network as well as
        //checking for deviations from the default configuration (1MBPS data rate)
        //The mesh is restarted on failure and failure count logged to failLog.txt
        //This makes the radios hot-swappable, disconnect & reconnect as desired, it should come up automatically
        if (millis() - failTimer > 1000) {
            failTimer = millis();
            if (radio.failureDetected > 0 || radio.getDataRate() != RF24_1MBPS) {
                radio.failureDetected = 0;
                std::ofstream myFile;
                myFile.open("failLog.txt");
                if (myFile.is_open()) {
                    myFile << ++failCounter << "\n";
                    myFile.close();
                }
                delay(500);
                mesh.begin();
            }
        }

        //When using interrupts, gw.poll(); needs to be called to handle incoming data from the network interface.
        gw.poll(1);
    }
    return 0;
}
