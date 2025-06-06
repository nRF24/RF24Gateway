/**
 * RF24Gateway NCurses interface - TMRh20 2015-2022
 * This is a generic tool for nodes supporting or combining with RF24Ethernet and/or RF24Network.
 *
 * The tool provides a simple interface for monitoring information and activity regarding the RF24Gateway: <br>
 * a: Interface statistics from /proc/net/dev <br>
 * b: RF24Mesh address/id assignments <br>
 * c: RF24Network/Radio information <br>
 * d: Active IP connections (optional) <br>
 *
 * **Requirements: NCurses**
 * Install NCurses: apt-get install libncurses5-dev
 *
 * **Controls**
 * Type 'h' to view the help menu
 *
 *
 */

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <RF24Mesh/RF24Mesh.h>
#include <RF24Gateway/RF24Gateway.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>

#include <netdb.h>
#include <ncurses.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <linux/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include <time.h>

/******************************************************************/
//User Configuration
RF24 radio(22, 0);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
RF24Gateway gw(radio, network, mesh);

uint8_t nodeID = 0;
int interruptPin = 24;
/******************************************************************/

WINDOW* win;
WINDOW* meshPad;
WINDOW* connPad;
WINDOW* devPad;
WINDOW* rf24Pad;
WINDOW* cfgPad;
WINDOW* renewPad;

void drawMain(void);
void drawHelp(void);
void drawCfg(bool isConf);

void drawDevPad(void);
void drawMeshPad(void);
void drawConnPad(void);
void drawRF24Pad(void);
void drawTopology();
bool handleKeyPad();

int maxX, maxY;
int padSelection = 0;
int meshScroll = 0;
int connScroll = 0;
int rf24Scroll = 0;
unsigned long updateRate = 1000;

uint32_t meshInfoTimer = 0;
uint32_t fifoTimer;
uint32_t fifoClears;
uint32_t mesh_timer = 0;
size_t networkPacketsRX = 0;
std::string subIP;
std::string tunStr("tun_nrf24");
bool showConnPad = false;
bool topo = true;

float bRX;
float bTX;

/******************************************************************/
/***********************MAIN***************************************/
void intHandler()
{
    gw.update(true);
}

