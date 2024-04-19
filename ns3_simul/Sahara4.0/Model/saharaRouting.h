#ifndef sahara_AGENT_IMPL_H
#define sahara_AGENT_IMPL_H

#include "ns3/event-garbage-collector.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/traced-callback.h"
#include "routingTable.h"
#include "saharaPacket.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "saharaSync.h"
#include "saharaQueue.h"

#include <map>
#include <vector>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <string>
#include <limits>
#include <fstream>

/*
    This is an Alpha implementation of the Sahara routing protocol.
    The implementation is only to test the concept of this routing protocol.
    It doesn't have all the features of the final implementation and in this case only few simple methods
    are used to send updates, messages and so on. Maybe the performarces are slightly different from the final implementation.
    and realistic implementation. Consider it as a prof of concept.
*/


namespace ns3
{
namespace sahara
{


class SaharaRouting : public Ipv4RoutingProtocol
{
  public:
   
    static TypeId GetTypeId();

    SaharaRouting();
    ~SaharaRouting() override;


  protected:
    void DoInitialize() override;
    void DoDispose() override;

   
  public:
    // From Ipv4RoutingProtocol
    Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p,
                               const Ipv4Header& header,
                               Ptr<NetDevice> oif,
                               Socket::SocketErrno& sockerr) override;

    bool RouteInput(Ptr<const Packet> p,
                    const Ipv4Header& header,
                    Ptr<const NetDevice> idev,
                    const UnicastForwardCallback& ucb,
                    const MulticastForwardCallback& mcb,
                    const LocalDeliverCallback& lcb,
                    const ErrorCallback& ecb) override;
    void SetIpv4(Ptr<Ipv4> ipv4) override;

    void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                           Time::Unit unit = Time::S) const override;

    void StartSetReconciliation();
    
    
 
  private:
    void NotifyInterfaceUp(uint32_t interface) override;
    void NotifyInterfaceDown(uint32_t interface) override;
    void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) override;

    Time m_periodicUpdateInterval;
    Ipv4Address m_mainAddress;
    Ptr<Ipv4> m_ipv4;
    std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
    std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddressesSET;

    Ptr<NetDevice> m_lo;
    bool m_routingHelloStatus;
    uint16_t m_intNodeID;
    RoutingTable r_Table;
    uint16_t m_saharaPort;
    uint16_t m_saharaPortSET;


    // managing timer for running node routing functions
    Timer m_auditFloodingTimer; // flooding
    Timer m_auditDijkstra; // disjkstra
    Timer m_auditLookUpPacketQueue; // queue
    Timer m_auditTimeoutAckSR; // timeout ack from children during set reconciliation


    // millisecond to start
    uint16_t m_timeToStartFlooding = 1000;
    uint16_t m_timeToStartDijskra = 10000;
    uint16_t m_timeToStartPacketQueue = 25000;

    // millisecond to set up frequency
    uint16_t m_frequencyFlooding = 30000;
    uint16_t m_frequencyDijskra = 38000;
    uint16_t m_frequencyLookUpPacketQueue = 2000;

    // ack reception timeslot
    uint16_t m_ackTimeSlot = 2000;

    uint32_t m_currentSequenceNumber;
    PacketQueue m_queue;

    // to manage set reconciliation
    SaharaSync m_sync;
    std::vector<bool> parentBF;
    std::map<Ipv4Address, std::vector<bool>> m_listBFChildren;
    std::map<Ipv4Address, bool> m_listSetRecDone;


    Ipv4Address m_parentIP = Ipv4Address("0.0.0.0");

    
    
    

  private:
    void SendHello();

    void RecvPacket(Ptr<Socket> socket);

    void StartRouting();

    void PauseRouting();

    // manage flooding and dijskra
    void ProcessHello(SaharaHeader sh);
    void ForwardHello(std::string msg);
    void SendPeriodicUpdate();
    bool IsOwnAddress (Ipv4Address originatorAddress);
    void BroadcastPacket(Ptr<Packet> packet);
    void AuditHellos();
    void Dijkstra();
    
    // manage packet queue
    void LookupQueue();
    void SendPacketFromQueue(Ipv4Address dst, Ipv4Address nextHop);

    // manage set reconciliation
    void BroadcastPacketSET(Ptr<Packet> packet);
    void ProcessSetReconciliation(SaharaHeader sh);
    void ReceiveSetRecReq(Ptr<Socket> socket);
    void SendOwnBF();
    void NoChildren(); 
    void ProcessSetAck(SaharaHeader sh);
    void ProcessReceivedMissing(SaharaHeader sh);
    void InverseSetRec();
    void ProcessReceivedMissingInverse(SaharaHeader sh);
    void SendPacketToDest(Ptr<Packet> packet, Ipv4Address dest);

    

    // to test what nodes contains after some times
    void PrintAllInfo();
    void PrintRoutingTable();
   

    
};

} // namespace sahara
} // namespace ns3

#endif /* sahara_AGENT_IMPL_H */


