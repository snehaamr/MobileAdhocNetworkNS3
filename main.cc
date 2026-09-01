#include "MobileAdhocTree.h"
#include "TreeStructureApp.h"
#include "WiFiNetworkSetup.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

#include <iomanip>
#include <iostream>
#include <vector>

using namespace ns3;

int
main(int argc, char* argv[])
{
    uint32_t nWifi = 40;
    uint32_t bufferSize = 10;
    double simTime = 100.0;
    double sendInterval = 2.0;
    double txInterval = 0.5;
    double heartbeatInterval = 1.0;
    double areaSize = 480.0;
    double txPower = 20.0;
    uint32_t seed = 1;
    uint32_t run = 1;
    std::string csvPath = "manet-stats.csv";

    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifi", "Number of Wi-Fi ad-hoc stations", nWifi);
    cmd.AddValue("bufferSize", "Per-node store-and-forward buffer size (packets)", bufferSize);
    cmd.AddValue("simTime", "Simulation duration in seconds", simTime);
    cmd.AddValue("sendInterval", "Data generation interval in seconds (non-root nodes)", sendInterval);
    cmd.AddValue("txInterval", "Buffer drain interval in seconds", txInterval);
    cmd.AddValue("heartbeatInterval", "Tree beacon interval in seconds", heartbeatInterval);
    cmd.AddValue("areaSize", "Square mobility area side length in meters", areaSize);
    cmd.AddValue("txPower", "Wi-Fi transmit power in dBm", txPower);
    cmd.AddValue("seed", "RngSeed", seed);
    cmd.AddValue("run", "RngRun", run);
    cmd.AddValue("csv", "CSV time-series output path", csvPath);
    cmd.Parse(argc, argv);

    if (nWifi == 0)
    {
        std::cerr << "nWifi must be at least 1 (node 0 is the tree root / sink)\n";
        return 1;
    }

    RngSeedManager::SetSeed(seed);
    RngSeedManager::SetRun(run);

    WiFiNetworkSetup wifiSetup(nWifi, txPower, areaSize);
    wifiSetup.SetupWiFi();
    wifiSetup.SetupMobility();

    NodeContainer wifiStaNodes = wifiSetup.GetNodes();

    InternetStackHelper stack;
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.0.0");
    address.Assign(wifiSetup.GetDevices());

    TreeStructureApp::EnableCsv(csvPath);

    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    std::vector<Ptr<TreeStructureApp>> apps;

    for (uint32_t i = 0; i < nWifi; i++)
    {
        Ptr<Socket> appSocket = Socket::CreateSocket(wifiStaNodes.Get(i), tid);
        Ptr<TreeStructureApp> app = CreateObject<TreeStructureApp>(appSocket, bufferSize);
        app->SetIntervals(sendInterval, txInterval, heartbeatInterval);
        wifiStaNodes.Get(i)->AddApplication(app);
        app->SetStartTime(Seconds(0.0));
        app->SetStopTime(Seconds(simTime));
        apps.push_back(app);
    }

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    uint64_t totalOriginated = 0;
    uint64_t totalSent = 0;
    uint64_t totalDelivered = 0;
    uint64_t totalDropped = 0;
    uint32_t attached = 0;

    std::cout << "\n=== Final per-node statistics ===\n";
    std::cout << std::left << std::setw(8) << "Node" << std::setw(10) << "Parent" << std::setw(10)
              << "Hops" << std::setw(12) << "Originated" << std::setw(10) << "Sent" << std::setw(12)
              << "Delivered" << std::setw(10) << "Dropped" << std::setw(10) << "Buffer"
              << "\n";

    for (const auto& app : apps)
    {
        totalOriginated += app->GetPacketsOriginated();
        totalSent += app->GetPacketsSent();
        totalDelivered += app->GetPacketsDelivered();
        totalDropped += app->GetPacketsDropped();
        if (app->GetNodeId() == 0 || app->GetParentId() >= 0)
        {
            attached++;
        }

        const int32_t hops = app->GetHopcount();
        std::cout << std::left << std::setw(8) << app->GetNodeId() << std::setw(10)
                  << app->GetParentId() << std::setw(10)
                  << (hops >= MobileAdhocTree::kInfiniteHops ? -1 : hops) << std::setw(12)
                  << app->GetPacketsOriginated() << std::setw(10) << app->GetPacketsSent()
                  << std::setw(12) << app->GetPacketsDelivered() << std::setw(10)
                  << app->GetPacketsDropped() << std::setw(10) << app->GetBufferOccupancy()
                  << "\n";
    }

    const double pdr =
        totalOriginated == 0 ? 0.0 : (100.0 * static_cast<double>(totalDelivered) / totalOriginated);

    std::cout << "\n=== Aggregate ===\n";
    std::cout << "Nodes attached to tree: " << attached << " / " << nWifi << "\n";
    std::cout << "Originated: " << totalOriginated << "\n";
    std::cout << "Transmitted (all hops): " << totalSent << "\n";
    std::cout << "Delivered at root: " << totalDelivered << "\n";
    std::cout << "Dropped (buffer overflow): " << totalDropped << "\n";
    std::cout << "Packet delivery ratio (root deliveries / originated): " << std::fixed
              << std::setprecision(2) << pdr << "%\n";
    std::cout << "Time-series CSV: " << csvPath << "\n";

    Simulator::Destroy();
    return 0;
}
