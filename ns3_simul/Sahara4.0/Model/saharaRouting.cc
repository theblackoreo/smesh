

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
#include <sys/time.h>
#include "ns3/random-variable-stream.h"
#include "ns3/udp-header.h"

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <string>
#include <limits>


namespace ns3
{

NS_LOG_COMPONENT_DEFINE("saharaRoutingProtocol");

namespace sahara
{
NS_OBJECT_ENSURE_REGISTERED(SaharaRouting);

TypeId
SaharaRouting::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::sahara::SaharaRouting")
            .SetParent<Ipv4RoutingProtocol>()
            .SetGroupName("sahara")
            .AddConstructor<SaharaRouting>();
           
    return tid;
}

SaharaRouting::SaharaRouting() 
{
  NS_LOG_DEBUG("Adding routing protocol...");
  Ptr<UniformRandomVariable> random = CreateObject<UniformRandomVariable>();
  m_staticRouting = Create<Ipv4StaticRouting> ();
}

SaharaRouting::~SaharaRouting()
{
}


    
void
SaharaRouting::SetIpv4(Ptr<Ipv4> ipv4)
{
    NS_LOG_DEBUG("Starting...");

    NS_ASSERT(ipv4);
    NS_ASSERT(!m_ipv4);
    m_ipv4 = ipv4;
    NS_LOG_DEBUG("Assigned IPV4");

    NS_ASSERT(m_ipv4->GetNInterfaces() == 1 &&
              m_ipv4->GetAddress(0, 0).GetLocal() == Ipv4Address("127.0.0.1"));
    m_lo = m_ipv4->GetNetDevice(0);
    NS_ASSERT(m_lo);
    NS_LOG_DEBUG("Assigned local address ");

    NS_LOG_DEBUG("Assigned main address...");

    m_saharaPort = 9;
    m_saharaPortSET = 10;
    

    NS_LOG_DEBUG("All setted, starting routing protocol...");

    Simulator::ScheduleNow(&SaharaRouting::DoStart, this);
}


void
SaharaRouting::TestRecv(Ptr<Socket> socket){

    NS_LOG_DEBUG("[TestRecv] Packet received");

    Address sourceAddr;
    Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
    InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
    Ipv4Address sourceAddress = inetSocketAddr.GetIpv4 ();
    NS_LOG_DEBUG("Processing hello...");
   
}

void 
SaharaRouting::DoStart(){
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

    for (uint32_t i = 0 ; i < m_ipv4->GetNInterfaces () ; i++)
    {
      Ipv4Address ipAddress = m_ipv4->GetAddress (i, 0).GetLocal ();

      if (ipAddress == Ipv4Address::GetLoopback ())
        continue;
      // Create socket on this interface
      Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
      socket->SetAllowBroadcast (true);
      InetSocketAddress inetAddr (m_ipv4->GetAddress (i, 0).GetLocal (), m_saharaPort);
      socket->SetRecvCallback(MakeCallback(&SaharaRouting::TestRecv, this));
      if (socket->Bind (inetAddr))
        {
          NS_FATAL_ERROR ("SaharaRouting::DoStart::Failed to bind socket!");
        }

      Ptr<NetDevice> netDevice = m_ipv4->GetNetDevice (i);
      socket->BindToNetDevice (netDevice);
      m_socketAddresses[socket] = m_ipv4->GetAddress (i, 0);
    }

    // create socket for set reconciliation
    for (uint32_t i = 0 ; i < m_ipv4->GetNInterfaces () ; i++)
    {
      Ipv4Address ipAddress = m_ipv4->GetAddress (i, 0).GetLocal ();

      if (ipAddress == Ipv4Address::GetLoopback ())
        continue;
      // Create socket on this interface
      Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
      socket->SetAllowBroadcast (true);
      InetSocketAddress inetAddr (m_ipv4->GetAddress (i, 0).GetLocal (), m_saharaPortSET);
      socket->SetRecvCallback(MakeCallback(&SaharaRouting::TestRecv, this));
      if (socket->Bind (inetAddr))
        {
          NS_FATAL_ERROR ("SaharaRouting::DoStart::Failed to bind socket!");
        }

      Ptr<NetDevice> netDevice = m_ipv4->GetNetDevice (i);
      socket->BindToNetDevice (netDevice);
      m_socketAddressesSET[socket] = m_ipv4->GetAddress (i, 0);
    }

    m_mainAddress = m_ipv4->GetAddress (1, 0).GetLocal ();

    

        // Configure timers
    m_auditHellosTimer.SetFunction (&SaharaRouting::AuditHellos, this);
    m_auditPingsTimer.SetFunction(&SaharaRouting::Dijkstra, this);

    // Start timers
     m_auditHellosTimer.Schedule (MilliSeconds (2000 + 10*static_cast<double>(intNodeID)));

     m_auditPingsTimer.Schedule (MilliSeconds (12000 + 10*static_cast<double>(intNodeID)));
    
    //Simulator::Schedule(Seconds(1.0 + (static_cast<double>(intNodeID) - ((9*static_cast<double>(intNodeID))/10))), &SaharaRouting::AuditHellos, this);
    
}

// object's destruction process 
void
SaharaRouting::DoDispose()
{
    // Close sockets
    for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::iterator iter = m_socketAddresses.begin ();
        iter != m_socketAddresses.end (); iter++)
        {
        iter->first->Close ();
        }
    m_socketAddresses.clear ();

