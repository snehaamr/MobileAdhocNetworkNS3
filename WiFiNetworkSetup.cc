#include "WiFiNetworkSetup.h"

#include "ns3/double.h"
#include "ns3/mobility-helper.h"
#include "ns3/rectangle.h"
#include "ns3/string.h"
#include "ns3/wifi-helper.h"
#include "ns3/wifi-mac-helper.h"
#include "ns3/yans-wifi-helper.h"

#include <sstream>

namespace ns3 {

WiFiNetworkSetup::WiFiNetworkSetup(uint32_t numNodes, double txPower, double areaSize)
    : m_nWifi(numNodes),
      m_txPower(txPower),
      m_areaSize(areaSize)
{
    m_wifiStaNodes.Create(m_nWifi);
}

void
WiFiNetworkSetup::SetupWiFi()
{
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("DsssRate1Mbps"),
                                 "ControlMode",
                                 StringValue("DsssRate1Mbps"));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    phy.Set("TxPowerStart", DoubleValue(m_txPower));
    phy.Set("TxPowerEnd", DoubleValue(m_txPower));

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    m_staDevices = wifi.Install(phy, mac, m_wifiStaNodes);
}

void
WiFiNetworkSetup::SetupMobility()
{
    MobilityHelper mobility;

    std::ostringstream xRange;
    std::ostringstream yRange;
    xRange << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_areaSize << "]";
    yRange << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_areaSize << "]";

    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                  "X",
                                  StringValue(xRange.str()),
                                  "Y",
                                  StringValue(yRange.str()));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(0.0, m_areaSize, 0.0, m_areaSize)),
                              "Speed",
                              StringValue("ns3::UniformRandomVariable[Min=1.0|Max=3.0]"),
                              "Distance",
                              DoubleValue(30.0));
    mobility.Install(m_wifiStaNodes);
}

NodeContainer
WiFiNetworkSetup::GetNodes() const
{
    return m_wifiStaNodes;
}

NetDeviceContainer
WiFiNetworkSetup::GetDevices() const
{
    return m_staDevices;
}

} // namespace ns3
