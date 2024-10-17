#include "saharaPacket.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include <iomanip>


namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("saharaHeader");

    namespace sahara
    {

    NS_OBJECT_ENSURE_REGISTERED(SaharaHeader);

    

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

            case SEND_MISSING_C2P:
                NS_LOG_DEBUG("Get serialize size SEND_MISSING_C2P");
                return (1+4+2+ (m_missing_tuples.size()*(20)) + 2 + GetSizeVotesList2() );
                break;

           case SEND_MISSING_P2C:
                NS_LOG_DEBUG("Get serialize size SEND_MISSING_P2C");
                return (1 + 4 + 2 + (m_missing_tuples.size() * (20)) + 2 +  GetSizeVotesList2()); // 1 byte msgType, 4 bytes parentIP, 2 bytes count
                break;


            case SR_HELLO:
                NS_LOG_DEBUG("Get serialize size SR_HELLO");
                return 15 + GetSizeVotesList(); // 1byte msg type, 4 byte origin IP, 4 byte parent IP, 2 byte rep, 2 byte gps, 2 byte bat, vote list
                break;

            case ROOT_SR_HELLO:
                return 11 + GetSizeVotesList(); // 1 byte msg type, 4 byte IP, 2 byte rep, 2 byte gps, 2 byte batO, vote list size
                break;

            case SEND_MISSING_P2C_ACK:
                return 5; // 1 byte msg type, 4 byte IP
                break;

            case ASK_BF:
                return 1+4+4+4+(m_bloomFilter.size()+7)/8 ; // 1 is msg type, ip of the sender, then size of BF and then BF
                break;

            case SEND_BF:
                return 1+4+2+4*(m_votesIPList.size())+m_bloomFilter.size()/8; // 1 msg type, 4 is the bf lenght, and then bf
                break; 

            case ENCRYPTED:
                if(m_encryptedSize == 0) NS_ASSERT("encrypted packet size was not set");
                return m_encryptedSize;
                break;
            
            case ASK_BF_P2C:
                return 1+4+4 ; // 1 is msg type, ip of the sender, size BF parent
                break;
            case SEND_BF_C2P:
                return 1+4+4+(m_bloomFilter.size()+7)/8 + 2+4*(m_votesIPList.size()); // 1 is msg type, ip of the sender, then size of BF and then BF
                break;



        }
       
    }

    void
    SaharaHeader::SetEncryptedPacketSize(u_int32_t encrSize){
        m_encryptedSize = encrSize;
    }

    void 
    SaharaHeader::WriteStringToBuffer(ns3::Buffer::Iterator &i, const std::string &str) const {
    // Write the length of the string first
    uint16_t length = static_cast<uint16_t>(str.size());
    i.WriteU16(length);  // Write the length of the string as a 16-bit unsigned integer

    // Write each character of the string
    for (size_t j = 0; j < str.size(); ++j) {
        i.WriteU8(static_cast<uint8_t>(str[j]));  // Write each character as an 8-bit unsigned integer
    }
}

