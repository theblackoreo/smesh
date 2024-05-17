

#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/saharaHelper.h"
#include "saharaMobility.h"
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

#include <fstream>
#include <iostream>

using namespace ns3;
using namespace dsr;

NS_LOG_COMPONENT_DEFINE("manet-routing-compare");

/**
 * Routing experiment class.
 *
 * It handles the creation and run of an experiment.
 */
class RoutingExperiment
{
  public:
    RoutingExperiment();
    /**
     * Run the experiment.
     */
    void Run();

    /**
     * Handles the command-line parameters.
     * \param argc The argument count.
     * \param argv The argument vector.
     */
    void CommandSetup(int argc, char** argv);
    //void UpdateVelocity(Ptr<SaharaMobility> m, double radius, const Vector& center, double omega);
    void PrintInfo (Ptr<SaharaMobility> m);
    void setPos(Ptr<SaharaMobility> m00, Ptr<SaharaMobility> m11, Vector v0, Vector v1);

  private:
    /**
     * Setup the receiving socket in a Sink Node.
     * \param addr The address of the node.
     * \param node The node pointer.
     * \return the socket.
     */
    Ptr<Socket> SetupPacketReceive(Ipv4Address addr, Ptr<Node> node);
    /**
     * Receive a packet.
     * \param socket The receiving socket.
     */
    void ReceivePacket(Ptr<Socket> socket);
    /**
     * Compute the throughput.
     */
    void CheckThroughput();

    uint32_t port{99};            //!< Receiving port number.
    uint32_t bytesTotal{0};      //!< Total received bytes.
    uint32_t packetsReceived{0}; //!< Total received packets.

    std::string m_CSVfileName{"manet-routing.output.csv"}; //!< CSV filename.
    int m_nSinks{4};                                      //!< Number of sink nodes.
    std::string m_protocolName{"SAHARA"};                    //!< Protocol name.
    double m_txp{0.15};                                     //!< Tx power.
    bool m_traceMobility{false};                           //!< Enable mobility tracing.
    bool m_flowMonitor{false};                             //!< Enable FlowMonitor.
};

RoutingExperiment::RoutingExperiment()
{
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


void RoutingExperiment::PrintInfo (Ptr<SaharaMobility> m)
{
    Ptr<Node> n0 =  ns3::NodeList::GetNode(0);

    std::cout << "n0 Vel:" << m->GetVelocity() << std::endl;


}

void RoutingExperiment::setPos(Ptr<SaharaMobility> m00, Ptr<SaharaMobility> m11, Vector v0, Vector v1){
    m00->SetPosition (v0);
    m11->SetPosition (v1);
  
}



static inline std::string
PrintReceivedPacket(Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress)
{
    std::ostringstream oss;

    oss << Simulator::Now().GetSeconds() << " " << socket->GetNode()->GetId();

    if (InetSocketAddress::IsMatchingType(senderAddress))
    {
        InetSocketAddress addr = InetSocketAddress::ConvertFrom(senderAddress);
        oss << " received one packet from " << addr.GetIpv4();
    }
    else
    {
        oss << " received one packet!";
    }
    return oss.str();
}

void
RoutingExperiment::ReceivePacket(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address senderAddress;
    while ((packet = socket->RecvFrom(senderAddress)))
    {
        bytesTotal += packet->GetSize();
        packetsReceived += 1;
        
        NS_LOG_UNCOND(PrintReceivedPacket(socket, packet, senderAddress));
    }
}

void
RoutingExperiment::CheckThroughput()
{
    double kbs = (bytesTotal * 8.0) / 1000;
    bytesTotal = 0;

    std::ofstream out(m_CSVfileName, std::ios::app);

    out << (Simulator::Now()).GetSeconds() << "," << kbs << "," << packetsReceived << ","
        << m_nSinks << "," << m_protocolName << "," << m_txp << "" << std::endl;

    out.close();
    packetsReceived = 0;
    Simulator::Schedule(Seconds(1.0), &RoutingExperiment::CheckThroughput, this);
}

Ptr<Socket>
RoutingExperiment::SetupPacketReceive(Ipv4Address addr, Ptr<Node> node)
{
    Ptr<Socket> recvSocket = Socket::CreateSocket (node, UdpSocketFactory::GetTypeId ());
    recvSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), port)); // Listen on port 9
    recvSocket->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));

    return recvSocket;
}

