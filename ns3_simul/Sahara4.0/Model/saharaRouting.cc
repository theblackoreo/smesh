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
  SaharaRouting::TestRecv(Ptr<Socket> socket){

      NS_LOG_DEBUG("[TestRecv] Packet received");

      Address sourceAddr;
      Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
      InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
      
      NS_LOG_DEBUG("Processing hello...");
      ProcessHello(packet);
    
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
        recvSocket->SetRecvCallback(MakeCallback(&SaharaRouting::TestRecv, this));
       

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
        InetSocketAddress inetAddr (m_ipv4->GetAddress (i, 0).GetLocal (), m_saharaPortSET);
        socket->SetRecvCallback(MakeCallback(&SaharaRouting::TestRecv, this));
        if (socket->Bind (inetAddr))
          {
            NS_FATAL_ERROR ("SaharaRouting::DoInitilize::Failed to bind socket!");
          }

        Ptr<NetDevice> netDevice = m_ipv4->GetNetDevice (i);
        socket->BindToNetDevice (netDevice);
        m_socketAddressesSET[socket] = m_ipv4->GetAddress (i, 0);
      }

      m_mainAddress = m_ipv4->GetAddress (1, 0).GetLocal ();


      // Configure timers
      m_auditFloodingTimer.SetFunction (&SaharaRouting::AuditHellos, this);
      m_auditDijkstra.SetFunction(&SaharaRouting::Dijkstra, this);
      m_auditLookUpPacketQueue.SetFunction(&SaharaRouting::LookupQueue, this);

      u_int32_t xxx = 10;
      m_queue.SetMaxQueueLen(xxx);
      m_queue.SetQueueTimeout(Seconds(30));

      // Start timers
      m_auditFloodingTimer.Schedule (MilliSeconds (m_timeToStartFlooding + 10*static_cast<double>(m_intNodeID)));

      m_auditDijkstra.Schedule (MilliSeconds (m_timeToStartDijskra + 10*static_cast<double>(m_intNodeID)));
      
      m_auditLookUpPacketQueue.Schedule (MilliSeconds (m_timeToStartPacketQueue + 10*static_cast<double>(m_intNodeID)));

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

          NS_LOG_DEBUG(header.GetSource() << "-> Need to Broadcast");
          NS_LOG_DEBUG(header.GetDestination() << "dest -> Need to Broadcast");

        
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
            Ptr<Packet> packetCopy = p->Copy();
            UdpHeader uh;
            packetCopy->RemoveHeader(uh);
            if(uh.GetSourcePort() == m_saharaPort){
              lcb(p, header, interfaceNum);
              return true;
            } else{
              lcb(p, header, interfaceNum);
              return true;
            }
        
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
      SaharaHeader saharaHeader = SaharaHeader(m_mainAddress, Ipv4Address("0.0.0.0"), m_intNodeID, init, def,def,def,def);
      
      //NS_LOG_DEBUG ("[SendHello] Sending Hello " << m_mainAddress << ", " << Ipv4Address("0.0.0.0") << ", " << m_intNodeID << ", " << init << ", " << init);    

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

    NS_LOG_DEBUG("Node: " << m_intNodeID << "-> " << originIP << ", " << hop1IP << ", " << reputation_O << ", "<<reputation_H);

    if(hop1IP == Ipv4Address("0.0.0.0")) {

      r_Table.AddTuple(originIP, m_mainAddress, reputation_O,m_intNodeID, GPS_O, GPS_H, battery_O, battery_H);
      // forward hello 
      Ptr<Packet> packet = Create<Packet> ();
      SaharaHeader saharaHeader = SaharaHeader(originIP, m_mainAddress, reputation_O, m_intNodeID, GPS_O, GPS_H, battery_O, battery_H);
      packet->AddHeader(saharaHeader);

      Simulator::Schedule(MilliSeconds(100 + 100*m_intNodeID), &SaharaRouting::BroadcastPacket, this, packet);

      // BroadcastPacket (packet);

      }else if(!r_Table.CheckDuplicate(originIP, hop1IP) and (originIP!=m_mainAddress)){

        r_Table.AddTuple(originIP, hop1IP, reputation_O,reputation_H, GPS_O, GPS_H, battery_O, battery_H);
        Ptr<Packet> packet = Create<Packet> ();
        SaharaHeader saharaHeader = SaharaHeader(originIP, hop1IP, reputation_O,reputation_H, GPS_O, GPS_H, battery_O, battery_H);
        packet->AddHeader(saharaHeader);
        Simulator::Schedule(MilliSeconds(100 + 100*m_intNodeID), &SaharaRouting::BroadcastPacket, this, packet);
              
      }
      
      if(m_intNodeID ==2){
        NS_LOG_DEBUG("Print RT -> " <<m_mainAddress );
        r_Table.PrintAll();        
      }

  }

    void
    SaharaRouting::Dijkstra(){
        
      NS_LOG_DEBUG("[Dijkstra] Entered");
      r_Table.RunDijkstra(m_mainAddress);
        
      //StartSetReconciliation();

      // reschedule timer to evaluation Dijkstra
      m_auditDijkstra.Schedule (MilliSeconds (m_frequencyDijskra));
    }

    void
    SaharaRouting::StartSetReconciliation(){
      
      // send bloom filter in broadcast
      if(m_intNodeID == 1){
        std::vector<bool> bloomFilter;
        bloomFilter = r_Table.CreateBloomFilter();
        
        // bloom filter received correctly
        std::vector<uint8_t> byteVector(bloomFilter.begin(), bloomFilter.end());
        Ptr<Packet> packet = Create<Packet>(&byteVector[0], byteVector.size());
        
        // broadcast BF
        BroadcastPacketSET(packet);
          
      }

    }

    void
    SaharaRouting::BroadcastPacketSET(Ptr<Packet> packet){
      NS_LOG_DEBUG(m_intNodeID << ": SENDING SET RECONCILIATION DATA");

      //NS_LOG_DEBUG ("[BroadcastPacket] Called");
      for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i = m_socketAddressesSET.begin (); i != m_socketAddressesSET.end (); i++)
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
          
      NS_LOG_DEBUG(m_intNodeID << ": Printing bloom filter received");
      std::string toPrintTosee;
        for (int i = 0; i < 16; ++i) {
          toPrintTosee += std::to_string(receivedBloomFilter[i]);
        }
        NS_LOG_DEBUG(m_intNodeID << ": bloom filter received ->" << toPrintTosee);
        r_Table.ProcessSetReconciliation(receivedBloomFilter);  
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
          sendPacketFromQueue(t, nextHop);

        }
      }

      m_auditLookUpPacketQueue.Schedule (MilliSeconds (m_frequencyLookUpPacketQueue));
      return;
    
    }

    void
    SaharaRouting::sendPacketFromQueue(Ipv4Address dst, Ipv4Address nextHop){

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

    
  } // namespace ns3
}

