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
  #include "ns3/wifi-net-device.h"
  #include "ns3/net-device-container.h"
#include "saharaSecurity.h"

#include "saharaCrypto.h"

#include "saharaMobility.h"

#include <map>
#include <vector>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <string>
#include <limits>
#include <fstream>
#include <deque> // Include deque

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

    
    void PrintNumberOfpackets();

    // mob
    bool GetIfRunning();

 
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
    uint16_t m_packets_processed;
    uint32_t m_tot_byte_processed;
    CryptoModule m_shCrypto;


    // managing timer for running node routing functions
    Timer m_auditFloodingTimer; // flooding
    Timer m_auditDijkstra; // disjkstra
    Timer m_auditLookUpPacketQueue; // queue
    Timer m_auditTimeoutAckSR; // timeout ack from children during set reconciliation
    Timer m_auditTimeoutAckInverseSR;
    Timer m_SR; // flooding
    Timer m_ackTimeoutAskToParentBF;
    Timer m_printRT;
    Timer m_ackTimeoutWaitMissingFromChild;

    // default node info
    uint16_t m_rep = 255;
    uint16_t m_gps = 10;
    uint16_t m_bat = 100;
    
    // modify this value to active flooding or set reconcilation, NOT BOTH at the same time
    bool m_flooding_ON = false;
    bool m_sr_ON = true;
    bool m_sr_dynamic_ON = true;

    // millisecond to start
    uint16_t m_timeToStartFlooding = 2000;
    uint16_t m_timeToStartDijskra = 20000; //it was 5000 default
    uint16_t m_timeToStartPacketQueue = 20000;// it was 6000 default

    // millisecond to set up frequency
    uint16_t m_frequencyFlooding = 0;// max number is like 65000
    uint16_t m_frequencyDijskra = 0;
    uint16_t m_frequencyLookUpPacketQueue = 0;
    uint16_t m_startSR = 2000;
    uint16_t m_frequencySR = 15000;

    uint16_t m_frequencyPrintRT = 6000;


    // ack reception timeslot
    uint16_t m_ackTimeSlot = 800;

    uint32_t m_currentSequenceNumber;
    PacketQueue m_queue;
   

    // mobility is to stop movements while set reconciliation is performed
    bool m_run;

    // set reconciliation variables
    Ipv4Address m_parentIP = Ipv4Address("0.0.0.0");
    std::vector<bool> m_parentBF;
    std::map<Ipv4Address, std::vector<bool>> m_listBFChildren;
    std::map<Ipv4Address, bool> m_listSetRecDone;
    bool m_SRCompleted = false;
    bool m_reset = true; 
    bool m_alreadyDone = false;
    bool m_previousChildProcessed = true;  // Flag to track if the parent is free to process a request
    std::queue<SaharaHeader> m_childRequestQueue;  // Queue to hold child requests
    
    
    // to test a new approach
    Timer m_auditTimeoutAckSRNEW; // timeout ack from children during set reconciliation

    // to test malicius node that drops packets
    bool m_nodeDeletePackets = true;
    uint16_t m_IDcompromisedNode = 17;

    SaharaSecurity m_ss = SaharaSecurity(r_Table);



    // to test 
    bool m_bfRequestInProgress = false;
    
    
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

    
    // BFS like and set reconciliation -> new approach
    void StartTopologyBuilding();
    void ProcessRootHello(SaharaHeader sh);
    void ProcessSRHello(SaharaHeader sh);
    void SendDataNew();
    void NoChildrenNew();
    void ProcessInverseAck(SaharaHeader sh);
    void AskToParentBF();
    void ReceivedFromParentBF(SaharaHeader sh);
    void SendToChildBF(SaharaHeader sh);
    void ResetVariablesUpdate();
    void AskBF2C(Ipv4Address ipChild);
    void SendBFC2P(SaharaHeader sh);
    void ReceivedFromChildBF(SaharaHeader sh);

    // Helper method to process the next request in the queue
    void ProcessNextChildRequest();
    bool IsAlreadyEnqueued(SaharaHeader sh) ;

    // Method to handle acknowledgment/response from the child
    void OnChildResponseReceived();

  // statistics
    void PrintStatistics();
    
  // test attack
    void ActiveDropping();



};

} // namespace sahara
} // namespace ns3

#endif /* sahara_AGENT_IMPL_H */


