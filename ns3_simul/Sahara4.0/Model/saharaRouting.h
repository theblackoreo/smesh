

#ifndef sahara_AGENT_IMPL_H
#define sahara_AGENT_IMPL_H

#include "ns3/event-garbage-collector.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/traced-callback.h"
#include "routingTable.h"

#include <map>
#include <vector>



namespace ns3
{
namespace sahara
{


class RoutingProtocol : public Ipv4RoutingProtocol
{
  public:
   
    static TypeId GetTypeId();

    RoutingProtocol();
    ~RoutingProtocol() override;


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

  private:
    void NotifyInterfaceUp(uint32_t interface) override;
    void NotifyInterfaceDown(uint32_t interface) override;
    void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) override;

    Time m_periodicUpdateInterval;
    Ipv4Address m_mainAddress;
    Ptr<Ipv4> m_ipv4;
    std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
    //RoutingTable m_routingTable;
    Ptr<NetDevice> m_lo;
    bool m_routingHelloStatus;
    int intNodeID;
    std::string s_myIP;

    RoutingTable r_Table;



    // origin_ip, hop1_ip, senderip, reputation, GPS, battery
    //std::tuple<std::string, std::string, std::string, uint16_t, uint16_t, uint16_t>> tuple;

  private:
    // start routing protocol
    void Start();

    /*
    // Queue packet until we find a route
    void DeferredRouteOutput(Ptr<const Packet> p,
                             const Ipv4Header& header,
                             UnicastForwardCallback ucb,
                             ErrorCallback ecb);

    // Look for any queued packets to send them out
    void LookForQueuedPackets();

    // send packet from queue
    void SendPacketFromQueue(Ipv4Address dst, Ptr<Ipv4Route> route);

    // Find socket with local interface address iface
    Ptr<Socket> FindSocketWithInterfaceAddress(Ipv4InterfaceAddress iface) const;
    */

    // Send a packet
    void SendHello(Ptr<Socket> socket);

    void TestRecv(Ptr<Socket> socket);

    void StartRouting();

    void PauseRouting();

    void ProcessHello(Ptr<const Packet> p);
    void ForwardHello(std::string msg);

    /*
    This is an Alpha implementation of the Sahara routing protocol.
    The implementation is only to test the concept of this routing protocol.
    It doesn't have all the features of the final implementation and in this case only few simple methods
    are used to send updates, messages and so on. Maybe the performarces are slightly different from the final implementation.
    and realistic implementation. Consider it as a prof of concept.
    */

   /*

    // start sahara routing, sending updates every X seconds and updating routing tables
    

    // pause sending hello messages (pause routing protocol)
    //

    // send hello message in broadcast
    void SendHello(Ptr<Socket> socket);

    // process hello message, forward it in case it is necessary and update routing table
    void processHello(Ptr<Packet> packet);
*/
    


    
};

} // namespace sahara
} // namespace ns3

#endif /* sahara_AGENT_IMPL_H */


