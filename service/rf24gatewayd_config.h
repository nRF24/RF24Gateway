/**
 * Shared by the rf24gatewayd service programs (headless, ncurses, ncursesint):
 * config file parsing, logging, signal handling, AES-CCM encryption and tun_nrf24 address setup.
 *
 * Built with -DRF24NETWORK_CCM=ON (RF24Network, RF24Mesh and RF24Gateway must be built the same
 * way), the radio is an RF24CCM. Encryption is then switched on with "encryption = 1" in the config.
 * With encryption off, RF24CCM behaves exactly like RF24.
 */
#ifndef RF24GATEWAYD_CONFIG_H
#define RF24GATEWAYD_CONFIG_H

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <RF24Mesh/RF24Mesh.h>
#include <RF24Gateway/RF24Gateway.h>
#if defined(RF24NETWORK_CCM)
    #include <RF24Network/RF24CCM.h>
#endif
#include <arpa/inet.h>
#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ifaddrs.h>
#include <map>
#include <sstream>
#include <string>
#include <syslog.h>
#include <vector>

#define RF24GATEWAYD_DEFAULT_CONFIG "/etc/rf24gateway/rf24gateway.conf"

// The radio, network, mesh and gateway types, with or without CCM support
#if defined(RF24NETWORK_CCM)
typedef RF24CCM gw_radio_t;
#else
typedef RF24 gw_radio_t;
#endif
typedef ESBNetwork<gw_radio_t> gw_network_t;
typedef ESBMesh<gw_network_t, gw_radio_t> gw_mesh_t;
typedef ESBGateway<gw_mesh_t, gw_network_t, gw_radio_t> gw_gateway_t;

struct GatewayConfig
{
    int cePin = 22;
    int csn = 0;
    int irqPin = 24;
    uint8_t nodeID = 0;
    uint8_t channel = 97;
    std::string dataRateName = "1m";
    rf24_datarate_e dataRate = RF24_1MBPS;
    std::string ip = "10.10.2.2";
    std::string netmask = "255.255.255.0";
    uint32_t checkInterval = 30000;
    bool forwardUserData = true;
    bool encryption = false;
    std::string keyFile = "/etc/rf24gateway/rf24gateway.key";
    std::vector<uint8_t> allowedNodes; // empty: any node may join
};

static volatile sig_atomic_t gwRunning = 1;

static void gwOnSignal(int) { gwRunning = 0; }

static void installSignalHandlers()
{
    std::signal(SIGTERM, gwOnSignal);
    std::signal(SIGINT, gwOnSignal);
    std::signal(SIGHUP, gwOnSignal);
}

// Log to stderr and syslog. In the GUI modes stderr is the screen window, which closes when the
// program exits, so syslog (journalctl -u rf24gateway) keeps startup errors visible
static void gwLog(int priority, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    va_list copy;
    va_copy(copy, args);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    vsyslog(priority, format, copy);
    va_end(copy);
    va_end(args);
}

static std::string gwTrim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return start == std::string::npos ? "" : s.substr(start, end - start + 1);
}

static rf24_datarate_e gwParseDataRate(const std::string& rate)
{
    if (rate == "250k") {
        return RF24_250KBPS;
    }
    if (rate == "2m") {
        return RF24_2MBPS;
    }
    return RF24_1MBPS;
}

// Parse the allowed_nodes list: decimal nodeIDs from 1 to 255, separated by commas and/or spaces.
// An empty value leaves the list empty, so any node may join
static bool gwParseAllowedNodes(std::string value, GatewayConfig& c)
{
    std::replace(value.begin(), value.end(), ',', ' ');
    std::istringstream in(value);
    std::string item;
    while (in >> item) {
        size_t used = 0;
        int id;
        try {
            id = std::stoi(item, &used, 10);
        }
        catch (const std::exception&) {
            return false;
        }
        if (used != item.size() || id < 1 || id > 255) {
            return false;
        }
        c.allowedNodes.push_back((uint8_t)id);
    }
    return true;
}

