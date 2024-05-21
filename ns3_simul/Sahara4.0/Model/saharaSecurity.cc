#include "saharaSecurity.h"

#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-l3-protocol.h" 
#include "ns3/udp-header.h"
#include "routingTable.h"
#include "ns3/timer.h"
#include <algorithm>



#include <iostream>
#include <tuple>
#include <string>
#include <limits>
#include <fstream>
#include <vector>

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
    
}

SaharaSecurity::~SaharaSecurity()
{
}



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
    std::ifstream infile("historyRoutingTable_3.txt");
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


    if(port==256 || port==12345){
    NS_LOG_DEBUG("Promiscuous packet received. Device IP: " << deviceIP << ", PacketID: "<< packet->GetUid() <<", source: " 
                        << sourceIP << ", port: " << port << ", packet from: " << ipv4Header.GetSource() << 
                                    ", packet dest: " << ipv4Header.GetDestination() << ", next hop (MAC): " << nextHopIP);
    }

    
    switch (port)
    {
    case 256:
        NS_LOG_DEBUG("Sahara Routing packet detected");
        break;
    
    case 12345:
        // NS_LOG_DEBUG("Standard message detected");
        // deviceIP, source (MAC), packet from, packet dest, nextHop
        VerifyDrop(packetCopy, deviceIP, sourceIP, ipv4Header.GetSource(), ipv4Header.GetDestination(), nextHopIP);
        break;
    
    default:
        break;
    }
    return true;  // Indicate the packet has been processed

}



void SaharaSecurity::VerifyDrop(Ptr<Packet> &packet, Ipv4Address deviceIP, Ipv4Address detectedSourceIP, Ipv4Address sourceIP, Ipv4Address destIP, Ipv4Address nextHopIp) {
    // deviceIP, source (MAC), detected source, packet from, packet dest,  nextHop
   

    // I'm the next hop so ignore
    if (deviceIP == nextHopIp || destIP == nextHopIp) return;

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

    //  + history.add(packetID, sourceIP)
    std::tuple<uint32_t, Ipv4Address> temp(packet->GetUid(), sourceIP);
    m_historyPackets.push_back(temp);

    // test
    /*
    NS_LOG_DEBUG("Status now:");
    for (const auto& outerPair : m_mapTimers) {
        std::cout << "Key: " << outerPair.first << std::endl;
        for (const auto& innerPair : outerPair.second) {
            std::cout << "    Inner Key: " << innerPair.first << std::endl;
            // Printing the address of the timer object
            std::cout << "    Timer Object Address: " << innerPair.second << std::endl;
        }
    }
    */

    // end test

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
                return;
            }
        }
        else{
            NS_LOG_DEBUG("Not found inner key");
        }
    }
    else{
        //  check if I have in the history a packetID and nextHopIP
        std::tuple<uint32_t, Ipv4Address> toSearch(packet->GetUid(), nextHopIp);

        auto it = std::find(m_historyPackets.begin(), m_historyPackets.end(), toSearch);
        if (it != m_historyPackets.end()) {
            m_historyPackets.erase(it);
            NS_LOG_DEBUG(deviceIP << ": Packet " << packet->GetUid() << " already received by " << nextHopIp);
            return;
        } 
        else if(m_rTable.checkExistenceOfNegh(deviceIP, nextHopIp)){
            Timer* temp = new Timer;
            temp->SetFunction(&SaharaSecurity::DropTimeoutExpired, this);
            temp->SetArguments(deviceIP, detectedSourceIP, nextHopIp);
            temp->Schedule(MilliSeconds(m_timoutRetransmittingPacket));
            m_mapTimers[packet->GetUid()][nextHopIp] = temp;
            NS_LOG_DEBUG(deviceIP << ": Timer created for packet " << packet->GetUid() << " and node (nextHop) " << m_nextHopIp);
        }
        
    }

}

void
SaharaSecurity::DropTimeoutExpired(Ipv4Address deviceIP, Ipv4Address sourceIP, Ipv4Address nextHopIP){

    NS_LOG_DEBUG("Timer expired, Device " << deviceIP << ": sourceIP " << sourceIP << ", node malicius " << nextHopIP);
      
}






// 

}
}