std::string 
SaharaHeader::ReadStringFromBuffer(ns3::Buffer::Iterator &i) {
    // Read the length of the string first
    uint16_t length = i.ReadU16();  // Read the length of the string from the buffer

    // Create a string and fill it with characters from the buffer
    std::string str;
    str.reserve(length);  // Reserve memory for the string
    for (uint16_t j = 0; j < length; ++j) {
        char c = static_cast<char>(i.ReadU8());  // Read each character as an 8-bit unsigned integer
        str.push_back(c);  // Append the character to the string
    }
    return str;
}



      void SaharaHeader::WriteVotesIntoBuffer(ns3::Buffer::Iterator &i) const {

        
        // Write evaluator IP
        WriteTo(i, m_votes.evaluatorIP);

        // Write list size
        uint32_t listSize = m_votes.votes.size();
        i.WriteHtonU32(listSize);

        // Write votes and signature
        for (const auto& vote : m_votes.votes) {
            WriteTo(i, vote.EvaluatedIP);
            i.WriteU8(static_cast<uint8_t>(vote.vote));
            NS_LOG_DEBUG("Vote of: " << vote.EvaluatedIP << " -> " << static_cast<int>(vote.vote));
        }

       uint32_t sigSize = m_votes.signature.size();
        NS_LOG_DEBUG("Signature size during serialization: " << sigSize);
        i.WriteHtonU32(sigSize);
        i.Write(reinterpret_cast<const uint8_t*>(m_votes.signature.c_str()), sigSize);



    }

      void SaharaHeader::WriteVotesIntoBuffer2(ns3::Buffer::Iterator &i) const {
    // Iteriamo su tutti i pacchetti di voto
    for (const auto& t : m_missing_VotePackets) {
        // Serializziamo l'IP del valutatore
        WriteTo(i, t.evaluatorIP);

        // Scriviamo la dimensione della lista di voti
        uint32_t listSize = t.votes.size();
        i.WriteHtonU32(listSize);

        // Serializziamo ogni voto nella lista
        for (const auto& vote : t.votes) {
            WriteTo(i, vote.EvaluatedIP);  // IP del nodo valutato
            i.WriteU8(static_cast<uint8_t>(vote.vote));  // Voto (come un byte)
            NS_LOG_DEBUG("Vote of: " << vote.EvaluatedIP << " -> " << static_cast<int>(vote.vote));
        }

        // Serializziamo la firma
        uint32_t sigSize = t.signature.size();
        NS_LOG_DEBUG("Signature size during serialization: " << sigSize);
        i.WriteHtonU32(sigSize);  // Scriviamo la dimensione della firma
        i.Write(reinterpret_cast<const uint8_t*>(t.signature.c_str()), sigSize);  // Scriviamo i byte della firma
    }
}

 void SaharaHeader::ReadVotesFromBuffer(ns3::Buffer::Iterator &i) {

   
    // Read evaluator IP
    ReadFrom(i, m_votes.evaluatorIP);
     NS_LOG_DEBUG("IP read " << m_votes.evaluatorIP);

    // Read list size
    uint32_t listSize = i.ReadNtohU32();

    // Read votes
    for (uint32_t l = 0; l < listSize; ++l) {
        Vote vote;
        ReadFrom(i, vote.EvaluatedIP);
        vote.vote = static_cast<VoteState>(i.ReadU8());
        m_votes.votes.push_back(vote);
        NS_LOG_DEBUG("Vote of: " << vote.EvaluatedIP << " -> " << static_cast<int>(vote.vote));
    }

    // Read signature
  uint32_t sigSize = i.ReadNtohU32();

    NS_LOG_DEBUG("Signature size during deserialization: " << sigSize);
    m_votes.signature.resize(sigSize);
    i.Read(reinterpret_cast<uint8_t*>(&m_votes.signature[0]), sigSize);

}

