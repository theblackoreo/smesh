#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h" // Include NetAnim module
#include "ns3/udp-client-server-helper.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sstream> // for stringstream
#include "ns3/saharaHelper.h"
#include <ns3/log.h>
#include "saharaMobility.h"
#include "ns3/saharaRouting.h"
#include "ns3/olsr-module.h"
#include <map>
#include "ns3/timer.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/energy-module.h" // Include energy module
#include "ns3/wifi-radio-energy-model-helper.h" // Include WiFi energy model helper
#include "ns3/ackTag.h"


using namespace ns3;

double msgSent;
double msgReceived;
std::map<uint32_t, EventId> packetTimers;
int packetLost = 0;
int totPackets = 0;
std::map<uint32_t, double> mapMsgSent;



// Function to print the remaining energy of each node
void ReportRemainingEnergy (Ptr<BasicEnergySource> energySource) {
  double remainingEnergy = energySource->GetRemainingEnergy();
  //NS_LOG_UNCOND("Remaining energy: " << remainingEnergy);
  Simulator::Schedule(Seconds(1.0), &ReportRemainingEnergy, energySource);
}

void UpdateVelocity(Ptr<SaharaMobility> m, double radius, const Vector& center, double omega) {
 
     // Get the current position of the node
    Vector nodePosition = m->GetPosition();

    // Calculate the current position relative to the center
    double relativePosX = nodePosition.x - center.x;
    double relativePosY = nodePosition.y - center.y;

     // Calculate the current distance from the center
    double currentDistance = std::sqrt(relativePosX * relativePosX + relativePosY * relativePosY);

    // If the current distance is not equal to the desired radius, adjust the position
    if (currentDistance > radius+2) {
        double scaleFactor = radius / currentDistance;
        relativePosX *= scaleFactor;
        relativePosY *= scaleFactor;

        // Update the node position to keep it on the circular path
        nodePosition.x = center.x + relativePosX;
        nodePosition.y = center.y + relativePosY;
        m->SetPosition(nodePosition);
    }

    

    // Calculate the velocity components based on angular velocity and relative position
    double velocityX = -omega * relativePosY;
    double velocityY = omega * relativePosX;
    
    // Set the calculated velocity vector for the node
    Vector newVelocity(velocityX, velocityY, 0.0);
    m->SetVelocityAndAcceleration(newVelocity, Vector(0, 0, 0));
}


void PrintInfo (Ptr<SaharaMobility> m)
{
    Ptr<Node> n0 =  ns3::NodeList::GetNode(0);

    std::cout << "n0 Vel:" << m->GetVelocity() << std::endl;


}

void PrintPacketLost(){
  NS_LOG_UNCOND("Tot packets: " << totPackets);
  NS_LOG_UNCOND("Packet lost: " << packetLost);
}

void setPos(Ptr<SaharaMobility> m00, Ptr<SaharaMobility> m11, Vector v0, Vector v1){
    m00->SetPosition (v0);
    m11->SetPosition (v1);
  
}

