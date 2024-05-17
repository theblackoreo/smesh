#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "saharaMobility.h"
#include "ns3/internet-module.h"
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
#include "ns3/olsr-helper.h"


using namespace ns3;

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

int main (int argc, char* argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);

    NodeContainer nodes;
    nodes.Create (32);

    // Create wifi
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;
  wifiPhy.Set ("TxPowerStart", DoubleValue(3.01));
  wifiPhy.Set ("TxPowerEnd", DoubleValue(3.01));
  wifiPhy.Set ("TxPowerLevels", UintegerValue(1));

  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

  wifiPhy.SetChannel(wifiChannel.Create());
  
  NetDeviceContainer devices = wifi.Install(wifiPhy, mac, nodes);

    MobilityHelper mob;
    mob.SetMobilityModel ("ns3::SaharaMobility");
    mob.Install (nodes);



    //Sahara routing
    SaharaHelper sahara;
    OlsrHelper olsr;
   

  // Internet stack
  InternetStackHelper internet;
  internet.SetRoutingHelper(olsr);
  internet.Install(nodes);

  // assign IP addresses and mount a static routing table (bacuse we need to change it)
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);


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
    
    
    
    Simulator::Stop (Seconds (60));
    Simulator::Run();
    Simulator::Destroy ();
    

}