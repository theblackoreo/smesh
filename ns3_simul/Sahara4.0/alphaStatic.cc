#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/wifi-helper.h"
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
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
#include <ns3/ipv4-flow-classifier.h>
#include "ns3/gnuplot.h"


#include "ns3/wifi-mac-helper.h"

using namespace ns3;

class RoutingExperiment {

public:
    RoutingExperiment() {
        m_bytesTotal = 0;
        m_packetsReceived = 0;
        m_port = 99;
        m_CSVfileName = "manet-routing.output";
        m_nodes = 10;
        m_protocolName = "";
        m_txp = 8.9048;
        m_total_time = 200;
        m_node_speed = 20;
        m_debugger = true;
        m_nSinks = 5;
        m_protocol = 1;
        m_node_pause = 0;
    }

    void ReceivePacket(Ptr<Socket> socket) {
        Address senderAddress;
        Ptr<Packet> packet = socket->RecvFrom(senderAddress);
        while (packet) {
            m_bytesTotal = m_bytesTotal + packet->GetSize();
             m_packetsReceived++;
            PrintReceivedPacket(socket, packet, senderAddress);
            packet = socket->RecvFrom(senderAddress);
        }
    }


    Ptr<Socket> SetupPacketReceive(Ipv4Address addr, Ptr<Node> node) {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        Ptr<Socket> sink = Socket::CreateSocket(node, UdpSocketFactory::GetTypeId ());
        InetSocketAddress local = InetSocketAddress(addr, m_port);
        sink->Bind(local);
        sink->SetRecvCallback(MakeCallback(&RoutingExperiment::ReceivePacket, this));
        return sink;
    }   

 
     void PrintReceivedPacket(Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress) {
        std::ostringstream oss;
        oss << Simulator::Now().GetSeconds() << " " << socket->GetNode()->GetId();
        if (InetSocketAddress::IsMatchingType(senderAddress)) {
            auto addr = InetSocketAddress::ConvertFrom(senderAddress);
            Ipv4Address ipv4 = addr.GetIpv4();
            oss << " received one packet from " << ipv4;
            std::cout << oss.str() << std::endl;
        } else {
            oss << " received one packet!";
            std::cout << oss.str() << std::endl;
        }
    }

