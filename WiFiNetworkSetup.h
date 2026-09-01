#ifndef WIFI_NETWORK_SETUP_H
#define WIFI_NETWORK_SETUP_H

#include "ns3/net-device-container.h"
#include "ns3/node-container.h"

#include <cstdint>

namespace ns3 {

class WiFiNetworkSetup
{
  public:
    WiFiNetworkSetup(uint32_t numNodes, double txPower = 20.0, double areaSize = 480.0);

    void SetupWiFi();
    void SetupMobility();

    NodeContainer GetNodes() const;
    NetDeviceContainer GetDevices() const;

  private:
    uint32_t m_nWifi;
    double m_txPower;
    double m_areaSize;
    NodeContainer m_wifiStaNodes;
    NetDeviceContainer m_staDevices;
};

} // namespace ns3

#endif // WIFI_NETWORK_SETUP_H