    for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::iterator iter = m_socketAddressesSET.begin ();
        iter != m_socketAddressesSET.end (); iter++)
        {
        iter->first->Close ();
        }
    m_socketAddressesSET.clear ();
    
    // Clear static routing
    m_staticRouting = 0;

    m_auditPingsTimer.Cancel ();
    m_auditHellosTimer.Cancel ();

    Ipv4RoutingProtocol::DoDispose();
    
}

// da completare
void
SaharaRouting::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{}

void
SaharaRouting::DoInitialize()
{
   
}

void
SaharaRouting::NotifyInterfaceUp(uint32_t i)
{
}

void
SaharaRouting::NotifyInterfaceDown(uint32_t i)
{
}

void
SaharaRouting::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
SaharaRouting::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
}

 

Ptr<Ipv4Route>
SaharaRouting::RouteOutput(Ptr<Packet> p,
                             const Ipv4Header& header,
                             Ptr<NetDevice> oif,
                             Socket::SocketErrno& sockerr){

    if (header.GetDestination() == m_mainAddress) {
        Ptr<Ipv4Route> localRoute = Create<Ipv4Route>();
        localRoute->SetDestination(m_mainAddress);
        localRoute->SetSource(m_mainAddress); // Assuming m_mainAddress is the local address
        localRoute->SetOutputDevice(oif); // Assuming oif is the output interface for the local node
        return localRoute;
      
    }
    else{
        // Packet needs to be forwarded
        Ptr<Ipv4Route> ipv4Route;

        Ipv4Address nextHop = r_Table.LookUpAddr(m_mainAddress, header.GetDestination());
        
       
        // Create a route for forwarding
        ipv4Route = Create<Ipv4Route>();
        ipv4Route->SetDestination(header.GetDestination());
        ipv4Route->SetSource(m_mainAddress); // Assuming m_mainAddress is the local address
        ipv4Route->SetGateway(nextHop); // Set the next hop as the gateway

        // Get the output interface for the local node
        int32_t interface = m_ipv4->GetInterfaceForAddress(m_mainAddress);
        ipv4Route->SetOutputDevice(m_ipv4->GetNetDevice(interface)); // Set the output device

        NS_LOG_DEBUG("[RouteOutput " << m_mainAddress << "] sending to -> " << header.GetDestination() << ", next hop -> " << nextHop);
        return ipv4Route;

    }

    }
    
    void
    SaharaRouting::PauseRouting(){
        m_routingHelloStatus = false;
         NS_LOG_DEBUG("PAUSED!");
    }


   


