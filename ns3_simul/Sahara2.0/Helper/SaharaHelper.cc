
#include "SaharaHelper.h"
#include "ns3/sahara.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/node-container.h"
#include "ns3/callback.h"
#include "ns3/udp-l4-protocol.h"
 

namespace ns3 {

GpsrHelper::GpsrHelper ()
  : Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::gpsr::RoutingProtocol");
}

GpsrHelper*
GpsrHelper::Copy (void) const
{
  return new GpsrHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
GpsrHelper::Create (Ptr<Node> node) const
{
  //Ptr<Ipv4L4Protocol> ipv4l4 = node->GetObject<Ipv4L4Protocol> ();
  Ptr<gpsr::RoutingProtocol> gpsr = m_agentFactory.Create<gpsr::RoutingProtocol> ();
  //gpsr->SetDownTarget (ipv4l4->GetDownTarget ());
  //ipv4l4->SetDownTarget (MakeCallback (&gpsr::RoutingProtocol::AddHeaders, gpsr));
  node->AggregateObject (gpsr);
  return gpsr;
}

void
GpsrHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}


void 
GpsrHelper::Install (void) const
{
  NodeContainer c = NodeContainer::GetGlobal ();
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = (*i);
      Ptr<UdpL4Protocol> udp = node->GetObject<UdpL4Protocol> ();
      Ptr<gpsr::RoutingProtocol> gpsr = node->GetObject<gpsr::RoutingProtocol> ();
      gpsr->SetDownTarget (udp->GetDownTarget ());
      udp->SetDownTarget (MakeCallback(&gpsr::RoutingProtocol::AddHeaders, gpsr));
    }


}

SaharaHelper::~SaharaHelper()
{
}

}
