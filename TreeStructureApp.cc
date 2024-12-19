#include "TreeStructureApp.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include <queue>

namespace ns3 {

TreeStructureApp::TreeStructureApp(Ptr<Socket> socket, uint32_t bufferSize)
    : m_socket(socket),
      m_packetSize(sizeof(MobileAdhocTree)),
      m_bufferSize(bufferSize),
      m_packetsSent(0),
      m_packetsDropped(0),
      m_packetsDelivered(0),
      m_running(false)
{
    payload = MobileAdhocTree();
    parent_payload = MobileAdhocTree();
}

TreeStructureApp::~TreeStructureApp()
{
    m_socket = 0;
}

void TreeStructureApp::StartApplication(void)
{
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
    InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), 80);

    m_socket->Bind(local);
    m_socket->SetAllowBroadcast(true);
    m_socket->Connect(remote);
    m_socket->SetRecvCallback(MakeCallback(&TreeStructureApp::ReceivePacket, this));

    payload.my_id = (int)m_socket->GetNode()->GetId();
    payload.x = m_socket->GetNode()->GetObject<MobilityModel>()->GetPosition().x;
    payload.y = m_socket->GetNode()->GetObject<MobilityModel>()->GetPosition().y;

    // If node is root (node 1), initialize with specific values
    if ((int)m_socket->GetNode()->GetId() == 1) {
        payload.parent = 0;
        payload.hopcount = 0;
    }

    // Schedule packet send event
    UniformVariable rand(0, 8);
    double tmp = rand.GetValue();
    m_sendEvent = Simulator::Schedule(Seconds(tmp), &TreeStructureApp::SendPacket, this);
}

void TreeStructureApp::StopApplication(void)
{
    m_running = false;

    if (m_sendEvent.IsRunning()) {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket) {
        m_socket->Close();
    }

    // Print final statistics
    PrintData();
}

void TreeStructureApp::SendPacket(void)
{
    // Check if the buffer is full, if so drop the packet
    if (m_packetBuffer.size() >= m_bufferSize) {
        m_packetsDropped++;
        std::cout << "Node " << payload.my_id << " dropped a packet. Buffer full!" << std::endl;
        return;
    }

    // Enqueue the packet to the buffer
    Ptr<Packet> packet = Create<Packet>((uint8_t*)&payload, m_packetSize);
    m_packetBuffer.push(packet);

    // Send the packet if there is space in the buffer
    if (!m_packetBuffer.empty()) {
        m_socket->Send(m_packetBuffer.front());
        m_packetBuffer.pop();
        m_packetsSent++;
        std::cout << "Node " << payload.my_id << " sent a packet." << std::endl;
    }

    // Schedule the next packet send
    m_sendEvent = Simulator::Schedule(Seconds(2.5), &TreeStructureApp::SendPacket, this);
}

void TreeStructureApp::ReceivePacket(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    MobileAdhocTree temp_payload;

    while (packet = socket->RecvFrom(from)) {
        if (packet->GetSize() == 0) {
            break;
        }

        packet->CopyData((uint8_t*)&temp_payload, m_packetSize);

        // Simulate data loss if buffer is full
        if (m_packetBuffer.size() < m_bufferSize) {
            m_packetBuffer.push(packet);  // Put received packet in the buffer
            m_packetsDelivered++;
        } else {
            m_packetsDropped++;  // Drop the packet if buffer is full
        }
    }

    // Print data periodically
    PrintData();
}

void TreeStructureApp::Heartbeat(void)
{
    if (payload.parent == parent_payload.parent) {
        if (payload.hopcount != parent_payload.hopcount + 1) {
            payload.hopcount = parent_payload.hopcount + 1;
        }
    } else {
        payload.hopcount = 1234;
        payload.parent = -1;
    }
}

void TreeStructureApp::PrintData(void)
{
    int timeInSec = (int)Simulator::Now().GetSeconds();
    if (timeInSec % 10 == 0) {
        std::cout << "NodeNo   Sent    Delivered   Dropped   Time" << std::endl;
    }

    std::cout << payload.my_id << "     " 
              << m_packetsSent << "      "
              << m_packetsDelivered << "        "
              << m_packetsDropped << "        "
              << timeInSec << std::endl;
}

}  // namespace ns3
