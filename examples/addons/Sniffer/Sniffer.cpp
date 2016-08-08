
/*
   RF24Network Scanner by TMRh20 Aug 2016
   Outputs data to console & UDP port 32001
   See the LUA script for Wireshark at https://github.com/TMRh20/RF24Gateway/tree/master/examples/addons
   
   Impersonate any RF24Network/RF24Mesh/RF24Gateway node & scan traffic without disruption to the network
*/

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <RF24Mesh/RF24Mesh.h>  
#include <RF24Gateway/RF24Gateway.h>

//RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ); 
RF24 radio(25,0);
RF24Network network(radio);
RF24Mesh mesh(radio,network);
RF24Gateway gw(radio,network,mesh);

uint8_t buffer[1000][32];
int sizes[1000];
int buffCtr = 0;
uint32_t mesh_timer = 0;



int main(int argc, char** argv) {

  radio.begin();
  radio.setAutoAck(0);
  
  // Because AutoAck is disabled, and we are not transmitting any payloads, we will not interfere with the network
  // Setup this node as a clone of any network node to scan for all payloads passing to or through that node
  gw.begin();
  
  
  
 while(1){
    
  // Buffer all payloads while the radio has data  
  while(radio.available()){
      sizes[buffCtr] = radio.getDynamicPayloadSize();
      radio.read(&buffer[buffCtr][0],sizes[buffCtr]);
      ++buffCtr;
  }
  
  // Dissect and print the data
  if(buffCtr){
      for(int i=0; i<buffCtr; i++){
          RF24NetworkHeader header;
          memcpy(&header,&buffer[i][0],8);

          // Send data to UDP output also
          RF24NetworkFrame frame = RF24NetworkFrame(header,&buffer[i][8],sizes[i]); 
          gw.sendUDP(0,frame);
          
          // Print the data
          printf("From: 0%o To: 0%o, hID: %d, Type: %d, Rsvd: %d\n",header.from_node,header.to_node,header.id,header.type,header.reserved);
          for(int j=8; j<sizes[i]; j++){
              printf("%02x",buffer[i][j]);
          }
          printf("\n");
          
      }
  }
  
  buffCtr = 0;
  
  while(!radio.available()){
      delay(2);
  }
  

}
}