void PrintPacketContent(Ptr<Packet> packet) {
    uint8_t buffer[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    std::cout << "Packet Content: ";
    for (uint32_t i = 0; i < packet->GetSize(); ++i) {
        std::cout << buffer[i];
    }
    std::cout << std::endl;
}

void sendMessageAck(Ptr<Socket> socket, Ipv4Address destIP, uint32_t uid){

      NS_LOG_UNCOND("sendMessageAck CALLED");
      // Create the ACK packet with a custom tag
        Ptr<Packet> ackPacket = Create<Packet>(100); // Create a 100-byte ACK packet

        // Add custom AckTag to the packet
        AckTag ackTag;
        ackTag.SetAckId(uid); // Store the UID of the received packet as ACK ID
        ackPacket->AddPacketTag(ackTag);

        // Send the ACK back to the sender
        if (socket->SendTo(ackPacket, 0, InetSocketAddress(destIP, 12345)) != -1) {
          // msgSent = Simulator::Now().GetSeconds();

            NS_LOG_UNCOND("ACK sent to: " << destIP << " uid " << uid );
        
      } else {
          std::cout << "error";
      }
}

void ReceivePacket(Ptr<Socket> socket) {
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from))) {

      //NS_LOG_UNCOND("Packet Received -> " << packet->GetUid()); 
      AckTag ackTag;

      if (packet->PeekPacketTag(ackTag) && ackTag.IsAck()) {
          //NS_LOG_UNCOND("Received an ACK, not sending another ACK.");
          //NS_LOG_UNCOND("ACK ID -> " << ackTag.GetAckId());
          continue;  
      }

        msgReceived = Simulator::Now().GetSeconds();

        // Verifica che l'UID del pacchetto esista
        auto it = mapMsgSent.find(packet->GetUid());
        if (it != mapMsgSent.end()) {
            double sentTime = it->second;
            double latency = msgReceived - sentTime;
            NS_LOG_UNCOND("Latency: " << latency * 1000 << " ms");

            auto timerIt = packetTimers.find(packet->GetUid());
            if (timerIt != packetTimers.end()) {
                Simulator::Cancel(timerIt->second);
                packetTimers.erase(timerIt);
            }

            // Non inviare nuovamente ACK per pacchetti già ricevuti
            mapMsgSent.erase(it);  // Rimuovi il pacchetto dalla mappa dei pacchetti inviati
          }
        
        Ipv4Address senderIp = InetSocketAddress::ConvertFrom(from).GetIpv4();
          Simulator::Schedule(MilliSeconds(100), sendMessageAck, socket, senderIp, packet->GetUid());
          Simulator::Schedule(MilliSeconds(300), sendMessageAck, socket, senderIp, packet->GetUid());


       // sendMessageAck(socket, senderIp, packet->GetUid());
        // sendMessageAck(socket, senderIp, packet->GetUid());
    }
}



void TimerCallback(uint32_t packetId, Ptr<Socket> socket, Ptr<Packet> pkt, uint16_t port){

 packetLost = packetLost +1;
  /*
  if (socket->SendTo(pkt, 0, InetSocketAddress(Ipv4Address("10.1.1.5"), port)) != -1) {
      msgSent = Simulator::Now().GetSeconds();
      

      totPackets = totPackets +1;
      
      EventId timerId;
      uint32_t packetId = pkt->GetUid();
      timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, pkt, port);
      packetTimers[pkt->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
*/
}

void sendMessage(Ptr<Socket> socket, uint16_t port){
    //std::string msg = "Hello World!";
      //NS_LOG_UNCOND("SENT:" << msg);

      Ptr<Packet> packet = Create<Packet>(1024);
      //NS_LOG_UNCOND("Sent ID = " << packet->GetUid());
     
      // Send the packet 
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.24"), port)) != -1) {
        //NS_LOG_UNCOND("message sent -> " << packet->GetUid());
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;

        totPackets = totPackets +1;
      
        EventId timerId;
        uint32_t packetId = packet->GetUid();

        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packet->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
}

void sendMessage2(Ptr<Socket> socket, uint16_t port){
    //std::string msg = "Hello World!";
      //NS_LOG_UNCOND("SENT:" << msg);

      Ptr<Packet> packet = Create<Packet>(1024);
      //NS_LOG_UNCOND("Sent ID = " << packet->GetUid());
     
      // Send the packet 
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.12"), port)) != -1) {
        //NS_LOG_UNCOND("message sent -> " << packet->GetUid());
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;

        totPackets = totPackets +1;
      
        EventId timerId;
        uint32_t packetId = packet->GetUid();

        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packet->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
}

void sendMessage3(Ptr<Socket> socket, uint16_t port){
    //std::string msg = "Hello World!";
      //NS_LOG_UNCOND("SENT:" << msg);

      Ptr<Packet> packet = Create<Packet>(1024);
      //NS_LOG_UNCOND("Sent ID = " << packet->GetUid());
     
      // Send the packet 
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.18"), port)) != -1) {
        //NS_LOG_UNCOND("message sent -> " << packet->GetUid());
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;

        totPackets = totPackets +1;
      
        EventId timerId;
        uint32_t packetId = packet->GetUid();

        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packet->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
}

