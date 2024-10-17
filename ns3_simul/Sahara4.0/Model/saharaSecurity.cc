#include "saharaSecurity.h"
#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/simulator.h"
#include <algorithm>  
#include "ns3/uinteger.h"
#include "ns3/ipv4-l3-protocol.h" 
#include "ns3/udp-header.h"
#include "routingTable.h"
#include "ns3/timer.h"
#include <algorithm>
#include "globalData.h"
#include <iostream>
#include <tuple>
#include <string>
#include <limits>
#include <fstream>
#include <vector>
#include <cmath>

/* 
This is related to the second part of the project, when a packet is captured by a node the PromiscuousCallback function 
is called. 
In order to distinguish control packet (packets used by sahara routing to build routing tables and so on) between data packets 
(standard data messages) the simples idea is looking at the port. 
Because saharaRouting process uses a specific port that is 256.
*/
namespace ns3{


NS_LOG_COMPONENT_DEFINE("saharaSecurity");

namespace sahara{



SaharaSecurity::SaharaSecurity(RoutingTable& routingTable) : m_rTable(routingTable)
{
    m_myNodeID = m_rTable.GetMyNodeID();
}

SaharaSecurity::~SaharaSecurity()
{
}



// It is called when SAHARA Sync terminates but it is not totally correct in the real scenario 
// because every node already knows the list of mac, id and ip of all other devices in the network. 
void 
SaharaSecurity::LoadMacIdMap() {

    std::ifstream infile("macAddresses.txt");

    if (!infile.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return ; // Return an empty map if the file couldn't be opened
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string id, mac;
        
        if (!(iss >> id >> mac)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue; // Skip malformed lines
        }

        m_mapMac[mac] = id;
       
    }

    infile.close();
    
}


void
SaharaSecurity::LoadLastBlock(){
    /*
    std::ifstream infile("historyRoutingTable_" + std::to_string(m_myNodeID) + ".txt");
    std::string line;
    std::vector<std::string> lastRoutingTable;
    bool isRoutingTable = false;

    if (!infile.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }

    while (std::getline(infile, line)) {
        if (line.find("Routing Table:") != std::string::npos) {
            isRoutingTable = true;
            lastRoutingTable.clear();  // Start a new routing table
            continue;
        }
        if (line.find("End of Block") != std::string::npos) {
            isRoutingTable = false;
            continue;
        }
        if (isRoutingTable && !line.empty()) {
            lastRoutingTable.push_back(line);
        }
    }

    infile.close();

    std::cout << "Last Routing Table:" << std::endl;
    for (const auto& entry : lastRoutingTable) {
        std::cout << entry << std::endl;
    }

    */
    

std::vector<Ipv4Address> myNeigh = m_rTable.GetVectOfNeigByIP(m_myIP);
std::map<Ipv4Address, uint16_t> mapIDRep = m_rTable.getMapIDRep();

// Iterate over mapIDRep and copy its values to m_idVotes if the IP is in myNeigh
for (const auto& entry : mapIDRep) {
    Ipv4Address nodeIP = entry.first;
    uint16_t value = entry.second;

    // Use std::find to check if nodeIP is in myNeigh
    if (std::find(myNeigh.begin(), myNeigh.end(), nodeIP) != myNeigh.end()) {
        // Cast the value to double and insert into m_idVotes
        m_idVotes[nodeIP] = std::make_pair(static_cast<double>(value), false);
        NS_LOG_DEBUG("Node " << nodeIP << " Not casted " << value << " Casted value " << m_idVotes[nodeIP].first);
    }
}



}

bool 
SaharaSecurity::PromiscuousCallback(ns3::Ptr<ns3::NetDevice> device, ns3::Ptr<const ns3::Packet> packet, uint16_t protocol, const ns3::Address &source, const ns3::Address &destination, ns3::NetDevice::PacketType packetType){

    ns3::Ptr<ns3::Packet> packetCopy = packet->Copy();
    
    ns3::Ipv4Header ipv4Header;
    packetCopy->RemoveHeader(ipv4Header);

    ns3::Ptr<ns3::Node> node = device->GetNode();
    ns3::Ptr<ns3::Ipv4> ipv4 = node->GetObject<ns3::Ipv4>();
    ns3::Ipv4Address deviceIP = ipv4->GetAddress(ipv4->GetInterfaceForDevice(device), 0).GetLocal();
    
    ns3::UdpHeader udpHeader;
    packetCopy->PeekHeader(udpHeader);
    uint16_t port = udpHeader.GetDestinationPort();
    
    
    ns3::Mac48Address addr = ns3::Mac48Address::ConvertFrom(source);
    std::ostringstream oss;
    oss << addr;

    std::string sourceIPString = "10.1.1." + m_mapMac[oss.str()];
    Ipv4Address sourceIP = Ipv4Address(sourceIPString.c_str());

    
    ns3::Mac48Address addrNextHop = ns3::Mac48Address::ConvertFrom(destination);
    std::ostringstream hop;
    hop << addrNextHop;
    std::string nextHopIPString = "10.1.1." + m_mapMac[hop.str()];
    Ipv4Address nextHopIP = Ipv4Address(nextHopIPString.c_str());

    m_deviceIP = deviceIP;
    m_nextHopIp = nextHopIP;
    m_sourceIP = sourceIP;
    m_fromIP = ipv4Header.GetSource();
    m_destIP = ipv4Header.GetDestination();
    m_packetID = packet->GetUid();


    if(port==256 || port==12345){}

    
    switch (port)
    {
    case 256:
       // NS_LOG_DEBUG("Sahara Routing packet detected");
        break;
    
    case 12345:
        NS_LOG_DEBUG("Promiscuous data packet received. Device IP: " << deviceIP << ", PacketID: "<< packet->GetUid() <<", source: " 
                            << sourceIP << ", port: " << port << ", packet from: " << ipv4Header.GetSource() << 
                                        ", packet dest: " << ipv4Header.GetDestination() << ", next hop (MAC): " << nextHopIP);
        
        //NS_LOG_DEBUG("Standard message detected");
        // deviceIP, source (MAC), packet from, packet dest, nextHop

        // function to verify is packet has been dropped
        VerifyDrop(packetCopy, deviceIP, sourceIP, ipv4Header.GetSource(), ipv4Header.GetDestination(), nextHopIP);
        break;
    
    default:
        break;
    }
    return true;  // Indicate the packet has been processed

}