// Parse simple "key = value" lines. '#' starts a comment. Missing keys keep their defaults
static bool loadGatewayConfig(const char* path, GatewayConfig& c)
{
    openlog("rf24gatewayd", 0, LOG_DAEMON);
    std::ifstream file(path);
    if (!file.is_open()) {
        gwLog(LOG_WARNING, "rf24gatewayd: cannot read %s, using defaults", path);
        return false;
    }
    std::map<std::string, std::string> cfg;
    std::string line;
    while (std::getline(file, line)) {
        line = gwTrim(line.substr(0, line.find('#')));
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            cfg[gwTrim(line.substr(0, eq))] = gwTrim(line.substr(eq + 1));
        }
    }
    try {
        if (cfg.count("ce_pin"))
            c.cePin = std::stoi(cfg["ce_pin"], nullptr, 0);
        if (cfg.count("csn"))
            c.csn = std::stoi(cfg["csn"], nullptr, 0);
        if (cfg.count("irq_pin"))
            c.irqPin = std::stoi(cfg["irq_pin"], nullptr, 0);
        if (cfg.count("node_id"))
            c.nodeID = std::stoi(cfg["node_id"], nullptr, 0);
        if (cfg.count("channel"))
            c.channel = std::stoi(cfg["channel"], nullptr, 0);
        if (cfg.count("check_interval_ms"))
            c.checkInterval = std::stoul(cfg["check_interval_ms"], nullptr, 0);
        if (cfg.count("forward_user_data"))
            c.forwardUserData = std::stoi(cfg["forward_user_data"], nullptr, 0);
        if (cfg.count("encryption"))
            c.encryption = std::stoi(cfg["encryption"], nullptr, 0);
    }
    catch (const std::exception&) {
        gwLog(LOG_ERR, "rf24gatewayd: a number in %s is invalid", path);
        return false;
    }
    if (cfg.count("data_rate"))
        c.dataRateName = cfg["data_rate"];
    if (cfg.count("ip"))
        c.ip = cfg["ip"];
    if (cfg.count("netmask"))
        c.netmask = cfg["netmask"];
    if (cfg.count("encryption_key_file"))
        c.keyFile = cfg["encryption_key_file"];
    c.dataRate = gwParseDataRate(c.dataRateName);
    if (cfg.count("allowed_nodes") && !gwParseAllowedNodes(cfg["allowed_nodes"], c)) {
        // Running without the list would let every node join, so refuse to start instead
        gwLog(LOG_ERR, "rf24gatewayd: allowed_nodes in %s must be nodeIDs from 1 to 255, separated by commas or spaces",
              path);
        exit(1);
    }
    if (!c.allowedNodes.empty()) {
        gwLog(LOG_INFO, "rf24gatewayd: allowed_nodes has %u nodeIDs. Other nodes are released from the mesh",
              (unsigned)c.allowedNodes.size());
        if (c.nodeID) {
            gwLog(LOG_WARNING, "rf24gatewayd: allowed_nodes only has an effect on the master gateway (node_id = 0)");
        }
    }
    return true;
}

// Release the mesh address of every node that is not in allowed_nodes. Only the master gateway holds
// the address list. Run it on every loop pass: an unlisted node holds a valid address from the moment
// the master assigns it until the next check. See docs/securing_the_network.md for the limitations
template<class mesh_t>
static void enforceAllowedNodes(mesh_t& mesh, const GatewayConfig& c)
{
    if (c.allowedNodes.empty()) {
        return;
    }
    for (int i = 0; i < mesh.addrListTop; i++) {
        if (mesh.addrList[i].address == 0
            || std::find(c.allowedNodes.begin(), c.allowedNodes.end(), mesh.addrList[i].nodeID) != c.allowedNodes.end()) {
            continue;
        }
        // syslog only: in the GUI modes stderr would draw over the ncurses screen
        syslog(LOG_NOTICE, "rf24gatewayd: node %d is not in allowed_nodes, released its address 0%o",
               mesh.addrList[i].nodeID, mesh.addrList[i].address);
        mesh.addrList[i].address = 0;
    }
}