    void Run() {
        
        m_CSVfileName = m_CSVfileName + "." + std::to_string(Simulator::Now().GetSeconds());

        std::string rate = "2048bps";
        std::string phyMode = "DsssRate11Mbps";
        std::string tr_name = m_CSVfileName + "-compare";
        m_protocolName = "protocol";

        Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue("64"));
        Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(rate));
        Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

        NodeContainer adhocNodes;
        adhocNodes.Create(m_nodes);

        NS_LOG_UNCOND("Nodes created: " << m_nodes);

        WifiHelper wifi;
        wifi.SetStandard(WIFI_STANDARD_80211n);

        YansWifiPhyHelper wifiPhy;
        YansWifiChannelHelper wifiChannel;

        wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
        wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
        wifiPhy.SetChannel(wifiChannel.Create());

        WifiMacHelper wifiMac;
        wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));

        wifiPhy.Set("TxPowerStart", DoubleValue(m_txp));
        wifiPhy.Set("TxPowerEnd", DoubleValue(m_txp));

        wifiMac.SetType("ns3::AdhocWifiMac");
        NetDeviceContainer adhocDevices = wifi.Install(wifiPhy, wifiMac, adhocNodes);

        NS_LOG_UNCOND("wifi installed on devices");

        MobilityHelper mobilityAdhoc;
        int streamIndex = 0;

        Ptr<PositionAllocator> taPositionAlloc;
        ObjectFactory pos;
        pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
        pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
        pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
        taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
        streamIndex += taPositionAlloc->AssignStreams(streamIndex);

        std::string ssSpeed = "ns3::UniformRandomVariable[Min=0.0|Max=" + std::to_string(m_node_speed) + "]";
        std::string ssPause = "ns3::ConstantRandomVariable[Constant=" + std::to_string(m_node_pause) + "]";

        mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel", "Speed", StringValue(ssSpeed), "Pause", StringValue(ssPause), "PositionAllocator", PointerValue(taPositionAlloc));
        mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
        mobilityAdhoc.Install(adhocNodes);

        NS_LOG_UNCOND("Mobility installed on devices");
        streamIndex += mobilityAdhoc.AssignStreams(adhocNodes, streamIndex);

        AodvHelper aodv;
        OlsrHelper olsr;
        DsdvHelper dsdv;
        DsrHelper dsr;
        DsrMainHelper dsrMain;
        Ipv4ListRoutingHelper list;
        InternetStackHelper internet;
        SaharaHelper sahara;

        if (m_protocol == 1) {
            /*
            list.Add(olsr, 100);
            m_protocolName = "OLSR";
            */

            list.Add(sahara, 100);
            m_protocolName = "SAHARA";



        } else if (m_protocol == 2) {
            list.Add(aodv, 100);
            m_protocolName = "AODV";
        } else if (m_protocol == 3) {
            list.Add(dsdv, 100);
            m_protocolName = "DSDV";
        } else if (m_protocol == 4) {
            m_protocolName = "DSR";
        } else {
            NS_FATAL_ERROR("No such protocol:" << m_protocol);
        }

        if (m_protocol < 4) {
            internet.SetRoutingHelper(list);
            internet.Install(adhocNodes);
        } else if (m_protocol == 4) {
            internet.Install(adhocNodes);
            dsrMain.Install(dsr, adhocNodes);
        }

        Ipv4AddressHelper addressAdhoc;
        addressAdhoc.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer adhocInterfaces = addressAdhoc.Assign(adhocDevices);

        OnOffHelper onoff1("ns3::UdpSocketFactory", Address());
        onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

        for (uint32_t i = 0; i < m_nSinks; i++) {

            Ptr<Socket> sink  = SetupPacketReceive(adhocInterfaces.GetAddress(i), adhocNodes.Get(i));
            
            InetSocketAddress remoteSocketAddress(adhocInterfaces.GetAddress(i), m_port);
            Address remoteAddress = Address(remoteSocketAddress);
            AddressValue remoteAddressValue = AddressValue(remoteAddress);

            onoff1.SetAttribute("Remote", remoteAddressValue);

            Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
            ApplicationContainer temp = onoff1.Install(adhocNodes.Get(i + m_nSinks));
            temp.Start(Seconds(var->GetValue(100.0, 101.0)));
            temp.Stop(Seconds(m_total_time));
        }

        std::string nodes = std::to_string(m_nSinks);
        std::string sNodeSpeed = std::to_string(m_node_speed);
        std::string sNodePause = std::to_string(m_node_pause);
        std::string sRate = rate;

        tr_name = tr_name + "_" + m_protocolName + "_" + nodes + "sinks_" + sNodeSpeed + "speed_" + sNodePause + "pause_" + sRate + "rate";
        m_CSVfileName = tr_name;

        AsciiTraceHelper ascii;
        MobilityHelper::EnableAsciiAll(ascii.CreateFileStream(m_CSVfileName + ".mob"));

        FlowMonitorHelper flowmon_helper;
        Ptr<FlowMonitor> monitor = flowmon_helper.InstallAll();
        monitor = flowmon_helper.GetMonitor();
        monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
        monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
        monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

        WriteHeaderCsv();
        Simulator::Schedule(Seconds(1.0), &RoutingExperiment::CheckThroughput, this);

        Simulator::Stop(Seconds(m_total_time));
        Simulator::Run();

        monitor->CheckForLostPackets();
        Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon_helper.GetClassifier());
        
        if (m_debugger) {
        std::ofstream file(m_CSVfileName + ".txt", std::ios::app);
        for (const auto& pair : monitor->GetFlowStats()) {
            uint32_t flow_id = pair.first;

            if (flow_id!=233443) {
                std::string proto = (classifier->FindFlow(flow_id)).protocol == 6 ? "TCP" : "UDP";
                file << "FlowID: " << flow_id << " (" << proto << " " << (classifier->FindFlow(flow_id)).sourceAddress << "/" << (classifier->FindFlow(flow_id)).sourcePort << " --> " << (classifier->FindFlow(flow_id)).destinationAddress << "/" << (classifier->FindFlow(flow_id)).destinationPort << ")\n";
                PrintStats(file, pair.second);
            }
        }
        file.close();
    }

        monitor->SerializeToXmlFile((m_CSVfileName + ".flowmon").c_str(), true, true);

        std::vector<double> delays;
        for (const auto& pair : monitor->GetFlowStats()) {
            if ( (classifier->FindFlow(pair.first)).protocol == 17 && (classifier->FindFlow(pair.first)).sourcePort == 698) {
                continue;
            }

            if (pair.second.rxPackets == 0) {
                delays.push_back(0);
            } else {
                delays.push_back(pair.second.delaySum.GetSeconds() / pair.second.rxPackets);
            }
        }

            Gnuplot plot;
            plot.SetTitle("My Plot");
            plot.SetTerminal("png");
            plot.SetOutputFilename(m_CSVfileName + ".png");

            ns3::Gnuplot2dDataset dataset;
            dataset.SetTitle("Delay Histogram");
            for (double delay : delays) {
                dataset.Add(delay, 1.0); // Add each data point
            }
            //dataset.Add(delays);

            plot.AddDataset(dataset);
            std::ofstream plotFile("okok.plt");
            plot.GenerateOutput(plotFile);

        Simulator::Destroy();
    }