bool
SaharaRouting::RouteInput(Ptr<const Packet> p,
                            const Ipv4Header& header,
                            Ptr<const NetDevice> idev,
                            const UnicastForwardCallback& ucb,
                            const MulticastForwardCallback& mcb,
                            const LocalDeliverCallback& lcb,
                            const ErrorCallback& ecb)
{   
    
    //NS_LOG_DEBUG ("[RouteInput] Called");
    Ipv4Address destinationAddress = header.GetDestination ();
    Ipv4Address sourceAddress = header.GetSource ();
    //NS_LOG_DEBUG ("[RouteInput]" << destinationAddress);
    //NS_LOG_DEBUG ("[RouteInput]" << sourceAddress);
    Ptr<Ipv4Route> ipv4Route;

    // Drop if packet was originated by this node
    if (IsOwnAddress(sourceAddress) == true) {
        return true; // avoid loops
    }
    
    // Check for local delivery
    uint32_t interfaceNum = m_ipv4->GetInterfaceForDevice (idev);
    if (m_ipv4->IsDestinationAddress (destinationAddress, interfaceNum)) {
        NS_LOG_DEBUG ( "[RouteInput " << m_mainAddress << "] " << "SOURCE-> " << sourceAddress);

        if (!lcb.IsNull ()) {
            Ptr<Packet> packetCopy = p->Copy();
            UdpHeader uh;
            packetCopy->RemoveHeader(uh);
            if(uh.GetSourcePort() == m_saharaPort){
              ProcessHello(packetCopy);
              //lcb(p, header, interfaceNum);
               return true;
            }
            else if(uh.GetSourcePort() == m_saharaPortSET){
              NS_LOG_DEBUG("OK RECEIVED SET RECONCILIATION REQUEST");
              ProcessSetReconciliation(packetCopy);
              return true;
            }
            else{
              NS_LOG_DEBUG(m_mainAddress << " Local deliver");
              lcb(p, header, interfaceNum);
               return true;
            }
        }
        return false;
    }
    else{
        
        Ptr<Ipv4Route> ipv4Route;
        Ipv4Address nextHop = r_Table.LookUpAddr(m_mainAddress, header.GetDestination ());
        ipv4Route = Create<Ipv4Route> ();
        ipv4Route->SetDestination (destinationAddress);
        ipv4Route->SetSource (m_mainAddress);
        ipv4Route->SetGateway (nextHop);
        int32_t interface = m_ipv4->GetInterfaceForAddress(m_mainAddress);
        ipv4Route->SetOutputDevice (m_ipv4->GetNetDevice (interface));

        NS_LOG_DEBUG("[RouteInput " << m_mainAddress << "] forwarding to -> " << nextHop);

        // UnicastForwardCallback = void ucb(Ptr<Ipv4Route>, Ptr<const Packet>, const Ipv4Header &)
        ucb (ipv4Route,p, header);
        return true;

    }
    
    
    return false;

    
}

bool
SaharaRouting::IsOwnAddress (Ipv4Address originatorAddress)
{
  // Check all interfaces
  for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i = m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    {
      Ipv4InterfaceAddress interfaceAddr = i->second;
      if (originatorAddress == interfaceAddr.GetLocal ())
        {
          return true;
        }
    }
  return false;

}


void 
SaharaRouting::SendHello(){



 //Ipv4Address destAddress = ResolveNodeIpAddress (0); // TODO remove this and "hello"
 // uint32_t sequenceNumber = GetNextSequenceNumber ();

    // NS_LOG_DEBUG ("[SendHello] Called");

   // if(intNodeID == 6){
    Ptr<Packet> packet = Create<Packet> ();
    //SaharaHeader saharaHeader = SaharaHeader();
    uint16_t init = 0;
    uint16_t def = 22;
    SaharaHeader saharaHeader = SaharaHeader(m_mainAddress, Ipv4Address("0.0.0.0"), intNodeID, init, def,def,def,def);
    
    //NS_LOG_DEBUG ("[SendHello] Sending Hello " << m_mainAddress << ", " << Ipv4Address("0.0.0.0") << ", " << intNodeID << ", " << init << ", " << init);    

    packet->AddHeader(saharaHeader);
    BroadcastPacket (packet);
  //  }

}

void
SaharaRouting::BroadcastPacket(Ptr<Packet> packet){

    //NS_LOG_DEBUG ("[BroadcastPacket] Called");
    for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i =
      m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    {
      //TRAFFIC_LOG( "Interface Addr: " << i->second.GetLocal());
      Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
      i->first->SendTo (packet, 0, InetSocketAddress (broadcastAddr, m_saharaPort));
    }

}