int main()
{

    gw.begin(nodeID);
    //mesh.setStaticAddress(8,1);

    //uint8_t nodeID = 22;
    //gw.begin(nodeID,3,RF24_2MBPS);

    //uint16_t address = 0;
    //gw.begin(address,3,RF24_2MBPS);

    /** Setup NCurses**/
    /*******************************/
    win = initscr();
    cbreak();
    noecho();
    getmaxyx(win, maxX, maxY);

    keypad(win, TRUE);
    start_color();
    curs_set(0);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);

    /** Setup Pads**/
    /*******************************/
    devPad = newpad(11, 40);
    meshPad = newpad(50, 50);
    rf24Pad = newpad(13, 40);
    connPad = newpad(21, 150);
    cfgPad = newpad(10, 40);
    renewPad = newpad(1, 35);

    scrollok(meshPad, true);
    scrollok(connPad, true);
    scrollok(rf24Pad, true);
    timeout(0);

    //MANUAL IP
    //char ip[] = "10.10.3.1";
    //char subnet[] = "255.255.255.0";
    //gw.setIP(ip,subnet);

    drawMain();
    if (!nodeID) {
        showConnPad = true;
    }
    radio.maskIRQ(1, 1, 0);
    attachInterrupt(interruptPin, INT_EDGE_FALLING, intHandler);

    /******************************************************************/
    /***********************LOOP***************************************/
    bool ok = true;
    uint32_t failCounter = 0;
    uint32_t failTimer = 0;

    while (1)
    {

        if (millis() - mesh_timer > 30000 && mesh.getNodeID() > 0)
        { //Every 30 seconds, test mesh connectivity
            if (!mesh.checkConnection())
            {
                werase(renewPad);
                mvwprintw(renewPad, 0, 0, "*Renewing Address*");
                prefresh(renewPad, 0, 0, 3, 26, 4, 55);
                ok = mesh.renewAddress() != MESH_DEFAULT_ADDRESS;
                if (ok)
                {
                    ok = true;
                    werase(renewPad);
                    prefresh(renewPad, 0, 0, 3, 26, 4, 55);
                }
            }
            mesh_timer = millis();
        }

        /** Read RF24Network Payloads (Do nothing with them currently) **/
        /*******************************/
        if (network.available())
        {
            ++networkPacketsRX;
            RF24NetworkHeader header;
            network.read(header, /*buf*/ 0, /*size*/ 0); // pop the RX payload from network.queue

            // send a timestamp to master
            struct timeStruct
            {
                uint8_t hr;
                uint8_t min;
            } myTime;

            time_t mTime;
            time(&mTime);
            struct tm* tm = localtime(&mTime);

            myTime.hr = tm->tm_hour;
            myTime.min = tm->tm_min;
            RF24NetworkHeader hdr(header.from_node, 1);
            network.write(hdr, &myTime, sizeof(myTime));
        }

        /**
         * The gateway handles all IP traffic (marked as EXTERNAL_DATA_TYPE) and passes it to the associated network interface
         * RF24Network user payloads are loaded into the user cache
         */
        gw.poll(1);

        /** Mesh address/id printout **/
        /*******************************/
        if (millis() - meshInfoTimer > updateRate)
        {

            getmaxyx(win, maxX, maxY);

            // Draw the pads on screen
            drawDevPad();
            prefresh(devPad, 0, 0, 4, 1, 15, 25);

            drawMeshPad();
            wscrl(meshPad, meshScroll);
            prefresh(meshPad, 0, 0, 4, 26, 15, 48);

            drawRF24Pad();
            prefresh(rf24Pad, 0, 0, 4, 52, 15, 78);

            if (showConnPad)
            {
                drawConnPad();
                wscrl(connPad, connScroll);
                prefresh(connPad, 0, 0, 17, 1, maxX - 1, maxY - 2);
            }
        } //MeshInfo Timer

        // Handles keyboard inputs etc.
        if (!handleKeyPad()) {
            return 0;
        }

        //This section checks for failures detected by RF24 & RF24Network as well as
        //checking for deviations from the default configuration (1MBPS data rate)
        //The mesh is restarted on failure and failure count logged to failLog.txt
        //This makes the radios hot-swappable, disconnect & reconnect as desired, it should come up automatically
        if (millis() - failTimer > 1000) {
            failTimer = millis();
            if (radio.failureDetected > 0 || radio.getDataRate() != RF24_1MBPS)
            {

                std::ofstream myFile;
                myFile.open("failLog.txt");
                if (myFile.is_open())
                {
                    myFile << ++failCounter << "\n";
                }
                myFile.close();
                mesh.begin();
                delay(1000);
                clear();
                drawMain();
                meshInfoTimer -= 1000;
                radio.failureDetected = 0;
            }
        }

    } //while 1

    erase();
    endwin();
    return 0;

} //main

/******************************************************************/
/******************Main Drawing Functions**************************/

void drawTopology()
{
    werase(connPad);
    wattroff(connPad, COLOR_PAIR(1));
    mvwhline(win, 16, 1, ACS_HLINE, maxY - 2);
    mvwprintw(win, 16, 10, "Mesh Topology");

    wattron(connPad, COLOR_PAIR(1));
    int connPadmaxX;
    connPadmaxX = getmaxx(connPad);

    for (int i = 01; i < 06; i++) {

        for (int j = 0; j < mesh.addrListTop; j++) {
            if (mesh.addrList[j].address == i) {
                wprintw(connPad, "0%o[%d]   |    ", mesh.addrList[j].address, mesh.addrList[j].nodeID);
            }
        }
    }
    wprintw(connPad, "\n");
    wattron(connPad, COLOR_PAIR(3));
    uint16_t g = 051;
    for (int h = 011; h <= 015; h++) {

        for (int i = h; i <= g; i += 010) {

            for (int j = 0; j < mesh.addrListTop; j++) {
                if (mesh.addrList[j].address == i) {
                    int x = getcurx(connPad);
                    if (x > connPadmaxX - 77) {
                        wprintw(connPad, "\n");
                    }
                    wprintw(connPad, "0%o[%d] ", mesh.addrList[j].address, mesh.addrList[j].nodeID);
                }
            }
        }
        g++;
        wprintw(connPad, "| ");
    }
    wprintw(connPad, "\n");
    wattron(connPad, COLOR_PAIR(4));
    g = 0411;
    for (int h = 0111; h <= 0145; h++) {

        for (int i = h; i <= g; i += 0100) {

            for (int j = 0; j < mesh.addrListTop; j++) {
                if (mesh.addrList[j].address == i) {
                    int x = getcurx(connPad);
                    if (x > connPadmaxX - 77) {
                        wprintw(connPad, "\n");
                    }
                    wprintw(connPad, "0%o[%d] ", mesh.addrList[j].address, mesh.addrList[j].nodeID);
                }
            }
        }
        g++;
    }
    wprintw(connPad, "\n");
    wattron(connPad, COLOR_PAIR(2));
    g = 04111;

    for (int h = 01111; h <= 01445; h++) {

        for (int i = h; i <= g; i += 01000) {

            for (int j = 0; j < mesh.addrListTop; j++) {
                if (mesh.addrList[j].address == i) {
                    int x = getcurx(connPad);
                    if (x > connPadmaxX - 77) {
                        wprintw(connPad, "\n");
                    }
                    wprintw(connPad, "0%o[%d] ", mesh.addrList[j].address, mesh.addrList[j].nodeID);
                }
            }
        }
        g++;
    }
}

