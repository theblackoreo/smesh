#include "saharaPacket.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("saharaHeader");

    namespace sahara
    {

    NS_OBJECT_ENSURE_REGISTERED(SaharaHeader);

    
    /*
    SaharaHeader::SaharaHeader(Ipv4Address originIP, Ipv4Address hop1IP, 
                                uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O,
                                uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H)
    {
        m_messageType = HELLO_MESSAGE;
        m_originIP = originIP;
        m_hop1IP = hop1IP;
        m_reputation_O = reputation_O;
        m_reputation_H = reputation_H;
        m_GPS_O = GPS_O;
        m_GPS_H = GPS_H;
        m_battery_O = battery_O;
        m_battery_H = battery_H;
        NS_LOG_DEBUG("HELLO SH creating...");
    }
    
    
    // set reconciliation ack
    SaharaHeader::SaharaHeader(Ipv4Address child, bool BF){
       
            m_messageType = SET_ACK;
            m_childIP = child;
            NS_LOG_DEBUG("SET_ACK SH creating...");
       
    }

    SaharaHeader::SaharaHeader(Ipv4Address parentIP, std::vector<bool> bloomFilter){
        m_messageType = SET_BF;
        m_parentIP = parentIP;
        m_bloomFilter = bloomFilter;
        NS_LOG_DEBUG("SET_BF SH creating...");
    }
    */

    SaharaHeader::SaharaHeader(){}
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
        NS_LOG_DEBUG("Get serialize size");
        switch (m_messageType)
        {
            case HELLO_MESSAGE:
                NS_LOG_DEBUG("Get serialize size HELLO");
                return 21; // 20 bytes for the tuple to be sent and 1 byte for the message type
                break;
            case SET_ACK:
                NS_LOG_DEBUG("Get serialize size SET_ACK");
                return  (1 + 4 + 2 + m_bloomFilter.size()); // 4 bytes fot the IP address and 1 byte for the message type
                break;
            case SET_BF:
                NS_LOG_DEBUG("Get serialize size SET_BF");
                return (1 + 4 + 2 + m_bloomFilter.size()); // 1 byte for the msgType, 4 for the child IP, 4 for the size of the BF and then BF size 
                break;
            case SEND_MISSING_C2P:
                NS_LOG_DEBUG("Get serialize size SEND_MISSING_C2P");
                return (1+4+ (m_missing_tuples.size()*20));
                break;
            case SEND_MISSING_P2C:
                NS_LOG_DEBUG("Get serialize size SEND_MISSING_P2C");
                return (1+4+ (m_missing_tuples.size()*20)); // 1 byte for msgType, 4 for the childIP, then size of missing tuples and then size of the bloom filter and then BF                 break;

        }
       
    }

    
    void
    SaharaHeader::Serialize(Buffer::Iterator i) const
    {   
        NS_LOG_DEBUG("Serialize called");
        i.WriteU8(m_messageType);
        size_t byteCount;
        std::vector<uint8_t> byteVector;

        switch (m_messageType)
        {
            case HELLO_MESSAGE:

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
                    break;

            case SET_ACK:
                    NS_LOG_DEBUG("SET_ACK serializing...");
                    WriteTo(i, m_childIP);
                    i.WriteU16(m_bloomFilter.size());
                    byteCount = (m_bloomFilter.size() + 7) / 8;
                    byteVector.resize(byteCount);

                    for (size_t i = 0; i < m_bloomFilter.size(); ++i) {
                        if (m_bloomFilter[i]) {
                            byteVector[i / 8] |= (1 << (i % 8)); // Set the corresponding bit in the byte
                        }
                    }
                    i.Write(&byteVector[0], byteVector.size());
                    break;
                
            case SET_BF:
                    NS_LOG_DEBUG("SET_BF serializing...");
                    WriteTo(i, m_parentIP);
                    i.WriteU16(m_bloomFilter.size());
                    byteCount = (m_bloomFilter.size() + 7) / 8;
                    byteVector.resize(byteCount);

                    for (size_t i = 0; i < m_bloomFilter.size(); ++i) {
                        if (m_bloomFilter[i]) {
                            byteVector[i / 8] |= (1 << (i % 8)); // Set the corresponding bit in the byte
                        }
                    }
                    i.Write(&byteVector[0], byteVector.size());
                    break;

            case SEND_MISSING_C2P:
                    
                    NS_LOG_DEBUG("SEND_MISSING_C2P serializing...");
                    WriteTo(i, m_childIP);
                    
                    for(const auto & t: m_missing_tuples){
                        WriteTo(i, std::get<0>(t));
                        WriteTo(i, std::get<1>(t));
                        i.WriteU16(std::get<2>(t));
                        i.WriteU16(std::get<3>(t));
                        i.WriteU16(std::get<4>(t));
                        i.WriteU16(std::get<5>(t));
                        i.WriteU16(std::get<6>(t));
                        i.WriteU16(std::get<7>(t));
                    }                
                    break;

            case SEND_MISSING_P2C:
                    NS_LOG_DEBUG("SEND_MISSING_P2C serializing...");
                    WriteTo(i, m_parentIP);
                    
                    for(const auto & t: m_missing_tuples){
                        WriteTo(i, std::get<0>(t));
                        WriteTo(i, std::get<1>(t));
                        i.WriteU16(std::get<2>(t));
                        i.WriteU16(std::get<3>(t));
                        i.WriteU16(std::get<4>(t));
                        i.WriteU16(std::get<5>(t));
                        i.WriteU16(std::get<6>(t));
                        i.WriteU16(std::get<7>(t));
                    }                
                    break;
        }
    }
    

    uint32_t
    SaharaHeader::Deserialize(Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        m_messageType = (MessageType)i.ReadU8();
        uint32_t dist;
        u_int16_t count;
        
        switch (m_messageType)
        {
        case HELLO_MESSAGE:
            NS_LOG_DEBUG("Deserialize HELLO");
            ReadFrom(i, m_originIP);
            ReadFrom(i, m_hop1IP);
            m_reputation_O = i.ReadU16();
            m_reputation_H = i.ReadU16();
            m_GPS_O = i.ReadU16();
            m_GPS_H = i.ReadU16();
            m_battery_O = i.ReadU16();
            m_battery_H = i.ReadU16();

            dist = i.GetDistanceFrom(start);
            NS_ASSERT(dist == GetSerializedSize());
            NS_LOG_DEBUG("messaged DESERIALIZED: " << m_originIP << ", " << m_hop1IP << ", " << m_reputation_O <<
            ", " << m_reputation_H << ", " << m_GPS_O  << ", " << m_GPS_H << ", " << m_battery_O  << ", " << m_battery_H);
            return dist;
            break;

        case SET_ACK:
            m_bloomFilter.clear();
            NS_LOG_DEBUG("Deserialize ACK");
            ReadFrom(i, m_childIP);
            sizeBF = i.ReadU16();
            
            count = 0;
            while (i.GetRemainingSize() > 0)
            {
                uint8_t byte = i.ReadU8();
                for (int j = 0; j < 8; ++j)
                {      
                    if(count >= sizeBF) {continue;}
                    bool bit = byte & (1 << j); // Extract the j-th bit from the byte
                    m_bloomFilter.push_back(bit); // Add the bit to the vector
                    count++;
                    
                }
            }
            dist = i.GetDistanceFrom(start);
            //NS_ASSERT(dist == GetSerializedSize());
            return dist;
            break;
        
        case SET_BF:
            m_bloomFilter.clear();
            ReadFrom(i, m_parentIP);
            
            sizeBF = i.ReadU16();
            
            count = 0;
            while (i.GetRemainingSize() > 0)
            {
                uint8_t byte = i.ReadU8();
                for (int j = 0; j < 8; ++j)
                {      
                    if(count >= sizeBF) {continue;}
                    bool bit = byte & (1 << j); // Extract the j-th bit from the byte
                    m_bloomFilter.push_back(bit); // Add the bit to the vector
                    count++;
                    
                }
            }
            dist = i.GetDistanceFrom(start);
            return dist;
            break;

        case SEND_MISSING_C2P:
            ReadFrom(i, m_childIP);
            while(i.GetRemainingSize() > 0){
                ReadFrom(i, m_originIP);
                ReadFrom(i, m_hop1IP);
                m_reputation_O = i.ReadU16();
                m_reputation_H = i.ReadU16();
                m_GPS_O = i.ReadU16();
                m_GPS_H = i.ReadU16();
                m_battery_O = i.ReadU16();
                m_battery_H = i.ReadU16();
                m_missing_tuples.push_back(std::make_tuple(m_originIP, m_hop1IP, m_reputation_O, m_reputation_H, m_GPS_O, m_GPS_H, m_battery_O, m_battery_H));
            }

            dist = i.GetDistanceFrom(start);
            NS_ASSERT(dist == GetSerializedSize());
            return dist;
            break;
        
        case SEND_MISSING_P2C:
            ReadFrom(i, m_parentIP);
            while(i.GetRemainingSize() > 0){
                ReadFrom(i, m_originIP);
                ReadFrom(i, m_hop1IP);
                m_reputation_O = i.ReadU16();
                m_reputation_H = i.ReadU16();
                m_GPS_O = i.ReadU16();
                m_GPS_H = i.ReadU16();
                m_battery_O = i.ReadU16();
                m_battery_H = i.ReadU16();
                m_missing_tuples.push_back(std::make_tuple(m_originIP, m_hop1IP, m_reputation_O, m_reputation_H, m_GPS_O, m_GPS_H, m_battery_O, m_battery_H));
            }

            dist = i.GetDistanceFrom(start);
            NS_ASSERT(dist == GetSerializedSize());
            return dist;
            break;
        }


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

    Ipv4Address
    SaharaHeader::GetChildIP(){
        return m_childIP;
    }

    std::vector<bool>
    SaharaHeader::GetBloomFilter(){
        return m_bloomFilter;
    }
    Ipv4Address
    SaharaHeader::GetParentIP(){
        return m_parentIP;
    }
    
    uint16_t
    SaharaHeader::GetSizeBF(){
        return sizeBF;
    }

    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>>
    SaharaHeader::GetMissingTuples(){
        return m_missing_tuples;
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
         void SaharaHeader::SetMessageType(MessageType messageType)
        {
            m_messageType = messageType;
        } 
        void SaharaHeader::SetBF(std::vector<bool> BF){
            m_bloomFilter = BF;
        }
        void SaharaHeader::SetParentIP(Ipv4Address parentIP){
            m_parentIP = parentIP;
        }

        void SaharaHeader::SetChildIP(Ipv4Address childIP){
            m_childIP = childIP;
        }
        
        void SaharaHeader::SetMissingTuples(std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> missingTuples){
            m_missing_tuples = missingTuples;
        }
        
     
    void
    SaharaHeader::Print(std::ostream& os) const
    {

    }
    } // namespace Sahara
} // namespace ns3