void
RoutingExperiment::CommandSetup(int argc, char** argv)
{
    CommandLine cmd(__FILE__);
    cmd.AddValue("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
    cmd.AddValue("traceMobility", "Enable mobility tracing", m_traceMobility);
    cmd.AddValue("protocol", "Routing protocol (OLSR, AODV, DSDV, DSR)", m_protocolName);
    cmd.AddValue("flowMonitor", "enable FlowMonitor", m_flowMonitor);
    cmd.Parse(argc, argv);

    std::vector<std::string> allowedProtocols{"OLSR", "AODV", "DSDV", "DSR", "SAHARA"};

    if (std::find(std::begin(allowedProtocols), std::end(allowedProtocols), m_protocolName) ==
        std::end(allowedProtocols))
    {
        NS_FATAL_ERROR("No such protocol:" << m_protocolName);
    }
}

int
main(int argc, char* argv[])
{
    RoutingExperiment experiment;
   LogComponentEnable("saharaRoutingProtocol", LOG_LEVEL_ALL);
    // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);
    //LogComponentEnable("routingTable", LOG_LEVEL_ALL);
    LogComponentEnable("DsdvRoutingProtocol", LOG_LEVEL_ALL);
     LogComponentEnable("OlsrRoutingProtocol", LOG_LEVEL_ALL);
    experiment.CommandSetup(argc, argv);
    experiment.Run();

    return 0;
}

void
RoutingExperiment::Run()
{
    Packet::EnablePrinting();

    // blank out the last output file and write the column headers
    std::ofstream out(m_CSVfileName);
    out << "SimulationSecond,"
        << "ReceiveRate,"
        << "PacketsReceived,"
        << "NumberOfSinks,"
        << "RoutingProtocol,"
        << "TransmissionPower" << std::endl;
    out.close();

    int nWifis = 100;

    double TotalTime = 30.0;
    std::string rate("500000bps");
    std::string phyMode("DsssRate11Mbps");
    std::string tr_name("manet-routing-compare");
    int nodeSpeed = 0; // in m/s
    int nodePause = 0;  // in s

    Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue("1024"));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(rate));

    // Set Non-unicastMode rate to unicast mode
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));


    NodeContainer adhocNodes;
    adhocNodes.Create(nWifis);

    // setting up wifi phy and channel using helpers
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n);

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    // Add a mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue(phyMode),
                                 "ControlMode",
                                 StringValue(phyMode));

    wifiPhy.Set("TxPowerStart", DoubleValue(m_txp));
    wifiPhy.Set("TxPowerEnd", DoubleValue(m_txp));

    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer adhocDevices = wifi.Install(wifiPhy, wifiMac, adhocNodes);

