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
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-header.h"
#include "ns3/packet.h"
#include "ns3/buffer.h"
#include "ns3/udp-header.h"
#include <cryptopp/osrng.h>
#include <cryptopp/secblock.h>
#include "ns3/aodv-module.h"



using namespace ns3;
double msgSent;
double msgReceived;
std::map<uint32_t, EventId> packetTimers;
int packetLost = 0;
int totPackets = 0;
std::map<uint32_t, double> mapMsgSent;


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

void ReceivePacket(Ptr<Socket> socket) {
    Ptr<Packet> packet;
    Address from;
    Ipv4Address receiverIp = socket->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

    while ((packet = socket->RecvFrom(from))) {
        msgReceived = Simulator::Now().GetSeconds();

        double throughput = (packet->GetSize() * 8 / (msgReceived - mapMsgSent.at(packet->GetUid()))) / 100000;  // Mbps

        auto it = packetTimers.find(packet->GetUid());
        if (it != packetTimers.end()) {
            // Cancel the timer associated with the packet
            Simulator::Cancel(it->second);
            packetTimers.erase(it);
        }

        NS_LOG_UNCOND("Throughput (Mbps): " << throughput);

        Ipv4Address senderIp = InetSocketAddress::ConvertFrom(from).GetIpv4();
        // Log received packet information here if needed
    }
}
void TimerCallback(uint32_t packetId, Ptr<Socket> socket, Ptr<Packet> pkt, uint16_t port) {
    packetLost++;
    if (socket->SendTo(pkt, 0, InetSocketAddress(Ipv4Address("10.1.1.24"), port)) != -1) {
        msgSent = Simulator::Now().GetSeconds();
        totPackets++;

        EventId timerId;
        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, pkt, port);
        packetTimers[packetId] = timerId;
    } else {
        std::cout << "Error sending packet" << std::endl;
    }
}


void sendMessage(Ptr<Socket> socket, uint16_t port) {
    Ptr<Packet> packet = Create<Packet>(1024);  // Create a packet of 1024 bytes

    if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.5"), port)) != -1) {
        msgSent = Simulator::Now().GetSeconds();
        mapMsgSent[packet->GetUid()] = msgSent;
        totPackets++;

        EventId timerId;
        uint32_t packetId = packet->GetUid();
        timerId = Simulator::Schedule(MilliSeconds(100), &TimerCallback, packetId, socket, packet, port);
        packetTimers[packetId] = timerId;
    } else {
        std::cout << "Error sending packet" << std::endl;
    }
}
void PrintPacketLost() {
    NS_LOG_UNCOND("Total packets: " << totPackets);
    NS_LOG_UNCOND("Packets lost: " << packetLost);
}


bool YourPromiscuousCallback(ns3::Ptr<ns3::NetDevice> device, ns3::Ptr<const ns3::Packet> packet, uint16_t protocol, const ns3::Address &source, const ns3::Address &destination, ns3::NetDevice::PacketType packetType)
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



int main (int argc, char *argv[])
{
  // Enable logging
  //LogComponentEnable("UdpSocketImpl", LOG_LEVEL_ALL);
  
  //LogComponentEnable("saharaRoutingProtocol", LOG_LEVEL_ALL);
  //LogComponentEnable("routingTable", LOG_LEVEL_ALL);

  //LogComponentEnable("OlsrRoutingProtocol", LOG_LEVEL_ALL);
  //LogComponentEnable("saharaSecurity", LOG_LEVEL_ALL);
  //LogComponentEnable("saharaBlock", LOG_LEVEL_ALL);

  //LogComponentEnable("saharaHeader", LOG_LEVEL_ALL);
  //LogComponentEnable("saharaQueue", LOG_LEVEL_ALL);

  
 
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Create nodes
  NodeContainer nodes;
  nodes.Create(70);
  
  //nodes.Get(1)->setm_nodeTestID(543);
  //uint32_t nodeID = nodes.Get(1)->getm_nodeTestID();
  //std::cout << "Packet ID node 1: " << nodeID << std::endl;

  //nodes.Get(1)->addTupleToRTable(17);

  //std::cout << "Routing table element: " << nodes.Get(1)->getTuple() << std::endl;



  // Create wifi
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ax);

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");
  


  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;
  wifiPhy.Set ("TxPowerStart", DoubleValue(0.5));
  wifiPhy.Set ("TxPowerEnd", DoubleValue(0.5));
  wifiPhy.Set ("TxPowerLevels", UintegerValue(1));
  

  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

  wifiPhy.SetChannel(wifiChannel.Create());
  
  NetDeviceContainer devices = wifi.Install(wifiPhy, mac, nodes);


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
  // Iterate over each NetDevice and enable promiscuous mode
for (uint32_t i = 0; i < devices.GetN(); ++i)
{
    ns3::Ptr<ns3::WifiNetDevice> wifiDevice = ns3::DynamicCast<ns3::WifiNetDevice>(devices.Get(i));
    if (wifiDevice)
    {
       // wifiDevice->GetMac()->SetPromisc();
        
        wifiDevice->SetPromiscReceiveCallback(MakeCallback(&YourPromiscuousCallback));
    }
}
*/


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
  
  /*
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0.0),
                                  "MinY", DoubleValue (0.0),
                                  "DeltaX", DoubleValue (33.0),
                                  "DeltaY", DoubleValue (33.0),
                                  "GridWidth", UintegerValue (6),
                                  "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");


  mobility.Install (nodes);
*/

    
  

 //Sahara routing
  SaharaHelper sahara;
  // Internet stack
  InternetStackHelper internet;

  OlsrHelper olsr;
  olsr.Set("HelloInterval", TimeValue(Seconds(11.0)));
  olsr.Set("TcInterval", TimeValue(Seconds(11.0)));
  olsr.Set("MidInterval", TimeValue(Seconds(11.0)));
  olsr.Set("HnaInterval", TimeValue(Seconds(11.0)));

  AodvHelper aodv;


  

  internet.SetRoutingHelper(sahara);
  internet.Install(nodes);
  

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
        for (double time = 10; time < 200; time += interval) {
            Simulator::Schedule(Seconds(time), &UpdateVelocity,m, radius, center, omega);
        }
        for (double time = 20; time < 300; time += interval) {
            Simulator::Schedule(Seconds(time), &UpdateVelocity,m, radius, center, omega);
        }
        
    }

    file.close();

  
  // suggestion from Pecorella
  // Ipv4ListRoutingHelper listRH;
  // SaharaHelper sahara;
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
  for(uint32_t i = 0; i < 30; i++){
    Ptr<Socket> recvSocket = Socket::CreateSocket (nodes.Get (i), UdpSocketFactory::GetTypeId ());
    recvSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 12345)); // Listen on port 9
    recvSocket->SetRecvCallback (MakeCallback (&ReceivePacket));
   
  }

    uint16_t port = 12345;
    Ptr<Socket> socket_sender = Socket::CreateSocket(nodes.Get(10), TypeId::LookupByName("ns3::UdpSocketFactory"));

   int i = 0;
    for(i = 0; i < 10000; i++){
      Simulator::Schedule(MilliSeconds(10000 + 100*i), &sendMessage, socket_sender, port);
    }

    // Print packet lost information
    //Simulator::Schedule(Seconds(28.0), &PrintPacketLost);

    /*

    for(int i = 0; i < 10; i++){
        Simulator::Schedule(MilliSeconds(44000 + 1000*i), &sendMessage, socket_sender, port);
    }
    */


   

    
    //}
 

  // Run simulation
  Simulator::Stop(Seconds(100.0));
  Simulator::Run();


}