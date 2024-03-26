#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/ipv4.h"
#include "ns3/internet-module.h"


namespace ns3 {

// inherits from Object
class Routing_Table : public Object
{
public:
  static TypeId GetTypeId (); // returns the type ID of the class (used by NS3 internally)

  Routing_Table (); // constructor
    /*
  void add_entry (Ipv4Address origin_IP, Ipv4Address next_hop_IP, 
                    int reputation, int GPS, int battery, std::string signature, time_t timestamp);
  
  void removeAllEntries();
  */
};

} // namespace ns3

