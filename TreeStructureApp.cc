#include "TreeStructureApp.h"

#include "MobileAdhocTree.h"

#include "ns3/double.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TreeStructureApp");
NS_OBJECT_ENSURE_REGISTERED(TreeStructureApp);

namespace {
constexpr uint16_t kAppPort = 5000;
constexpr int32_t kRootId = 0;
} // namespace

std::ofstream TreeStructureApp::s_csv;
bool TreeStructureApp::s_csvHeaderWritten = false;

TypeId
TreeStructureApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TreeStructureApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<TreeStructureApp>();
    return tid;
}

TreeStructureApp::TreeStructureApp()
    : m_socket(nullptr),
      m_bufferSize(10),
      m_packetsOriginated(0),
      m_packetsSent(0),
      m_packetsDropped(0),
      m_packetsDelivered(0),
      m_nextSeq(0),
      m_nodeId(-1),
      m_parentId(-1),
      m_hopcount(MobileAdhocTree::kInfiniteHops),
      m_lastParentBeacon(Seconds(0)),
      m_sendInterval(2.0),
      m_txInterval(0.5),
      m_heartbeatInterval(1.0),
      m_statsInterval(10.0),
      m_running(false)
{
}

TreeStructureApp::TreeStructureApp(Ptr<Socket> socket, uint32_t bufferSize)
    : TreeStructureApp()
{
    Configure(socket, bufferSize);
}

TreeStructureApp::~TreeStructureApp()
{
    m_socket = nullptr;
}

void
TreeStructureApp::Configure(Ptr<Socket> socket, uint32_t bufferSize)
{
    m_socket = socket;
    m_bufferSize = bufferSize;
}

void
TreeStructureApp::SetIntervals(double sendInterval, double txInterval, double heartbeatInterval)
{
    m_sendInterval = sendInterval;
    m_txInterval = txInterval;
    m_heartbeatInterval = heartbeatInterval;
}

void
TreeStructureApp::SetStatsInterval(double statsInterval)
{
    m_statsInterval = statsInterval;
}

void
TreeStructureApp::EnableCsv(const std::string& path)
{
    if (s_csv.is_open())
    {
        s_csv.close();
    }
    s_csv.open(path.c_str(), std::ios::out | std::ios::trunc);
    s_csvHeaderWritten = false;
    if (s_csv.is_open())
    {
        s_csv << "simTime,nodeId,parent,hopcount,originated,sent,delivered,dropped,bufferOccupancy\n";
        s_csvHeaderWritten = true;
        s_csv.flush();
    }
}

uint32_t
TreeStructureApp::GetPacketsOriginated() const
{
    return m_packetsOriginated;
}

uint32_t
TreeStructureApp::GetPacketsSent() const
{
    return m_packetsSent;
}

uint32_t
TreeStructureApp::GetPacketsDelivered() const
{
    return m_packetsDelivered;
}

uint32_t
TreeStructureApp::GetPacketsDropped() const
{
    return m_packetsDropped;
}

int32_t
TreeStructureApp::GetNodeId() const
{
    return m_nodeId;
}

int32_t
TreeStructureApp::GetParentId() const
{
    return m_parentId;
}

int32_t
TreeStructureApp::GetHopcount() const
{
    return m_hopcount;
}

uint32_t
TreeStructureApp::GetBufferOccupancy() const
{
    return static_cast<uint32_t>(m_packetBuffer.size());
}

