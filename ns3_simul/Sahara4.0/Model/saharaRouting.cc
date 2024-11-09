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
  #include <fstream>
  #include "saharaSecurity.h"
  #include "globalData.h"




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

      m_saharaPort = 256;
      

      NS_LOG_DEBUG("All setted, starting routing protocol...");

  }


  void
  SaharaRouting::RecvPacket(Ptr<Socket> socket){

      //NS_LOG_DEBUG("[RecvPacket] Packet received");

      Address sourceAddr;
      Ptr<Packet> packet = socket->RecvFrom (sourceAddr);

      //InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
      NS_LOG_DEBUG(packet->GetSize());
      m_tot_byte_processed = m_tot_byte_processed + packet->GetSize();

      SaharaHeader sh;


      packet->RemoveHeader(sh);

      NS_LOG_DEBUG("[RecvPacket] Packet received TYPE -> " << sh.GetMessageType());

     
      
      
      switch (sh.GetMessageType())
      {
      case sahara::SaharaHeader::HELLO_MESSAGE:
        ProcessHello(sh);
        break;
     
      case sahara::SaharaHeader::SEND_MISSING_C2P:
        
        ProcessReceivedMissing(sh);
        break;

      case sahara::SaharaHeader::SEND_MISSING_P2C:
       
        ProcessReceivedMissingInverse(sh);
        break;
      
      case sahara::SaharaHeader::SR_HELLO:
        
        ProcessSRHello(sh);
        break;
      
      case sahara::SaharaHeader::ROOT_SR_HELLO:
        
        ProcessRootHello(sh);
        break;

      case sahara::SaharaHeader::SEND_MISSING_P2C_ACK:
      
        ProcessInverseAck(sh);
        break;

      case sahara::SaharaHeader::ASK_BF:
        
        SendToChildBF(sh);
        break;
        
      case sahara::SaharaHeader::SEND_BF:
        
        ReceivedFromParentBF(sh);
        break;
      
      case sahara::SaharaHeader::ASK_BF_P2C:
        SendBFC2P(sh);
        break;

      case sahara::SaharaHeader::SEND_BF_C2P:
        ReceivedFromChildBF(sh);
        break;
    
  }
  }

   // A node receives set reconciliation request when it receives a BF from a node 
  void
  SaharaRouting::ReceiveSetRecReq(Ptr<Socket> socket){

    NS_LOG_DEBUG(m_intNodeID << " [SET RECONC]" << "Received req");

    Address sourceAddr;
    Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
  }


  void
  SaharaRouting::DoDispose()
  {
      // Close sockets for hellos
      for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::iterator iter = m_socketAddresses.begin ();
          iter != m_socketAddresses.end (); iter++)
          {
          iter->first->Close ();
          }
      m_socketAddresses.clear ();

      // closing socket for set reconciliation
      for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::iterator iter = m_socketAddressesSET.begin ();
          iter != m_socketAddressesSET.end (); iter++)
          {
          iter->first->Close ();
          }
      m_socketAddressesSET.clear ();
      
      

      m_auditDijkstra.Cancel ();
      m_auditFloodingTimer.Cancel ();
      m_auditLookUpPacketQueue.Cancel();

      Ipv4RoutingProtocol::DoDispose();
      
  }

  // da completare
  void
  SaharaRouting::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
  {}

  void
  SaharaRouting::DoInitialize()
  {
    Ipv4Address ipv4Addr =  m_ipv4->GetAddress(1, 0).GetLocal();
      std::ostringstream oss;
      ipv4Addr.Print(oss);
      std::string ip_string = oss.str();

      m_ss.SetMyIP(ipv4Addr);

      size_t lastDotPos = ip_string.find_last_of('.');

      if (lastDotPos != std::string::npos) {
          // Extract the substring after the last dot
          std::string numberAfterLastDot = ip_string.substr(lastDotPos + 1);
          std::istringstream iss(numberAfterLastDot);
          iss>>m_intNodeID;

          NS_LOG_DEBUG("node id: " + m_intNodeID);
      } else {
          NS_LOG_DEBUG("error ip");
      }
      GlobalData::WriteDataToFile(m_intNodeID, m_rep, m_gps, m_bat);

      GlobalData::UpdateAllowedIPs();

      for (uint32_t i = 0 ; i < m_ipv4->GetNInterfaces () ; i++)
      {
        Ipv4Address ipAddress = m_ipv4->GetAddress (i, 0).GetLocal ();
        if (ipAddress == Ipv4Address::GetLoopback ())
          continue;
        // Create socket on this interface
        Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
        socket->SetAllowBroadcast (true);
        // Create socket for receive on this interface
        Ptr<Socket> recvSocket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
        recvSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_saharaPort)); // Listen on port 9
        recvSocket->SetAllowBroadcast (true);
        recvSocket->SetRecvCallback(MakeCallback(&SaharaRouting::RecvPacket, this));
        Ptr<NetDevice> netDevice = m_ipv4->GetNetDevice (i);
        recvSocket->BindToNetDevice (netDevice);
        //socket->BindToNetDevice (netDevice);
        m_socketAddresses[socket] = m_ipv4->GetAddress (i, 0);
      }

     // create sockets for set reconciliation
      for (uint32_t i = 0 ; i < m_ipv4->GetNInterfaces () ; i++)
      {
        Ipv4Address ipAddress = m_ipv4->GetAddress (i, 0).GetLocal ();
        if (ipAddress == Ipv4Address::GetLoopback ())
          continue;
        // Create socket on this interface
        Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
        socket->SetAllowBroadcast (true);
        // Create socket for receive on this interface
        Ptr<Socket> recvSocket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
        recvSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_saharaPortSET)); 
        recvSocket->SetAllowBroadcast (true);
        recvSocket->SetRecvCallback(MakeCallback(&SaharaRouting::ReceiveSetRecReq, this));
        Ptr<NetDevice> netDevice = m_ipv4->GetNetDevice (i);
        recvSocket->BindToNetDevice (netDevice);
        m_socketAddressesSET[socket] = m_ipv4->GetAddress (i, 0);

      }

      m_mainAddress = m_ipv4->GetAddress (1, 0).GetLocal ();



    
     
      for (uint32_t i = 0 ; i < m_ipv4->GetNInterfaces () ; i++)
      {
        Ptr<NetDevice> netDevice = m_ipv4->GetNetDevice (i);
        ns3::Ptr<ns3::WifiNetDevice> wifiDevice = ns3::DynamicCast<ns3::WifiNetDevice>(netDevice);
         if (wifiDevice)
    {
       // wifiDevice->GetMac()->SetPromisc();
        
        wifiDevice->SetPromiscReceiveCallback(MakeCallback(&SaharaSecurity::PromiscuousCallback, &m_ss));
    }
      }
      
     
     
     
    

      // Configure timers
      m_auditFloodingTimer.SetFunction (&SaharaRouting::AuditHellos, this);
      m_auditDijkstra.SetFunction(&SaharaRouting::Dijkstra, this);
      m_auditLookUpPacketQueue.SetFunction(&SaharaRouting::LookupQueue, this);
      m_auditTimeoutAckSR.SetFunction (&SaharaRouting::NoChildren, this);
      m_auditTimeoutAckSRNEW.SetFunction(&SaharaRouting::NoChildrenNew,this);
      m_auditTimeoutAckInverseSR.SetFunction(&SaharaRouting::InverseSetRec, this);
      m_SR.SetFunction(&SaharaRouting::StartTopologyBuilding, this);
      m_ackTimeoutAskToParentBF.SetFunction(&SaharaRouting::AskToParentBF, this);
      //m_ackTimeoutWaitMissingFromChild.SetFunction(&SaharaRouting::SendToChildBF, this);

      

      // queue lenght on node 
      u_int32_t xxx = 100;
      m_queue.SetMaxQueueLen(xxx);
      m_queue.SetQueueTimeout(Seconds(50));

      m_packets_processed = 0;
      m_tot_byte_processed = 0;

      ns3::Ptr<ns3::UniformRandomVariable> rng = ns3::CreateObject<ns3::UniformRandomVariable>();
      rng->SetAttribute("Min", ns3::DoubleValue(15.0));
      rng->SetAttribute("Max", ns3::DoubleValue(50.0));
      double randomNumber = rng->GetValue();

      ns3::Ptr<ns3::UniformRandomVariable> rng2 = ns3::CreateObject<ns3::UniformRandomVariable>();
      rng2->SetAttribute("Min", ns3::DoubleValue(10.0));
      rng2->SetAttribute("Max", ns3::DoubleValue(50.0));
      double randomNumber2 = rng2->GetValue();

      
      // inizialize file 
      std::string nodeIDString = std::to_string(m_intNodeID);
      std::string fileName = "historyRoutingTable_" + nodeIDString + ".txt";
      r_Table.SetFile(fileName);
      r_Table.SetMyNodeID(m_intNodeID);


      if(m_flooding_ON){

        m_auditFloodingTimer.Schedule (MilliSeconds (m_timeToStartFlooding + randomNumber*static_cast<double>(m_intNodeID)));
        m_auditDijkstra.Schedule (MilliSeconds (m_timeToStartDijskra + randomNumber*static_cast<double>(m_intNodeID)));
        Simulator::Schedule(MilliSeconds(7000+ m_intNodeID), &SaharaRouting::PrintStatistics, this);
        
      }

      if(m_sr_ON){
        
        m_SR.Schedule(MilliSeconds(m_startSR));
        
        //Simulator::Schedule(MilliSeconds(12000), &SaharaRouting::StartTopologyBuilding, this);

       Simulator::Schedule(MilliSeconds(7000+ m_intNodeID), &SaharaRouting::PrintStatistics, this);

        Simulator::Schedule(
    MilliSeconds(6000 + m_intNodeID),
    MakeCallback(static_cast<void (SaharaRouting::*)()>(&SaharaRouting::PrintRoutingTable), this)
);






       //Simulator::Schedule(MilliSeconds(15000), &SaharaRouting::ActiveDropping, this);
        

      }
      

     m_auditLookUpPacketQueue.Schedule (MilliSeconds (m_timeToStartPacketQueue + 2*static_cast<double>(m_intNodeID)));
     
      
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
      
      if(!GlobalData::IpAllowed(header.GetDestination())) return 0;
      if(!GlobalData::IpAllowed(m_mainAddress)) return 0;
      
      
      NS_LOG_DEBUG( m_intNodeID << " [RouteOutput]");
      // manage broadcast
      for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i =
        m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
      {
        Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
        if(header.GetDestination() == broadcastAddr){

          NS_LOG_DEBUG( m_intNodeID << " [RouteOutput]: msg sent in broadcast");
          Ptr<Ipv4Route> ipv4Route = Create<Ipv4Route>();
          ipv4Route->SetDestination(broadcastAddr);
          ipv4Route->SetSource(m_mainAddress); // Assuming m_mainAddress is the local address
          int32_t interface = m_ipv4->GetInterfaceForAddress(m_mainAddress);
          ipv4Route->SetOutputDevice(m_ipv4->GetNetDevice(interface)); 
          ipv4Route->SetGateway(broadcastAddr);
          return ipv4Route;

        }

        else if(header.GetDestination() == m_mainAddress){
          Ptr<Ipv4Route> localRoute = Create<Ipv4Route>();
          localRoute->SetDestination(m_mainAddress);
          localRoute->SetSource(m_mainAddress); // Assuming m_mainAddress is the local address
          localRoute->SetOutputDevice(oif); // Assuming oif is the output interface for the local node
          return localRoute;
        }
        else{
         

          Ipv4Address nextHop = r_Table.LookUpAddr(m_mainAddress, header.GetDestination());
          NS_LOG_DEBUG( "Node Id " << m_intNodeID << ", Destination " <<  header.GetDestination() <<", Next hop: " << nextHop);
          if(nextHop ==  "0.0.0.0"){
            NS_LOG_DEBUG("Next hop not found, local delivery to queue...");
            int32_t interface = m_ipv4->GetInterfaceForAddress(m_mainAddress);
            Ptr<Ipv4Route> localRoute = Create<Ipv4Route>();
            localRoute->SetDestination(header.GetDestination());
            localRoute->SetSource(Ipv4Address("0.0.0.0"));
            localRoute->SetGateway(m_mainAddress);
            
            localRoute->SetOutputDevice(m_ipv4->GetNetDevice(interface));
            return localRoute;
          }
                
          Ptr<Ipv4Route> ipv4Route;
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
     
      Ptr<Ipv4Route> ipv4Route = 0;
      return ipv4Route;

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

      
     
      if(!GlobalData::IpAllowed(header.GetSource())) return true;
      if(!GlobalData::IpAllowed(header.GetDestination())) return true;
      
      
      if(m_nodeDeletePackets && m_IDcompromisedNode == m_intNodeID && ((Simulator::Now().GetSeconds() > 7 && Simulator::Now().GetSeconds() < 17)|| 
      (Simulator::Now().GetSeconds() > 22 && Simulator::Now().GetSeconds() < 32) || 
      (Simulator::Now().GetSeconds() > 37 && Simulator::Now().GetSeconds() < 47) ||
      (Simulator::Now().GetSeconds() > 52 && Simulator::Now().GetSeconds() < 62) ||
        (Simulator::Now().GetSeconds() > 67 && Simulator::Now().GetSeconds() < 77) ||
         (Simulator::Now().GetSeconds() > 81 && Simulator::Now().GetSeconds() < 91))) {
        ns3::Ptr<ns3::UniformRandomVariable> rng = ns3::CreateObject<ns3::UniformRandomVariable>();
        rng->SetAttribute("Min", ns3::DoubleValue(0.0));
        rng->SetAttribute("Max", ns3::DoubleValue(100.0));
        double randomNumber = rng->GetValue();
        if(randomNumber < 99){
          return false;
        }
      }
      
      m_packets_processed = m_packets_processed + 1;
      
      if(Simulator::Now().GetSeconds() > 12 && Simulator::Now().GetSeconds() < 25 && m_intNodeID == 9999){ // 9999 wrong
         ns3::Ptr<ns3::UniformRandomVariable> rng = ns3::CreateObject<ns3::UniformRandomVariable>();
          rng->SetAttribute("Min", ns3::DoubleValue(0.0));
          rng->SetAttribute("Max", ns3::DoubleValue(100.0));
          double randomNumber = rng->GetValue();
          if(randomNumber < 80){
            return false;
          }
      }

      
      

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
            //Ptr<Packet> packetCopy = p->Copy();
            //UdpHeader uh;
            //packetCopy->RemoveHeader(uh);
            lcb(p, header, interfaceNum);
            return true;
            /*
            if(uh.GetSourcePort() == m_saharaPort){
              NS_LOG_DEBUG ( "[RouteInput] is hello");
              lcb(p, header, interfaceNum);
              return true;
            } else{
              NS_LOG_DEBUG ( "[RouteInput] is set reconciliation");
              lcb(p, header, interfaceNum);
              return true;
            }
            */
        
          }
          return false;
      }
      else{
          
          Ptr<Ipv4Route> ipv4Route;
          Ipv4Address nextHop = r_Table.LookUpAddr(m_mainAddress, header.GetDestination ());
          
          // route not found yet -> packet added to queue
          if(nextHop ==  Ipv4Address("0.0.0.0")){
          
            NS_LOG_DEBUG("Hop not found, enqueue packet...");

            QueueEntry newEntry(p, header, ucb, ecb);
            bool result = m_queue.Enqueue(newEntry);
            if (result)
            {
              NS_LOG_DEBUG("Added packet " << p->GetUid() << " to queue.");
            } 
          return true;
          }
          

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

    // if(m_intNodeID == 6){
      Ptr<Packet> packet = Create<Packet> ();
      uint16_t init = 0;
      uint16_t def = 22;
      SaharaHeader sh;
      
      sh.SetMessageType(sahara::SaharaHeader::HELLO_MESSAGE);
      sh.SetOriginIP(m_mainAddress);
      sh.SetHop1IP(Ipv4Address("0.0.0.0"));
      sh.SetReputation_O(m_intNodeID);
      sh.SetReputation_H(init);
      sh.SetGPS_O(def);
      sh.SetGPS_H(def);
      sh.SetBattery_H(def);
      sh.SetBattery_O(def);

     // SaharaHeader(m_mainAddress, Ipv4Address("0.0.0.0"), m_intNodeID, init, def,def,def,def);
      
      //NS_LOG_DEBUG ("[SendHello] Sending Hello " << m_mainAddress << ", " << Ipv4Address("0.0.0.0") << ", " << m_intNodeID << ", " << init << ", " << init);    

      packet->AddHeader(sh);
      
      BroadcastPacket (packet);
    //  }

  }

  void
  SaharaRouting::BroadcastPacket(Ptr<Packet> packet){
    // packet = m_shCrypto.EncryptHeader(packet);

      //NS_LOG_DEBUG ("[BroadcastPacket] Called");
      for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i =
        m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
      {
        Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
        i->first->SendTo (packet, 0, InetSocketAddress (broadcastAddr, m_saharaPort));
      }

  }


  void
  SaharaRouting::AuditHellos()
  {
    
    //r_Table.DeleteAll();
      
    //Broadcast a fresh HELLO message to immediate neighbors
    SendHello ();
    //Timer re-initialization
    m_auditFloodingTimer.Schedule (MilliSeconds (m_frequencyFlooding));

  }