void drawMain()
{

    erase();

    attron(COLOR_PAIR(1));
    wprintw(win, "RF24Gateway Ncurses Interface by TMRh20 - 2015-2022\n");
    whline(win, ACS_HLINE, maxY - 2);
    attroff(COLOR_PAIR(1));
    refresh();
    /** Display Network Interface Info **/
    /*******************************/

    //Interface Information
    struct ifaddrs *ifap, *ifa;
    int family, s, n;
    char host[NI_MAXHOST];

retryIF:

    getifaddrs(&ifap);
    for (ifa = ifap, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
    {
        if (tunStr.compare(ifa->ifa_name) != 0 || ifa->ifa_addr == NULL)
        {

            if (ifa->ifa_next == NULL)
            {
                drawCfg(0);
                goto retryIF;
            }
            else
            {
                continue;
            }
        }
        mvwprintw(win, 2, 0, "%8s ", ifa->ifa_name);

        family = ifa->ifa_addr->sa_family;

        //This is an IP interface, display the IP
        if (family == AF_INET || family == AF_INET6)
        {
            s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s == 0)
            {
                wprintw(win, "IP: %s\n", host);
                std::string str1 = host;
                unsigned found = str1.find_last_of(".");
                subIP = str1.substr(0, found);
                break;
            }
        }
    }

    mvwhline(win, 16, 1, ACS_HLINE, maxY - 2);

    refresh();
}

/******************************************************************/

void drawHelp()
{

    drawMain();

    attron(COLOR_PAIR(1));
    mvwprintw(win, 4, 1, "**RF24Gateway NCurses Help Menu**");
    wprintw(win, " Controls:\n");
    wprintw(win, " ARROW_UP/DOWN: Scroll up/down in supported menus\n");
    wprintw(win, " ARROW_LEFT/RIGHT: Scroll between supported menus\n");
    wprintw(win, " 'c' key: Open IP configuration menu\n");
    wprintw(win, " 't' key: Display topology of mesh\n");
    wprintw(win, " 'w' key: Increase frame-rate of display\n");
    wprintw(win, " 's' key: Decrease frame-rate of display\n");
    wprintw(win, " 'a' key: Display active IP connections\n");
    wprintw(win, " 'h' key: Display this menu\n");

    timeout(30000);
    getch();
    timeout(0);

    drawMain();
}

/******************************************************************/

void drawCfg(bool isConf)
{

    if (isConf)
    {
        drawMain();
    }
    nocbreak();
    echo();
    timeout(30000);

    sleep(1);
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 4, 1, isConf ? "IP Configuration\n" : "**Interface Not Configured:**\n");
    mvwprintw(win, 5, 1, "IP 10.10.2.2 and Subnet Mask 255.255.255.0 is default\n");
    wattroff(win, COLOR_PAIR(1));
    mvwprintw(win, 6, 1, "Enter IP Address: \n");
    refresh();

    char ip[20], mask[20];
    mvgetstr(6, 19, ip);

    mvwprintw(win, 7, 1, "Enter Subnet Mask: \n");
    refresh();

    mvgetstr(7, 20, mask);

    if (strlen(ip) >= 6 && strlen(mask) >= 7)
    {
        if (gw.setIP(ip, mask) < 0) {
            mvwprintw(win, 8, 1, "Unable to set IP/Subnet \n");
            uint32_t UID = getuid();
            if (UID) {
                mvwprintw(win, 9, 1, "Not running as root, configure as follows:\n");
                mvwprintw(win, 10, 1, "sudo ip tuntap add dev tun_nrf24 mode tun user %s multi_queue\n", getlogin());
                mvwprintw(win, 11, 1, "sudo ifconfig tun_nrf24 10.10.2.2/24\n");
            }
            refresh();
            sleep(10);
        }
    }
    else
    {
        mvwprintw(win, 8, 1, "Unable to set IP/Subnet \n");
        refresh();
        sleep(3);
    }

    timeout(0);
    cbreak();
    noecho();

    drawMain();
}

