
#ifndef sahara_HELPER_H
#define sahara_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

#include <map>
#include <set>

namespace ns3
{


class saharaHelper : public Ipv4RoutingHelper
{
  public:

    saharaHelper();
    ~saharaHelper() override;

    saharaHelper* Copy() const override;


    Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;


    void Set(std::string name, const AttributeValue& value);


  private:
    ObjectFactory m_agentFactory;
    
    
};

} 

#endif /* sahara_HELPER_H */