void
SaharaRouting::ProcessHello(SaharaHeader sh) {
  
  //NS_LOG_DEBUG("[ProcessHello] processing hello...");
  
  Ipv4Address originIP = sh.GetOriginIP();
  Ipv4Address hop1IP = sh.GetHop1IP();
  uint16_t reputation_O = sh.GetReputation_O();
  uint16_t GPS_O = sh.GetGPS_O();
  uint16_t battery_O = sh.GetBattery_O();
  uint16_t reputation_H = sh.GetReputation_H();
  uint16_t GPS_H = sh.GetGPS_H();
  uint16_t battery_H = sh.GetBattery_H();

 // NS_LOG_DEBUG("Node: " << m_intNodeID << "-> " << originIP << ", " << hop1IP << ", " << reputation_O << ", " << reputation_H);

  // Only proceed if the packet should be forwarded
  if (hop1IP == Ipv4Address("0.0.0.0")) {
    
    // Add the origin IP information to the routing table
    r_Table.AddTuple(originIP, m_mainAddress, reputation_O, m_intNodeID, GPS_O, GPS_H, battery_O, battery_H);
    
    // Forward the Hello packet with updated information
    Ptr<Packet> packet = Create<Packet>();
    SaharaHeader sh;

    sh.SetMessageType(sahara::SaharaHeader::HELLO_MESSAGE);
    sh.SetOriginIP(originIP);
    sh.SetHop1IP(m_mainAddress);
    sh.SetReputation_O(reputation_O);
    sh.SetReputation_H(m_intNodeID);
    sh.SetGPS_O(GPS_O);
    sh.SetGPS_H(GPS_H);
    sh.SetBattery_H(battery_O);
    sh.SetBattery_O(battery_H);
    
    packet->AddHeader(sh);
    
    // Schedule the packet to be broadcasted with a delay proportional to node ID
    Simulator::Schedule(MilliSeconds(5 * m_intNodeID), &SaharaRouting::BroadcastPacket, this, packet);

  } else if (!r_Table.CheckDuplicate(originIP, hop1IP) && (originIP != m_mainAddress)) {
    
    // Add the non-duplicate tuple to the routing table
    r_Table.AddTuple(originIP, hop1IP, reputation_O, reputation_H, GPS_O, GPS_H, battery_O, battery_H);

    // Forward the Hello packet with updated information
    Ptr<Packet> packet = Create<Packet>();
    SaharaHeader sh;

    sh.SetMessageType(sahara::SaharaHeader::HELLO_MESSAGE);
    sh.SetOriginIP(originIP);
    sh.SetHop1IP(hop1IP);
    sh.SetReputation_O(reputation_O);
    sh.SetReputation_H(reputation_H);
    sh.SetGPS_O(GPS_O);
    sh.SetGPS_H(GPS_H);
    sh.SetBattery_H(battery_O);
    sh.SetBattery_O(battery_H);
    
    packet->AddHeader(sh);

    // Schedule the packet to be broadcasted with a delay proportional to node ID
    Simulator::Schedule(MilliSeconds(5 * m_intNodeID), &SaharaRouting::BroadcastPacket, this, packet);
  }

  //NS_LOG_DEBUG("Print RT -> " << m_mainAddress );
  //r_Table.PrintAll();        
}

    void
    SaharaRouting::Dijkstra(){
        
      NS_LOG_DEBUG("[Dijkstra] Entered");

      r_Table.RunDijkstra(m_mainAddress);
      

      //r_Table.UpdateFileHistory();
        
      
      
      // reschedule timer to evaluation Dijkstra
      if(!m_auditDijkstra.IsRunning() && m_flooding_ON){
        m_auditDijkstra.Schedule (MilliSeconds (m_frequencyDijskra));
      }
      
      
     
      
    }

  
    void
    SaharaRouting::LookupQueue(){
      NS_LOG_DEBUG("Node " << m_intNodeID << " [LookupQueue] Entered");
      if(m_queue.GetSize() == 0) return;
      NS_LOG_DEBUG("Node " << m_intNodeID << " [LookupQueue] There are packets in queue");
      
      Ipv4Address nextHop;
      for(const auto& t : m_queue.GetAllPacketsDest()){
        NS_LOG_DEBUG("Node " << m_intNodeID << " [LookupQueue] Packet in queue-> " << t);
        nextHop = r_Table.LookUpAddr(m_mainAddress, t);
        if(nextHop != "0.0.0.0"){
          NS_LOG_DEBUG("Node " << m_intNodeID << " [LookupQueue] Next-hop found -> " << nextHop);
          SendPacketFromQueue(t, nextHop);

        }
      }

      m_auditLookUpPacketQueue.Schedule (MilliSeconds (m_frequencyLookUpPacketQueue));
      return;
    
    }

    void
    SaharaRouting::SendPacketFromQueue(Ipv4Address dst, Ipv4Address nextHop){

      QueueEntry queueEntry;
       if (m_queue.Dequeue(dst, queueEntry)){
          Ptr<Packet> p = ConstCast<Packet>(queueEntry.GetPacket());
          UnicastForwardCallback ucb = queueEntry.GetUnicastForwardCallback();
          Ipv4Header header = queueEntry.GetIpv4Header();

          Ptr<Ipv4Route> ipv4Route;
          ipv4Route = Create<Ipv4Route> ();
          ipv4Route->SetDestination (dst);
          if(header.GetSource() == "0.0.0.0"){
            ipv4Route->SetSource (m_mainAddress);
            header.SetSource(m_mainAddress);
          }else {
            ipv4Route->SetSource (header.GetSource());
            header.SetSource(header.GetSource());
          }
          ipv4Route->SetGateway (nextHop);
          int32_t interface = m_ipv4->GetInterfaceForAddress(m_mainAddress);
          ipv4Route->SetOutputDevice (m_ipv4->GetNetDevice (interface));

          ucb(ipv4Route, p, header);
       }
    }

  /*SET RECONCILIATION__________________________________________________________________________________________________________________________________________________*/
  /*
  This function is used in order to perform set reconciliation in a distributed way.
  The source node sends its BF to the neighbors
  The Neighbors will sign source as parent, reply with an ack and send its BF to own neighbors.
  If no ack is received in a timeframe:
    1) initial BF not received due to wifi conflics
    2) ack not received due to wifi conflics
  Partial solution: send again BF and/or ack
  If no ack is received in a timeframe for the second time -> no children
    - node without children can start set reconciliation with the parent
  If ack is received means that ne node has neighbors, no action are performed until all neighbors will 
  send back the BF to start set reconciliation backward


  NB: the main idea is to perform set reconciliation during a timeframe in which flooding is not perfomerded 
  otherwise the routing table will be modified. An alternive solution is perfom set reconciliation using a copy of
  the routing table in particular instant of time. But this second solution is more computational complex.
  For now, let's use the first proposal. 
  */



    // when a node sends in broadcast a packet it will wait for acks from children. 
    // In case node doesn't receive at least one ack in a timeframe -> no children
    // ASSUMPTION: we assume that acks if sent will arrive at 100%, otherwise a duble ack mechanims or other solutions 
    // need to be implemeted
    // NB-> implement static routing for direct paths ????? maybe not beacuse it doesn't know the neighbor yet

  

  // root starts set reconciliation by sending a message type: ROOT_SR_HELLO 
  void
  SaharaRouting::StartTopologyBuilding(){

    // 1) send BF to neighbors in broadcast and wait ack
    
      // reset all routing tables variables and also all routing varibles

      if(m_intNodeID == 12){ // this is only to test (node 12 is the root), but in the real case the node with highest rep and lower id will start

        if(!m_reset){
          ResetVariablesUpdate();
        }
        NS_LOG_DEBUG(m_intNodeID << "-> [START_TOPOLOGY_BUILDING]");

        // the root is the parent of itself
        m_parentIP = m_mainAddress;

        Ptr<Packet> packet = Create<Packet> ();
    
        SaharaHeader sh;
        
        sh.SetMessageType(sahara::SaharaHeader::ROOT_SR_HELLO);
        sh.SetOriginIP(m_mainAddress);

        GlobalData::ReadDataFromFile(m_intNodeID, m_rep, m_gps, m_bat);
      
        
          sh.SetReputation_O(r_Table.GetNodeReputation(m_mainAddress));
        

        sh.SetGPS_O(m_gps);
        sh.SetBattery_O(m_bat);
       

        SaharaHeader::VotePacket myVTlist = m_ss.GetMyVotesList();
        
         std::cout << "EvaluatorIP: " << myVTlist.evaluatorIP << std::endl;
        std::cout << "Votes: " << std::endl;
        for (const auto& vote : myVTlist.votes) {
           std::cout << "EvaluatedIP " << vote.EvaluatedIP << " Vote " << std::endl;
        } 
        std::cout << "Signature: " << myVTlist.signature << std::endl;

        sh.SetVotes(m_ss.GetMyVotesList());

       


        
        

       

        //sh.SetVote_O(SaharaHeader::VoteState::Undefined);
        //sh.SetSign_O("ms49U12luTGuWlIKuCOi1Ar3u6xy82kMQPPyXY4hoTPOUhPF");

        //NS_LOG_DEBUG ("[SendHello] Sending Hello " << m_mainAddress << ", " << Ipv4Address("0.0.0.0") << ", " << m_intNodeID << ", " << init << ", " << init);    


        packet->AddHeader(sh);
       
        //m_auditTimeoutAckSRNEW.Schedule(MilliSeconds(m_ackTimeSlot + 2*static_cast<double>(m_intNodeID))); 

        Simulator::Schedule(MilliSeconds(10), &SaharaRouting::BroadcastPacket, this, packet);

        // every m_frequencySR do update
        m_SR.Schedule(MilliSeconds(m_frequencySR));
        
      }
  }


  // all root's children receive the ROOT_SR_HELLO
  void
  SaharaRouting::ProcessRootHello(SaharaHeader sh){

     if(!m_reset){
          ResetVariablesUpdate();
      }
    
    GlobalData::ReadDataFromFile(m_intNodeID, m_rep, m_gps, m_bat);

    Ipv4Address originIP = sh.GetOriginIP();
    uint16_t reputation_O = sh.GetReputation_O();
    uint16_t GPS_O = sh.GetGPS_O();
    uint16_t battery_O = sh.GetBattery_O();

    SaharaHeader::VoteState vote_O = sh.GetVote_O();
    //std::string sign_O = sh.GetSign_O();

    r_Table.SetVotes(originIP, sh.GetVotes());
    
    
   

    NS_LOG_DEBUG(m_intNodeID << "-> [ProcessRootHello] received hello from parent: " << originIP);


    // set node's parent
    m_parentIP = originIP;


    SaharaHeader shAck;
    shAck.SetMessageType(sahara::SaharaHeader::SR_HELLO);
    shAck.SetOriginIP(m_mainAddress);
    shAck.SetParentIP(m_parentIP);

   
    shAck.SetReputation_O(r_Table.GetNodeReputation(m_mainAddress));
  
    shAck.SetGPS_O(m_gps);
    shAck.SetBattery_O(m_bat);

    // set vote and signatures
    //shAck.SetVote_O(m_ss.GetVoteByNodeIP(m_parentIP));
    //shAck.SetSign_O("parent2luTGuWlIKuCOi1Ar3u6xy82kMQPPyXY4hoTPOUhPF");

    //r_Table.AddVote(m_parentIP, m_mainAddress, m_ss.GetVoteByNodeIP(m_parentIP), "parent2luTGuWlIKuCOi1Ar3u6xy82kMQPPyXY4hoTPOUhPF");

    //SaharaHeader(m_mainAddress, true);
    Ptr<Packet> ackPacket = Create<Packet>();
    ackPacket->AddHeader(shAck);

    m_auditTimeoutAckSRNEW.Schedule(MilliSeconds(m_ackTimeSlot + 10*static_cast<double>(m_intNodeID))); // ACTIVATE LATER
    Simulator::Schedule(MilliSeconds(2*m_intNodeID), &SaharaRouting::BroadcastPacket, this, ackPacket);  
    
   
    r_Table.AddTuple(originIP, m_mainAddress, reputation_O, r_Table.GetNodeReputation(m_mainAddress), GPS_O, m_gps, battery_O, m_bat);
    

  }

  // children receives the SR_HELLO message and process it
  void
  SaharaRouting::ProcessSRHello(SaharaHeader sh){
     if(!m_reset){
          ResetVariablesUpdate();
        }

    GlobalData::ReadDataFromFile(m_intNodeID, m_rep, m_gps, m_bat);
    NS_LOG_DEBUG(m_intNodeID << " -> [ProcessSRHello] received data from: " << sh.GetOriginIP());

    // in this case I'm the parent so the reply comes from a child
     if(sh.GetParentIP() == m_mainAddress){

        NS_LOG_DEBUG(m_intNodeID << "NewAck received from " << sh.GetOriginIP() << " , stopping timer");
        // stop timer beacause I've at least one child
        m_auditTimeoutAckSRNEW.Cancel(); //

        // add child to list of children
        m_listSetRecDone[sh.GetOriginIP()] = false;


        r_Table.SetVotes(sh.GetOriginIP(), sh.GetVotes());

        //r_Table.AddVote(m_mainAddress, sh.GetOriginIP(), sh.GetVote_O(), sh.GetSign_O());


        // add a tuple containing my info and the list of sender nodes -> it creates a pair
        
        r_Table.AddTuple(m_mainAddress, sh.GetOriginIP(), r_Table.GetNodeReputation(m_mainAddress), sh.GetReputation_O(), m_gps, sh.GetGPS_O(), m_bat, sh.GetBattery_O());

     }
     // in this case I've not the parent, so it means the sender is my parent
     else if (m_parentIP == Ipv4Address("0.0.0.0")){

        NS_LOG_DEBUG(m_intNodeID << " -> [ProcessSRHello]: No parent, sender is my parent, sender ->" << sh.GetOriginIP());

        // set sender as my parent
        m_parentIP = sh.GetOriginIP();

        r_Table.SetVotes(sh.GetOriginIP(), sh.GetVotes());

        // add a tuple containing my info + parent info -> create a tuple

        //r_Table.AddVote(sh.GetOriginIP(), sh.GetParentIP(), sh.GetVote_O(), sh.GetSign_O());

        //r_Table.AddVote(m_mainAddress, sh.GetOriginIP(), m_ss.GetVoteByNodeIP(sh.GetOriginIP()), "CHILDt2luTGuWlIKuCOi1Ar3u6xy82kMQPPyXY4hoTPOUhPF");
       
        r_Table.AddTuple(m_mainAddress, sh.GetOriginIP(), r_Table.GetNodeReputation(m_mainAddress), sh.GetReputation_O(), m_gps, sh.GetGPS_O(), m_bat, sh.GetBattery_O());
          

        SendDataNew();
        
     }
     // in this case I already have a parent and the sender is not my parent -> store information only because the sender is my neighbor
     else {
        
        NS_LOG_DEBUG(m_intNodeID << " -> [ProcessSRHello]: Already have a parent");

        r_Table.SetVotes(sh.GetOriginIP(), sh.GetVotes());

         // add a tuple containing my info + parent info -> create a tuple

         //r_Table.AddVote(sh.GetOriginIP(), sh.GetParentIP(), sh.GetVote_O(), sh.GetSign_O());

         //r_Table.AddVote(m_mainAddress, sh.GetOriginIP(), m_ss.GetVoteByNodeIP(sh.GetOriginIP()), "CHILDw2luTGuWlIKuCOi1Ar3u6xy82kMQPPyXY4hoTPOUhPF");
         
         r_Table.AddTuple(m_mainAddress, sh.GetOriginIP(), r_Table.GetNodeReputation(m_mainAddress), sh.GetReputation_O(), m_gps, sh.GetGPS_O(), m_bat, sh.GetBattery_O());
            
     }

  }

    // if a node doesn't receive SR_HELLO in a given time slot, we assume that a node has not children
    void
    SaharaRouting::NoChildren(){
      
      // run diskra on partial table to give a route to neighbors even if the connection is direct this step is mandatory
      r_Table.RunDijkstra(m_mainAddress);
      

      // root node -> 12 is for example
      if(m_intNodeID==12){

        // finish to do set rec from child, not starting the inverse process
        for(const auto& t : m_listSetRecDone){
          NS_LOG_DEBUG("check complete sr" << t.first);
          m_listSetRecDone[t.first] = false; // this ensure that the node has to start inverse SR with all children
        }

        // start inverse set reconcilition from root to children
        InverseSetRec();
        return;
      } 

      // backward set recinciliation with parent (child -> parent)

      // start set reconciliation with parent when a node doesn't have children 
      NS_LOG_DEBUG(m_intNodeID << ": [NoChildren] NO Ack received. Starting set reconciliation with parent");

      // ask to parent the bloom filter and continue the procedure
      AskToParentBF();

      
     
    }

    void
    SaharaRouting::AskToParentBF(){
      
      SaharaHeader sh;
      sh.SetMessageType(sahara::SaharaHeader::ASK_BF);
      sh.SetOriginIP(m_mainAddress);
      sh.SetRTDim(r_Table.GetSizeRoutingTable());
      NS_LOG_DEBUG("BF the node " << m_intNodeID);
      if(m_sr_dynamic_ON) {
        // like an empy bf because it will send later through the new new messages ASK_BF_P2C and SEND_BF_C2P
        std::vector<bool> empyBF;
        empyBF.push_back(true);
        sh.SetBF(empyBF);
      }
      else {
        r_Table.CreateBloomFilter();
        sh.SetBF(r_Table.GetBloomFilter());
      }
      
      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader(sh);

      Simulator::Schedule(MilliSeconds(3*m_intNodeID), &SaharaRouting:: SendPacketToDest, this, packet, m_parentIP);
      if(!m_ackTimeoutAskToParentBF.IsRunning()) m_ackTimeoutAskToParentBF.Schedule(MilliSeconds(m_ackTimeSlot));

      NS_LOG_DEBUG(m_intNodeID << ", [AskToParentBF], asked BF to parent -> " << m_parentIP);

    }

   
  void SaharaRouting::SendToChildBF(SaharaHeader sh) {
    // Check if we are currently processing a child request
    if (m_previousChildProcessed) {
        m_previousChildProcessed = false;

        // Update routing table
        r_Table.RunDijkstra(m_mainAddress); 

        NS_LOG_DEBUG(m_intNodeID << ", [SendToChildBF], received askBF from -> " << sh.GetOriginIP() 
                     << ", SIZE CHILD RT: " << sh.GetRTdim());

        // Store Bloom Filter received from the child
        m_listBFChildren[sh.GetOriginIP()] = sh.GetBloomFilter();
        NS_LOG_DEBUG(m_intNodeID << "BF OF CHILD " << sh.GetOriginIP());

        // Prepare response Bloom Filter
        SaharaHeader ack;
        ack.SetMessageType(sahara::SaharaHeader::SEND_BF);
        if (m_sr_dynamic_ON) {
            ack.SetBF(r_Table.GetDynamicBloomFilterIfActive(sh.GetRTdim(), false));
            
        } else {
            ack.SetBF(r_Table.GetBloomFilter());
        }

        // send already known evaluator votes (send only evaluator IP)
        ack.SetListVotesIP(r_Table.GetListVotesIP());

        // Create and send the packet
        Ptr<Packet> packet = Create<Packet>();
        packet->AddHeader(ack);

        Simulator::Schedule(MilliSeconds(5 * m_intNodeID), &SaharaRouting::SendPacketToDest, this, packet, sh.GetOriginIP());

    } else {
        // Parent is busy, check if the same request is already in the queue to avoid duplicates
        if (!IsAlreadyEnqueued(sh)) {
            NS_LOG_DEBUG(m_intNodeID << ", [SendToChildBF], parent is busy, enqueuing request from -> " << sh.GetOriginIP());
            m_childRequestQueue.push(sh);
        }
    }
}

