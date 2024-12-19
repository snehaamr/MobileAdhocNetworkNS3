#include "WiFiNetworkSetup.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"

namespace ns3 {

WiFiNetworkSetup::WiFiNetworkSetup(uint32_t numNodes, double txPower)
    : nWifi(numNodes), txPower(txPower) {}

void WiFiNetworkSetup::SetupWiFi() {
    WifiHelper wifi = WifiHelper::Default();
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    phy.SetChannel(channel.Create());

    NqosWifiMacHelper mac = NqosWifiMacHelper::Default();
    mac.SetType("ns3::AdhocWifiMac");

    staDevices = wifi.Install(phy, mac, wifiStaNodes);

    phy.Set("TxPowerEnd", DoubleValue(txPower));
    phy.Set("TxPowerStart", DoubleValue(txPower));
}

void WiFiNetworkSetup::SetupMobility() {
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=480]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=480]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel");
    mobility.Install(wifiStaNodes);
}

NodeContainer WiFiNetworkSetup::GetNodes() const {
    return wifiStaNodes;
}

} // namespace ns3