/******************************************************************/
/******************Curses Pad Functions****************************/

void drawDevPad()
{

    werase(devPad);

    std::string line;
    std::ifstream inFile;
    inFile.open("/proc/net/dev");
    while (inFile.good())
    {
        getline(inFile, line); // get line from file
        // search
        if (line.find(tunStr) != std::string::npos)
        {

            char txBytes[20], txPackets[20], txErrs[20];
            char rxBytes[20], rxPackets[20], rxErrs[20];
            char dummy[20];

            whline(devPad, ACS_HLINE, 20);
            mvwprintw(devPad, 0, 3, " IF Stats: ");

            sscanf(line.c_str(), "%s %s %s %s %s %s %s %s %s %s %s %s", dummy, rxBytes, rxPackets, rxErrs, dummy, dummy, dummy, dummy, dummy, txBytes, txPackets, txErrs);

            mvwprintw(devPad, 1, 0, "RX Bytes %s\nRX Packets %s\nRX Errs %s\nTX Bytes %s\nTX Packets %s\nTX Errs %s\n", rxBytes, rxPackets, rxErrs, txBytes, txPackets, txErrs);

            float bytesRNow = strtof(rxBytes, NULL) - bRX;
            float bytesTNow = strtof(txBytes, NULL) - bTX;

            float RKBS = bytesRNow > 0 ? bytesRNow / updateRate : 0.0;
            float TKBS = bytesTNow > 0 ? bytesTNow / updateRate : 0.0;

            wprintw(devPad, "\nRX %.03f KB/s\n", RKBS);
            wprintw(devPad, "TX %.03f KB/s\n", TKBS);

            bRX = strtof(rxBytes, NULL);
            bTX = strtof(txBytes, NULL);
        }
    }
    inFile.close();
}

/******************************************************************/

void drawMeshPad()
{

    werase(meshPad);

    meshInfoTimer = millis();
    //uint8_t pos=5;
    for (int i = 0; i < mesh.addrListTop; i++)
    {
        mvwprintw(meshPad, i + 1, 0, " Address: 0%o ID: %d \n", mesh.addrList[i].address, mesh.addrList[i].nodeID);
    }

    meshScroll = rf24_max(meshScroll, 0);

    if (padSelection == 0)
    {
        wattron(meshPad, COLOR_PAIR(1));
        mvwhline(meshPad, meshScroll, 0, ACS_HLINE, maxY);
        wattroff(meshPad, COLOR_PAIR(1));
        mvwprintw(meshPad, meshScroll, 3, " Mesh Info: ");
    }
    else
    {
        wattroff(meshPad, COLOR_PAIR(1));
        mvwhline(meshPad, meshScroll, 0, ACS_HLINE, maxY);
        mvwprintw(meshPad, meshScroll, 3, " Mesh Info: ");
    }
}

/******************************************************************/

void drawRF24Pad()
{

    werase(rf24Pad);
    mvwprintw(rf24Pad, 1, 0, "Address: 0%o\n", mesh.mesh_address);
    wprintw(rf24Pad, "nodeID: %d\n", mesh.getNodeID());
    wprintw(rf24Pad, "En Mesh: %s\n", gw.meshEnabled() ? "True" : "False");
    radio.maskIRQ(1, 1, 1);
    int dr = radio.getDataRate();
    int pa = radio.getPALevel();
    radio.maskIRQ(1, 1, 0);
    // clang-format off
    wprintw(rf24Pad, "Data-Rate: %s\n", dr == 0 ? "1MBPS" : dr == 1 ? "2MBPS"
                                                          : dr == 2 ? "250KBPS"
                                                                    : "ERROR");
    wprintw(rf24Pad, "PA Level: %s\n", pa == 0 ? "MIN" : pa == 1 ? "LOW"
                                                       : pa == 2 ? "HIGH"
                                                       : pa == 3 ? "MAX"
                                                                 : "ERROR");
    // clang-format on
    wprintw(rf24Pad, "IF Type: %s\n", gw.config_TUN == 1 ? "TUN" : "TAP");
    wprintw(rf24Pad, "IF Drops: %u\n", gw.ifDropped());
#if defined(ENABLE_NETWORK_STATS)
    uint32_t ok, fail;
    network.failures(&fail, &ok);
    wprintw(rf24Pad, "TX Packets(sys): %u\n", ok);
    wprintw(rf24Pad, "TX Drops: %u\n", fail);
#endif
    wprintw(rf24Pad, "RX Packets(user): %lu\n", networkPacketsRX);
    if (gw.fifoCleared)
    {
        ++fifoClears;
        gw.fifoCleared = false;
    }
    wprintw(rf24Pad, "Interrupts Missed: %u\n", fifoClears);
    wprintw(rf24Pad, "Network Overruns: %u\n", gw.networkOverruns);
    wprintw(rf24Pad, "Network Corruption: %u", gw.networkCorruption);

    if (padSelection == 1)
    {
        wattron(rf24Pad, COLOR_PAIR(1));
        mvwhline(rf24Pad, rf24Scroll, 0, ACS_HLINE, maxY);
        wattroff(rf24Pad, COLOR_PAIR(1));
        mvwprintw(rf24Pad, rf24Scroll, 3, " RF24Network Info: ");
    }
    else
    {
        wattroff(rf24Pad, COLOR_PAIR(1));
        mvwhline(rf24Pad, rf24Scroll, 0, ACS_HLINE, maxY);
        mvwprintw(rf24Pad, rf24Scroll, 3, " RF24Network Info: ");
    }
}

