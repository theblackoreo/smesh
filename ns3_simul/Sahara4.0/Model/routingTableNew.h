#ifndef ROUTING_TABLE_NEW_H
#define ROUTING_TABLE_NEW_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include "ns3/ipv4-address.h"


namespace ns3{

// Define a struct for storing information about IP nodes
struct IPNode {

    uint8_t reputation;
    uint64_t gpsCoordinate;
    uint8_t battery;
    
    // Constructor
    IPNode(uint8_t reputation, uint64_t gpsCoordinate, uint8_t battery);

    IPNode() : reputation(0), gpsCoordinate(0), battery(0) {}
};

// Define a struct for storing vote information
struct Vote {

    Ipv4Address judgeIP;
    Ipv4Address defendandIP;
    bool vote;
    std::string signature;
    
    // Constructor
    Vote(Ipv4Address judgeIP, Ipv4Address defendandIP, bool vote, std::string signature);
};

// Define a routing table class
class RoutingTableNew {
public:
    std::map<Ipv4Address, IPNode> m_ipInfos;
    std::vector<std::tuple<Ipv4Address, Ipv4Address>> m_neighborPairs;
    std::vector<Vote> m_votes;

    // Method to add IP node information
    void addIPNodeInfo(Ipv4Address ip, IPNode info);

    // Method to add neighbor information
    void addNeighborInfo(Ipv4Address ip1, Ipv4Address ip2);

    // Method to add a vote
    void addVote(Ipv4Address judgeIP, Ipv4Address defendantIP, bool vote, std::string signature);
};

}
#endif // ROUTING_TABLE_H