/*
  MobilityHelper mobilityAdhoc;
  mobilityAdhoc.SetMobilityModel ("ns3::SaharaMobility");
  mobilityAdhoc.Install (adhocNodes); 

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
        Ptr<SaharaMobility> m = DynamicCast<SaharaMobility>(adhocNodes.Get(nodeID)->GetObject<MobilityModel> ());

        Ptr<ns3::sahara::SaharaRouting> rr = DynamicCast<ns3::sahara::SaharaRouting>(adhocNodes.Get(0)->GetObject<Ipv4RoutingProtocol> ());
        
        


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


     MobilityHelper mobilityAdhoc;
  mobilityAdhoc.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0.0),
                                  "MinY", DoubleValue (0.0),
                                  "DeltaX", DoubleValue (30.0),
                                  "DeltaY", DoubleValue (30.0),
                                  "GridWidth", UintegerValue (10),
                                  "LayoutType", StringValue ("RowFirst"));

  mobilityAdhoc.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    
  mobilityAdhoc.Install(adhocNodes);
  
    /*
    MobilityHelper mobilityAdhoc;
    int64_t streamIndex = 0; // used to get consistent mobility across scenarios

    ObjectFactory pos;
    pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
    pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
    pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

    Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
    streamIndex += taPositionAlloc->AssignStreams(streamIndex);

    std::stringstream ssSpeed;
    ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
    std::stringstream ssPause;
    ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
    mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                                   "Speed",
                                   StringValue(ssSpeed.str()),
                                   "Pause",
                                   StringValue(ssPause.str()),
                                   "PositionAllocator",
                                   PointerValue(taPositionAlloc));
    mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
    mobilityAdhoc.Install(adhocNodes);
    streamIndex += mobilityAdhoc.AssignStreams(adhocNodes, streamIndex);
    

   MobilityHelper mobility;
    ObjectFactory pos;
    pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
    pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));

    std::ostringstream speedConstantRandomVariableStream;
    speedConstantRandomVariableStream << "ns3::ConstantRandomVariable[Constant=" << 1
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

     mobility.Install(adhocNodes);
     */

    AodvHelper aodv;
    OlsrHelper olsr;
    DsdvHelper dsdv;
    DsrHelper dsr;
    SaharaHelper sahara;
    DsrMainHelper dsrMain;
    Ipv4ListRoutingHelper list;
    InternetStackHelper internet;

    if (m_protocolName == "OLSR")
    {   
        olsr.Set("HelloInterval", TimeValue(Seconds(7.0)));
        olsr.Set("TcInterval", TimeValue(Seconds(7.0)));

        list.Add(olsr, 100);
        internet.SetRoutingHelper(list);
        internet.Install(adhocNodes);
    }
    else if (m_protocolName == "AODV")
    {
       aodv.Set("HelloInterval", TimeValue(Seconds(7.0)));
        
        list.Add(aodv, 100);
        internet.SetRoutingHelper(list);
        internet.Install(adhocNodes);
    }
    else if (m_protocolName == "DSDV")
    {
        dsdv.Set("PeriodicUpdateInterval", TimeValue(Seconds(12.0)));
        dsdv.Set("SettlingTime", TimeValue(Seconds(9.0)));
        list.Add(dsdv, 100);
        internet.SetRoutingHelper(list);
        internet.Install(adhocNodes);
    }
     else if (m_protocolName == "SAHARA")
    {
        list.Add(sahara, 100);
        internet.SetRoutingHelper(list);
        internet.Install(adhocNodes);
    }
    else if (m_protocolName == "DSR")
    {
        internet.Install(adhocNodes);
        dsrMain.Install(dsr, adhocNodes);
        if (m_flowMonitor)
        {
            NS_FATAL_ERROR("Error: FlowMonitor does not work with DSR. Terminating.");
        }
    }
    else
    {
        NS_FATAL_ERROR("No such protocol:" << m_protocolName);
    }

    NS_LOG_INFO("assigning ip address");

    Ipv4AddressHelper addressAdhoc;
    addressAdhoc.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer adhocInterfaces;
    adhocInterfaces = addressAdhoc.Assign(adhocDevices);

    OnOffHelper onoff1("ns3::UdpSocketFactory", Address());
    onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

    for (int i = 0; i < m_nSinks; i++)
    {
        Ptr<Socket> sink = SetupPacketReceive(adhocInterfaces.GetAddress(i), adhocNodes.Get(i));

        AddressValue remoteAddress(InetSocketAddress(adhocInterfaces.GetAddress(i), port));
        onoff1.SetAttribute("Remote", remoteAddress);

        Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
        ApplicationContainer temp = onoff1.Install(adhocNodes.Get(i + m_nSinks));
        temp.Start(Seconds(var->GetValue(15.0, 30.0)));
        temp.Stop(Seconds(TotalTime));
    }

    std::stringstream ss;
    ss << nWifis;
    std::string nodes = ss.str();

    std::stringstream ss2;
    ss2 << nodeSpeed;
    std::string sNodeSpeed = ss2.str();

    std::stringstream ss3;
    ss3 << nodePause;
    std::string sNodePause = ss3.str();

    std::stringstream ss4;
    ss4 << rate;
    std::string sRate = ss4.str();

    // NS_LOG_INFO("Configure Tracing.");
    // tr_name = tr_name + "_" + m_protocolName +"_" + nodes + "nodes_" + sNodeSpeed + "speed_" +
    // sNodePause + "pause_" + sRate + "rate";

    // AsciiTraceHelper ascii;
    // Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream(tr_name + ".tr");
    // wifiPhy.EnableAsciiAll(osw);
    AsciiTraceHelper ascii;
    MobilityHelper::EnableAsciiAll(ascii.CreateFileStream(tr_name + ".mob"));

    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> flowmon;
    if (m_flowMonitor)
    {
        flowmon = flowmonHelper.InstallAll();
    }

    NS_LOG_INFO("Run Simulation.");

    CheckThroughput();

    Simulator::Stop(Seconds(TotalTime));
    Simulator::Run();

    if (m_flowMonitor)
    {
        flowmon->SerializeToXmlFile(tr_name + ".flowmon", false, false);
    }

    Simulator::Destroy();
}