/******************************************************************/

void drawConnPad()
{
    if (topo) {
        drawTopology();
        return;
    }
    wattroff(connPad, COLOR_PAIR(2));
    int ctr = 0;
    size_t lCtr = 1;
    std::string line;
    werase(connPad);
    //box(connPad,ACS_VLINE,ACS_HLINE);

    std::ifstream inFile;
    inFile.open("/proc/net/nf_conntrack");
    while (inFile.good())
    {
        getline(inFile, line); // get line from file
                               // search
        if (line.find(subIP) != std::string::npos)
        {

            std::string src = "src";
            unsigned fnd = line.find(src);
            fnd = line.find_last_of("0123456789", fnd - 2);
            fnd = line.find_last_of(" ", fnd - 2);
            unsigned findEnd = line.find(" mark=");
            line = line.substr(fnd, findEnd - fnd);
            mvwprintw(connPad, ctr++, 0, "%lu %s\n", lCtr++, line.c_str());

            if (ctr > maxX - 15)
            {
                break;
            }
        }
    }

    inFile.close();
    mvwhline(win, 16, 1, ACS_HLINE, maxY - 2);
    mvwprintw(win, 16, 10, "Active IP Connections:");
}

/******************************************************************/

/** Handle keyboard input **/
/*******************************/
bool handleKeyPad()
{

    int myChar = getch();

    if (myChar > -1)
    {
        //cout << myChar << endl;
        switch (myChar)
        {
            // a: En/Disable display of active connections
            case 'a':
                if (topo) {
                    showConnPad = true;
                }
                else {
                    showConnPad = !showConnPad;
                }
                topo = false;
                if (!showConnPad)
                {
                    werase(connPad);
                    prefresh(connPad, 0, 0, 15, 1, maxX - 1, maxY - 2);
                    drawMain();
                }
                break;
            // w: Increase frame-rate of curses display
            case 'w':
                if (updateRate > 100)
                {
                    updateRate -= 100;
                }
                mvwprintw(win, 2, 27, "Refresh Rate: %.1f fps", 1000.0 / updateRate);
                refresh();
                break;
            // s: Decrease frame-rate of curses display
            case 's':
                updateRate += 100;
                mvwprintw(win, 2, 27, "Refresh Rate: %.1f fps   \t", 1000.0 / updateRate);
                refresh();
                break;
            // c: Display IP configuration menu
            case 'c':
                drawCfg(1);
                break;
            // h: Display help menu
            case 'h':
                drawHelp();
                break;
            case 'x':
                erase();
                endwin();
                return 0;
                break;
            case KEY_UP:
                if (padSelection == 0)
                {
                    meshScroll++;
                }
                else if (padSelection == 1)
                {
                    connScroll = rf24_max(connScroll++, 0);
                }
                break;
            case KEY_DOWN:
                if (padSelection == 0)
                {
                    meshScroll--;
                }
                else if (padSelection == 1)
                {
                    connScroll--;
                }
                break;
            case KEY_RIGHT:
                padSelection++;
                padSelection = std::min(padSelection, 1);
                break; //right
            case KEY_LEFT:
                padSelection--;
                padSelection = std::max(padSelection, 0);
                break; //left
            case 'Q':
                erase();
                endwin();
                return 0;
                break;
            case 't':
                topo = true;
                showConnPad = !showConnPad;
                clear();
                drawMain();
                meshInfoTimer -= 1000;
                break;
        }
    }
    return 1;
}