// Read a 16 byte key written as 32 hex digits. Accepts plain hex ("0102..."), a C array as printed
// by the installer or xxd -i ("0x01, 0x02, ..."), or a line pasted from a sketch
// ("uint8_t key[16] = {0x01, ...};", where only the part between the braces is read). '#' starts a comment
[[maybe_unused]] static bool loadKeyFile(const std::string& path, uint8_t key[16])
{
    std::ifstream file(path);
    if (!file.is_open()) {
        gwLog(LOG_ERR, "rf24gatewayd: cannot read the encryption key file %s", path.c_str());
        return false;
    }
    std::string text, line;
    while (std::getline(file, line)) {
        text += line.substr(0, line.find('#')) + "\n";
    }
    size_t open = text.find('{');
    if (open != std::string::npos) {
        text = text.substr(open + 1, text.find('}', open) - open - 1);
    }
    std::string hex;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '0' && i + 1 < text.size() && (text[i + 1] == 'x' || text[i + 1] == 'X')) {
            ++i; // skip a "0x" prefix
        }
        else if (isxdigit((unsigned char)text[i])) {
            hex += text[i];
        }
    }
    if (hex.size() != 32) {
        gwLog(LOG_ERR, "rf24gatewayd: %s must hold a 16 byte key (32 hex digits), found %u digits", path.c_str(),
              (unsigned)hex.size());
        return false;
    }
    for (int i = 0; i < 16; ++i) {
        key[i] = (uint8_t)std::stoi(hex.substr(i * 2, 2), nullptr, 16);
    }
    return true;
}

// Turn on AES-CCM encryption if the config asks for it. Must run before gw.begin(), which sets the
// radio retries that RF24CCM raises while encryption is enabled. Returns false if the gateway can't run
static bool setupEncryption(gw_radio_t& radio, GatewayConfig& c)
{
    if (!c.encryption) {
        return true;
    }
#if defined(RF24NETWORK_CCM)
    uint8_t key[RF24CCM_KEY_SIZE];
    if (!loadKeyFile(c.keyFile, key)) {
        return false;
    }
    radio.setKey(key);
    radio.enableEncryption = true;
    memset(key, 0, sizeof(key));
    gwLog(LOG_INFO, "rf24gatewayd: AES-CCM encryption enabled. Nodes without the same key can't communicate");
    return true;
#else
    (void)radio;
    gwLog(LOG_ERR, "rf24gatewayd: encryption = 1, but rf24gatewayd was built without CCM support");
    gwLog(LOG_ERR, "Rebuild RF24Network, RF24Mesh, RF24Gateway and the service with -DRF24NETWORK_CCM=ON");
    return false;
#endif
}

// True if the named interface already has the given IPv4 address
static bool interfaceHasIP(const std::string& ifName, const std::string& ip)
{
    struct ifaddrs* ifap;
    if (getifaddrs(&ifap) < 0) {
        return false;
    }
    bool found = false;
    char host[INET_ADDRSTRLEN];
    for (struct ifaddrs* ifa = ifap; ifa != NULL && !found; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET || ifName != ifa->ifa_name) {
            continue;
        }
        struct sockaddr_in* sin = (struct sockaddr_in*)ifa->ifa_addr;
        found = inet_ntop(AF_INET, &sin->sin_addr, host, sizeof(host)) && ip == host;
    }
    freeifaddrs(ifap);
    return found;
}

// Set the tun_nrf24 address. Without CAP_NET_ADMIN, setIP() fails, which is fine if an admin
// already configured the interface. Returns false if the gateway can't run
template<class gateway_t>
static bool setGatewayIP(gateway_t& gw, GatewayConfig& c, const std::string& tunName)
{
    if (gw.setIP(&c.ip[0], &c.netmask[0]) >= 0 || interfaceHasIP(tunName, c.ip)) {
        return true;
    }
    gwLog(LOG_ERR, "rf24gatewayd: failed to set %s/%s on %s", c.ip.c_str(), c.netmask.c_str(), tunName.c_str());
    gwLog(LOG_ERR, "Run with CAP_NET_ADMIN (the systemd unit grants it), or preconfigure the interface:");
    gwLog(LOG_ERR, "  sudo ip tuntap add dev %s mode tun user $USER multi_queue", tunName.c_str());
    gwLog(LOG_ERR, "  sudo ip addr add %s/%s dev %s && sudo ip link set %s up", c.ip.c_str(), c.netmask.c_str(),
          tunName.c_str(), tunName.c_str());
    return false;
}

#endif // RF24GATEWAYD_CONFIG_H
