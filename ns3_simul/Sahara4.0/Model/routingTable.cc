

#include "routingTable.h"


#include <vector>
#include <tuple>

namespace ns3
{
	NS_LOG_COMPONENT_DEFINE("routingTable");

	RoutingTable::RoutingTable(){}
	
    
	bool
	RoutingTable::AddTuple(Ipv4Address originIP, Ipv4Address hop1IP, uint8_t reputation, uint8_t GPS, uint8_t battery){
		tuples.push_back(std::make_tuple(originIP, hop1IP, reputation, GPS, battery));
        
		return true;
    
	}
	
    std::tuple<Ipv4Address,Ipv4Address,uint8_t,uint8_t,uint8_t>
    RoutingTable::getLastTupleTest(){

        if (tuples.empty()) {
            
            NS_LOG_DEBUG("Routing table empty");
            return std::make_tuple(Ipv4Address("0.0.0.0"), Ipv4Address("0.0.0.0"), 0, 0, 0);
        }
        else {
         auto lastTuple = tuples.back();
         return lastTuple;
        }
    }


    bool
    RoutingTable::CheckDuplicate(Ipv4Address originIP, Ipv4Address hop1IP){
            
            // Iterate through each tuple in the vector
        for (const auto& tuple : tuples) {
            
            // Check if myIP matches the first element of the current tuple
            if (std::get<0>(tuple) == originIP and std::get<1>(tuple) == hop1IP) {
                return true; // Found a match, return true
            }
        }
        return false; // IP not found in any tuple

    }

    size_t
    RoutingTable::GetSize(){
        
        
        size_t tuplesSize = tuples.size();
        NS_LOG_DEBUG(tuplesSize);
        return tuplesSize;
       
    }


    void
    RoutingTable::PrintAll(){
        NS_LOG_DEBUG("Printing routing table...");
         for (const auto& tuple : tuples) {
            // Extract elements from the tuple
            const Ipv4Address& originIP = std::get<0>(tuple);
            const Ipv4Address& hop1IP = std::get<1>(tuple);
            uint8_t rep = std::get<2>(tuple);
            uint8_t GPS = std::get<3>(tuple);
            uint8_t battery = std::get<4>(tuple);
            
            // Print elements
             NS_LOG_DEBUG("Tuple: " << originIP << " " << hop1IP << " " << rep << " " << GPS << " " << battery);
    }
     NS_LOG_DEBUG("End______________________________________");
    }

}