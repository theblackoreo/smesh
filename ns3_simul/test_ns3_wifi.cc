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

void sendMessage(Ptr<Socket> socket, uint16_t port , std::string msg){
    //std::string msg = "Hello World!";

      Ptr<Packet> packet = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);

      // Send the packet in broadcast
      if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), port)) != -1) {
      } else {
          std::cout << "error";
      }
}

void sendUpdate(Ptr<Socket> socket, uint16_t port){
  // message structure [id, IP_origin, IP_sender, IP_1_hop, rep_origin, GPS, battery%]

  Address address;
  u_int16_t rep_origin, GPS, battery; 
  Ipv4Address IP_origin = socket->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
  Ipv4Address IP_sender = socket->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
  Ipv4Address IP_1_hop = socket->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
  GPS = 1234;
  battery = 50;
  rep_origin = 255;

  std::stringstream ss;
  ss << "1" << IP_origin << IP_sender << IP_1_hop << rep_origin << GPS << battery;
  std::string msg = ss.str();

  Ptr<Packet> packet = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);

  // Send the packet in broadcast
    if (socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), port)) != -1) {
      } else {
          std::cout << "error";
      }

}






std::string encrypt(std::string plaintext, int key) {
    std::string ciphertext = "";
    for (char& c : plaintext) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            ciphertext += static_cast<char>((c - base + key) % 26 + base);
        } else {
            ciphertext += c; // Keep non-alphabetic characters unchanged
        }
    }
    return ciphertext;
}

std::string decrypt(std::string ciphertext, int key) {
    return encrypt(ciphertext, -key); // Decryption is just encryption with negative key
}


int main (int argc, char *argv[])
{
  // Enable logging
  //LogComponentEnable("UdpSocketImpl", LOG_LEVEL_ALL);
 // LogComponentEnable("Ipv4Interface", LOG_LEVEL_ALL);


  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Create nodes
  NodeContainer nodes;
  nodes.Create(3);

  // Create wifi
  WifiHelper wifi;
  //wifi.SetStandard(ns3::WIFI_STANDARD_H);

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

  wifiPhy.SetChannel(wifiChannel.Create());
  
  NetDeviceContainer devices = wifi.Install(wifiPhy, mac, nodes);

  // Mobility of the nodes ( da rivedere )
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0.0),
                                  "MinY", DoubleValue (0.0),
                                  "DeltaX", DoubleValue (200.0),
                                  "DeltaY", DoubleValue (10.0),
                                  "GridWidth", UintegerValue (3),
                                  "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);


  //random mobility 
  /*mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                              "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                              "Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                              "Z", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
mobility.Install(nodes);*/

  // Internet stack
  InternetStackHelper internet;
  internet.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);


  // RECEIVERs socket
  for(uint32_t i = 0; i < 3; i++){
    Ptr<Socket> recvSocket = Socket::CreateSocket (nodes.Get (i), UdpSocketFactory::GetTypeId ());
    recvSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 9)); // Listen on port 9
    recvSocket->SetRecvCallback (MakeCallback (&ReceivePacket));
  }

 int key = 3;
    // message structure [id, IP_origin, IP_sender, IP_1_hop, rep_IP_origin, GPS, battery%]
    std::string msg_plain = "Hello, World!";

    // Encrypt
    std::string encrypted_text = encrypt(msg_plain, key);
    std::cout << "Encrypted: " << encrypted_text << std::endl;

    // Decrypt
    std::string decrypted_text = decrypt(encrypted_text, key);
    std::cout << "Decrypted: " << decrypted_text << std::endl;




 uint16_t port = 9;
 for(uint16_t i=0; i < 3; i++){
  Ptr<Socket> socket_sender = Socket::CreateSocket (nodes.Get (i), TypeId::LookupByName ("ns3::UdpSocketFactory"));
  //InetSocketAddress remote = InetSocketAddress(Ipv4Address("255.255.255.255"), 9);
  socket_sender->SetAllowBroadcast(true);
  Simulator::Schedule(Seconds(2.0 + (0.1 * i)), &sendUpdate, socket_sender, port);
 }
  // Run simulation
  Simulator::Stop(Seconds(10.0));
  Simulator::Run();


}