bool SaharaRouting::IsAlreadyEnqueued(SaharaHeader sh) {
    // Create a copy of the queue to iterate over
    std::queue<SaharaHeader> tempQueue = m_childRequestQueue;

    while (!tempQueue.empty()) {
        SaharaHeader queued_sh = tempQueue.front();
        tempQueue.pop();
        if (queued_sh.GetOriginIP() == sh.GetOriginIP()) {
            return true;  // Request already enqueued
        }
    }
    return false;
}


void SaharaRouting::OnChildResponseReceived() {
    // Mark that the previous child request has been processed
    m_previousChildProcessed = true;

    // Process next request if there is one in the queue
    if (!m_childRequestQueue.empty()) {
        SaharaHeader nextRequest = m_childRequestQueue.front();
        m_childRequestQueue.pop();

        NS_LOG_DEBUG(m_intNodeID << ", [OnChildResponseReceived], processing next child request from -> " << nextRequest.GetOriginIP());

        // Re-process next request
        SendToChildBF(nextRequest);
    } else {
        NS_LOG_DEBUG(m_intNodeID << ", [OnChildResponseReceived], no more requests in the queue.");
    }
}


    // after asked the BF of the parent, child can now evaluate parent's missing tuples
    void
    SaharaRouting::ReceivedFromParentBF(SaharaHeader sh){
      m_ackTimeoutAskToParentBF.Cancel();

      NS_LOG_DEBUG(m_intNodeID << ", [ReceivedFromParentBF], received BF from parent -> " << m_parentIP);

      // retreive BF of the parent
      m_parentBF = sh.GetBloomFilter();

      //std::vector<Ipv4Address> lisIP = sh.GetListVotesIP();
   


      r_Table.ProcessSetReconciliationDynamic(m_parentBF); // TO IMPLEMENT BOOL SWITCH

      // send missing tuples to parent
      Ptr<Packet> packet = Create<Packet> ();
      SaharaHeader shm;
      shm.SetMessageType(sahara::SaharaHeader::SEND_MISSING_C2P);
      shm.SetChildIP(m_mainAddress);
      
      shm.SetMissingTuples(r_Table.GetMissingTuples());

      // add missing tuples votes
      shm.SetMissingVotePackets(r_Table.GetMissingVotePackets(sh.GetListVotesIP())); //



      packet->AddHeader(shm);
      
      NS_LOG_DEBUG(m_intNodeID << ", [ReceivedFromParentBF], sending missing tuples to parent -> " << m_parentIP);
      Simulator::Schedule(MilliSeconds(4*m_intNodeID), &SaharaRouting:: SendPacketToDest, this, packet, m_parentIP);
       
      //NS_LOG_DEBUG ("[SendHello] Sending Hello " << m_mainAddress << ", " << Ipv4Address("0.0.0.0") << ", " << m_intNodeID << ", " << init << ", " << init);    

    }

    void 
    SaharaRouting::ProcessReceivedMissing(SaharaHeader sh){

        std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> missingT = sh.GetMissingTuples();
         NS_LOG_DEBUG(m_intNodeID << " -> Printing missing");
        
        if(missingT.size() == 0){
          NS_LOG_DEBUG("No missing tuples in my node");
        } else{

        for (const auto& tuple : missingT) {
            
            // Extract elements from the tuple
            const Ipv4Address& originIP = std::get<0>(tuple);
            const Ipv4Address& hop1IP = std::get<1>(tuple);
            uint16_t repO = std::get<2>(tuple);
            uint16_t repH = std::get<3>(tuple);
            uint16_t GPSO = std::get<4>(tuple);
            uint16_t GPSH = std::get<5>(tuple);
            uint16_t batteryO = std::get<6>(tuple);
            uint16_t batteryH = std::get<7>(tuple);
            /*
            SaharaHeader::VoteState voteO =  static_cast<SaharaHeader::VoteState>(std::get<8>(tuple));
            SaharaHeader::VoteState voteH =  static_cast<SaharaHeader::VoteState>(std::get<9>(tuple));
            std::string signature = std::get<10>(tuple);
            */

            r_Table.AddTuple(originIP, hop1IP, repO, repH, GPSO, GPSH, batteryO, batteryH);
            
            //std::cout <<  "ADD TUPLE CALLED ProcessReceivedMissing : my address " << originIP << "rep " << repO << " origin address " << hop1IP  << "rep add " << repH<< std::endl;

                
            // Print elements
           // NS_LOG_DEBUG("Tuple Missing in my node: " << originIP << ", " << hop1IP << ", " << repO << ", " << repH << ", " << GPSO <<", " << GPSH <<", " << batteryO <<", " << batteryH);
        }
       
        
        }
        
        // Every parent node receives own missing tuples from every child either if a child doesn't have children or if a child has completed SR with sub-children

        // Every parent checks if there are still children that have not sent missing tuples

        // now store missing votes received by the child
        r_Table.AddReceivedMissingVotePackets(sh.GetMissingVotePackets());
       

        m_listSetRecDone[sh.GetChildIP()] = true;
        

        for(const auto & t : m_listSetRecDone){
          if(!t.second) {
            NS_LOG_DEBUG("THERE IS A NODE THAT HAS NOT SENT " << t.first);

            OnChildResponseReceived();
            return; // if there is at least one child that has not sent missing I need to wait otherwise send back to own parent
          }
        }

        // no more children to wait from missing tuples
        Simulator::Schedule(MilliSeconds(2), &SaharaRouting:: NoChildren, this);
    
    }



    void
    SaharaRouting::BroadcastPacketSET(Ptr<Packet> packet){
      NS_LOG_DEBUG(m_intNodeID << ": SENDING SET RECONCILIATION DATA");
      
      //packet = m_shCrypto.EncryptHeader(packet);

      //NS_LOG_DEBUG ("[BroadcastPacket] Called");
      for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i = m_socketAddressesSET.begin (); i != m_socketAddressesSET.end (); i++)
      {
        //TRAFFIC_LOG( "Interface Addr: " << i->second.GetLocal());
        Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
        i->first->SendTo (packet, 0, InetSocketAddress (broadcastAddr, m_saharaPort));
      }
    }


    void 
    SaharaRouting::InverseSetRec(){
      
      NS_LOG_DEBUG( m_intNodeID << ", [InverseSerRec]");
      
      if(m_listBFChildren.size() == 0 and !m_SRCompleted){
        m_SRCompleted = true;
        NS_LOG_DEBUG(m_intNodeID << " -> No children, END SET RECONCILIATION FOR THIS NODE");
        
        
       
       
       
        
        r_Table.GenerateRepMap();
         r_Table.UpdateFileHistory();
          Dijkstra();
           m_ss.ResetVariables();
        m_ss.LoadMacIdMap();
        m_ss.LoadLastBlock();
        NS_LOG_DEBUG(m_intNodeID << " VOTES AFTER SET RECONCILIATION");
        r_Table.PrintVotes();
         m_reset = false;

        //r_Table.SetAllTupleFalse();
        PrintAllInfo(); 

      
        GlobalData::UpdateAllowedIPs();
         
        
        // m_packets_processed = 0;
        // m_tot_byte_processed = 0;
        return;
      }
      int i = 0;
      for(const auto& t : m_listBFChildren){

          if(m_listSetRecDone[t.first] == false){
            NS_LOG_DEBUG(m_intNodeID << " [Inverse SR], Child -> " << t.first);
            if(m_sr_dynamic_ON){
              i = i + 10*m_intNodeID;
              Simulator::Schedule(MilliSeconds(i), &SaharaRouting:: AskBF2C, this, t.first);
              
            }
            else{
            SaharaHeader sh;
            sh.SetMessageType(sahara::SaharaHeader::SEND_MISSING_P2C);
            sh.SetParentIP(m_mainAddress);
            r_Table.ProcessSetReconciliation(t.second);
            sh.SetMissingTuples(r_Table.GetMissingTuples());
            

            Ptr<Packet> packet = Create<Packet>();
            packet->AddHeader(sh);
            // start ack timer
            i = i + 10*m_intNodeID;
            Simulator::Schedule(MilliSeconds(i), &SaharaRouting:: SendPacketToDest, this, packet, t.first);
            }
          }
      }

      for(const auto& t : m_listSetRecDone) {
        if(t.second == false){
          if(!m_auditTimeoutAckInverseSR.IsRunning()){
            m_auditTimeoutAckInverseSR.Schedule(MilliSeconds(m_ackTimeSlot)); 
            return;
          }
        }
      }

      if(!m_alreadyDone and !m_SRCompleted){
        NS_LOG_DEBUG(m_intNodeID << ", [SET RECONCILIATION] TERMINATED SUCCESSFULLY]");
       
        r_Table.GenerateRepMap();
        r_Table.UpdateFileHistory();
        Dijkstra();
        m_ss.ResetVariables();
          m_ss.LoadMacIdMap();
        m_ss.LoadLastBlock();
         NS_LOG_DEBUG(m_intNodeID << " VOTES AFTER SET RECONCILIATION");
        r_Table.PrintVotes();
       // r_Table.SetAllTupleFalse();
        PrintAllInfo();
        // m_packets_processed = 0;
        // m_tot_byte_processed = 0;
        m_reset = false; 
        m_alreadyDone =  true;
        GlobalData::UpdateAllowedIPs();

        
      }

    }

    void
    SaharaRouting::AskBF2C(Ipv4Address ipChild){
      NS_LOG_DEBUG(m_intNodeID << "ASK BF TO CHILD");
      SaharaHeader sh;
      sh.SetMessageType(sahara::SaharaHeader::ASK_BF_P2C);
      sh.SetOriginIP(m_mainAddress);
      sh.SetRTDim(r_Table.GetSizeRoutingTable());
      Ptr<Packet> packet = Create<Packet>();
      packet->AddHeader(sh);
      Simulator::Schedule(MilliSeconds(10), &SaharaRouting:: SendPacketToDest, this, packet, ipChild);

    }

    void
    SaharaRouting::SendBFC2P(SaharaHeader shReceived){
      SaharaHeader sh;
      sh.SetMessageType(sahara::SaharaHeader::SEND_BF_C2P);
      sh.SetOriginIP(m_mainAddress);
      sh.SetBF(r_Table.GetDynamicBloomFilterIfActive(shReceived.GetRTdim(), true));

      // need to send list of my ips where I already have votes
      sh.SetListVotesIP(r_Table.GetListVotesIP());


      Ptr<Packet> packet = Create<Packet>();
      packet->AddHeader(sh);
      Simulator::Schedule(MilliSeconds(10), &SaharaRouting:: SendPacketToDest, this, packet, shReceived.GetOriginIP());
    }

    void
    SaharaRouting::ReceivedFromChildBF(SaharaHeader shReceived){

         // Store the Bloom filter received from the child
        m_listBFChildren[shReceived.GetOriginIP()] = shReceived.GetBloomFilter();
        NS_LOG_DEBUG(m_intNodeID << "BF OF CHILD RECEIVED" << shReceived.GetOriginIP());

         

        SaharaHeader sh;
            sh.SetMessageType(sahara::SaharaHeader::SEND_MISSING_P2C);
            sh.SetParentIP(m_mainAddress);
            r_Table.ProcessSetReconciliationDynamic(shReceived.GetBloomFilter());
            sh.SetMissingTuples(r_Table.GetMissingTuples());

            // add missing tuples votes
            sh.SetMissingVotePackets(r_Table.GetMissingVotePackets(shReceived.GetListVotesIP())); //

            

            Ptr<Packet> packet = Create<Packet>();
            packet->AddHeader(sh);
            // start ack timer
          
          
            Simulator::Schedule(MilliSeconds(50), &SaharaRouting:: SendPacketToDest, this, packet, shReceived.GetOriginIP());

    }

    void
    SaharaRouting::ProcessReceivedMissingInverse(SaharaHeader sh){

         NS_LOG_DEBUG(m_intNodeID << "Received missing tuples from parent");
         std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> missingT = sh.GetMissingTuples();

        if(missingT.size() == 0){
          NS_LOG_DEBUG("No missing tuples in my node");
        } else{

        for (const auto& tuple : missingT) {
            // Extract elements from the tuple
            const Ipv4Address& originIP = std::get<0>(tuple);
            const Ipv4Address& hop1IP = std::get<1>(tuple);
            uint16_t repO = std::get<2>(tuple);
            uint16_t repH = std::get<3>(tuple);
            uint16_t GPSO = std::get<4>(tuple);
            uint16_t GPSH = std::get<5>(tuple);
            uint16_t batteryO = std::get<6>(tuple);
            uint16_t batteryH = std::get<7>(tuple);
            /*
            SaharaHeader::VoteState voteO =  static_cast<SaharaHeader::VoteState>(std::get<8>(tuple));
            SaharaHeader::VoteState voteH =  static_cast<SaharaHeader::VoteState>(std::get<9>(tuple));
            std::string signature = std::get<10>(tuple);
            */


            r_Table.AddTuple(originIP, hop1IP, repO, repH, GPSO, GPSH, batteryO, batteryH);
                
            // Print elements
            //NS_LOG_DEBUG("Tuple Missing in my node received from parent: " << originIP << ", " << hop1IP << ", " << repO << ", " << repH << ", " << GPSO <<", " << GPSH <<", " << batteryO <<", " << batteryH);
        }
        }
        
        //r_Table.RunDijkstra(m_mainAddress);
        r_Table.AddReceivedMissingVotePackets(sh.GetMissingVotePackets());

      SaharaHeader invAck;
      invAck.SetMessageType(sahara::SaharaHeader::SEND_MISSING_P2C_ACK);
      invAck.SetOriginIP(m_mainAddress);

      Ptr<Packet> packet = Create<Packet>();
      packet->AddHeader(invAck);
      
      Simulator::Schedule(MilliSeconds(2*m_intNodeID), &SaharaRouting:: SendPacketToDest, this, packet, m_parentIP);

      // now do the same with children
      for(const auto& t : m_listSetRecDone){
        m_listSetRecDone[t.first] = false;
      }

      ns3::Ptr<ns3::UniformRandomVariable> rng = ns3::CreateObject<ns3::UniformRandomVariable>();
      rng->SetAttribute("Min", ns3::DoubleValue(10.0));
      rng->SetAttribute("Max", ns3::DoubleValue(100.0));
      double randomNumber = rng->GetValue();
      Simulator::Schedule(MilliSeconds(randomNumber), &SaharaRouting::InverseSetRec, this);
    
    }

    void
    SaharaRouting::SendPacketToDest(Ptr<Packet> packet, Ipv4Address dest){
          NS_LOG_DEBUG(m_intNodeID << "[SendPacketToDest] To -> " << dest);
          
          // packet = m_shCrypto.EncryptHeader(packet);
          for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i = m_socketAddressesSET.begin (); i != m_socketAddressesSET.end (); i++)
          {
            if(i->first->SendTo (packet, 0, InetSocketAddress (dest, m_saharaPort)) != -1) return;

            else{
              NS_LOG_DEBUG(m_intNodeID << ", Failed to send message, error -> " << i->first->GetErrno());
              if(m_auditTimeoutAckInverseSR.IsRunning()) m_auditTimeoutAckInverseSR.Cancel();
              
            }
            
          }
    }

    void
    SaharaRouting::ProcessInverseAck(SaharaHeader sh){
    
      NS_LOG_DEBUG(m_intNodeID << "[ProcessInverseAck], Ack received from -> " << sh.GetOriginIP());
      m_listSetRecDone[sh.GetOriginIP()] = true;

    }


    void 
    SaharaRouting::PrintAllInfo(){
      
      NS_LOG_DEBUG("___________________________________________________________");
      NS_LOG_DEBUG("Priting all infos about current node: " << m_intNodeID);

      NS_LOG_DEBUG("Packet processed by node to do SR -> " << m_packets_processed);
      NS_LOG_DEBUG("Total byte processed by node to do SR -> " << m_tot_byte_processed);

      /*
       std::vector<bool> myBF = r_Table.GetBloomFilter();
       std::string toPrintToSee;
        for (int i = 0; i < myBF.size(); ++i) {
            toPrintToSee += std::to_string(myBF[i]);
        }
        NS_LOG_DEBUG(m_intNodeID << ": my bloom filter -> " << toPrintToSee);



      NS_LOG_DEBUG(m_intNodeID << ", has parent -> " << m_parentIP);

      // this is only to print
        std::string toPrintTosee;
        for (int i = 0; i < m_parentBF.size(); ++i) {
            toPrintTosee += std::to_string(m_parentBF[i]);
        }
        NS_LOG_DEBUG(m_intNodeID << ": bloom filter of the parent -> " << toPrintTosee);
        */

        NS_LOG_DEBUG(m_intNodeID << " -> Children: ");
        

        for(const auto& t : m_listSetRecDone){
          NS_LOG_DEBUG(t.first << ", checked? " << t.second);
        }
        
      NS_LOG_DEBUG("___________________________________________________________");

       r_Table.PrintAll();

    }

    void
    SaharaRouting::PrintStatistics(){

      NS_LOG_DEBUG("Node " << m_intNodeID);

      NS_LOG_DEBUG("#P = " << m_packets_processed);
      NS_LOG_DEBUG("#B = " << m_tot_byte_processed);

      

    }

    void
    SaharaRouting::PrintRoutingTable(){
      NS_LOG_DEBUG("Node: " << m_intNodeID);
      NS_LOG_DEBUG("Parent: " << m_parentIP);
      r_Table.PrintAll();
    }

  void
  SaharaRouting::SendDataNew(){

        NS_LOG_DEBUG(m_intNodeID << "-> [SEND_DATA_NEW]");
        GlobalData::ReadDataFromFile(m_intNodeID, m_rep, m_gps, m_bat);
       // send my data in broadcast 


        SaharaHeader shAck;
        shAck.SetMessageType(sahara::SaharaHeader::SR_HELLO);
        shAck.SetOriginIP(m_mainAddress);
        shAck.SetParentIP(m_parentIP);

        shAck.SetReputation_O(r_Table.GetNodeReputation(m_mainAddress));
        
       
        shAck.SetGPS_O(m_gps);
        shAck.SetBattery_O(m_bat);

        shAck.SetVotes(m_ss.GetMyVotesList());

        //SaharaHeader(m_mainAddress, true);
        Ptr<Packet> ackPacket = Create<Packet>();
        ackPacket->AddHeader(shAck);
        
        //NS_LOG_DEBUG ("[SendHello] Sending Hello " << m_mainAddress << ", " << Ipv4Address("0.0.0.0") << ", " << m_intNodeID << ", " << init << ", " << init);    

        // start timer to see if I have children
        r_Table.PrintAll();
        m_auditTimeoutAckSRNEW.Schedule(MilliSeconds(m_ackTimeSlot + 2*static_cast<double>(m_intNodeID))); // to start from siblings backward set reconciliation
        
        Simulator::Schedule(MilliSeconds(2*m_intNodeID), &SaharaRouting::BroadcastPacket, this, ackPacket);  
        
        
  }

  void
  SaharaRouting::NoChildrenNew(){
    NS_LOG_DEBUG(m_intNodeID << " -> No Children");

    //NS_LOG_DEBUG("Updatig routing table");
    //r_Table.UpdateRoutingTable();
    //Dijkstra();
    NoChildren();
  }

  // every time a new round of SR is perfomed
  void
  SaharaRouting::ResetVariablesUpdate(){
    NS_LOG_DEBUG(m_intNodeID << "[ResetVariablesUpdate] requested");

    // it seems a problem not related with the rouiting table
    /*
    RoutingTable r_Table2;
    r_Table = r_Table2;

     // inizialize file 
      std::string nodeIDString = std::to_string(m_intNodeID);
      std::string fileName = "historyRoutingTable_" + nodeIDString + ".txt";
      r_Table.SetFile(fileName);
      r_Table.SetMyNodeID(m_intNodeID);
      */

     /*
     // cancel timers if running
    m_auditTimeoutAckSRNEW.Cancel();
    m_ackTimeoutAskToParentBF.Cancel();
    m_auditTimeoutAckInverseSR.Cancel();
    m_auditDijkstra.Cancel();
    m_auditLookUpPacketQueue.Cancel();
    m_SR.Cancel();

   

    // Assign functions to timers
    m_auditDijkstra.SetFunction(&SaharaRouting::Dijkstra, this);
    m_auditLookUpPacketQueue.SetFunction(&SaharaRouting::LookupQueue, this);
    m_auditTimeoutAckSR.SetFunction (&SaharaRouting::NoChildren, this);
    m_auditTimeoutAckSRNEW.SetFunction(&SaharaRouting::NoChildrenNew,this);
    m_auditTimeoutAckInverseSR.SetFunction(&SaharaRouting::InverseSetRec, this);
    m_SR.SetFunction(&SaharaRouting::StartTopologyBuilding, this);
    m_ackTimeoutAskToParentBF.SetFunction(&SaharaRouting::AskToParentBF, this);
    */


    // reset statistic counters
    m_packets_processed = 0;
    m_tot_byte_processed = 0;

    // reset the reset
    m_reset = true;

    // reset parent IP for the node
    m_parentIP = Ipv4Address("0.0.0.0");


    m_parentBF.clear();
    m_listBFChildren.clear();
    m_listSetRecDone.clear();

    m_previousChildProcessed = true;
    m_alreadyDone = false;
    m_SRCompleted = false;

    while (!m_childRequestQueue.empty())
    {
        m_childRequestQueue.pop();
    }

    r_Table.ResetVariables();
    

    
  }


  
  void
  SaharaRouting::PrintNumberOfpackets(){
    
    NS_LOG_DEBUG( m_intNodeID << ", Hello Packets -> " << m_packets_processed);
  }

  bool
  SaharaRouting::GetIfRunning(){
    return m_run;
  }

  void SaharaRouting::ActiveDropping(){
    m_nodeDeletePackets = true;
  }


