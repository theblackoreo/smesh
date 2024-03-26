

#include "ns3/saharaRouting.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-packet-info-tag.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/uinteger.h"

#include <iomanip>
#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("saharaRoutingProtocol");

namespace sahara
{
NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);


TypeId
RoutingProtocol::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::sahara::RoutingProtocol")
            .SetParent<Ipv4RoutingProtocol>()
            .SetGroupName("sahara")
            .AddConstructor<RoutingProtocol>();
           
    return tid;
}

RoutingProtocol::RoutingProtocol() 
   
{
}

RoutingProtocol::~RoutingProtocol()
{
}

void
RoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4)
{
    
    NS_ASSERT(ipv4);
    NS_ASSERT(!m_ipv4);
    m_ipv4 = ipv4;
    NS_ASSERT(m_ipv4->GetNInterfaces() == 1 &&
              m_ipv4->GetAddress(0, 0).GetLocal() == Ipv4Address("127.0.0.1"));
    m_lo = m_ipv4->GetNetDevice(0);
    NS_ASSERT(m_lo);
    
    NS_LOG_DEBUG("All setted, starting routing protocol...");

    Simulator::ScheduleNow(&RoutingProtocol::Start, this);
    
}

void
RoutingProtocol::TestSend(Ptr<Socket> socket)
{
    std::string msg = "Hello World!";
  
  
  Ptr<Packet> packet = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);

   
  // Send the packet in broadcast
  if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 222)) != -1){
    NS_LOG_DEBUG("Packet sent!");
  } else {
      NS_LOG_DEBUG("error");
  }
  
}

void
RoutingProtocol::TestRecv(Ptr<Socket> socket){
   
   Ptr<Packet> packet;
    Address from;
    Ipv4Address receiverIp = socket->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

    while ((packet = socket->RecvFrom (from))) {
        Ipv4Address senderIp = InetSocketAddress::ConvertFrom(from).GetIpv4();
        NS_LOG_DEBUG(receiverIp << ": Received a packet from " << senderIp);
        
        
    }

}

void 
RoutingProtocol::Start(){

    

    Ptr<Socket> recvSocket = Socket::CreateSocket (GetObject<Node>(), UdpSocketFactory::GetTypeId());
    NS_ASSERT(recvSocket);
    recvSocket->Bind(InetSocketAddress(Ipv4Address::GetAny (),222)); // Listen on port 222
    recvSocket->SetAllowBroadcast(true);
    recvSocket->SetRecvCallback(MakeCallback(&RoutingProtocol::TestRecv, this));
    

    Ptr<Socket> socket_sender = Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
    socket_sender->SetAllowBroadcast(true);

    if(m_ipv4->GetAddress(1, 0).GetLocal() == Ipv4Address("10.1.1.2")) {
    Simulator::Schedule(Seconds(2.0), &RoutingProtocol::TestSend, this, socket_sender);
    }
    
}

// object's destruction process 
void
RoutingProtocol::DoDispose()
{
    m_ipv4 = nullptr;
    for (auto iter = m_socketAddresses.begin(); iter != m_socketAddresses.end(); iter++)
    {
        iter->first->Close();
    }
    m_socketAddresses.clear();
    Ipv4RoutingProtocol::DoDispose();
}

// da completare
void
RoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{}

/*
void
RoutingProtocol::TestSend()
{
    Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
    NS_ASSERT(l3);
    std::string msg = "Hello World!";
    Ptr<Packet> p = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);
    Ipv4Address dst = Ipv4Address("255.255.255.255");
    Ipv4Address src = m_ipv4->GetAddress(1, 0).GetLocal();
    l3->Send(p, src, dst, header.GetProtocol(), route);
}
*/





void
RoutingProtocol::DoInitialize()
{
   
}

void
RoutingProtocol::NotifyInterfaceUp(uint32_t i)
{
}

void
RoutingProtocol::NotifyInterfaceDown(uint32_t i)
{
}

void
RoutingProtocol::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
RoutingProtocol::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
}

bool
RoutingProtocol::RouteInput(Ptr<const Packet> p,
                            const Ipv4Header& header,
                            Ptr<const NetDevice> idev,
                            const UnicastForwardCallback& ucb,
                            const MulticastForwardCallback& mcb,
                            const LocalDeliverCallback& lcb,
                            const ErrorCallback& ecb)
{
    return false;


} // namespace sahara
Ptr<Ipv4Route>
RoutingProtocol::RouteOutput(Ptr<Packet> p,
                             const Ipv4Header& header,
                             Ptr<NetDevice> oif,
                             Socket::SocketErrno& sockerr){
    Ptr<Ipv4Route> rtentry;
    return rtentry;
                             }


} // namespace ns3
}

