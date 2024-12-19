#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "WiFiNetworkSetup.h"
#include "TreeStructureApp.h"

using namespace ns3;

int main(int argc, char* argv[]) {
    uint32_t nWifi = 40;  // Number of nodes
    uint32_t bufferSize = 10;  // Buffer size (number of packets each node can store)
    CommandLine cmd;
    cmd.AddValue("nWifi", "Number of Wi-Fi stations", nWifi);
    cmd.AddValue("bufferSize", "Buffer size for each node", bufferSize);
    cmd.Parse(argc, argv);

    // Setup Wi-Fi and mobility
    WiFiNetworkSetup wifiSetup(nWifi);
    wifiSetup.SetupWiFi();
    wifiSetup.SetupMobility();

    NodeContainer wifiStaNodes = wifiSetup.GetNodes();

    InternetStackHelper stack;
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.0.0");
    Ipv4InterfaceContainer wifiInterfaces = address.Assign(wifiSetup.GetDevices());

    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

    // Create applications with buffer size for each node
    for (uint32_t i = 0; i < nWifi; i++) {
        Ptr<Socket> appSocket = Socket::CreateSocket(wifiStaNodes.Get(i), tid);
        Ptr<TreeStructureApp> app = CreateObject<TreeStructureApp>(appSocket, bufferSize);
        wifiStaNodes.Get(i)->AddApplication(app);
        app->SetStartTime(Seconds(0));
        app->SetStopTime(Seconds(10000));
    }

    Simulator::Stop(Seconds(10000));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
