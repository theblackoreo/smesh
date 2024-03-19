from ns import ns

def print_packet_content(packet):
    buffer = bytearray(packet.GetSize())
    packet.CopyData(buffer, packet.GetSize())
    print("Packet Content:", buffer)

def receive_packet(socket):
    packet = ns.network.Packet()
    addr = ns.network.Address()
    receiver_ip = socket.GetNode().GetObject(ns.internet.Ipv4.GetTypeId()).GetAddress(1, 0).GetLocal()

    while socket.RecvFrom(addr, packet):
        sender_ip = ns.network.InetSocketAddress(addr).GetIpv4()
        print(receiver_ip, ": Received a packet from", sender_ip)
        print_packet_content(packet)

def send_message(socket, port, msg):
    packet = ns.network.Packet(ns.core.uint8x_t(msg.encode()))
    if socket.SendTo(packet, 0, ns.network.InetSocketAddress(ns.network.Ipv4.GetBroadcast(), port)) != -1:
        print_packet_content(packet)
    else:
        print("error")

def main():
    cmd = ns.core.CommandLine()
    cmd.Parse(ns.core.ns3.AllArguments())

    nodes = ns.network.NodeContainer()
    nodes.Create(3)

    wifi = ns.wifi.WifiHelper()
    wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211n)

    mac = ns.wifi.WifiMacHelper()
    mac.SetType("ns3::AdhocWifiMac")

    wifiPhy = ns.wifi.YansWifiPhyHelper()
    wifiChannel = ns.wifi.YansWifiChannelHelper()
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel")
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel")

    wifiPhy.SetChannel(wifiChannel.Create())
  
    devices = wifi.Install(wifiPhy, mac, nodes)

    mobility = ns.mobility.MobilityHelper()
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", ns.core.DoubleValue(0.0),
                                  "MinY", ns.core.DoubleValue(0.0),
                                  "DeltaX", ns.core.DoubleValue(10.0),
                                  "DeltaY", ns.core.DoubleValue(10.0),
                                  "GridWidth", ns.core.UintegerValue(3),
                                  "LayoutType", ns.core.StringValue("RowFirst"))

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel")
    mobility.Install(nodes)

    internet = ns.internet.InternetStackHelper()
    internet.Install(nodes)

    ipv4 = ns.internet.Ipv4AddressHelper()
    ipv4.SetBase("10.1.1.0", "255.255.255.0")

    interfaces = ipv4.Assign(devices)

    socket_sender = ns.network.Socket.CreateSocket(nodes.Get(2), ns.core.TypeId.LookupByName("ns3::UdpSocketFactory"))
    remote = ns.network.InetSocketAddress(ns.network.Ipv4Address("255.255.255.255"), 80)
    socket_sender.SetAllowBroadcast(True)
    socket_sender.Connect(remote)

    recv_socket = ns.network.Socket.CreateSocket(nodes.Get(0), ns.network.UdpSocketFactory.GetTypeId())
    recv_socket.Bind(ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9))
    recv_socket.SetRecvCallback(ns.core.MakeCallback(receive_packet))

    recv_socket2 = ns.network.Socket.CreateSocket(nodes.Get(1), ns.network.UdpSocketFactory.GetTypeId())
    recv_socket2.Bind(ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9))
    recv_socket2.SetRecvCallback(ns.core.MakeCallback(receive_packet))

    port = 9
    def send_wrapper():
        send_message(socket_sender, port, "Hello World!")
    ns.core.Simulator.Schedule(ns.core.Seconds(2.0), send_wrapper)

    ns.core.Simulator.Stop(ns.core.Seconds(10.0))
    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()

if __name__ == "__main__":
    main()
