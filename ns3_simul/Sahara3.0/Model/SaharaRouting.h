#ifndef SAHARA_AGENT_IMPL_H
#define SAHARA_AGENT_IMPL_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/socket.h"
#include "ns3/traced-callback.h"

#include <map>
#include <vector>

namespace ns3 {
namespace Sahara {

/// An OLSR's routing table entry.
struct RoutingTableEntry
{
    Ipv4Address originIP;
    Ipv4Address hop1IP;
    Ipv4Address senderIP;
    uint32_t reputation;
    uint32_t GPS;
    uint32_t battery;

    RoutingTableEntry()
        : // default values
          originIP(),
          hop1IP(),
          senderIP(),
          reputation(0),
          GPS(0),
          battery(0)
    {
    }
};

class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
    static const uint16_t SAHARA_PORT_NUMBER; //!< port number (698)
    static TypeId GetTypeId();

    RoutingProtocol();
    ~RoutingProtocol() override;
    void SetMainInterface(uint32_t interface);

    std::vector<RoutingTableEntry> GetRoutingTableEntries() const;

    int64_t AssignStreams(int64_t stream);

private:
    std::map<Ipv4Address, RoutingTableEntry> m_table; //!< Data structure for the routing table.

    EventGarbageCollector m_events; //!< Running events.

    Time m_helloInterval; //!< HELLO messages' emission interval.

    // Helper functions
    void ProcessHello(const MessageHeader& msg);
    void AddEntry(const Ipv4Address& originIP, const Ipv4Address& hop1IP, const Ipv4Address& senderIP, uint32_t reputation, uint32_t GPS, uint32_t battery);

public:
    // From Ipv4RoutingProtocol
    Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header& header, Ptr<NetDevice> oif, Socket::SocketErrno& sockerr) override;
    bool RouteInput(Ptr<const Packet> p, const Ipv4Header& header, Ptr<const NetDevice> idev, const UnicastForwardCallback& ucb, const MulticastForwardCallback& mcb, const LocalDeliverCallback& lcb, const ErrorCallback& ecb) override;
    void SetIpv4(Ptr<Ipv4> ipv4) override;
    void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const override;

private:
    void NotifyInterfaceUp(uint32_t interface) override;
    void NotifyInterfaceDown(uint32_t interface) override;
    void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) override;

    // Timer handlers
    Timer m_helloTimer; //!< Timer for the HELLO message.
    void HelloTimerExpire();
};

} // namespace olsr
} // namespace ns3

#endif /* SAHARA_AGENT_IMPL_H */
