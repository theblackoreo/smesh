

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
    void TestSend(Ptr<Socket> socket);

    void TestRecv(Ptr<Socket> socket);
    
};

} // namespace sahara
} // namespace ns3

#endif /* sahara_AGENT_IMPL_H */