void
SaharaRouting::AuditHellos()
{
    // reset routing table avery 30 seconds, maybe it is not efficient 
    // but allow to delete nodes that are not anymore neighbors
   
    //r_Table.DeleteAll();
    
  //Broadcast a fresh HELLO message to immediate neighbors
  
  SendHello ();
  //bool sendMsg = false;
  
  // Reschedule timer, every 30 seconds it sends updates
  m_auditHellosTimer.Schedule (MilliSeconds (30000));

}

 void
    SaharaRouting::ProcessHello(Ptr<Packet> packet){
        

        NS_LOG_DEBUG("[ProcessHello] processing hello...");

        SaharaHeader sh;
        packet->RemoveHeader(sh);
        Ipv4Address originIP = sh.GetOriginIP();
        Ipv4Address hop1IP = sh.GetHop1IP();
        uint16_t reputation_O = sh.GetReputation_O();
        uint16_t GPS_O = sh.GetGPS_O();
        uint16_t battery_O = sh.GetBattery_O();
        uint16_t reputation_H = sh.GetReputation_H();
        uint16_t GPS_H = sh.GetGPS_H();
        uint16_t battery_H = sh.GetBattery_H();

        NS_LOG_DEBUG("Node: " << intNodeID << "-> " << originIP << ", " << hop1IP << ", " << reputation_O << ", "<<reputation_H);

        if(hop1IP == Ipv4Address("0.0.0.0")) {

           r_Table.AddTuple(originIP, m_mainAddress, reputation_O,intNodeID, GPS_O, GPS_H, battery_O, battery_H);
           // forward hello 
           Ptr<Packet> packet = Create<Packet> ();
           SaharaHeader saharaHeader = SaharaHeader(originIP, m_mainAddress, reputation_O, intNodeID, GPS_O, GPS_H, battery_O, battery_H);
           packet->AddHeader(saharaHeader);

           Simulator::Schedule(MilliSeconds(100 + 100*intNodeID), &SaharaRouting::BroadcastPacket, this, packet);

          // BroadcastPacket (packet);

        }else if(!r_Table.CheckDuplicate(originIP, hop1IP) and (originIP!=m_mainAddress)){

             r_Table.AddTuple(originIP, hop1IP, reputation_O,reputation_H, GPS_O, GPS_H, battery_O, battery_H);
             Ptr<Packet> packet = Create<Packet> ();
             SaharaHeader saharaHeader = SaharaHeader(originIP, hop1IP, reputation_O,reputation_H, GPS_O, GPS_H, battery_O, battery_H);
             packet->AddHeader(saharaHeader);
             Simulator::Schedule(MilliSeconds(100 + 100*intNodeID), &SaharaRouting::BroadcastPacket, this, packet);
             
        }
       if(intNodeID ==2){
            NS_LOG_DEBUG("Print RT -> " <<m_mainAddress );
            r_Table.PrintAll();
            
        }

    }

void
SaharaRouting::Dijkstra(){
     
    NS_LOG_DEBUG("[Dijkstra] Entered");
    r_Table.RunDijkstra(m_mainAddress);
    
    StartSetReconciliation();

    m_auditPingsTimer.Schedule (MilliSeconds (300000));
}


// to delete -> not usefull
void
SaharaRouting::addQueue(Ptr<Packet> p){
    //NS_LOG_DEBUG("Add to queue");
    myQueue.push(p);
}


void
SaharaRouting::StartSetReconciliation(){

    // create bloom filter
      //r_Table.CreateBloomFilter();

    // send bloom filter in broadcast
    if(intNodeID == 1){
      std::vector<bool> bloomFilter;
      bloomFilter = r_Table.CreateBloomFilter();

      
      
      // bloom filter received correclty
      
      std::vector<uint8_t> byteVector(bloomFilter.begin(), bloomFilter.end());


      Ptr<Packet> packet = Create<Packet>(&byteVector[0], byteVector.size());

      
      BroadcastPacketSET(packet);
      
    }

}

void
SaharaRouting::BroadcastPacketSET(Ptr<Packet> packet){
    NS_LOG_DEBUG(intNodeID << ": SENDING SET RECONCILIATION DATA");
    //NS_LOG_DEBUG ("[BroadcastPacket] Called");
    for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i =
      m_socketAddressesSET.begin (); i != m_socketAddressesSET.end (); i++)
    {
      //TRAFFIC_LOG( "Interface Addr: " << i->second.GetLocal());
      Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
      i->first->SendTo (packet, 0, InetSocketAddress (broadcastAddr, m_saharaPortSET));
    }

}

void
SaharaRouting::ProcessSetReconciliation(Ptr<Packet> packet){

 uint32_t size = packet->GetSize();
      uint8_t buffer[size];
      packet->CopyData(buffer, size);

      std::vector<bool> receivedBloomFilter;
      for (uint32_t i = 0; i < size; i++) {
          receivedBloomFilter.push_back(buffer[i] == 1);
      }
      
      NS_LOG_DEBUG(intNodeID << ": Printing bloom filter received");
      std::string toPrintTosee;
            for (int i = 0; i < 16; ++i) {
                toPrintTosee += std::to_string(receivedBloomFilter[i]);
            }
      NS_LOG_DEBUG(intNodeID << ": bloom filter received ->" << toPrintTosee);

      r_Table.ProcessSetReconciliation(receivedBloomFilter);

      
}



} // namespace ns3
}

