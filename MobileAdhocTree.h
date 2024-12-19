#ifndef MOBILE_ADHOC_TREE_H
#define MOBILE_ADHOC_TREE_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3 {
  
class MobileAdhocTree {
public:
    int hopcount;
    int parent;
    int my_id;
    double x;
    double y;

    MobileAdhocTree();
    ~MobileAdhocTree();
};

} // namespace ns3

#endif // MOBILE_ADHOC_TREE_H
