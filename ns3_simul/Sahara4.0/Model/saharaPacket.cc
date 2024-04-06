
    #include "saharaPacket.h"

    #include "ns3/address-utils.h"
    #include "ns3/packet.h"

    namespace ns3
    {
        
    NS_LOG_COMPONENT_DEFINE("saharaHeader");

    namespace sahara
    {

    NS_OBJECT_ENSURE_REGISTERED(SaharaHeader);

        SaharaHeader::SaharaHeader(Ipv4Address originIP, Ipv4Address hop1IP, 
                                uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O,
                                uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H)
            {
            m_originIP = originIP;
            m_hop1IP = hop1IP;
            m_reputation_O = reputation_O;
            m_reputation_H = reputation_H;
            m_GPS_O = GPS_O;
            m_GPS_H = GPS_H;
            m_battery_O = battery_O;
            m_battery_H = battery_H;

           //NS_LOG_DEBUG("header stored: "<< m_originIP << ", " << m_hop1IP << ", " << m_reputation_O);

        }

    SaharaHeader::~SaharaHeader()
    {
    }


    TypeId
    SaharaHeader::GetTypeId()
    {
        static TypeId tid = TypeId("ns3::sahara::SaharaHeader")
                                .SetParent<Header>()
                                .SetGroupName("Sahara")
                                .AddConstructor<SaharaHeader>();
        return tid;
    }

    TypeId
    SaharaHeader::GetInstanceTypeId() const
    {
        return GetTypeId();
    }

    uint32_t
    SaharaHeader::GetSerializedSize() const
    {
        return 20;
    }

    void
    SaharaHeader::Serialize(Buffer::Iterator i) const
    {   

        WriteTo(i, m_originIP);
        WriteTo(i, m_hop1IP);
        i.WriteU16(m_reputation_O);
        i.WriteU16(m_reputation_H);
        i.WriteU16(m_GPS_O);
        i.WriteU16(m_GPS_H);
        i.WriteU16(m_battery_O);
        i.WriteU16(m_battery_H);
        NS_LOG_DEBUG("messaged serialized: " << m_originIP << ", " << m_hop1IP << ", " << m_reputation_O <<
        ", " << m_reputation_H << ", " << m_GPS_O  << ", " << m_GPS_H << ", " << m_battery_O  << ", " << m_battery_H);
    }

    uint32_t
    SaharaHeader::Deserialize(Buffer::Iterator start)
    {
        Buffer::Iterator i = start;

        ReadFrom(i, m_originIP);
        ReadFrom(i, m_hop1IP);
        m_reputation_O = i.ReadU16();
        m_reputation_H = i.ReadU16();
        m_GPS_O = i.ReadU16();
        m_GPS_H = i.ReadU16();
        m_battery_O = i.ReadU16();
        m_battery_H = i.ReadU16();

        uint32_t dist = i.GetDistanceFrom(start);
        NS_ASSERT(dist == GetSerializedSize());
        NS_LOG_DEBUG("messaged DESERIALIZED: " << m_originIP << ", " << m_hop1IP << ", " << m_reputation_O <<
        ", " << m_reputation_H << ", " << m_GPS_O  << ", " << m_GPS_H << ", " << m_battery_O  << ", " << m_battery_H);
        return dist;
    }

    Ipv4Address
    SaharaHeader::GetOriginIP(){
        return m_originIP;
    }
    Ipv4Address
    SaharaHeader::GetHop1IP(){
        return m_hop1IP;
    }

    uint16_t
    SaharaHeader::GetReputation_O(){
        return m_reputation_O;
    }
    uint16_t
    SaharaHeader::GetGPS_O(){
        return m_GPS_O;
    }
    uint16_t
    SaharaHeader::GetBattery_O(){
        return m_battery_O;
    }

    uint16_t
    SaharaHeader::GetReputation_H(){
        return m_reputation_H;
    }
    uint16_t
    SaharaHeader::GetGPS_H(){
        return m_GPS_H;
    }
    uint16_t
    SaharaHeader::GetBattery_H(){
        return m_battery_H;
    }


    void SaharaHeader::SetOriginIP(Ipv4Address orIP)
        {
            m_originIP = orIP;
        }

        void SaharaHeader::SetHop1IP(Ipv4Address h1)
        {
            m_hop1IP = h1;
        }


        void SaharaHeader::SetReputation_O(uint16_t rep)
        {
            m_reputation_O = rep;
        }

        void SaharaHeader::SetGPS_O(uint16_t gps)
        {
            m_GPS_O = gps;
        }

        void SaharaHeader::SetBattery_O(uint16_t bat)
        {
            m_battery_O = bat;
        }

         void SaharaHeader::SetReputation_H(uint16_t rep)
        {
            m_reputation_H = rep;
        }

        void SaharaHeader::SetGPS_H(uint16_t gps)
        {
            m_GPS_H = gps;
        }

        void SaharaHeader::SetBattery_H(uint16_t bat)
        {
            m_battery_H = bat;
        }


    



    void
    SaharaHeader::Print(std::ostream& os) const
    {
    
    }
    } // namespace Sahara
    } // namespace ns3