/*
  bool
  SaharaRouting::YourPromiscuousCallback(ns3::Ptr<ns3::NetDevice> device, ns3::Ptr<const ns3::Packet> packet, uint16_t protocol, const ns3::Address &source, const ns3::Address &destination, ns3::NetDevice::PacketType packetType)
{
    
    ns3::Ptr<ns3::Packet> packetCopy = packet->Copy();

    ns3::Ipv4Header ipv4Header;
    packetCopy->RemoveHeader(ipv4Header);

    ns3::Ptr<ns3::Node> node = device->GetNode();
    ns3::Ptr<ns3::Ipv4> ipv4 = node->GetObject<ns3::Ipv4>();
    ns3::Ipv4Address ipv4Addr = ipv4->GetAddress(ipv4->GetInterfaceForDevice(device), 0).GetLocal();
    
    ns3::UdpHeader udpHeader;
    packetCopy->PeekHeader(udpHeader);
    uint16_t port = udpHeader.GetDestinationPort();
    

    NS_LOG_UNCOND("Promiscuous packet received: " << ipv4Addr << " source: " << source << " port: " << port << " from: " << ipv4Header.GetSource() << " dest: " << ipv4Header.GetDestination() << "next hop: " << destination);
    return true;  // Indicate the packet has been processed
}
*/

  /* How to manage updates in an efficient way ? That is a good challenge

  One idea:
  Since every node knows the complete routing table, it can check which is the in charge to start set reconcilaition update. 
  If a node starts it and it is not in charge, it can be penalised by neighbors... (security part). 

  The idea is that each node has the hash of each routing tuple in the routing table. Beacause every node has previously 
  calculated it in order to create own bloom filter -> so, it doesn't add computational cost

  1) The node in charge starts set_rec_update by sending in broadcast a HELLO_SR_ROOT. 

  2) The nodes that receives it will calculate the hash of the tuple.
    - if the hash is already present in the routing table -> flag the tuple of the routing table like "checked"
    - if the hash is not present, it means that parent is not the same as the previus, so parentIP has changed,
      so, store the new tuple and the new hash in the routing table and flag with "checked"

  3) The children now will send in broadcast and HELLO_SR specifying its parent
    - If the receiver is the parent it will store the node as children. 
      -- If the hash of the tuple is already present in the routing table -> flag the tuple as "checked"
      -- If the hash of the tuple is not present -> and the new tuple and flag it as "checked"
    - If the receiver has already the parent:
      -- If the hash of the tuple is already present in the routing table -> flag the tuple as "checked"
      -- If the hash of the tuple is not present -> and the new tuple and flag it as "checked"
    - if the reciver has not parent yet it will add parent and check like before. 
  
4) when a node doesn't receive an ack before starting set reconciliation with the parent a UpdateRoutingTable is called
  where there will be deleted the tuples not flagged with "checked".
  Now we have the updated routing table, if the network doesn't change bloom filters will be the same and no data to transmi
  the tuples is sent so ---->>> total size of bytes it reduced up xxx%? to evaluate.
  Worst case scenario is when all the topolofy has changed
     
  // timeline: A node (highest rep, lowest ip) send in broadcas */

  } // namespace ns3
}

