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
#include <ns3/packet.h>


#include "ns3/config-store-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/inet-socket-address.h"

using namespace ns3;

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

    while ((packet = socket->RecvFrom (from))) {
        Ipv4Address senderIp = InetSocketAddress::ConvertFrom(from).GetIpv4();
        NS_LOG_UNCOND(receiverIp << ": Received a packet from " << senderIp);
        PrintPacketContent(packet);
        // Handle the received packet as needed
    }
}

void sendMessage(Ptr<Socket> socket, uint16_t port){
    std::string msg = "Hello World!";
         NS_LOG_UNCOND("SENT:" << msg);
      Ptr<Packet> packet = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);

      // Send the packet in broadcast
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address("10.1.1.3"), port)) != -1) {
      } else {
          std::cout << "error";
      }
}

void run (){

}

int main (int argc, char *argv[])
{

  // Enable logging
  //LogComponentEnable("UdpSocketImpl", LOG_LEVEL_ALL);
  
  //LogComponentEnable("saharaRoutingProtocol", LOG_LEVEL_ALL);
  //LogComponentEnable("routingTable", LOG_LEVEL_ALL);
  //LogComponentEnable("saharaHeader", LOG_LEVEL_ALL);
  //LogComponentEnable("saharaQueue", LOG_LEVEL_ALL);
   LogComponentEnable("Packet", ns3::LOG_LEVEL_INFO);


  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Create nodes
  NodeContainer adhocNodes;
  adhocNodes.Create(6);
  
  // Create wifi
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set ("TxPowerStart", DoubleValue(0.01));
  wifiPhy.Set ("TxPowerEnd", DoubleValue(0.01));

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

  wifiPhy.SetChannel(wifiChannel.Create());
  
  NetDeviceContainer adhocDevices = wifi.Install(wifiPhy, mac, adhocNodes);

  MobilityHelper mobilityAdhoc;
  int streamIndex = 0;
  ObjectFactory pos;
  pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
  pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
  pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
  streamIndex = streamIndex + taPositionAlloc.AssignStreams(streamIndex);

  mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                              "Speed",
                              StringValue(speedConstantRandomVariableStream.str()),
                              "Pause",
                              StringValue("ns3::ConstantRandomVariable[Constant=2.0]"),
                              "PositionAllocator",
                              PointerValue(taPositionAlloc));

  mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
  streamIndex = streamIndex + mobilityAdhoc.AssignStreams(adhocNodes, streamIndex)
  mobility.Install (adhocNodes);  

  //Sahara routing
  SaharaHelper sahara;
   

  // Internet stack
  InternetStackHelper internet;
  internet.SetRoutingHelper(sahara);
  internet.Install(adhocNodes);

  // assign IP addresses and mount a static routing table (bacuse we need to change it)
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(adhocDevices);

  Ptr<Application> onoff1;
  OnOffHelper onoffHelper("ns3::UdpSocketFactory", Address());
  // Set attributes for on and off times
  onoffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

  onoff1 = onoffHelper.Install(NodeContainer::GetGlobal());

  for (uint32_t i = 0; i < 10; ++i) {
    Ptr<Socket> sink = SetupPacketReceive(adhocInterfaces.GetAddress(i), adhocNodes.Get(i));

    ns3::AddressValue remoteAddress(ns3::InetSocketAddress(adhocInterfaces.GetAddress(i), port));
    onoff1.SetAttribute("Remote", remoteAddress);

    Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
    var->SetAttribute("Min", DoubleValue(100.0));
    var->SetAttribute("Max", DoubleValue(101.0));
    var->SetAttribute("Stream", IntegerValue(1));

    ApplicationContainer temp = onoff1.Install(adhocNodes.Get(i + m_nSinks));
    temp.Start(Seconds(var->GetValue()));
    temp.Stop(Seconds(30));
}





  // RECEIVERs socket
  for(uint32_t i = 0; i < 3; i++){
    Ptr<Socket> recvSocket = Socket::CreateSocket (nodes.Get (i), UdpSocketFactory::GetTypeId ());
    recvSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 12345)); // Listen on port 9
    recvSocket->SetRecvCallback (MakeCallback (&ReceivePacket));
   
  }

    uint16_t port = 12345;
    //for(uint16_t i=0; i < 3; i++){
    Ptr<Socket> socket_sender = Socket::CreateSocket (nodes.Get (0), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    //InetSocketAddress remote = InetSocketAddress(Ipv4Address("255.255.255.255"), 9);
    // socket_sender->SetAllowBroadcast(true);
    Simulator::Schedule(Seconds(1.0), &sendMessage, socket_sender, port);
    //}
 

  // Run simulation
  Simulator::Stop(Seconds(100.0));
  Simulator::Run();


}