void sendMessage4(Ptr<Socket> socket, uint16_t port){
    //std::string msg = "Hello World!";
      //NS_LOG_UNCOND("SENT:" << msg);

      Ptr<Packet> packet = Create<Packet>(1024);
      //NS_LOG_UNCOND("Sent ID = " << packet->GetUid());
     
      // Send the packet 
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.11"), port)) != -1) {
        //NS_LOG_UNCOND("message sent -> " << packet->GetUid());
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;

        totPackets = totPackets +1;
      
        EventId timerId;
        uint32_t packetId = packet->GetUid();

        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packet->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
}

void sendMessage5(Ptr<Socket> socket, uint16_t port){
    //std::string msg = "Hello World!";
      //NS_LOG_UNCOND("SENT:" << msg);

      Ptr<Packet> packet = Create<Packet>(1024);
      //NS_LOG_UNCOND("Sent ID = " << packet->GetUid());
     
      // Send the packet 
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.1"), port)) != -1) {
        //NS_LOG_UNCOND("message sent -> " << packet->GetUid());
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;

        totPackets = totPackets +1;
      
        EventId timerId;
        uint32_t packetId = packet->GetUid();

        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packet->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
}

void sendMessage6(Ptr<Socket> socket, uint16_t port){
    //std::string msg = "Hello World!";
      //NS_LOG_UNCOND("SENT:" << msg);

      Ptr<Packet> packet = Create<Packet>(1024);
      //NS_LOG_UNCOND("Sent ID = " << packet->GetUid());
     
      // Send the packet 
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.8"), port)) != -1) {
        //NS_LOG_UNCOND("message sent -> " << packet->GetUid());
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;

        totPackets = totPackets +1;
      
        EventId timerId;
        uint32_t packetId = packet->GetUid();

        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packet->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
}

void sendMessage7(Ptr<Socket> socket, uint16_t port){
    //std::string msg = "Hello World!";
      //NS_LOG_UNCOND("SENT:" << msg);

      Ptr<Packet> packet = Create<Packet>(1024);
      //NS_LOG_UNCOND("Sent ID = " << packet->GetUid());
     
      // Send the packet 
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.15"), port)) != -1) {
        //NS_LOG_UNCOND("message sent -> " << packet->GetUid());
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;

        totPackets = totPackets +1;
      
        EventId timerId;
        uint32_t packetId = packet->GetUid();

        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packet->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
}

void sendMessage8(Ptr<Socket> socket, uint16_t port){
    //std::string msg = "Hello World!";
      //NS_LOG_UNCOND("SENT:" << msg);

      Ptr<Packet> packet = Create<Packet>(1024);
      //NS_LOG_UNCOND("Sent ID = " << packet->GetUid());
     
      // Send the packet 
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.21"), port)) != -1) {
        //NS_LOG_UNCOND("message sent -> " << packet->GetUid());
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;

        totPackets = totPackets +1;
      
        EventId timerId;
        uint32_t packetId = packet->GetUid();

        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packet->GetUid()] = timerId;

      } else {
          std::cout << "error";
      }
}





int main (int argc, char *argv[])
{
  // Enable logging
  //LogComponentEnable("UdpSocketImpl", LOG_LEVEL_ALL);
  
  //LogComponentEnable("saharaRoutingProtocol", LOG_LEVEL_ALL);
  //LogComponentEnable("routingTable", LOG_LEVEL_ALL);
  //LogComponentEnable("OlsrRoutingProtocol", LOG_LEVEL_ALL);
  //LogComponentEnable("saharaCrypto", LOG_LEVEL_ALL);
    LogComponentEnable("saharaSecurity", LOG_LEVEL_ALL);
  //LogComponentEnable("saharaHeader", LOG_LEVEL_ALL);
  //LogComponentEnable("saharaQueue", LOG_LEVEL_ALL);
  

  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Create nodes
  NodeContainer nodes;
  nodes.Create(30);
  


  // Create wifi
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ax);

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;
  wifiPhy.Set ("TxPowerStart", DoubleValue(6.2));
  wifiPhy.Set ("TxPowerEnd", DoubleValue(6.2));
  wifiPhy.Set ("TxPowerLevels", UintegerValue(1));

  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

  wifiPhy.SetChannel(wifiChannel.Create());
  
  NetDeviceContainer devices = wifi.Install(wifiPhy, mac, nodes);

  // Set up energy source and WiFi energy consumption model
  BasicEnergySourceHelper energySourceHelper;
  energySourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(10000.0)); // 10000 Joules initial energy

  EnergySourceContainer energySources = energySourceHelper.Install(nodes);

  WifiRadioEnergyModelHelper wifiEnergyHelper;
  DeviceEnergyModelContainer deviceModels = wifiEnergyHelper.Install(devices, energySources);

  // Report the remaining energy periodically
  for (uint32_t i = 0; i < nodes.GetN(); ++i) {
    if(i == 10){
       Ptr<BasicEnergySource> energySource = DynamicCast<BasicEnergySource>(energySources.Get(i));
       Simulator::Schedule(Seconds(1.0), &ReportRemainingEnergy, energySource);

    }
  }
   

