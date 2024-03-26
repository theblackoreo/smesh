

#include "saharaRouting.h"
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
#include "ns3/ipv4-l3-protocol.h" 
#include "ns3/ipv4-address.h"
#include "ns3/node.h"


#include <iomanip>
#include <iostream>
#include <tuple>
#include <string>

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
RoutingProtocol::TestRecv(Ptr<Socket> socket){
   NS_LOG_DEBUG("Received!");
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

    
    
    Ipv4Address ipv4Addr =  m_ipv4->GetAddress(1, 0).GetLocal();
    std::ostringstream oss;
    ipv4Addr.Print(oss);
    std::string ip_string = oss.str();

    size_t lastDotPos = ip_string.find_last_of('.');

    if (lastDotPos != std::string::npos) {
        // Extract the substring after the last dot
        std::string numberAfterLastDot = ip_string.substr(lastDotPos + 1);

        // Output the extracted number

        std::istringstream iss(numberAfterLastDot);
        iss>>intNodeID;
        //intNodeID = numberAfterLastDot.Get();
         NS_LOG_DEBUG("node id: " + intNodeID);
    } else {
        NS_LOG_DEBUG("error ip");
    }
    std::ostringstream t_myip;
  (m_ipv4->GetAddress(1, 0).GetLocal()).Print(t_myip);
  s_myIP = t_myip.str();

    Ptr<Socket> recvSocket = Socket::CreateSocket (GetObject<Node>(), UdpSocketFactory::GetTypeId ());
    recvSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 9)); // Listen on port 9    
    recvSocket->SetRecvCallback(MakeCallback(&RoutingProtocol::TestRecv, this));
    
    m_routingHelloStatus = true;
    StartRouting();
    
    Simulator::Schedule(Seconds(100.0), &RoutingProtocol::PauseRouting, this);
    
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

    //NS_LOG_DEBUG("Received a packet");
    ProcessHello(p);
    return true;
} 

Ptr<Ipv4Route>
RoutingProtocol::RouteOutput(Ptr<Packet> p,
                             const Ipv4Header& header,
                             Ptr<NetDevice> oif,
                             Socket::SocketErrno& sockerr){
    Ptr<Ipv4Route> rtentry;
    return rtentry;
                             }


void
RoutingProtocol::SendHello(Ptr<Socket> socket)
{   

  
  // origin_ip, hop1_ip, senderip, reputation, GPS, battery
  std::string msg = s_myIP + "," + "n" + "," + s_myIP + "," + "12" + "," + "50" + "," + "10";


  Ptr<Packet> packet = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);
  // Send the packet in broadcast
  if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 9)) != -1){
    NS_LOG_DEBUG("Packet sent!");
  } else {
      NS_LOG_DEBUG("error");
  }
   Simulator::Schedule(Seconds(20.0), &RoutingProtocol::SendHello, this, socket);
}


 // start sahara routing, sending updates every X seconds and updating routing tables
    void 
    RoutingProtocol::StartRouting(){
        
        Ptr<Socket> socket_sender = Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
    socket_sender->SetAllowBroadcast(true);
       
      // if (intNodeID == 1){
        if(m_routingHelloStatus){
            // it depends by the number of nodes (HP: at most 10 now)
            // so is 9 * d / 10
            // if at most 100 is 99 * d / 100
            
            Simulator::Schedule(Seconds(2.0 + (static_cast<double>(intNodeID) - ((9*static_cast<double>(intNodeID))/10))), &RoutingProtocol::SendHello, this, socket_sender);   
        }  
       //}
    }
    

    void
    RoutingProtocol::PauseRouting(){
        m_routingHelloStatus = false;
         NS_LOG_DEBUG("PAUSED!");
    }


    void
    RoutingProtocol::ProcessHello(Ptr<const Packet> p){
        uint8_t buffer[p->GetSize()];
        p->CopyData(buffer, p->GetSize());
        std::string originIP;
        std::string hop1IP;
        std::string senderIP;
        std::string rep;
        std::string GPS;
        std::string battery;
        bool start = false;
      

        std::vector<std::string*> fields = {&originIP, &hop1IP, &senderIP, &rep, &GPS, &battery};
        size_t fieldIndex = 0;

        // retreive message content
        for (size_t i = 0; i < p->GetSize(); ++i) {
             if(buffer[i] == '1') start = true;
             if(start) {
            if (buffer[i] == ',') {
                ++fieldIndex;
                continue;
            }
            if (fieldIndex < fields.size()) {
                *(fields[fieldIndex]) += buffer[i];
            }
             }
        }

        
        // check if need to forward
        if(s_myIP != originIP and s_myIP != senderIP and hop1IP == "n"){
            NS_LOG_DEBUG(s_myIP + "-> Received: " + originIP + "," + hop1IP + "," + senderIP + "," + rep + "," + GPS + "," + battery);

            
            // store updated tuple with new 1hop
            if(!(r_Table.CheckDuplicate(Ipv4Address(originIP.c_str()), Ipv4Address(hop1IP.c_str())))){
                r_Table.AddTuple(Ipv4Address(originIP.c_str()), Ipv4Address(s_myIP.c_str()), 2, 3, 32);
             }
            
            std::tuple myTuple = r_Table.getLastTupleTest();
            Ipv4Address firstIp = std::get<0>(myTuple);
            //NS_LOG_DEBUG(firstIp);  

            std::string msg =  originIP + "," +  s_myIP + "," + s_myIP + "," + "12" + "," + "50" + "," + "10";

            Simulator::Schedule(Seconds(1.0 + (static_cast<double>(intNodeID) - ((9*static_cast<double>(intNodeID))/10))), &RoutingProtocol::ForwardHello, this, msg);   
        }

        else if(!(r_Table.CheckDuplicate(Ipv4Address(originIP.c_str()), Ipv4Address(hop1IP.c_str())))){

            NS_LOG_DEBUG(s_myIP + "-> Received forwarded msg " + originIP + "," + hop1IP + "," + senderIP + "," + rep + "," + GPS + "," + battery);
            r_Table.AddTuple(Ipv4Address(originIP.c_str()), Ipv4Address(hop1IP.c_str()), 2, 3, 32);
            std::string msg =  originIP + "," +  hop1IP + "," + s_myIP + "," + "12" + "," + "50" + "," + "10";
            Simulator::Schedule(Seconds(1.0 + (static_cast<double>(intNodeID) - ((9*static_cast<double>(intNodeID))/10))), &RoutingProtocol::ForwardHello, this, msg);   
            if(intNodeID == 10) r_Table.PrintAll();
        }

    }

    void
   RoutingProtocol::ForwardHello(std::string msg){
        
        Ptr<Packet> pkt = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);

        Ptr<Socket> send_s = Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
        send_s->SetAllowBroadcast(true);

        if (send_s->SendTo(pkt, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 9)) != -1){
        NS_LOG_DEBUG(s_myIP + "-> Forward hello, sent!");
    } else {
        NS_LOG_DEBUG("error forwarding");
    }



    }


} // namespace ns3
}

