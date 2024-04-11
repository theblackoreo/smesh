import csv
import sys
import os
import time
import matplotlib

matplotlib.use('Agg')

import matplotlib.pyplot as plt

from ns import ns
import ns.sahara
import ns.applications
import ns.flow_monitor
import ns.core
import ns.internet
import ns.mobility
import ns.network
import ns.wifi

ns.core.LogComponentEnable("saharaRoutingProtocol", ns.core.LOG_LEVEL_INFO)

adhocNodes = ns.network.NodeContainer()
adhocNodes.Create(6)

wifi = ns.wifi.WifiHelper()
wifi.SetStandard(ns.wifi.WIFI_PHY_STANDARD_80211b)


wifiMac = ns.wifi.WifiMacHelper()
wifiMac.SetType ("ns3::AdhocWifiMac")


wifiPhy = ns.wifi.YansWifiPhyHelper.Default()
wifiChannel = ns.wifi.YansWifiChannelHelper()
wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel")
wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel")
wifiPhy.SetChannel(wifiChannel.Create())

wifiPhy.Set("TxPowerStart", ns.core.DoubleValue(0.01))
wifiPhy.Set("TxPowerEnd", ns.core.DoubleValue(0.01))

adhocDevices = wifi.Install(wifiPhy, wifiMac, adhocNodes)

mobilityAdhoc = ns.mobility.MobilityHelper()
streamIndex = 0  # used to get consistent mobility across scenarios

pos = ns.core.ObjectFactory()
pos.SetTypeId("ns3::RandomRectanglePositionAllocator")
pos.Set("X", ns.core.StringValue("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"))
pos.Set("Y", ns.core.StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"))


taPositionAlloc = pos.Create().GetObject(ns.mobility.PositionAllocator.GetTypeId())  
streamIndex += taPositionAlloc.AssignStreams(streamIndex)

ssSpeed = "ns3::UniformRandomVariable[Min=0.0|Max=%s]" % 2 # change
ssPause = "ns3::ConstantRandomVariable[Constant=%s]" % 2 # change

mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                                       "Speed", ns.core.StringValue(ssSpeed),
                                       "Pause", ns.core.StringValue(ssPause),
                                       "PositionAllocator", ns.core.PointerValue(taPositionAlloc))
mobilityAdhoc.SetPositionAllocator(taPositionAlloc)
mobilityAdhoc.Install(adhocNodes)
streamIndex += mobilityAdhoc.AssignStreams(adhocNodes, streamIndex)

aodv = ns.aodv.AodvHelper()
olsr = ns.olsr.OlsrHelper()
dsdv = ns.dsdv.DsdvHelper()
dsr = ns.dsr.DsrHelper()
        
list = ns.internet.Ipv4ListRoutingHelper()
internet = ns.internet.InternetStackHelper()


#add olsr to the routing table list
list.Add(olsr, 100)

internet.SetRoutingHelper(list)
internet.Install(adhocNodes)

print("assigning ip address") 

addressAdhoc = ns.internet.Ipv4AddressHelper()
addressAdhoc.SetBase(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0"))
adhocInterfaces = addressAdhoc.Assign(adhocDevices)

onoff1 = ns.applications.OnOffHelper("ns3::UdpSocketFactory", ns.network.Address())
onoff1.SetAttribute("OnTime", ns.core.StringValue("ns3::ConstantRandomVariable[Constant=1.0]"))
onoff1.SetAttribute("OffTime", ns.core.StringValue("ns3::ConstantRandomVariable[Constant=0.0]"))





