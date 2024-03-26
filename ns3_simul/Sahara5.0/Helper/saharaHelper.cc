
#include "ns3/saharaHelper.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/saharaRouting.h"
#include "ns3/ptr.h"

namespace ns3
{

saharaHelper::~saharaHelper()
{
}


saharaHelper::saharaHelper () : Ipv4RoutingHelper()
{
    m_agentFactory.SetTypeId("ns3::sahara::RoutingProtocol");
}


saharaHelper*
saharaHelper::Copy() const
{
    return new saharaHelper(*this);
}


Ptr<Ipv4RoutingProtocol>
saharaHelper::Create(Ptr<Node> node) const
{
    Ptr<sahara::RoutingProtocol> agent = m_agentFactory.Create<sahara::RoutingProtocol>();
    node->AggregateObject(agent);
    return agent;
}

void
saharaHelper::Set(std::string name, const AttributeValue& value)
{
    m_agentFactory.Set(name, value);
}




} // namespace ns3