void
TreeStructureApp::StartApplication()
{
    m_running = true;
    m_nodeId = static_cast<int32_t>(GetNode()->GetId());
    m_localAddress = GetLocalAddress();

    if (m_nodeId == kRootId)
    {
        m_parentId = -1;
        m_hopcount = 0;
    }
    else
    {
        m_parentId = -1;
        m_hopcount = MobileAdhocTree::kInfiniteHops;
    }

    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), kAppPort);
    m_socket->Bind(local);
    m_socket->SetAllowBroadcast(true);
    m_socket->SetRecvCallback(MakeCallback(&TreeStructureApp::ReceivePacket, this));

    Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable>();
    jitter->SetAttribute("Min", DoubleValue(0.0));
    jitter->SetAttribute("Max", DoubleValue(m_heartbeatInterval));

    m_heartbeatEvent =
        Simulator::Schedule(Seconds(jitter->GetValue()), &TreeStructureApp::Heartbeat, this);
    m_txEvent = Simulator::Schedule(Seconds(m_txInterval), &TreeStructureApp::Transmit, this);
    if (m_nodeId != kRootId)
    {
        m_sendEvent = Simulator::Schedule(Seconds(jitter->GetValue() + m_sendInterval),
                                          &TreeStructureApp::GenerateTraffic,
                                          this);
    }
    m_statsEvent =
        Simulator::Schedule(Seconds(m_statsInterval), &TreeStructureApp::PrintData, this);
}

void
TreeStructureApp::StopApplication()
{
    m_running = false;
    CancelEvents();
    PrintData();

    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
TreeStructureApp::CancelEvents()
{
    Simulator::Cancel(m_sendEvent);
    Simulator::Cancel(m_txEvent);
    Simulator::Cancel(m_heartbeatEvent);
    Simulator::Cancel(m_statsEvent);
}

Ipv4Address
TreeStructureApp::GetLocalAddress() const
{
    Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
    if (!ipv4 || ipv4->GetNInterfaces() < 2)
    {
        return Ipv4Address();
    }
    return ipv4->GetAddress(1, 0).GetLocal();
}

void
TreeStructureApp::RefreshPosition()
{
    Ptr<MobilityModel> mobility = GetNode()->GetObject<MobilityModel>();
    if (mobility)
    {
        Vector pos = mobility->GetPosition();
        NS_LOG_DEBUG("Node " << m_nodeId << " at (" << pos.x << "," << pos.y << ")");
    }
}

void
TreeStructureApp::CheckParentTimeout()
{
    if (m_nodeId == kRootId)
    {
        m_parentId = -1;
        m_hopcount = 0;
        return;
    }
    if (m_parentId < 0)
    {
        return;
    }
    if (Simulator::Now() - m_lastParentBeacon > Seconds(m_heartbeatInterval * 3.0))
    {
        m_parentId = -1;
        m_hopcount = MobileAdhocTree::kInfiniteHops;
        m_parentAddress = Ipv4Address();
    }
}

void
TreeStructureApp::FillHeader(MobileAdhocTree& hdr, uint8_t type) const
{
    Ptr<MobilityModel> mobility = GetNode()->GetObject<MobilityModel>();
    Vector pos = mobility ? mobility->GetPosition() : Vector();

    hdr.SetType(type);
    hdr.SetMyId(m_nodeId);
    hdr.SetParent(m_parentId);
    hdr.SetHopcount(m_hopcount);
    hdr.SetIpv4(m_localAddress.Get());
    hdr.SetX(pos.x);
    hdr.SetY(pos.y);
}

void
TreeStructureApp::Heartbeat()
{
    if (!m_running)
    {
        return;
    }

    CheckParentTimeout();
    RefreshPosition();

    Ptr<Packet> packet = Create<Packet>();
    MobileAdhocTree hdr;
    FillHeader(hdr, MobileAdhocTree::BEACON);
    hdr.SetOriginator(m_nodeId);
    hdr.SetSeq(0);
    packet->AddHeader(hdr);

    m_socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), kAppPort));

    m_heartbeatEvent =
        Simulator::Schedule(Seconds(m_heartbeatInterval), &TreeStructureApp::Heartbeat, this);
}

void
TreeStructureApp::GenerateTraffic()
{
    if (!m_running)
    {
        return;
    }

    if (m_packetBuffer.size() >= m_bufferSize)
    {
        m_packetsDropped++;
        NS_LOG_INFO("Node " << m_nodeId << " dropped originated packet (buffer full)");
    }
    else
    {
        Ptr<Packet> packet = Create<Packet>();
        MobileAdhocTree hdr;
        FillHeader(hdr, MobileAdhocTree::DATA);
        hdr.SetOriginator(m_nodeId);
        hdr.SetSeq(m_nextSeq++);
        packet->AddHeader(hdr);
        m_packetBuffer.push(packet);
        m_packetsOriginated++;
    }

    m_sendEvent =
        Simulator::Schedule(Seconds(m_sendInterval), &TreeStructureApp::GenerateTraffic, this);
}

