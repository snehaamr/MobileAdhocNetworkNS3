#ifndef WIFI_NETWORK_SETUP_H
#define WIFI_NETWORK_SETUP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

namespace ns3 {

class WiFiNetworkSetup {
public:
    WiFiNetworkSetup(uint32_t numNodes, double txPower = 20);
    void SetupWiFi();
    void SetupMobility();
    NodeContainer GetNodes() const;

private:
    uint32_t nWifi;
    double txPower;
    NodeContainer wifiStaNodes;
    NetDeviceContainer staDevices;
};

} // namespace ns3

#endif // WIFI_NETWORK_SETUP_H
