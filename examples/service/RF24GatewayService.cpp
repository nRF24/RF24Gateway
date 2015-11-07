/* RF24GatewayService - enabled the RF24 Gateway to run as
 * a service on the pi. 
 *
 */

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <RF24Mesh/RF24Mesh.h>  
#include <RF24Gateway/RF24Gateway.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

using namespace std;

void display_usage();
void process();

#define DAEMON_NAME "RF24Gateway"

struct globalArgs_t {
    int verbosity;                /* -v option */
    int daemon;                    /* -d option */  
    int cePin;                    /* -c option */
    int csPin;                    /* -s option */
    int channel;                  /* -o option */
    char const *ip;                    /* -i option */
    char const *subnet;                /* -u option */
} globalArgs;

static const char *optString = "c:s:i:u:odvh?";

static const struct option longOpts[] = {
    { "cepin", no_argument, NULL, 'c' },
    { "cspin", no_argument, NULL, 's' },
    { "ip", no_argument, NULL, 'i' },
    { "subnet", no_argument, NULL, 'u' },
    { "daemon", no_argument, NULL, 'd' },
    { "channel", no_argument, NULL, 'o' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

RF24 radio(22,0);
RF24Network network(radio);
RF24Mesh mesh(radio,network);
RF24Gateway gw(radio,network,mesh);

int main( int argc, char *argv[] )
{
    int opt = 0;
    int longIndex = 0;
    
    /* Initialize globalArgs before we get to work. */
    globalArgs.daemon = 0;        /* false */
    globalArgs.cePin = 22;
    globalArgs.csPin = 0;
    globalArgs.ip = "10.10.2.2";
    globalArgs.subnet = "255.255.255.0";
    globalArgs.verbosity = 0;
    globalArgs.channel = 60;
    
    /* Process the arguments with getopt_long(), then 
     * populate globalArgs. 
     */
    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    while( opt != -1 ) {
        switch( opt ) {
            case 'c':
                globalArgs.cePin = atoi(optarg);
                break;
            case 's':
                globalArgs.csPin = atoi(optarg);
                break;
            case 'o':
                globalArgs.channel = atoi(optarg);
                break;
            case 'i':
                globalArgs.ip = optarg;
                break;
            case 'u':
                globalArgs.subnet = optarg;
                break; 
            case 'd':
                globalArgs.daemon++;
                break;
            case 'v':
                globalArgs.verbosity++;
                break;
            case 'h':    /* fall-through is intentional */
            case '?':
                display_usage();
                break;
            default:
                /* You won't actually get here. */
                break;
        }
        
        opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    }

    printf( "TEST \n" );    
    if(!globalArgs.daemon) {
        /* Print configuration */
        printf( "    CE Pin: %d\n", globalArgs.cePin );
        printf( "    CS Pin: %d\n", globalArgs.csPin );
        printf( "IP Address: %s\n", globalArgs.ip );
        printf( "    Subnet: %s\n", globalArgs.subnet );
        printf( " Verbosity: %d\n", globalArgs.verbosity );
        printf( "    Daemon: %d\n", globalArgs.daemon );
        printf( "   Channel: %d\n", globalArgs.channel );
    } else {
        //Set our Logging Mask and open the Log
        setlogmask(LOG_UPTO(LOG_NOTICE));
        openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
        
        syslog(LOG_INFO, "Entering Daemon");
        pid_t pid, sid;
    
        //Fork the Parent Process
        pid = fork();
    
        if (pid < 0) { exit(EXIT_FAILURE); }
    
        //We got a good pid, Close the Parent Process
        if (pid > 0) { exit(EXIT_SUCCESS); }
    
        //Change File Mask
        umask(0);
    
        //Create a new Signature Id for our child
        sid = setsid();
        if (sid < 0) { exit(EXIT_FAILURE); }
    
        //Change Directory
        //If we cant find the directory we exit with failure.
        if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }
    
        //Close Standard File Descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);        

    }
    
   
    printf("Starting Execution\n");


    radio.setChannel(globalArgs.channel);
    gw.begin();
    gw.setIP(const_cast<char*>(globalArgs.ip), const_cast<char*>(globalArgs.subnet));
	printf("GW setup at, type: %s:%s \n",globalArgs.ip, globalArgs.subnet);
	
	  
    //----------------
    //Main Process
    //----------------
    while(true){
        // The gateway handles all IP traffic (marked as EXTERNAL_DATA_TYPE) and passes it to the associated network interface
        // RF24Network user payloads are loaded into the user cache
        gw.update();
    
        if(network.available()) {
            RF24NetworkHeader header;
            size_t size = network.peek(header);
            uint8_t buf[size];
            network.read(header,&buf,size);
            if(globalArgs.verbosity >= 1) {
                printf("Received Network Message, type: %d id %d \n",header.type,header.id);
            }
        }
		
		if (0) { //Every 30 seconds, test mesh connectivity
            for(int i=0; i<mesh.addrListTop; i++) {
                printf(" Address: 0%o ID: %d \n",mesh.addrList[i].address,mesh.addrList[i].nodeID);
            }		
        }
    }
    
    if(globalArgs.daemon) {
        closelog ();
    }
    
    return EXIT_SUCCESS;
}

void process() {
    syslog (LOG_NOTICE, "Writing to my Syslog");
}   



/* Display program usage, and exit.
 */
void display_usage()
{
    exit( EXIT_FAILURE );
}
