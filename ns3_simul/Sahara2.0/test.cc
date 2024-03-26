#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "sahara.h"

using namespace ns3;

int main (int argc, char *argv[])
{
    // Enable logging
    LogComponentEnable ("Sahara", LOG_LEVEL_INFO);

    // Create nodes
    NodeContainer nodes;
    nodes.Create (2);

    // Create point-to-point link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    // Install internet stack on nodes
    InternetStackHelper internet;
    internet.Install (nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    NetDeviceContainer devices = pointToPoint.Install (nodes);
    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    /*
    // Set custom routing protocol
    Ptr<Sahara> routingProtocol = CreateObject<Sahara> ();
    Ptr<Ipv4> ipv4 = nodes.Get (1)->GetObject<Ipv4> ();
    ipv4->SetRoutingProtocol (routingProtocol);


    
    // Run simulation
    Simulator::Stop (Seconds (10.0));
    */
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
