#include "Routing_Table.h"
#include <tuple>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>


namespace ns3 {

 //it is a macro that ensures that the class is registered with the NS3 type system
 // it is necessary for NS3
NS_OBJECT_ENSURE_REGISTERED (Routing_Table);

// TypeId is a ns3 class to uniquely identify types within the simulation framework
TypeId Routing_Table::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Routing_Table") //unique identifier for the class
    .SetParent<Object> ()
    .AddConstructor<Routing_Table> ();
  return tid;
}

private:
    std::vector<std::tuple<Ipv4Address, Ipv4Address, int, int, int, std::string, time_t>> routing_table;

public:
    //constructor
    Routing_Table::Routing_Table () { 
        std::cout << "Creating routing table for node " << node->GetId() << std::endl;
    }


// Method to get the routing table
    std::vector<std::tuple<Ipv4Address, Ipv4Address, int, int, int, std::string, time_t>> getRoutingTable() const {
        return routing_table;
    }


/*
void Routing_Table::add_entry (Ipv4Address origin_IP, Ipv4Address next_hop_IP, 
                    int reputation, int GPS, int battery, std::string, time_t timestamp)
{
  std::tuple<Ipv4Address, Ipv4Address, int, int, int, string, time_t> entry;
  entry = std::make_tuple(origin_IP, next_hop_IP, reputation, GPS, battery, signature, timestamp);
  routing_table.push_back(entry);
}
*/

}

