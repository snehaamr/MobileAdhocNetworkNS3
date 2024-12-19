#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "MobileAdhocTree.h"

namespace ns3 {

class TreeStructureApp : public Application 
{
public:
    TreeStructureApp (Ptr<Socket> socket, uint32_t bufferSize);
    virtual ~TreeStructureApp();
    
    void StartApplication(void) override;
    void StopApplication(void) override;
    void SendPacket(void);
    void ReceivePacket(Ptr<Socket> socket);
    void Heartbeat(void);

    void PrintData(void);

private:
    Ptr<Socket> m_socket;
    uint32_t m_packetSize;
    uint32_t m_bufferSize;  // Buffer size (max number of packets)
    uint32_t m_packetsSent;
    uint32_t m_packetsDropped;  // Track dropped packets
    uint32_t m_packetsDelivered;  // Track delivered packets

    std::queue<Ptr<Packet>> m_packetBuffer;  // Queue to simulate packet buffer
    MobileAdhocTree payload;
    MobileAdhocTree parent_payload;

    EventId m_sendEvent;
    bool m_running;
};

} // namespace ns3
