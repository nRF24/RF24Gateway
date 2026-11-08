/**
 * rf24gatewayd (headless) - RF24Gateway service without a user interface
 *
 * Based on the RF24GatewayNode example. Settings come from a config file (default
 * /etc/rf24gateway/rf24gateway.conf). Logs go to journald: journalctl -u rf24gateway -f
 * SIGTERM/SIGINT shut down cleanly: the DHCP list is saved and the radio powered down.
 * AES-CCM encryption: see rf24gatewayd_config.h.
 */
#include "rf24gatewayd_config.h"

int main(int argc, char** argv)
{
    // journald gets one line at a time instead of 4KB blocks
    setvbuf(stdout, nullptr, _IOLBF, 0);

    GatewayConfig cfg;
    loadGatewayConfig(argc > 1 ? argv[1] : RF24GATEWAYD_DEFAULT_CONFIG, cfg);

    gw_radio_t radio(cfg.cePin, cfg.csn);
    gw_network_t network(radio);
    gw_mesh_t mesh(radio, network);
    gw_gateway_t gw(radio, network, mesh);

    installSignalHandlers();

    printf("starting: ce=%d csn=%d node=%d channel=%d rate=%s ip=%s/%s encryption=%s\n", cfg.cePin, cfg.csn, cfg.nodeID,
           cfg.channel, cfg.dataRateName.c_str(), cfg.ip.c_str(), cfg.netmask.c_str(), cfg.encryption ? "on" : "off");

    if (!setupEncryption(radio, cfg)) {
        return 1;
    }
    gw.begin(cfg.nodeID, cfg.channel, cfg.dataRate);
    if (!setGatewayIP(gw, cfg, "tun_nrf24")) {
        return 1;
    }
    printf("gateway running\n");

    uint32_t meshTimer = millis();
    uint32_t failTimer = millis();
    uint32_t restarts = 0;

    while (gwRunning) {
        gw.update();
        enforceAllowedNodes(mesh, cfg);

        // Non-IP RF24Network payloads are forwarded as UDP (see the examples/clients scripts)
        if (network.available()) {
            RF24NetworkHeader header;
            size_t size = network.peek(header);
            uint8_t buf[size];
            network.read(header, &buf, size);
            if (cfg.forwardUserData) {
                RF24NetworkFrame frame = RF24NetworkFrame(header, buf, size);
                gw.sendUDP(mesh.getNodeID(header.from_node), frame);
            }
        }

        // A child gateway (node_id != 0) keeps its mesh address current
        if (cfg.nodeID && millis() - meshTimer > cfg.checkInterval) {
            meshTimer = millis();
            if (!mesh.checkConnection()) {
                printf("mesh connection lost, renewing address\n");
                mesh.renewAddress();
            }
        }

        // Restart the radio if it was unplugged or lost its configuration
        if (millis() - failTimer > 1000) {
            failTimer = millis();
            if (radio.failureDetected > 0 || radio.getDataRate() != cfg.dataRate) {
                radio.failureDetected = 0;
                gwLog(LOG_WARNING, "radio failure detected, restarting mesh (restart #%u)", ++restarts);
                delay(500);
                mesh.begin(cfg.channel, cfg.dataRate);
            }
        }
        delay(2);
    }

    printf("stopping\n");
    if (cfg.nodeID == 0) {
        mesh.saveDHCP();
    }
    radio.powerDown();
    return 0;
}
