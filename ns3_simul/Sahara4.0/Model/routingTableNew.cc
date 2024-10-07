#include "routingTableNew.h"


namespace ns3
{

// Implementing the constructors for the structs
IPNode::IPNode(uint8_t rep, uint64_t gpsCoordinate, uint8_t battery) 
    : reputation(rep), gpsCoordinate(gpsCoordinate), battery(battery) {}

Vote::Vote(Ipv4Address judgeIP, Ipv4Address defendandIP, bool vote, std::string signature) 
    : judgeIP(judgeIP), defendandIP(defendandIP), vote(vote), signature(signature) {}

// Implementing the methods for the RoutingTable class

// Method to add IP node information
void 
RoutingTableNew::addIPNodeInfo(Ipv4Address ip, IPNode info) {
    m_ipInfos[ip] = info;
}

// Method to add neighbor information
void 
RoutingTableNew::addNeighborInfo(Ipv4Address ip1, Ipv4Address ip2) {
    m_neighborPairs.push_back(std::tuple(ip1, ip2));
}

// Method to add a vote
void 
RoutingTableNew::addVote(Ipv4Address judgeIP, Ipv4Address defendantIP, bool vote, std::string signature) {
    m_votes.push_back(Vote(judgeIP, defendantIP, vote, signature));
}



}
