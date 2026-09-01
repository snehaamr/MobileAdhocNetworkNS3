#ifndef TREE_STRUCTURE_APP_H
#define TREE_STRUCTURE_APP_H

#include "MobileAdhocTree.h"

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"

#include <cstdint>
#include <fstream>
#include <queue>
#include <set>
#include <string>
#include <utility>

namespace ns3 {

class TreeStructureApp : public Application
{
  public:
    static TypeId GetTypeId();

    TreeStructureApp();
    TreeStructureApp(Ptr<Socket> socket, uint32_t bufferSize);
    ~TreeStructureApp() override;

    void Configure(Ptr<Socket> socket, uint32_t bufferSize);
    void SetIntervals(double sendInterval, double txInterval, double heartbeatInterval);
    void SetStatsInterval(double statsInterval);

    static void EnableCsv(const std::string& path);

    uint32_t GetPacketsOriginated() const;
    uint32_t GetPacketsSent() const;
    uint32_t GetPacketsDelivered() const;
    uint32_t GetUniqueDelivered() const;
    uint32_t GetPacketsDropped() const;
    int32_t GetNodeId() const;
    int32_t GetParentId() const;
    int32_t GetHopcount() const;
    uint32_t GetBufferOccupancy() const;

  private:
    void StartApplication() override;
    void StopApplication() override;

    void GenerateTraffic();
    void Transmit();
    void Heartbeat();
    void ReceivePacket(Ptr<Socket> socket);
    void PrintData();
    void CancelEvents();
    void RefreshPosition();
    void CheckParentTimeout();
    void HandleBeacon(const MobileAdhocTree& hdr, const Address& from);
    void HandleData(Ptr<Packet> packetWithHeader);
    void FillHeader(MobileAdhocTree& hdr, uint8_t type) const;
    Ipv4Address GetLocalAddress() const;

    Ptr<Socket> m_socket;
    uint32_t m_bufferSize;
    uint32_t m_packetsOriginated;
    uint32_t m_packetsSent;
    uint32_t m_packetsDropped;
    uint32_t m_packetsDelivered;
    uint32_t m_uniqueDelivered;
    uint32_t m_nextSeq;
    std::set<std::pair<int32_t, uint32_t>> m_seenAtRoot;

    int32_t m_nodeId;
    int32_t m_parentId;
    int32_t m_hopcount;
    Ipv4Address m_localAddress;
    Ipv4Address m_parentAddress;
    Time m_lastParentBeacon;

    double m_sendInterval;
    double m_txInterval;
    double m_heartbeatInterval;
    double m_statsInterval;

    std::queue<Ptr<Packet>> m_packetBuffer;

    EventId m_sendEvent;
    EventId m_txEvent;
    EventId m_heartbeatEvent;
    EventId m_statsEvent;
    bool m_running;

    static std::ofstream s_csv;
    static bool s_csvHeaderWritten;
};

} // namespace ns3

#endif // TREE_STRUCTURE_APP_H