void
TreeStructureApp::Transmit()
{
    if (!m_running)
    {
        return;
    }

    if (!m_packetBuffer.empty() && m_parentId >= 0 && !m_parentAddress.IsAny() &&
        m_nodeId != kRootId)
    {
        Ptr<Packet> packet = m_packetBuffer.front();
        m_packetBuffer.pop();
        m_socket->SendTo(packet, 0, InetSocketAddress(m_parentAddress, kAppPort));
        m_packetsSent++;
    }

    m_txEvent = Simulator::Schedule(Seconds(m_txInterval), &TreeStructureApp::Transmit, this);
}

void
TreeStructureApp::HandleBeacon(const MobileAdhocTree& hdr, const Address& from)
{
    if (hdr.GetMyId() == m_nodeId)
    {
        return;
    }
    if (m_nodeId == kRootId)
    {
        return;
    }
    if (hdr.GetHopcount() >= MobileAdhocTree::kInfiniteHops)
    {
        return;
    }
    // Avoid one-hop routing loops.
    if (hdr.GetParent() == m_nodeId)
    {
        return;
    }

    Ipv4Address senderAddr = InetSocketAddress::ConvertFrom(from).GetIpv4();
    if (hdr.GetIpv4() != 0)
    {
        senderAddr = Ipv4Address(hdr.GetIpv4());
    }

    if (hdr.GetMyId() == m_parentId)
    {
        m_lastParentBeacon = Simulator::Now();
        m_hopcount = hdr.GetHopcount() + 1;
        m_parentAddress = senderAddr;
    }

    const int32_t candidateHops = hdr.GetHopcount() + 1;
    if (candidateHops < m_hopcount)
    {
        m_parentId = hdr.GetMyId();
        m_hopcount = candidateHops;
        m_parentAddress = senderAddr;
        m_lastParentBeacon = Simulator::Now();
    }
}

void
TreeStructureApp::HandleData(Ptr<Packet> packetWithHeader)
{
    if (m_nodeId == kRootId)
    {
        m_packetsDelivered++;
        return;
    }

    if (m_packetBuffer.size() >= m_bufferSize)
    {
        m_packetsDropped++;
        NS_LOG_INFO("Node " << m_nodeId << " dropped forwarded packet (buffer full)");
        return;
    }

    // Keep the on-wire header intact so the next hop can parse it.
    m_packetBuffer.push(packetWithHeader->Copy());
}

void
TreeStructureApp::ReceivePacket(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> packet;
    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0)
        {
            break;
        }

        MobileAdhocTree hdr;
        packet->PeekHeader(hdr);
        if (hdr.GetType() == MobileAdhocTree::BEACON)
        {
            HandleBeacon(hdr, from);
        }
        else if (hdr.GetType() == MobileAdhocTree::DATA)
        {
            HandleData(packet);
        }
    }
}

void
TreeStructureApp::PrintData()
{
    if (!s_csvHeaderWritten || !s_csv.is_open())
    {
        if (m_running)
        {
            m_statsEvent =
                Simulator::Schedule(Seconds(m_statsInterval), &TreeStructureApp::PrintData, this);
        }
        return;
    }

    const double now = Simulator::Now().GetSeconds();
    s_csv << now << "," << m_nodeId << "," << m_parentId << "," << m_hopcount << ","
          << m_packetsOriginated << "," << m_packetsSent << "," << m_packetsDelivered << ","
          << m_packetsDropped << "," << m_packetBuffer.size() << "\n";
    s_csv.flush();

    if (m_running)
    {
        m_statsEvent =
            Simulator::Schedule(Seconds(m_statsInterval), &TreeStructureApp::PrintData, this);
    }
}

} // namespace ns3
