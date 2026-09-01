#ifndef MOBILE_ADHOC_TREE_H
#define MOBILE_ADHOC_TREE_H

#include "ns3/header.h"

#include <cstdint>
#include <ostream>

namespace ns3 {

/**
 * On-the-wire header for tree beacons and forwarded data packets.
 */
class MobileAdhocTree : public Header
{
  public:
    static const uint8_t BEACON = 0;
    static const uint8_t DATA = 1;
    static const int32_t kInfiniteHops = 10000;

    MobileAdhocTree();
    ~MobileAdhocTree() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;

    void SetType(uint8_t type);
    uint8_t GetType() const;

    void SetSeq(uint32_t seq);
    uint32_t GetSeq() const;

    void SetHopcount(int32_t hopcount);
    int32_t GetHopcount() const;

    void SetParent(int32_t parent);
    int32_t GetParent() const;

    void SetMyId(int32_t myId);
    int32_t GetMyId() const;

    void SetOriginator(int32_t originator);
    int32_t GetOriginator() const;

    void SetIpv4(uint32_t ipv4);
    uint32_t GetIpv4() const;

    void SetX(double x);
    double GetX() const;

    void SetY(double y);
    double GetY() const;

  private:
    uint8_t m_type;
    uint32_t m_seq;
    int32_t m_hopcount;
    int32_t m_parent;
    int32_t m_myId;
    int32_t m_originator;
    uint32_t m_ipv4;
    double m_x;
    double m_y;
};

} // namespace ns3

#endif // MOBILE_ADHOC_TREE_H
