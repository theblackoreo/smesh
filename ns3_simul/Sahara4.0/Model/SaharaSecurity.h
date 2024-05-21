
#ifndef SAHARA_SECURITY_H
#define SAHARA_SECURITY_H

#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-net-device.h"
#include "ns3/net-device-container.h"
#include "routingTable.h"
#include "ns3/ipv4-address.h"
#include "ns3/timer.h"


#include <memory>
#include <map>
#include <vector>
#include <tuple>



namespace ns3{

namespace sahara{
class SaharaSecurity{

public:
	
	SaharaSecurity(RoutingTable& routingTable);
    ~SaharaSecurity();

    void LoadLastBlock();
    void LoadMacIdMap();

    bool PromiscuousCallback(ns3::Ptr<ns3::NetDevice> device, ns3::Ptr<const ns3::Packet> packet, uint16_t protocol, const ns3::Address &source, const ns3::Address &destination, ns3::NetDevice::PacketType packetType);

    // functions to verify various types of attacks

    // verify if node in charge to forward drops the packet
    void VerifyDrop(Ptr<Packet> &packet, Ipv4Address deviceIP, Ipv4Address detectedSourceIP, Ipv4Address sourceIP, Ipv4Address destIP, Ipv4Address nextHopIp);

    void DropTimeoutExpired(Ipv4Address deviceIP, Ipv4Address sourceIP, Ipv4Address nextHopIP);




private:

    uint16_t m_timoutRetransmittingPacket = 500;
    Ipv4Address m_deviceIP;
    Ipv4Address m_nextHopIp;
    Ipv4Address m_sourceIP;
    Ipv4Address m_destIP;
    Ipv4Address m_fromIP;
    uint32_t m_packetID;




    RoutingTable& m_rTable;
    std::map<std::string, std::string> m_mapMac;

    // these are used to verify packet drop
    std::vector<std::tuple<uint32_t, Ipv4Address>> m_historyPackets;
    std::map<uint32_t, std::map<Ipv4Address, Timer*>> m_mapTimers; 

    


private:


};


}
}

#endif

