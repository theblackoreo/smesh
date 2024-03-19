#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UdpBroadcastExample");

void ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
  {
    uint8_t buffer[packet->GetSize ()];
    packet->CopyData(buffer, packet->GetSize ());
    std::string receivedMessage = std::string((char*)buffer, packet->GetSize ());
    NS_LOG_UNCOND("Received message: " << receivedMessage);
  }
}


int main (int argc, char *argv[])
{
          LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpBroadcastExample", LOG_LEVEL_INFO);
    LogComponentEnable("UdpBroadcastExample", LOG_LEVEL_ALL);

  CommandLine cmd;
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);

  NodeContainer nodes;
  nodes.Create (2);

  NetDeviceContainer devices;

 InternetStackHelper internet;
  internet.Install(nodes);


  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  uint16_t port = 9;  // Porta UDP arbitraria

  // Nodo sorgente
  Ptr<Socket> source = Socket::CreateSocket (nodes.Get(0), UdpSocketFactory::GetTypeId());
  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), port);
  source->Connect (remote);


   // Nodo destinazione
  Ptr<Socket> sink = Socket::CreateSocket (nodes.Get(1), UdpSocketFactory::GetTypeId());
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&ReceivePacket));

  // Invia il messaggio in broadcast
  Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t*>("ciao, come va?"), strlen("ciao, come va?"));
  source->Send (packet);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