 /*
    A -> B -> C
    \   /
     \ /
      O
    In this case A wants to send a packet to B. 
    O will capture the packet after B already forwards it. So O could capture the packet forwarded from B before 
    capture the packet from A. 
    The solutions is to maintain an history of packets received in B.

    A sends a packet with ID = 223
    O receives:

    sniffed: 1. packetID 223, source B, nextHop C

    + history.add(packetID 223, source B)
    check if in the timers I have a packetID 223 linked with node C
    if yes: 
        if I have a timer ON -> Stop and delete tuple
    if not:
        check if I have in the history a packetID 223 and source C
        if yes:
            // means already received 
            return;
        if not:
            // means to start timer
        start timer with packetID 223, linked to node C
    
    sniffed: 2. packetID 223, source A, nextHop B
    + history.add(packetID 223, source A)

    check if in the timers I have a packetID 223 linked with node B
    if yes: 
        if I have a timer ON -> Stop and delete tuple
    if not:
        check if I have in the history a packetID 223 and source B
        if yes:
            // means alredy received
            return;
        if not:
            // means to start timer
            start timer with packetID 223, linked to nextHop
    so in general is:

     sniffed: packetID, sourceIP, nextHopIP ...

    + history.add(packetID, sourceIP)

    check if in the timers I have a packetID linked with nextHopIP
    if yes: 
        if I have a timer ON -> Stop and delete tuple
    if not:
        check if I have in the history a packetID and nextHopIP
        if yes:
            // means alredy received
            return;
        if not:
            // means to start timer
            start timer with packetID, linked to nextHop

    data structures?
    history -> packetID, source (vector of tuples)
    timers -> PacketID, nextHopID : timer
    */
   

void SaharaSecurity::VerifyDrop(Ptr<Packet> &packet, Ipv4Address deviceIP, Ipv4Address detectedSourceIP, Ipv4Address sourceIP, Ipv4Address destIP, Ipv4Address nextHopIp) {
    // deviceIP, source (MAC), detected source, packet from, packet dest,  nextHop
   
    // I'm the next hop so ignore
    if (deviceIP == nextHopIp) {
        UpdateNodeReputationPositive(detectedSourceIP);
        return;
    }

    //  + history.add(packetID, sourceIP)
    std::tuple<uint32_t, Ipv4Address> temp(packet->GetUid(), nextHopIp);
    m_historyPackets.push_back(temp);

    // check if in the timers I have a packetID linked with nextHopIP
    auto it = m_mapTimers.find(packet->GetUid());
    if (it != m_mapTimers.end()) {
        NS_LOG_DEBUG("Timer found ");
        auto innerIt = it->second.find(detectedSourceIP);
        if (innerIt != it->second.end()) {
            if(innerIt->second->IsRunning()){
                it->second.erase(innerIt);
                if (it->second.empty()) {
                    innerIt->second->Cancel();
                    m_mapTimers.erase(it);
                }
                NS_LOG_DEBUG(deviceIP << ": Timer canceled for packet " << packet->GetUid() << " and node " << detectedSourceIP);
                UpdateNodeReputationPositive(detectedSourceIP);
                return;
            }
        }
        else{
            NS_LOG_DEBUG("Not found inner key");
        }
    }
    else{
        
        //  check if I have in the history a packetID and nextHopIP
        std::tuple<uint32_t, Ipv4Address> toSearch(packet->GetUid(), sourceIP);
        auto it = std::find(m_historyPackets.begin(), m_historyPackets.end(), toSearch);
        if (it != m_historyPackets.end()) {
            m_historyPackets.erase(it);
            NS_LOG_DEBUG(deviceIP << ": Packet " << packet->GetUid() << " already received with source " << sourceIP);
            UpdateNodeReputationPositive(detectedSourceIP);
            return;
        } 
        else if(m_rTable.checkExistenceOfNegh(deviceIP, nextHopIp) && destIP != nextHopIp){
            Timer* temp = new Timer;
            temp->SetFunction(&SaharaSecurity::DropTimeoutExpired, this);
            temp->SetArguments(deviceIP, detectedSourceIP, nextHopIp);
            temp->Schedule(MilliSeconds(m_timoutRetransmittingPacket));
            m_mapTimers[packet->GetUid()][nextHopIp] = temp;
            NS_LOG_DEBUG(deviceIP << ": Timer created for packet " << packet->GetUid() << " and node (nextHop) " << m_nextHopIp);
        }
       
        
    }

}

// packets has been not forwarded by a node before time expires
void
SaharaSecurity::DropTimeoutExpired(Ipv4Address deviceIP, Ipv4Address sourceIP, Ipv4Address nextHopIP){

    NS_LOG_DEBUG("Timer expired, Device " << deviceIP << ": sourceIP " << sourceIP << ", node potentially malicius " << nextHopIP);

    // -1 on reputation of that node, return true if rep is still in range otherwise to be isolate
    if(UpdateNodeReputation(nextHopIP)){

    }else{
        NS_LOG_DEBUG("Node to isolate ->" << nextHopIP);
        // to be isolated
        GenerateNewKey(nextHopIP);

    }
      
}

void
SaharaSecurity::GenerateNewKey(Ipv4Address nextHopIP){

    std::ifstream infile("ips.txt");

    std::ofstream tempFile("temp.txt");
    if (!tempFile.is_open()) {
        std::cerr << "Error opening temporary file." << std::endl;
        return;
    }

    std::string line;
    bool found = false;
    while (getline(infile, line)) {
        std::ostringstream oss;
        nextHopIP.Print(oss);
        if (line.find(oss.str()) == std::string::npos) {
            tempFile << line << std::endl;
        } else {
            found = true;
        }
    }

    infile.close();
    tempFile.close();

    if (found) {
        if (remove("ips.txt") != 0) {
            std::cerr << "Error deleting original file." << std::endl;
        } else if (rename("temp.txt", "ips.txt") != 0) {
            std::cerr << "Error renaming temporary file." << std::endl;
        } else {
            std::cout << "IP address deleted successfully." << std::endl;
        }
    } else {
        std::cerr << "IP address not found in the file." << std::endl;
        remove("temp.txt"); // Clean up the temporary file
    }

    std::string l;
    while (std::getline(infile, l)) {
        std::istringstream iss(l);
        std::string id, mac;
        
        if (!(iss >> id >> mac)) {
            std::cerr << "Error reading line: " << l << std::endl;
            continue; // Skip malformed lines
        }

        m_mapMac[mac] = id;
       
    }

    infile.close();

    
    // emulate key generation removing ip from the list
    auto it = std::find(GlobalData::m_allowedIPs.begin(), GlobalData::m_allowedIPs.end(), nextHopIP);
    if (it != GlobalData::m_allowedIPs.end()) {
        // Step 4: Use erase to remove the element
        GlobalData::m_allowedIPs.erase(it);
        return;
    }
   
}

bool
SaharaSecurity::UpdateNodeReputation(Ipv4Address detectedNodeIP){
      /*
      uint16_t detectedNodeID;
      std::ostringstream oss;
      nodeIP.Print(oss);
      std::string ip_string = oss.str();
      size_t lastDotPos = ip_string.find_last_of('.');
      if (lastDotPos != std::string::npos) {
          // Extract the substring after the last dot
          std::string numberAfterLastDot = ip_string.substr(lastDotPos + 1);
          std::istringstream iss(numberAfterLastDot);
          iss>>detectedNodeID;
      } else {
      }
    */

    /* we start from previous reputation score and :
    - for a good action -> reputation increase logaritmically
    - for a bad action -> repuation decreases exponentially
    */
    auto it = m_idVotes.find(detectedNodeIP);
    if (it == m_idVotes.end()) {
        m_idVotes[detectedNodeIP].first = m_rTable.GetNodeReputation(detectedNodeIP);
    }

    NS_LOG_DEBUG("detected node: " << detectedNodeIP << "status of vote before decrementing: "  << m_idVotes[detectedNodeIP].first);
    m_idVotes[detectedNodeIP].first = m_idVotes[detectedNodeIP].first - (1 - 0.5)*m_idVotes[detectedNodeIP].first;
    m_idVotes[detectedNodeIP].second = true;
    
    NS_LOG_DEBUG("detected node: " << detectedNodeIP << "status of vote: "  << m_idVotes[detectedNodeIP].first);

    return true;
}

void
SaharaSecurity::UpdateNodeReputationPositive(Ipv4Address detectedNodeIP){
    auto it = m_idVotes.find(detectedNodeIP);
    if (it == m_idVotes.end()) {
        m_idVotes[detectedNodeIP].first = m_rTable.GetNodeReputation(detectedNodeIP);
    }

    NS_LOG_DEBUG("detected node: " << detectedNodeIP << "status of vote before INCREMENTING: "  << m_idVotes[detectedNodeIP].first);
    m_idVotes[detectedNodeIP].first = m_idVotes[detectedNodeIP].first + (0.1)*m_idVotes[detectedNodeIP].first;
    m_idVotes[detectedNodeIP].second = true;
    
    NS_LOG_DEBUG("detected node: " << detectedNodeIP << "status of vote: "  << m_idVotes[detectedNodeIP].first);
    
}
SaharaHeader::VoteState 
SaharaSecurity::GetVoteByNodeIP(Ipv4Address nodeIp){
    /*
    std::ostringstream oss1;
    nodeIp.Print(oss1);
    std::string ip_string1 = oss1.str();
    size_t lastDotPos1 = ip_string1.find_last_of('.');
    uint16_t nodeIDTuple1;
    if (lastDotPos1 != std::string::npos) {
        // Extract the substring after the last dot for the first IP
        std::string numberAfterLastDot1 = ip_string1.substr(lastDotPos1 + 1);
        std::istringstream iss1(numberAfterLastDot1);
        iss1 >> nodeIDTuple1;
    } else {
        NS_LOG_DEBUG("error getting ID from first IP");
    }
    */

    if(m_idVotes[nodeIp].first == 0 || m_idVotes[nodeIp].second == false){
        return SaharaHeader::VoteState::Undefined;
    }
    else if(m_idVotes[nodeIp].first < 50){
        return SaharaHeader::VoteState::Negative;
    } 
    else {
        return SaharaHeader::VoteState::Positive;
    }


}

SaharaHeader::VotePacket 
SaharaSecurity::GetMyVotesList() {
    std::list<SaharaHeader::Vote> myVoteList;
    
    for(auto t = m_idVotes.begin(); t != m_idVotes.end(); ++t) {
        SaharaHeader::Vote toAdd;
        toAdd.EvaluatedIP = t->first;

        if(t->second.first == 0 || (t->second.second == false)) {
            toAdd.vote = SaharaHeader::VoteState::Undefined;
        } else if(t->second.first < 50) {
            toAdd.vote = SaharaHeader::VoteState::Negative;
        } else {
            toAdd.vote = SaharaHeader::VoteState::Positive;
        }

        myVoteList.push_back(toAdd);
    }

    // Se non ci sono voti, restituire un pacchetto vuoto
    if(myVoteList.empty()) {
            
        ns3::sahara::SaharaHeader::VotePacket votePacketNull(m_myIP, myVoteList, "");

        return votePacketNull;
    }

    ns3::sahara::SaharaHeader::VotePacket votePacket(m_myIP, myVoteList, "miss_u_a_bit");
    return votePacket;
}


void  
SaharaSecurity::SetMyIP(Ipv4Address myIP){
    m_myIP = myIP;
}



}
}

