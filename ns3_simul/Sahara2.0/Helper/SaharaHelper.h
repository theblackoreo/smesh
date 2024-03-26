
#ifndef SAHARA_HELPER_H
#define SAHARA_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"


namespace ns3 {

class GpsrHelper : public Ipv4RoutingHelper
{
public:
  SaharaHelper ();
  // This method is mainly for internal use by the other helpers;
  SaharaHelper* Copy (void) const;

  //This method will be called by ns3::InternetStackHelper::Install
  virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;

  //This method controls the attributes of ns3::sahara::RoutingProtocol
  void Set (std::string name, const AttributeValue &value);

  void Install (void) const;

private:
  ObjectFactory m_agentFactory;
};

}
#endif /* SAHARA_HELPER_H */
