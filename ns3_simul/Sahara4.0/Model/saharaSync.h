
#ifndef SAHARA_SYNC_H
#define SAHARA_SYNC_H

#include "routingTable.h"
#include "ns3/ipv4-routing-protocol.h"


namespace ns3{


class SaharaSync {
    public:
        SaharaSync();
        SaharaSync(Ptr<Ipv4RoutingProtocol> routing);

    private:
        Ptr<Ipv4RoutingProtocol> r_routing;
        
    };

} // SAHARA_SYNC_H

#endif