void SaharaHeader::ReadVotesFromBuffer2(ns3::Buffer::Iterator &i, uint16_t sizeMissingVoteList) {
    m_missing_VotePackets.clear();

    while (sizeMissingVoteList > 0) {
        VotePacket t;

        // Deserializziamo l'IP del valutatore
        ReadFrom(i, t.evaluatorIP);
        // Leggiamo la dimensione della lista di voti
        uint32_t listSize = i.ReadNtohU32();
        t.votes.resize(listSize);
        // Deserializziamo ogni voto
        for (auto& vote : t.votes) {
            ReadFrom(i, vote.EvaluatedIP);
            vote.vote = static_cast<ns3::sahara::SaharaHeader::VoteState>(i.ReadU8());
        }
        // Deserializziamo la firma
        uint32_t sigSize = i.ReadNtohU32();

        // Check if there's enough data remaining in the buffer to read the signature
        if (i.GetRemainingSize() < sigSize) {
            return; // Handle the error appropriately
        }
        // Use a std::vector to manage the signature buffer safely
        std::vector<char> sigBuffer(sigSize);
        i.Read(reinterpret_cast<uint8_t*>(sigBuffer.data()), sigSize);
        t.signature = std::string(sigBuffer.begin(), sigBuffer.end()); // Reconstruct the signature as std::string
        m_missing_VotePackets.push_back(t);

        // Decrement the sizeMissingVoteList to ensure correct termination
        sizeMissingVoteList -= 1;
    }
}

   uint32_t SaharaHeader::GetSizeVotesList() const {
        
        uint32_t totalSize = sizeof(uint32_t); 
        
        totalSize += sizeof(uint32_t); 

        for (const auto& vote : m_votes.votes) {
            totalSize += sizeof(uint32_t);  // EvaluatedIP size
            totalSize += sizeof(uint8_t);      // Vote state size
        }

        totalSize += sizeof(uint32_t); // Signature size field
        totalSize += m_votes.signature.size(); // Actual signature size
        return totalSize;
    }

    uint32_t SaharaHeader::GetSizeVotesList2() const {
    uint32_t totalSize = sizeof(uint32_t);  // Iniziamo con la dimensione del campo "size" della lista dei pacchetti di voto

    // Iteriamo su tutti i pacchetti di voto in m_missing_VotePackets
    for (const auto& packet : m_missing_VotePackets) {
        totalSize += sizeof(uint32_t);  // EvaluatorIP (IP del valutatore)

        totalSize += sizeof(uint32_t);  // Field per la dimensione della lista dei voti

        // Iteriamo su ogni voto all'interno del pacchetto
        for (const auto& vote : packet.votes) {
            totalSize += sizeof(uint32_t);  // EvaluatedIP (IP del nodo valutato)
            totalSize += sizeof(uint8_t);   // Dimensione del campo voto (vote state)
        }

        totalSize += sizeof(uint32_t);  // Field per la dimensione della firma (signature size)
        totalSize += packet.signature.size();  // Dimensione effettiva della firma
    }

    return totalSize;
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


            case SEND_MISSING_C2P:
                    
                    NS_LOG_DEBUG("SEND_MISSING_C2P serializing...");
                    WriteTo(i, m_childIP);
                    i.WriteU16(m_missing_tuples.size()); // Write the count of tuples
                    
                    for(const auto & t: m_missing_tuples){
                        WriteTo(i, std::get<0>(t));
                        WriteTo(i, std::get<1>(t));
                        i.WriteU16(std::get<2>(t));
                        i.WriteU16(std::get<3>(t));
                        i.WriteU16(std::get<4>(t));
                        i.WriteU16(std::get<5>(t));
                        i.WriteU16(std::get<6>(t));
                        i.WriteU16(std::get<7>(t));
                        /*
                        i.WriteU8(static_cast<uint8_t>(std::get<8>(t)));
                        i.WriteU8(static_cast<uint8_t>(std::get<9>(t)));
                        WriteStringToBuffer(i, std::get<10>(t));
                        */

                    }
                    
                    i.WriteU16(m_missing_VotePackets.size());
                    WriteVotesIntoBuffer2(i);


                    break;

            case SEND_MISSING_P2C:
                        NS_LOG_DEBUG("SEND_MISSING_P2C serializing...");
                        WriteTo(i, m_parentIP);
                        i.WriteU16(m_missing_tuples.size()); // Write the count of tuples
                        NS_LOG_DEBUG("Serializing " << m_missing_tuples.size() << " missing tuples");
                        for (const auto& t : m_missing_tuples) {
                            // Serialize each field
                            WriteTo(i, std::get<0>(t));
                            WriteTo(i, std::get<1>(t));
                            i.WriteU16(std::get<2>(t));
                            i.WriteU16(std::get<3>(t));
                            i.WriteU16(std::get<4>(t));
                            i.WriteU16(std::get<5>(t));
                            i.WriteU16(std::get<6>(t));
                            i.WriteU16(std::get<7>(t));
                            /*
                            i.WriteU8(static_cast<uint8_t>(std::get<8>(t)));
                            i.WriteU8(static_cast<uint8_t>(std::get<9>(t)));
                            WriteStringToBuffer(i, std::get<10>(t));
                            */
                        }

                         i.WriteU16(m_missing_VotePackets.size());
                         WriteVotesIntoBuffer2(i);


                        break;

           case SR_HELLO:
                WriteTo(i, m_originIP);
                WriteTo(i, m_parentIP);
                i.WriteU16(m_reputation_O);
                i.WriteU16(m_GPS_O);
                i.WriteU16(m_battery_O);
                WriteVotesIntoBuffer(i);
                
                break;
            case ROOT_SR_HELLO:
                WriteTo(i, m_originIP);
                i.WriteU16(m_reputation_O);
                i.WriteU16(m_GPS_O);
                i.WriteU16(m_battery_O);
                WriteVotesIntoBuffer(i);
                
                break;

            case SEND_MISSING_P2C_ACK:
                WriteTo(i, m_originIP);
                break;
            
           case ASK_BF:
                NS_LOG_DEBUG("[SERIALIZING ASK BF]");
                WriteTo(i, m_originIP);
                
                //WriteVotesIntoBuffer(i);


                i.WriteU16(m_bloomFilter.size());
                NS_LOG_DEBUG("Size of Bloom Filter (bits): " << m_bloomFilter.size());

                i.WriteU16(m_rtDim);
                byteCount = (m_bloomFilter.size() + 7) / 8;
                byteVector.resize(byteCount, 0); // Inizializza a zero

                for (size_t i = 0; i < m_bloomFilter.size(); ++i) {
                    if (m_bloomFilter[i]) {
                        byteVector[i / 8] |= (1 << (i % 8));
                    }
                }

                i.Write(&byteVector[0], byteVector.size());
                NS_LOG_DEBUG("[byteVectorSize]: " << byteVector.size() << " ByteCount: " << byteCount);
                NS_LOG_DEBUG("[END SERIALIZING ASK BF]");
                break;


            case SEND_BF:
                i.WriteU16(m_bloomFilter.size());

                // serialize list of evalutor votes ip    

                i.WriteU16(m_votesIPList.size()); // 2 bytes

                // Scrivi ogni Ipv4Address
                for (const auto& addr : m_votesIPList) {
                    WriteTo(i, addr);
                }
                
                byteCount = (m_bloomFilter.size() + 7) / 8;
                byteVector.resize(byteCount);
                    for (size_t i = 0; i < m_bloomFilter.size(); ++i) {
                        if (m_bloomFilter[i]) {
                            byteVector[i / 8] |= (1 << (i % 8)); // Set the corresponding bit in the byte
                        }
                    }
                    i.Write(&byteVector[0], byteVector.size());
                break;
            
            case ASK_BF_P2C:
                WriteTo(i, m_originIP);
                i.WriteU16(m_rtDim);
                break;
            
            case SEND_BF_C2P:
                NS_LOG_DEBUG("[SERIALIZING SEND BF C2P]");
                WriteTo(i, m_originIP);
                i.WriteU16(m_bloomFilter.size());
                NS_LOG_DEBUG("Size of Bloom Filter (bits): " << m_bloomFilter.size());

                byteCount = (m_bloomFilter.size() + 7) / 8;
                byteVector.resize(byteCount, 0); // Inizializza a zero

                for (size_t i = 0; i < m_bloomFilter.size(); ++i) {
                    if (m_bloomFilter[i]) {
                        byteVector[i / 8] |= (1 << (i % 8));
                    }
                }

                i.Write(&byteVector[0], byteVector.size());

                i.WriteU16(m_votesIPList.size()); // 2 bytes

                // Scrivi ogni Ipv4Address
                for (const auto& addr : m_votesIPList) {
                    WriteTo(i, addr);
                }

                NS_LOG_DEBUG("[byteVectorSize]: " << byteVector.size() << " ByteCount: " << byteCount);
                NS_LOG_DEBUG("[END SERIALIZING ASK BF]");
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
        uint16_t sizeList;
        uint16_t sizeMissingVoteList;
        uint16_t sizeMissingTuples;
        
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

       
       

        case SEND_MISSING_C2P:
            ReadFrom(i, m_childIP);
            sizeMissingTuples = i.ReadU16();

            while(sizeMissingTuples > 0){ // Ensure there are at least 20 bytes left
                ReadFrom(i, m_originIP);
                ReadFrom(i, m_hop1IP);
                m_reputation_O = i.ReadU16();
                m_reputation_H = i.ReadU16();
                m_GPS_O = i.ReadU16();
                m_GPS_H = i.ReadU16();
                m_battery_O = i.ReadU16();
                m_battery_H = i.ReadU16();
                /*
                m_vote_O = static_cast<VoteState>(i.ReadU8());
                m_vote_H = static_cast<VoteState>(i.ReadU8());
                m_sign = ReadStringFromBuffer(i);
                */

                m_missing_tuples.push_back(std::make_tuple(m_originIP, m_hop1IP, m_reputation_O, m_reputation_H, m_GPS_O, m_GPS_H, m_battery_O, m_battery_H));
                sizeMissingTuples -= 1;
            }
             
            sizeMissingVoteList = i.ReadU16();
            
            ReadVotesFromBuffer2(i, sizeMissingVoteList);


            
            return dist;
            break;
        
       case SEND_MISSING_P2C:
       {
            ReadFrom(i, m_parentIP);
            uint16_t tupleCount = i.ReadU16(); // Read the count of tuples
            NS_LOG_DEBUG("Deserializing " << tupleCount << " missing tuples");
            for (uint16_t idx = 0; idx < tupleCount; ++idx) {
                if (i.GetRemainingSize() < 20) {
                    NS_LOG_ERROR("Not enough data to read a tuple. Remaining size: " << i.GetRemainingSize());
                    break;
                }
                ReadFrom(i, m_originIP);
                ReadFrom(i, m_hop1IP);
                m_reputation_O = i.ReadU16();
                m_reputation_H = i.ReadU16();
                m_GPS_O = i.ReadU16();
                m_GPS_H = i.ReadU16();
                m_battery_O = i.ReadU16();
                m_battery_H = i.ReadU16();
                /*
                m_vote_O = static_cast<VoteState>(i.ReadU8());
                m_vote_H = static_cast<VoteState>(i.ReadU8());
                m_sign = ReadStringFromBuffer(i);
                */


                m_missing_tuples.push_back(std::make_tuple(
                    m_originIP, m_hop1IP, m_reputation_O, m_reputation_H,
                    m_GPS_O, m_GPS_H, m_battery_O, m_battery_H));
            }


            sizeMissingVoteList = i.ReadU16();
            ReadVotesFromBuffer2(i, sizeMissingVoteList);


            return dist;
            break;
       }


         case SR_HELLO:
                ReadFrom(i, m_originIP);
                ReadFrom(i, m_parentIP);
                m_reputation_O = i.ReadU16();
                m_GPS_O = i.ReadU16();
                m_battery_O = i.ReadU16();
                ReadVotesFromBuffer(i);
                return dist;
                break;
            case ROOT_SR_HELLO:
                ReadFrom(i, m_originIP);
                m_reputation_O = i.ReadU16();
                m_GPS_O = i.ReadU16();
                m_battery_O = i.ReadU16();
                ReadVotesFromBuffer(i);
                return dist;
                break;

        case SEND_MISSING_P2C_ACK:
            ReadFrom(i, m_originIP);
            dist = i.GetDistanceFrom(start);
            return dist;
            break;
        case ASK_BF:
           

            m_bloomFilter.clear();
            ReadFrom(i, m_originIP);
             NS_LOG_DEBUG("Deserialize BF FROM " << m_originIP);
            sizeBF = i.ReadU16();
            m_rtDim = i.ReadU16();
            count = 0;
            while (i.GetRemainingSize() > 0 && count < sizeBF) {
                uint8_t byte = i.ReadU8();
                for (int j = 0; j < 8; ++j) {
                    if(count >= sizeBF) { break; }
                    bool bit = byte & (1 << j);
                    m_bloomFilter.push_back(bit);
                    count++;
                }
            }


            dist = i.GetDistanceFrom(start);
            NS_LOG_DEBUG("Size of Bloom Filter (bits): " << m_bloomFilter.size());


                   
            return dist;
            break;
        case SEND_BF:
            m_bloomFilter.clear();
            m_votesIPList.clear();

            sizeBF = i.ReadU16();
            sizeList = i.ReadU16();

            // Ipv4Address
            for (uint32_t j = 0; j < sizeList; ++j) {
                Ipv4Address addr;
                ReadFrom(i, addr);
                m_votesIPList.push_back(addr);
            }


            count = 0;
            while (i.GetRemainingSize() > 0)
            {
                uint8_t byte = i.ReadU8();
                for (int j = 0; j < 8; ++j)
                {      
                    if(count >= sizeBF) {break;}
                    bool bit = byte & (1 << j); // Extract the j-th bit from the byte
                    m_bloomFilter.push_back(bit); // Add the bit to the vector
                    count++;
                        
                }
            }
            dist = i.GetDistanceFrom(start);

            return dist;
            break;
        
        case ASK_BF_P2C:
            ReadFrom(i, m_originIP);
            m_rtDim = i.ReadU16();
            dist = i.GetDistanceFrom(start);
            return dist;
            break;
        
        case SEND_BF_C2P:
            m_bloomFilter.clear();
            ReadFrom(i, m_originIP);
             NS_LOG_DEBUG("Deserialize BF FROM " << m_originIP);
            sizeBF = i.ReadU16();
            count = 0;
            while (i.GetRemainingSize() > 0 && count < sizeBF) {
                uint8_t byte = i.ReadU8();
                for (int j = 0; j < 8; ++j) {
                    if(count >= sizeBF) { break; }
                    bool bit = byte & (1 << j);
                    m_bloomFilter.push_back(bit);
                    count++;
                }
            }

            sizeList = i.ReadU16();

            // Ipv4Address
            for (uint32_t j = 0; j < sizeList; ++j) {
                Ipv4Address addr;
                ReadFrom(i, addr);
                m_votesIPList.push_back(addr);
            }



            dist = i.GetDistanceFrom(start);
            NS_LOG_DEBUG("Size of Bloom Filter (bits): " << m_bloomFilter.size());


                   
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

    uint16_t
    SaharaHeader::GetRTdim(){
        return m_rtDim;
    }

    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>>
    SaharaHeader::GetMissingTuples(){
        return m_missing_tuples;
    }

    SaharaHeader::VoteState
    SaharaHeader::GetVote_O(){
        return m_vote_O;
    }

    SaharaHeader::VoteState
    SaharaHeader::GetVote_H(){
        return m_vote_H;
    }

    std::string
    SaharaHeader::GetSign_O(){
        return m_sign_O;
    }

    std::string
    SaharaHeader::GetSign_H(){
        return m_sign_H;
    }

      SaharaHeader::VotePacket
      SaharaHeader::GetVotes() {
        return m_votes;
    }

    std::vector<Ipv4Address> 
    SaharaHeader::GetListVotesIP(){
        return m_votesIPList;
    }
    const std::vector<ns3::sahara::SaharaHeader::VotePacket>& 
    SaharaHeader::GetMissingVotePackets() const {
        return m_missing_VotePackets;
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
        void SaharaHeader::SetRTDim(uint16_t dim){
            m_rtDim = dim;
        }

        void SaharaHeader::SetVote_O(VoteState voteO){
            m_vote_O = voteO;
        }
        
        void SaharaHeader::SetVote_H(VoteState voteH){
            m_vote_H = voteH;
        }

        void SaharaHeader::SetSign_O(std::string signO){
            m_sign_O = signO;
        }

        void SaharaHeader::SetSign_H(std::string signH){
            m_sign_H = signH;
        }

       void SaharaHeader::SetVotes(VotePacket votePacket) {
        m_votes = votePacket;
    }
        void SaharaHeader::SetListVotesIP(std::vector<Ipv4Address> votesIPList){
            m_votesIPList = votesIPList;
        }
     void SaharaHeader::SetMissingVotePackets(const std::vector<ns3::sahara::SaharaHeader::VotePacket>& votePackets) {
        m_missing_VotePackets = votePackets;
    }
    
    void
    SaharaHeader::Print(std::ostream& os) const
    {

    }
    } // namespace Sahara
} // namespace ns3
