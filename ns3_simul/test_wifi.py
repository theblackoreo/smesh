try:
    from ns import ns
except ModuleNotFoundError:
    raise SystemExit(
        "Error: ns3 Python module not found;"
        " Python bindings may not be enabled"
        " or your PYTHONPATH might not be properly configured"
    )
import time
import sys

def print_packet_content(packet):
    buffer = ns.core.uint8_t_vector()
    packet.CopyData(buffer)
    print("Packet Content:", end=" ")
    for i in range(packet.GetSize()):
        print(chr(buffer[i]), end="")
    print()

def receive_packet(socket):
    packet = ns.core.Packet()
    from_address = ns.network.Address()
    receiver_ip = socket.GetNode().GetObject(ns.internet.Ipv4.GetTypeId()).GetAddress(1, 0).GetLocal()

    while True:
        packet = socket.RecvFrom(from_address)
        sender_ip = ns.network.InetSocketAddress.Cast(from_address).GetIpv4()
        print(receiver_ip, ": Received a packet from", sender_ip)
        print_packet_content(packet)

def send_message(socket, port, msg):
    packet = ns.core.Packet(ns.core.uint8_t(len(msg) + 1))
    for i in range(len(msg)):
        packet.GetPayload()[i] = ord(msg[i])
    socket.SendTo(packet, 0, ns.network.InetSocketAddress(ns.internet.Ipv4.GetBroadcast(), port))
    print_packet_content(packet)

def main():
    ns.core.CommandLine().Parse(sys.argv)

    # Create nodes
    nodes = ns.network.NodeContainer()
    nodes.Create(3)

    # Create wifi
    wifi = ns.wifi.WifiHelper()
    wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211n)

    mac = ns.wifi.WifiMacHelper()
    mac.SetType("ns3::AdhocWifiMac")

    
    wifi_phy = ns.wifi.YansWifiPhyHelper()
    wifi_channel = ns.wifi.YansWifiChannelHelper()
    wifi_channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel")
    wifi_channel.AddPropagationLoss("ns3::FriisPropagationLossModel")

    wifi_phy.SetChannel(wifi_channel.Create())

    devices = wifi.Install(wifi_phy, mac, nodes)

    '''
    # Mobility of the nodes
    mobility = ns.mobility.MobilityHelper()
    position_allocator = ns.mobility.GridPositionAllocator()
    position_allocator.SetAttribute("MinX", ns.core.DoubleValue(0.0))
    position_allocator.SetAttribute("MinY", ns.core.DoubleValue(0.0))
    position_allocator.SetAttribute("DeltaX", ns.core.DoubleValue(10.0))
    position_allocator.SetAttribute("DeltaY", ns.core.DoubleValue(10.0))
    position_allocator.SetAttribute("GridWidth", ns.core.UintegerValue(3))
    position_allocator.SetAttribute("LayoutType", ns.core.StringValue("RowFirst"))

    mobility.SetPositionAllocator(position_allocator)
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel")
    mobility.Install(nodes)
    '''

    
    # Internet stack
    internet = ns.internet.InternetStackHelper()
    internet.Install(nodes)

    ipv4 = ns.internet.Ipv4AddressHelper()
    ipv4.SetBase(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0"))

    interfaces = ipv4.Assign(devices)



    #sink = ns.network.Socket.CreateSocket(nodes.Get(0),ns.core.TypeId.LookupByName("ns3::UdpSocketFactory")) 
    #sink.Bind(ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9).ConvertTo())

    
    
    recv_socket = ns.network.Socket.CreateSocket(nodes.Get(0), ns.network.UdpSocketFactory.GetTypeId())
    recv_socket.Bind(ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9).ConvertTo())
    recv_socket.SetRecvCallback(ns.core.MakeCallback(receive_packet))

    '''
    recv_socket2 = ns.network.Socket.CreateSocket(nodes.Get(1), ns.network.UdpSocketFactory.GetTypeId())
    recv_socket2.Bind(ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9).ConvertTo())

    recv_socket2.SetRecvCallback(ns.core.MakeCallback(receive_packet))

    recv_socket3 = ns.network.Socket.CreateSocket(nodes.Get(2), ns.network.UdpSocketFactory.GetTypeId())
    recv_socket3.Bind(ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9).ConvertTo())
    recv_socket3.SetRecvCallback(ns.core.MakeCallback(receive_packet))


    

    
    # SENDER socket
    socket_sender = ns.network.Socket.CreateSocket(nodes.Get(0), ns.core.TypeId.LookupByName("ns3::UdpSocketFactory"))
    remote = ns.network.InetSocketAddress(ns.network.Ipv4Address("255.255.255.255"), 9)
    socket_sender.SetAllowBroadcast(True)

    socket_sender2 = ns.network.Socket.CreateSocket(nodes.Get(1), ns.core.TypeId.LookupByName("ns3::UdpSocketFactory"))
    socket_sender2.SetAllowBroadcast(True)

    socket_sender3 = ns.network.Socket.CreateSocket(nodes.Get(2), ns.core.TypeId.LookupByName("ns3::UdpSocketFactory"))
    socket_sender3.SetAllowBroadcast(True)

    port = 9
    # Schedule the sending of the packet after 2 seconds
    ns.core.Simulator.Schedule(ns.core.Seconds(2.0), send_message, socket_sender, port, "Hello World1!")
    ns.core.Simulator.Schedule(ns.core.Seconds(2.1), send_message, socket_sender2, port, "Hello World2!")
    ns.core.Simulator.Schedule(ns.core.Seconds(2.2), send_message, socket_sender3, port, "Hello World3!")
    '''
    
    # Run simulation

    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()

if __name__ == '__main__':
    main()