private:
    void WriteHeaderCsv() {
        std::ofstream csvfile((m_CSVfileName + ".csv").c_str(), std::ios::out);
        csvfile << "SimulationSecond;ReceiveRate;PacketsReceived;PacketDeliveryRatio;NumberOfSinks;RoutingProtocol;TransmissionPower" << std::endl;
        csvfile.close();
    }

    void CheckThroughput() {
        double kbs = (m_bytesTotal * 8.0) / 1000;
        m_bytesTotal = 0;
        int now = Simulator::Now().GetSeconds();

        double pdr = m_packetsReceived / (4 * m_nSinks);

        std::ofstream csvfile((m_CSVfileName + ".csv").c_str(), std::ios::app);
        csvfile << now << ";" << kbs << ";" << m_packetsReceived << ";" << pdr << ";" << m_nSinks << ";" << m_protocolName << ";" << m_txp << std::endl;
        csvfile.close();

        m_packetsReceived = 0;
        Simulator::Schedule(Seconds(1.0), &RoutingExperiment::CheckThroughput, this);
    }

    static void PrintStats(std::ostream& output, const FlowMonitor::FlowStats& stats) {
        output << "  Tx Bytes: " << stats.txBytes << std::endl;
        output << "  Rx Bytes: " << stats.rxBytes << std::endl;
        output << "  Tx Packets: " << stats.txPackets << std::endl;
        output << "  Rx Packets: " << stats.rxPackets << std::endl;
        output << "  Lost Packets: " << stats.lostPackets << std::endl;
        if (stats.rxPackets > 0) {
            output << "  Mean{Delay}: " << (stats.delaySum.GetSeconds() / stats.rxPackets) << std::endl;
            output << "  Mean{Jitter}: " << (stats.jitterSum.GetSeconds() / (stats.rxPackets - 1)) << std::endl;
            output << "  Mean{Hop Count}: " << (static_cast<double>(stats.timesForwarded) / stats.rxPackets + 1) << std::endl;
        }

        for (uint32_t reason = 0; reason < stats.packetsDropped.size(); ++reason) {
            output << "  Packets dropped by reason " << reason << ": " << stats.packetsDropped[reason] << std::endl;
        }
    }

    uint64_t m_bytesTotal;
    uint32_t m_packetsReceived;
    uint16_t m_port;
    std::string m_CSVfileName;
    uint32_t m_nodes;
    std::string m_protocolName;
    double m_txp;
    int m_total_time;
    int m_node_speed;
    bool m_debugger;
    uint32_t m_nSinks;
    int m_protocol;
    int m_node_pause;
};

int main(int argc, char* argv[]) {
    RoutingExperiment experiment;
    
    NS_LOG_UNCOND("Starting experiment...");
    experiment.Run();

   
}