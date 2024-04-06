
#include "ns3/saharaHelper.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/saharaRouting.h"
#include "ns3/ptr.h"

namespace ns3
{

SaharaHelper::~SaharaHelper()
{
}


SaharaHelper::SaharaHelper () : Ipv4RoutingHelper()
{
    m_agentFactory.SetTypeId("ns3::sahara::SaharaRouting");
}


SaharaHelper*
SaharaHelper::Copy() const
{
    return new SaharaHelper(*this);
}


Ptr<Ipv4RoutingProtocol>
SaharaHelper::Create(Ptr<Node> node) const
{
    Ptr<sahara::SaharaRouting> agent = m_agentFactory.Create<sahara::SaharaRouting>();
    node->AggregateObject(agent);
    
    return agent;
}

void
SaharaHelper::Set(std::string name, const AttributeValue& value)
{
    m_agentFactory.Set(name, value);
}




} // namespace ns3