// write mac addresses into files and public keys
    std::ofstream outFile;
    outFile.open("macAddresses.txt");

for (uint32_t i = 0; i < devices.GetN(); ++i) {
    Ptr<NetDevice> netDevice = devices.Get(i);
    Ptr<Node> node = netDevice->GetNode();

    Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice>(netDevice);

    if (wifiNetDevice) {
         if (outFile.is_open())
    {
        Mac48Address macAddr = wifiNetDevice->GetMac()->GetAddress();
        outFile << node->GetId() + 1 << " " << macAddr << std::endl;
    }
    else
    {
        std::cerr << "Unable to open file for writing" << std::endl;
    } 
    }
}
  // Close the file stream
    outFile.close();

    /*
    MobilityHelper mobility;
    ObjectFactory pos;
    pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
    pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"));
    pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"));

    std::ostringstream speedConstantRandomVariableStream;
    speedConstantRandomVariableStream << "ns3::ConstantRandomVariable[Constant=" << 0
                                      << "]";

    Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
    mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                              "Speed",
                              StringValue(speedConstantRandomVariableStream.str()),
                              "Pause",
                              StringValue("ns3::ConstantRandomVariable[Constant=2.0]"),
                              "PositionAllocator",
                              PointerValue(taPositionAlloc));
    mobility.SetPositionAllocator(taPositionAlloc);
    
    
    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(30.0),
                                  "DeltaY",
                                  DoubleValue(30.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-500, 500, -500, 500))
                              ); 
              */                

  // Mobility of the nodes
  
  
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0.0),
                                  "MinY", DoubleValue (0.0),
                                  "DeltaX", DoubleValue (60.0),
                                  "DeltaY", DoubleValue (60.0),
                                  "GridWidth", UintegerValue (6),
                                  "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");


  mobility.Install (nodes);
    
  

 //Sahara routing
  SaharaHelper sahara;
  // Internet stack
  InternetStackHelper internet;

  

  OlsrHelper olsr;

  AodvHelper aodv;
  aodv.Set("HelloInterval", TimeValue(Seconds(7)));       // Pacchetti HELLO ogni 1 secondo
  aodv.Set("ActiveRouteTimeout", TimeValue(Seconds(7))); // Timeout dei percorsi attivi di 10 secondi
  aodv.Set("MyRouteTimeout", TimeValue(Seconds(7)));     // Tempo massimo di utilizzo di una route

  DsdvHelper dsdv;
  dsdv.Set("PeriodicUpdateInterval", TimeValue(Seconds(8)));  // Aggiornamenti ogni 5 secondi

  internet.SetRoutingHelper(sahara);

  internet.Install(nodes);
  
  /*
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::SaharaMobility");
  mobility.Install (nodes); 

   std::ifstream file("/home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/mobility.csv");
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << "mobility.csv" << std::endl;
    }
    std::string line;

    while (std::getline(file, line)) {

        std::istringstream lineStream(line);
        
        std::string value;

        std::getline(lineStream, value, ',');
        int nodeID = std::stoi(value);
        std::cout << "NodeId ->" << nodeID << std::endl;
        Ptr<SaharaMobility> m = DynamicCast<SaharaMobility>(nodes.Get(nodeID)->GetObject<MobilityModel> ());
        Ptr<ns3::sahara::SaharaRouting> rr = DynamicCast<ns3::sahara::SaharaRouting>(nodes.Get(0)->GetObject<Ipv4RoutingProtocol> ());
        
        


        std::getline(lineStream, value, ',');
        double posX = std::stod(value);

        std::getline(lineStream, value, ',');
        double posY = std::stod(value);

        Vector v = Vector(posX, posY, 0);
        m->SetPosition(v);

        std::getline(lineStream, value, ',');
        double radius = std::stod(value);
        

        Vector center(400.0, 400.0, 0.0);

        std::getline(lineStream, value, ',');
        double omega = std::stod(value); // Angular velocity in radians per second

    
        Vector nodePosition = m->GetPosition();
        double relativePosX = nodePosition.x - center.x;
        double relativePosY = nodePosition.y - center.y;

        // Calculate the initial velocity vector based on the initial position and angular velocity
        double velX = -omega * relativePosY;  // Velocity component in X direction
        double velY = omega * relativePosX;  // Velocity component in Y direction
        Vector initialVelocity(velX, velY, 0.0);

        m->SetVelocityAndAcceleration(initialVelocity, Vector(0, 0, 0));
        double interval = 0.1;  // Interval in seconds
        for (double time = 0; time < 1000; time += interval) {
            Simulator::Schedule(Seconds(time), &UpdateVelocity,m, radius, center, omega);
        }
        
    }

    file.close();
    */

  
  // suggestion from Pecorella
   //Ipv4ListRoutingHelper listRH;
   //SaharaHelper sahara;
  // listRH.Add(sahara, 0);
  // Ipv4StaticRoutingHelper staticRh;
  // listRH.Add(staticRh, 5);



  // assign IP addresses and mount a static routing table (bacuse we need to change it)
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // create shared key
    using namespace CryptoPP;
    // Initialize Random Number Generator
    AutoSeededRandomPool rng;
    SecByteBlock key(16); // 16 bytes key (128 bits)
    rng.GenerateBlock(key, key.size());
    std::string hexKey;
    StringSource ss(key, key.size(), true, new HexEncoder(new StringSink(hexKey)));

    // write ip file
    std::ofstream ipFile;
    ipFile.open("ips.txt");
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        Ptr<Node> node = nodes.Get(i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        Ipv4Address ipAddress = ipv4->GetAddress(1, 0).GetLocal(); // Assuming first interface
        ipFile << ipAddress << std::endl;
    }
    ipFile << "10.1.1.255" << std::endl;
    ipFile.close();

  
  // RECEIVERs socket
  for(uint32_t i = 0; i < 24; i++){
    Ptr<Socket> recvSocket = Socket::CreateSocket (nodes.Get (i), UdpSocketFactory::GetTypeId ());
    recvSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 12345)); // Listen on port 9
    recvSocket->SetRecvCallback (MakeCallback (&ReceivePacket));
   
  }

    uint16_t port = 12345;

    /*
    Ptr<Socket> socket_sender = Socket::CreateSocket (nodes.Get (7), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    //InetSocketAddress remote = InetSocketAddress(Ipv4Address("255.255.255.255"), 9);
    // socket_sender->SetAllowBroadcast(true);

    int i = 0;
    for(i = 0; i < 12; i++){
      Simulator::Schedule(MilliSeconds(8000 + 1000*i), &sendMessage, socket_sender, port);
    }

*/
  int totalMessages = 150;
  int messagesPerNode = 3;
  int nodesToSend = totalMessages / messagesPerNode;  // Numero di nodi che devono inviare messaggi

  int messageCounter = 0;
  for (int i = 0; i < 8 && messageCounter < nodesToSend; i++) {
      int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
      if (randomNode == 23) continue;  // Salta il nodo 23

      Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));

      // Invia 10 messaggi consecutivi da questo nodo
      for (int j = 0; j < messagesPerNode; j++) {
          Simulator::Schedule(Seconds(7 + i + (0.8 * j)), &sendMessage, socket_sender, port);  // Invia ogni messaggio distanziato di 0.1 secondi
      }
      messageCounter++;
  }

  messageCounter = 0;
  for (int i = 0; i < 8 && messageCounter < nodesToSend; i++) {
      int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
      if (randomNode == 11) continue;  // Salta il nodo 23

      Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));

      // Invia 10 messaggi consecutivi da questo nodo
      for (int j = 0; j < messagesPerNode; j++) {
          Simulator::Schedule(Seconds(22 + i + (0.8 * j)), &sendMessage2, socket_sender, port);  // Invia ogni messaggio distanziato di 0.1 secondi
      }
      messageCounter++;
  }

  messageCounter = 0;
  for (int i = 0; i < 8 && messageCounter < nodesToSend; i++) {
      int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
      if (randomNode == 10) continue;  // Salta il nodo 23

      Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));

      // Invia 10 messaggi consecutivi da questo nodo
      for (int j = 0; j < messagesPerNode; j++) {
          Simulator::Schedule(Seconds(37 + i + (0.8 * j)), &sendMessage3, socket_sender, port);  // Invia ogni messaggio distanziato di 0.1 secondi
      }
      messageCounter++;
  }

  messageCounter = 0;
  for (int i = 0; i < 8 && messageCounter < nodesToSend; i++) {
      int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
      if (randomNode == 17) continue;  // Salta il nodo 23

      Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));

      // Invia 10 messaggi consecutivi da questo nodo
      for (int j = 0; j < messagesPerNode; j++) {
          Simulator::Schedule(Seconds(52 + i + (0.8 * j)), &sendMessage4, socket_sender, port);  // Invia ogni messaggio distanziato di 0.1 secondi
      }
      messageCounter++;
  }

  messageCounter = 0;
  for (int i = 0; i < 8 && messageCounter < nodesToSend; i++) {
      int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
      if (randomNode == 0) continue;  // Salta il nodo 23

      Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));

      // Invia 10 messaggi consecutivi da questo nodo
      for (int j = 0; j < messagesPerNode; j++) {
          Simulator::Schedule(Seconds(67 + i + (0.8 * j)), &sendMessage5, socket_sender, port);  // Invia ogni messaggio distanziato di 0.1 secondi
      }
      messageCounter++;
  }

  messageCounter = 0;
  for (int i = 0; i < 8 && messageCounter < nodesToSend; i++) {
      int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
      if (randomNode == 7) continue;  // Salta il nodo 23

      Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));

      // Invia 10 messaggi consecutivi da questo nodo
      for (int j = 0; j < messagesPerNode; j++) {
          Simulator::Schedule(Seconds(81 + i + (0.8 * j)), &sendMessage6, socket_sender, port);  // Invia ogni messaggio distanziato di 0.1 secondi
      }
      messageCounter++;
  }

   messageCounter = 0;
  for (int i = 0; i < 8 && messageCounter < nodesToSend; i++) {
      int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
      if (randomNode == 14) continue;  // Salta il nodo 23

      Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));

      // Invia 10 messaggi consecutivi da questo nodo
      for (int j = 0; j < messagesPerNode; j++) {
          Simulator::Schedule(Seconds(81 + i + (0.6 * j)), &sendMessage7, socket_sender, port);  // Invia ogni messaggio distanziato di 0.1 secondi
      }
      messageCounter++;
  }

  messageCounter = 0;
  for (int i = 0; i < 8 && messageCounter < nodesToSend; i++) {
      int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
      if (randomNode == 20) continue;  // Salta il nodo 23

      Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));

      // Invia 10 messaggi consecutivi da questo nodo
      for (int j = 0; j < messagesPerNode; j++) {
          Simulator::Schedule(Seconds(81 + i + (0.7 * j)), &sendMessage8, socket_sender, port);  // Invia ogni messaggio distanziato di 0.1 secondi
      }
      messageCounter++;
  }





/*
// Intervallo 27-40 secondi
for (int i = 0; i < 14; i++) {
    int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
    Ptr<Socket> socket_sender = Socket::CreateSocket (nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));
    
    Simulator::Schedule(Seconds(27 + i), &sendMessage, socket_sender, port);
}

// Intervallo 48-60 secondi
for (int i = 0; i < 13; i++) {
    int randomNode = rand() % 30;  // Seleziona un nodo casuale da 0 a 29
    Ptr<Socket> socket_sender = Socket::CreateSocket (nodes.Get(randomNode), TypeId::LookupByName("ns3::UdpSocketFactory"));
    
    Simulator::Schedule(Seconds(48 + i), &sendMessage, socket_sender, port);
}
*/


   



 Simulator::Schedule(MilliSeconds(41000), &PrintPacketLost);
    //}
 

  // Run simulation
  Simulator::Stop(Seconds(200.0));
  Simulator::Run();


}