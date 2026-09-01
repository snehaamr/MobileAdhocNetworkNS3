#include "MobileAdhocTree.h"
#include "TreeStructureApp.h"
#include "WiFiNetworkSetup.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-flow-probe.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

#include <iomanip>
#include <iostream>
#include <vector>

using namespace ns3;

namespace {

uint64_t
DroppedForReason(const FlowMonitor::FlowStats& stats, uint32_t reason)
{
    if (reason < stats.packetsDropped.size())
    {
        return stats.packetsDropped[reason];
    }
    return 0;
}

} // namespace

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
    std::string flowmonPath = "flowmon.xml";

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
    cmd.AddValue("flowmon", "FlowMonitor XML path (empty to skip the file)", flowmonPath);
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

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    uint64_t totalOriginated = 0;
    uint64_t totalSent = 0;
    uint64_t totalDelivered = 0;
    uint64_t totalUnique = 0;
    uint64_t totalDropped = 0;
    uint32_t attached = 0;

    std::cout << "\n=== Final per-node statistics ===\n";
    std::cout << std::left << std::setw(8) << "Node" << std::setw(10) << "Parent" << std::setw(10)
              << "Hops" << std::setw(12) << "Originated" << std::setw(10) << "Sent" << std::setw(12)
              << "Delivered" << std::setw(10) << "Unique" << std::setw(10) << "Dropped"
              << std::setw(10) << "Buffer"
              << "\n";

    for (const auto& app : apps)
    {
        totalOriginated += app->GetPacketsOriginated();
        totalSent += app->GetPacketsSent();
        totalDelivered += app->GetPacketsDelivered();
        totalUnique += app->GetUniqueDelivered();
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
                  << app->GetUniqueDelivered() << std::setw(10) << app->GetPacketsDropped()
                  << std::setw(10) << app->GetBufferOccupancy() << "\n";
    }

    monitor->CheckForLostPackets(Seconds(1.0));
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());

    uint64_t uniTx = 0;
    uint64_t uniRx = 0;
    uint64_t uniLost = 0;
    uint64_t bcastTx = 0;
    uint64_t dropMacQueue = 0;
    uint64_t dropNoRoute = 0;
    uint64_t dropOther = 0;

    for (const auto& kv : monitor->GetFlowStats())
    {
        Ipv4FlowClassifier::FiveTuple tuple = classifier->FindFlow(kv.first);
        const FlowMonitor::FlowStats& stats = kv.second;
        const bool isBroadcast = tuple.destinationAddress.IsBroadcast();

        if (isBroadcast)
        {
            bcastTx += stats.txPackets;
            continue;
        }

        uniTx += stats.txPackets;
        uniRx += stats.rxPackets;
        uniLost += stats.lostPackets;
        dropNoRoute += DroppedForReason(stats, Ipv4FlowProbe::DROP_NO_ROUTE);
        dropMacQueue += DroppedForReason(stats, Ipv4FlowProbe::DROP_QUEUE);
        dropMacQueue += DroppedForReason(stats, Ipv4FlowProbe::DROP_QUEUE_DISC);

        uint64_t accounted = DroppedForReason(stats, Ipv4FlowProbe::DROP_NO_ROUTE) +
                             DroppedForReason(stats, Ipv4FlowProbe::DROP_QUEUE) +
                             DroppedForReason(stats, Ipv4FlowProbe::DROP_QUEUE_DISC);
        uint64_t allReasonDrops = 0;
        for (uint32_t n : stats.packetsDropped)
        {
            allReasonDrops += n;
        }
        if (allReasonDrops > accounted)
        {
            dropOther += allReasonDrops - accounted;
        }
    }

    if (!flowmonPath.empty())
    {
        monitor->SerializeToXmlFile(flowmonPath, true, true);
    }

    const double pdr =
        totalOriginated == 0 ? 0.0 : (100.0 * static_cast<double>(totalDelivered) / totalOriginated);
    const double uniquePdr =
        totalOriginated == 0 ? 0.0 : (100.0 * static_cast<double>(totalUnique) / totalOriginated);

    std::cout << "\n=== Aggregate ===\n";
    std::cout << "Nodes attached to tree: " << attached << " / " << nWifi << "\n";
    std::cout << "Originated: " << totalOriginated << "\n";
    std::cout << "Transmitted (all hops): " << totalSent << "\n";
    std::cout << "Delivered at root (including duplicates): " << totalDelivered << "\n";
    std::cout << "Unique delivered at root: " << totalUnique << "\n";
    std::cout << "Dropped (application buffer overflow): " << totalDropped << "\n";
    std::cout << "Packet delivery ratio (root deliveries / originated): " << std::fixed
              << std::setprecision(2) << pdr << "%\n";
    std::cout << "Unique PDR (unique root deliveries / originated): " << uniquePdr << "%\n";

    std::cout << "\n=== FlowMonitor (unicast data path; beacons excluded) ===\n";
    std::cout << "Unicast TX packets: " << uniTx << "\n";
    std::cout << "Unicast RX packets: " << uniRx << "\n";
    std::cout << "Unicast lost (not received in time): " << uniLost << "\n";
    std::cout << "Beacon/broadcast TX packets: " << bcastTx << "\n";
    std::cout << "IPv4/MAC drops: no-route=" << dropNoRoute << " net-device/queue=" << dropMacQueue
              << " other=" << dropOther << "\n";
    if (!flowmonPath.empty())
    {
        std::cout << "FlowMonitor XML: " << flowmonPath << "\n";
    }
    std::cout << "Time-series CSV: " << csvPath << "\n";

    // Stable one-liner for sweep scripts (do not reformat without updating sweep_buffers.py).
    std::cout << "SUMMARY"
              << " nWifi=" << nWifi
              << " bufferSize=" << bufferSize
              << " simTime=" << simTime
              << " seed=" << seed
              << " run=" << run
              << " originated=" << totalOriginated
              << " delivered=" << totalDelivered
              << " uniqueDelivered=" << totalUnique
              << " bufferDropped=" << totalDropped
              << " attached=" << attached
              << " pdr=" << std::setprecision(4) << pdr
              << " uniquePdr=" << uniquePdr
              << " flowUnicastTx=" << uniTx
              << " flowUnicastRx=" << uniRx
              << " flowUnicastLost=" << uniLost
              << " flowMacQueueDrops=" << dropMacQueue
              << " flowNoRouteDrops=" << dropNoRoute
              << "\n";

    Simulator::Destroy();
    return 0;
}
