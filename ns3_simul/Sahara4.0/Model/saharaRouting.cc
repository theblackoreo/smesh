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

      m_saharaPort = 9;
      m_saharaPortSET = 10;
      

      NS_LOG_DEBUG("All setted, starting routing protocol...");

  }


  void
  SaharaRouting::RecvPacket(Ptr<Socket> socket){

      //NS_LOG_DEBUG("[RecvPacket] Packet received");

      Address sourceAddr;
      Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
      //InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);

      SaharaHeader sh;
      packet->RemoveHeader(sh);

       NS_LOG_DEBUG("[RecvPacket] Packet received TYPE -> " << sh.GetMessageType());

      switch (sh.GetMessageType())
      {
      case sahara::SaharaHeader::HELLO_MESSAGE:
       
        ProcessHello(sh);
        break;
      case sahara::SaharaHeader::SET_BF:
       
        ProcessSetReconciliation(sh);
        break;
      case sahara::SaharaHeader::SET_ACK:
        ProcessSetAck(sh);
        break;

      case sahara::SaharaHeader::SEND_MISSING_C2P:
        ProcessReceivedMissing(sh);
        break;
      

      case sahara::SaharaHeader::SEND_MISSING_P2C:
        ProcessReceivedMissingInverse(sh);
        break;
      }
      
    
  }

   // A node receives set reconciliation request when it receives a BF from a node 
  void
  SaharaRouting::ReceiveSetRecReq(Ptr<Socket> socket){

    NS_LOG_DEBUG(m_intNodeID << " [SET RECONC]" << "Received req");

    Address sourceAddr;
    Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
    
    //ProcessSetReconciliation(packet);

  
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


      // Configure timers
      m_auditFloodingTimer.SetFunction (&SaharaRouting::AuditHellos, this);
      m_auditDijkstra.SetFunction(&SaharaRouting::Dijkstra, this);
      m_auditLookUpPacketQueue.SetFunction(&SaharaRouting::LookupQueue, this);
      m_auditTimeoutAckSR.SetFunction (&SaharaRouting::NoChildren, this);

      u_int32_t xxx = 100;
      m_queue.SetMaxQueueLen(xxx);
      m_queue.SetQueueTimeout(Seconds(30));


    
     
      
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


      // Start timers
      m_auditFloodingTimer.Schedule (MilliSeconds (m_timeToStartFlooding + randomNumber*static_cast<double>(m_intNodeID)));

      // double flooding, to test if it is better
      //Simulator::Schedule(MilliSeconds(m_timeToStartFlooding + 2000 + randomNumber2*static_cast<double>(m_intNodeID)), &SaharaRouting::SendHello, this);

      m_auditDijkstra.Schedule (MilliSeconds (m_timeToStartDijskra + randomNumber*static_cast<double>(m_intNodeID)));
      
      m_auditLookUpPacketQueue.Schedule (MilliSeconds (m_timeToStartPacketQueue + 2*static_cast<double>(m_intNodeID)));

      Simulator::Schedule(MilliSeconds(25000 + m_intNodeID), &SaharaRouting::PrintAllInfo, this);


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
      
    
       
      // manage broadcast
      for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i =
        m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
      {
        Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
        if(header.GetDestination() == broadcastAddr){

        
          Ptr<Ipv4Route> ipv4Route = Create<Ipv4Route>();
          ipv4Route->SetDestination(broadcastAddr);
          ipv4Route->SetSource(m_mainAddress); // Assuming m_mainAddress is the local address
          //ipv4Route->SetOutputDevice(oif); // Assuming oif is the output interface for the local node
          //ipv4Route->SetGateway(Ipv4Address("255.255.255.255"));
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
          NS_LOG_DEBUG("Next hop: " << nextHop);
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


    ns3::Ptr<ns3::UniformRandomVariable> rng = ns3::CreateObject<ns3::UniformRandomVariable>();
    rng->SetAttribute("Min", ns3::DoubleValue(0.0));
    rng->SetAttribute("Max", ns3::DoubleValue(10.0));
    double randomNumber = rng->GetValue();
    //Timer re-initialization
    m_auditFloodingTimer.Schedule (MilliSeconds (m_frequencyFlooding));

  }

  void
  SaharaRouting::ProcessHello(SaharaHeader sh){
  
    NS_LOG_DEBUG("[ProcessHello] processing hello...");
    
    Ipv4Address originIP = sh.GetOriginIP();
    Ipv4Address hop1IP = sh.GetHop1IP();
    uint16_t reputation_O = sh.GetReputation_O();
    uint16_t GPS_O = sh.GetGPS_O();
    uint16_t battery_O = sh.GetBattery_O();
    uint16_t reputation_H = sh.GetReputation_H();
    uint16_t GPS_H = sh.GetGPS_H();
    uint16_t battery_H = sh.GetBattery_H();

    NS_LOG_DEBUG("Node: " << m_intNodeID << "-> " << originIP << ", " << hop1IP << ", " << reputation_O << ", "<<reputation_H);

    if(hop1IP == Ipv4Address("0.0.0.0")) {
      
        r_Table.AddTuple(originIP, m_mainAddress, reputation_O,m_intNodeID, GPS_O, GPS_H, battery_O, battery_H);
        // forward hello 
        Ptr<Packet> packet = Create<Packet> ();
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
        
        //SaharaHeader(originIP, m_mainAddress, reputation_O, m_intNodeID, GPS_O, GPS_H, battery_O, battery_H);
        packet->AddHeader(sh);
      
      Simulator::Schedule(MilliSeconds(2 + m_intNodeID), &SaharaRouting::BroadcastPacket, this, packet);

      // BroadcastPacket (packet);

      }else if(!r_Table.CheckDuplicate(originIP, hop1IP) and (originIP!=m_mainAddress)){

        r_Table.AddTuple(originIP, hop1IP, reputation_O,reputation_H, GPS_O, GPS_H, battery_O, battery_H);
        Ptr<Packet> packet = Create<Packet> ();
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
        //SaharaHeader(originIP, hop1IP, reputation_O,reputation_H, GPS_O, GPS_H, battery_O, battery_H);
        packet->AddHeader(sh);
        Simulator::Schedule(MilliSeconds(2 + m_intNodeID), &SaharaRouting::BroadcastPacket, this, packet);
              
      }
      
     
        NS_LOG_DEBUG("Print RT -> " <<m_mainAddress );
        r_Table.PrintAll();        
      

  }

    void
    SaharaRouting::Dijkstra(){
        
      NS_LOG_DEBUG("[Dijkstra] Entered");

     
      r_Table.RunDijkstra(m_mainAddress);
      r_Table.UpdateFileHistory();
        
      
  
      // reschedule timer to evaluation Dijkstra
      m_auditDijkstra.Schedule (MilliSeconds (m_frequencyDijskra));
      
      if(m_intNodeID == 1){
        Simulator::Schedule(MilliSeconds(12000 + 100*m_intNodeID), &SaharaRouting::StartSetReconciliation, this);
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

  /*SET RECONCILIATION_______________________________________________________________________________________*/
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


 
  void
  SaharaRouting::StartSetReconciliation(){
    
    // 1) send BF to neighbors in broadcast and wait ack 
      if(m_intNodeID == 1){ // this is only to test (node 1 is the root)

        NS_LOG_DEBUG(m_intNodeID << "-> [START_SET_RECONCILIATION]");
        std::vector<bool> bloomFilter;
        bloomFilter = r_Table.CreateBloomFilter();
        m_parentIP = m_mainAddress;


        Ptr<Packet> packet = Create<Packet> ();
        SaharaHeader sh;
        sh.SetMessageType(sahara::SaharaHeader::SET_BF);
        sh.SetParentIP(m_mainAddress);
        sh.SetBF(bloomFilter);

        packet->AddHeader(sh);
        
        // broadcast BF using a dedicated port for set_reconciliation
        m_auditTimeoutAckSR.Schedule(MilliSeconds(m_ackTimeSlot + 2*static_cast<double>(m_intNodeID)));

        
        BroadcastPacketSET(packet); 
      }
      
  }


  void
  SaharaRouting::ProcessSetAck(SaharaHeader sh){
          NS_LOG_DEBUG(m_intNodeID << " Ack received, stopping timer");
          
          m_listBFChildren[sh.GetChildIP()] = sh.GetBloomFilter();
          m_listSetRecDone[sh.GetChildIP()] = false;
          m_auditTimeoutAckSR.Cancel();

          std::vector<bool> receivedBF;
          receivedBF = sh.GetBloomFilter();


          std::string toPrintTosee;
        for (int i = 0; i < receivedBF.size(); ++i) {
            toPrintTosee += std::to_string(receivedBF[i]);
        }
          NS_LOG_DEBUG(m_intNodeID << "-> bf of the child " << sh.GetChildIP() <<  "-> size: " << sh.GetSizeBF() << " -> " << toPrintTosee);
  }

  void
    SaharaRouting::ProcessSetReconciliation(SaharaHeader sh){

      if(m_parentIP != "0.0.0.0"){
        return;
      }
      
       
        m_parentIP = sh.GetParentIP();
        parentBF = sh.GetBloomFilter();
       

        NS_LOG_DEBUG("Parent IP -> " << m_parentIP);


        // this is only to print
        std::string toPrintTosee;
        for (int i = 0; i < parentBF.size(); ++i) {
            toPrintTosee += std::to_string(parentBF[i]);
        }
        NS_LOG_DEBUG(m_intNodeID << ": bloom filter received -> " << toPrintTosee);
        

        // reply with ack
        SaharaHeader shAck;
        shAck.SetMessageType(sahara::SaharaHeader::SET_ACK);
        shAck.SetChildIP(m_mainAddress);
        shAck.SetBF(r_Table.CreateBloomFilter());
        

        //SaharaHeader(m_mainAddress, true);
        Ptr<Packet> ackPacket = Create<Packet>();
        ackPacket->AddHeader(shAck);

        Simulator::Schedule(MilliSeconds(10 + 2*m_intNodeID), &SaharaRouting:: SendPacketToDest, this, ackPacket, m_parentIP);

        
        // send own bloom filter
        SendOwnBF();
      
    }

    void
    SaharaRouting::SendOwnBF(){

        NS_LOG_DEBUG(m_intNodeID << "-> [SendOwnBF]");
        std::vector<bool> bloomFilter;
        bloomFilter = r_Table.CreateBloomFilter();

        Ptr<Packet> packet = Create<Packet> ();
        SaharaHeader sh;
        sh.SetMessageType(sahara::SaharaHeader::SET_BF);
        sh.SetParentIP(m_mainAddress);
        sh.SetBF(bloomFilter);

        packet->AddHeader(sh);

        // broadcast BF using a dedicated port for set_reconciliation
        m_auditTimeoutAckSR.Schedule(MilliSeconds(m_ackTimeSlot + 2*static_cast<double>(m_intNodeID)));

        
        Simulator::Schedule(MilliSeconds(10 + 2*m_intNodeID), &SaharaRouting::BroadcastPacketSET, this, packet);  

    }


    // when a node sends in broadcast a packet it will wait for acks from children. 
    // In case node doesn't receive at least one ack in a timeframe -> no children
    // ASSUMPTION: we assume that acks if sent will arrive at 100%, otherwise a duble ack mechanims or other solutions 
    // need to be implemeted
    // NB-> implement static routing for direct paths ????? maybe not beacuse it doesn't know the neighbor yet
    void
    SaharaRouting::NoChildren(){
      
      // under the assumption that is node 1 that wants to start the set reconciliation
      PrintAllInfo();

      if(m_intNodeID==1){
        // finish to do set rec from child, not starting the inverse process
        InverseSetRec();
        return;
      } 


      // start set reconciliation with parent
      NS_LOG_DEBUG(m_intNodeID << ": [NoChildren] NO Ack received. Starting set reconciliation with parent");

      
      r_Table.ProcessSetReconciliation(parentBF);

      // send missing tuples to sender
      

      Ptr<Packet> packet = Create<Packet> ();
      SaharaHeader sh;
      sh.SetMessageType(sahara::SaharaHeader::SEND_MISSING_C2P);
      sh.SetChildIP(m_mainAddress);
      
      sh.SetMissingTuples(r_Table.GetMissingTuples());
     

      packet->AddHeader(sh);
      Simulator::Schedule(MilliSeconds(10 + 2*m_intNodeID), &SaharaRouting:: SendPacketToDest, this, packet, m_parentIP);
       
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
            r_Table.AddTuple(originIP, hop1IP, repO, repH, GPSO, GPSH, batteryO, batteryH);
                
            // Print elements
            NS_LOG_DEBUG("Tuple Missing in my node: " << originIP << ", " << hop1IP << ", " << repO << ", " << repH << ", " << GPSO <<", " << GPSH <<", " << batteryO <<", " << batteryH);
        }
        NS_LOG_DEBUG("End______________________________________");
        }
        r_Table.RunDijkstra(m_mainAddress);

        // Every parent node receives own missing tuples from every child either if a child doesn't have children or if a child has completed SR with sub-children

        // Every parent checks if there are still children that have not sent missing tuples

        m_listSetRecDone[sh.GetChildIP()] = true;

        for(const auto & t : m_listSetRecDone){
          if(!t.second) return; // if there is at least one child that has not sent missing I need to wait otherwise send back to own parent
        }

        // no more children to wait from missing tuples
        Simulator::Schedule(MilliSeconds(10 + 2*m_intNodeID), &SaharaRouting:: NoChildren, this);
    
    }



    void
    SaharaRouting::BroadcastPacketSET(Ptr<Packet> packet){
      NS_LOG_DEBUG(m_intNodeID << ": SENDING SET RECONCILIATION DATA");

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

      if(m_listBFChildren.size() == 0){
        NS_LOG_DEBUG(m_intNodeID << " -> No children, END SET RECONCILIATION FOR THIS NODE");
        r_Table.UpdateFileHistory();
        return;
      }
      
      for(const auto& t : m_listBFChildren){
          SaharaHeader sh;
          sh.SetMessageType(sahara::SaharaHeader::SEND_MISSING_P2C);
          sh.SetParentIP(m_mainAddress);
          r_Table.ProcessSetReconciliation(t.second);
          sh.SetMissingTuples(r_Table.GetMissingTuples());
          Ptr<Packet> packet = Create<Packet>();
          packet->AddHeader(sh);

          Simulator::Schedule(MilliSeconds(10 + 2*m_intNodeID), &SaharaRouting:: SendPacketToDest, this, packet, t.first);
 
      }

      NS_LOG_DEBUG(m_intNodeID << " -> Yes children, just sent final set, END SET RECONCILIATION FOR THIS NODE");
      r_Table.UpdateFileHistory();
    }

    void
    SaharaRouting::ProcessReceivedMissingInverse(SaharaHeader sh){

         NS_LOG_DEBUG(m_intNodeID << "Received missing tuples from parent");
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
            r_Table.AddTuple(originIP, hop1IP, repO, repH, GPSO, GPSH, batteryO, batteryH);
                
            // Print elements
            NS_LOG_DEBUG("Tuple Missing in my node received from parent: " << originIP << ", " << hop1IP << ", " << repO << ", " << repH << ", " << GPSO <<", " << GPSH <<", " << batteryO <<", " << batteryH);
        }
        NS_LOG_DEBUG("End______________________________________");
        }
         r_Table.RunDijkstra(m_mainAddress);

      // now do the same with children
      Simulator::Schedule(MilliSeconds(10 + 2*m_intNodeID), &SaharaRouting::InverseSetRec, this);
    
    }

    void
    SaharaRouting::SendPacketToDest(Ptr<Packet> packet, Ipv4Address dest){
          for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i = m_socketAddressesSET.begin (); i != m_socketAddressesSET.end (); i++)
          {
            Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
            i->first->SendTo (packet, 0, InetSocketAddress (dest, m_saharaPort));
          }
    } 



    void 
    SaharaRouting::PrintAllInfo(){
      
      NS_LOG_DEBUG("___________________________________________________________");
      NS_LOG_DEBUG("Priting all infos about current node");

      NS_LOG_DEBUG(m_intNodeID << ", has parent -> " << m_parentIP);

      // this is only to print
        std::string toPrintTosee;
        for (int i = 0; i < parentBF.size(); ++i) {
            toPrintTosee += std::to_string(parentBF[i]);
        }
        NS_LOG_DEBUG(m_intNodeID << ": bloom filter of the parent -> " << toPrintTosee);
        NS_LOG_DEBUG(m_intNodeID << " -> Children: ");

        for(const auto& t : m_listSetRecDone){
          NS_LOG_DEBUG(t.first << ", checked? " << t.second);
        }
        
      NS_LOG_DEBUG("___________________________________________________________");

       r_Table.PrintAll();

    }

    void
    SaharaRouting::PrintRoutingTable(){
      r_Table.PrintAll();
    }



    
  } // namespace ns3
}

