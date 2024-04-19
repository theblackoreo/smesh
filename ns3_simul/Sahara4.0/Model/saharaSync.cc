#include "saharaSync.h"
#include <vector>
#include <tuple>
#include <cmath>

namespace ns3
{   /*
        This function is used in order to perform set reconciliation in a distributed way.
        The source node sends its BF to the neighbors
        The Neighbors will sign source as parent, reply with an ack and send its BF to own neighbors.
        If no ack is received in a timeframe:
            1) initial BF not received due to wifi conflics
            2) ack not received due to wifi conflics
        Partial solution: send again BF and/or ack
        If no ack is received in a timeframe for the second time -> no children
            - node without children can start set reconciliation with the parent
        If ack is received means that ne node has neighbors, no action are performed until all neighbors will 
        send back the BF to start set reconciliation backward
  */

    NS_LOG_COMPONENT_DEFINE("saharaSync");

    SaharaSync::SaharaSync(){}
    SaharaSync::SaharaSync(Ptr<Ipv4RoutingProtocol> routing) : 
        r_routing(routing)
    {}

    
       


}