#include "MobileAdhocTree.h"

#include "ns3/buffer.h"
#include "ns3/log.h"

#include <cstring>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MobileAdhocTree");
NS_OBJECT_ENSURE_REGISTERED(MobileAdhocTree);

static uint64_t
DoubleToU64(double value)
{
    uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static double
U64ToDouble(uint64_t bits)
{
    double value = 0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

MobileAdhocTree::MobileAdhocTree()
    : m_type(BEACON),
      m_seq(0),
      m_hopcount(kInfiniteHops),
      m_parent(-1),
      m_myId(-1),
      m_originator(-1),
      m_ipv4(0),
      m_x(0.0),
      m_y(0.0)
{
}

MobileAdhocTree::~MobileAdhocTree()
{
}

TypeId
MobileAdhocTree::GetTypeId()
{
    static TypeId tid = TypeId("ns3::MobileAdhocTree")
                            .SetParent<Header>()
                            .SetGroupName("Applications")
                            .AddConstructor<MobileAdhocTree>();
    return tid;
}

TypeId
MobileAdhocTree::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
MobileAdhocTree::GetSerializedSize() const
{
    // type(1) + seq(4) + hopcount(4) + parent(4) + myId(4) + originator(4) + ipv4(4) + x(8) + y(8)
    return 41;
}

void
MobileAdhocTree::Serialize(Buffer::Iterator start) const
{
    start.WriteU8(m_type);
    start.WriteHtonU32(m_seq);
    start.WriteHtonU32(static_cast<uint32_t>(m_hopcount));
    start.WriteHtonU32(static_cast<uint32_t>(m_parent));
    start.WriteHtonU32(static_cast<uint32_t>(m_myId));
    start.WriteHtonU32(static_cast<uint32_t>(m_originator));
    start.WriteHtonU32(m_ipv4);
    start.WriteHtonU64(DoubleToU64(m_x));
    start.WriteHtonU64(DoubleToU64(m_y));
}

uint32_t
MobileAdhocTree::Deserialize(Buffer::Iterator start)
{
    m_type = start.ReadU8();
    m_seq = start.ReadNtohU32();
    m_hopcount = static_cast<int32_t>(start.ReadNtohU32());
    m_parent = static_cast<int32_t>(start.ReadNtohU32());
    m_myId = static_cast<int32_t>(start.ReadNtohU32());
    m_originator = static_cast<int32_t>(start.ReadNtohU32());
    m_ipv4 = start.ReadNtohU32();
    m_x = U64ToDouble(start.ReadNtohU64());
    m_y = U64ToDouble(start.ReadNtohU64());
    return GetSerializedSize();
}

void
MobileAdhocTree::Print(std::ostream& os) const
{
    os << "type=" << static_cast<uint32_t>(m_type) << " id=" << m_myId << " parent=" << m_parent
       << " hops=" << m_hopcount << " originator=" << m_originator << " seq=" << m_seq;
}

void
MobileAdhocTree::SetType(uint8_t type)
{
    m_type = type;
}

uint8_t
MobileAdhocTree::GetType() const
{
    return m_type;
}

void
MobileAdhocTree::SetSeq(uint32_t seq)
{
    m_seq = seq;
}

uint32_t
MobileAdhocTree::GetSeq() const
{
    return m_seq;
}

void
MobileAdhocTree::SetHopcount(int32_t hopcount)
{
    m_hopcount = hopcount;
}

int32_t
MobileAdhocTree::GetHopcount() const
{
    return m_hopcount;
}

void
MobileAdhocTree::SetParent(int32_t parent)
{
    m_parent = parent;
}

int32_t
MobileAdhocTree::GetParent() const
{
    return m_parent;
}

void
MobileAdhocTree::SetMyId(int32_t myId)
{
    m_myId = myId;
}

int32_t
MobileAdhocTree::GetMyId() const
{
    return m_myId;
}

void
MobileAdhocTree::SetOriginator(int32_t originator)
{
    m_originator = originator;
}

int32_t
MobileAdhocTree::GetOriginator() const
{
    return m_originator;
}

void
MobileAdhocTree::SetIpv4(uint32_t ipv4)
{
    m_ipv4 = ipv4;
}

uint32_t
MobileAdhocTree::GetIpv4() const
{
    return m_ipv4;
}

void
MobileAdhocTree::SetX(double x)
{
    m_x = x;
}

double
MobileAdhocTree::GetX() const
{
    return m_x;
}

void
MobileAdhocTree::SetY(double y)
{
    m_y = y;
}

double
MobileAdhocTree::GetY() const
{
    return m_y;
}

} // namespace ns3
