#include "routingTable.h"
#include <vector>
#include <tuple>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <limits>
#include "globalData.h"




namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("routingTable");

    RoutingTable::RoutingTable(){
        double partial = (m_nTuplesWorstCase * log(m_probabilityFP) ) / pow(log(2),2);
        m_optimalNumberBits = ceil(partial*(-1));
        m_bloomFilter.resize(m_optimalNumberBits);
        Block m_blockchain;
    }

    RoutingTable::~RoutingTable(){
         m_outputFile.close();
    }

    RoutingTable& RoutingTable::operator=(const RoutingTable& other) {
   

    return *this;
}

 
    // init file routing tables history
    void
    RoutingTable::SetFile(std::string fileName){
      m_outputFile.open(fileName, std::ios::out | std::ios::trunc);
      if (!m_outputFile.is_open()) {
        std::cerr << "Failed to open the output file." << std::endl;
    }

    }

    /* OLD BUT VALID
    bool RoutingTable::AddTuple(Ipv4Address originIP, Ipv4Address hop1IP, uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O, uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H) {
    
    // security part:
    if(!GlobalData::IpAllowed(originIP)) return false;
    if(!GlobalData::IpAllowed(hop1IP)) return false;

    //std::cout << originIP << " " << reputation_O << " " <<hop1IP << " " <<reputation_H << std::endl;
    if(CheckDuplicate(originIP, hop1IP)) return false; // it is not useful I think // check later

    // Tuple doesn't exist, add it to the vector
    if (originIP.Get() > hop1IP.Get()) {
        TupleType tup = std::make_tuple(hop1IP, originIP, reputation_H, reputation_O, GPS_H, GPS_O, battery_H, battery_O);
        //m_tuples.push_back(tup);
        HashTuple(tup); // add tuple if not present and checks it with true
    } else {
        TupleType tup = std::make_tuple(originIP, hop1IP, reputation_O, reputation_H, GPS_O, GPS_H, battery_O, battery_H);
        //m_tuples.push_back(tup);
        HashTuple(tup);
    }
    return true;
}
*/

    bool RoutingTable::AddTuple(Ipv4Address originIP, Ipv4Address hop1IP, uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O, uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H) {
        
        // security part:
        if(!GlobalData::IpAllowed(originIP)) return false;
        if(!GlobalData::IpAllowed(hop1IP)) return false;

        //std::cout << originIP << " " << reputation_O << " " <<hop1IP << " " <<reputation_H << std::endl;
        //if(CheckDuplicate(originIP, hop1IP)) return false; // it is not useful I think // check later

        if (originIP.Get() > hop1IP.Get()) {
            TupleType tup = std::make_tuple(hop1IP, originIP, reputation_H, reputation_O, GPS_H, GPS_O, battery_H, battery_O);
            m_tuples.push_back(tup);
        } else {
            TupleType tup = std::make_tuple(originIP, hop1IP, reputation_O, reputation_H, GPS_O, GPS_H, battery_O, battery_H);
            m_tuples.push_back(tup);
        }
        return true;
    }

    bool RoutingTable::AddTupleFlooding(Ipv4Address originIP, Ipv4Address hop1IP, uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O, uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H) {

    //std::cout << originIP << " " << reputation_O << " " <<hop1IP << " " <<reputation_H << std::endl;
    if(CheckDuplicate(originIP, hop1IP)) return false;

    // Tuple doesn't exist, add it to the vector
    if (originIP.Get() > hop1IP.Get()) {
        TupleType tup = std::make_tuple(hop1IP, originIP, reputation_H, reputation_O, GPS_H, GPS_O, battery_H, battery_O);
    } else {
        TupleType tup = std::make_tuple(originIP, hop1IP, reputation_O, reputation_H, GPS_O, GPS_H, battery_O, battery_H);
    }
    return true;
}

    void RoutingTable::AddEvaluatorVotes(const Ipv4Address& evaluatorIP, ns3::sahara::SaharaHeader::VotePacket votePacket) {
        m_globalVotes[evaluatorIP] = votePacket;
    }

    void RoutingTable::AddReceivedMissingVotePackets(std::vector<ns3::sahara::SaharaHeader::VotePacket> listVotePackets){

        for(const auto& t : listVotePackets){
            m_globalVotes[t.evaluatorIP] = t;
           
        }
        NS_LOG_DEBUG(m_myNodeID << ": VOTES ADDED (RECEIVED BY CHILD)");
       
    }


   

    
        
    std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>
    RoutingTable::getLastTupleTest(){

        if (m_tuples.empty()) {
                
            //NS_LOG_DEBUG("Routing table empty");
            return std::make_tuple(Ipv4Address("0.0.0.0"), Ipv4Address("0.0.0.0"), 0, 0, 0,0,0,0);
        }
        else {
            auto lastTuple = m_tuples.back();
            return lastTuple;
        }
    }

    // Method not used here -> maybe to delete 
    bool
    RoutingTable::CheckDuplicate(Ipv4Address originIP, Ipv4Address hop1IP){
                
        // Iterate through each tuple in the vector
        for (const auto& tuple : m_tuples) {
                
            // Check if myIP matches the first element of the current tuple
            if ((std::get<0>(tuple) == originIP and std::get<1>(tuple) == hop1IP) or (std::get<1>(tuple) == originIP and std::get<0>(tuple) == hop1IP)) {
                    return true; // Found a match, return true
            }
        }
        
        return false; // IP not found in any tuple
    }

    size_t
    RoutingTable::GetSize(){
            
        size_t tuplesSize = m_tuples.size();
        //NS_LOG_DEBUG("size of routing table: " << tuplesSize);
        return tuplesSize;
    }
    
    void
    RoutingTable::HashTuple(TupleType& tup){

        CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

        // Extract elements from the tuple
        Ipv4Address srcAddress = std::get<0>(tup);
        Ipv4Address dstAddress = std::get<1>(tup);
        uint16_t port1 = std::get<2>(tup);
        uint16_t port2 = std::get<3>(tup);
        uint16_t port3 = std::get<4>(tup);
        uint16_t port4 = std::get<5>(tup);
        uint16_t port5 = std::get<6>(tup);
        uint16_t port6 = std::get<7>(tup);
       

        // Convert tuple elements to strings or bytes (as needed)
        std::ostringstream oss;
        oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
        std::string concatenatedData = oss.str();
        bool maybePresent = true;
        
       
        // Check each hashing function with different prefixes
        for (uint8_t prefix = 0; prefix < m_num_hash_functions; ++prefix) {
            // Create a new input with the prefix
            std::string inputWithPrefix = std::to_string(prefix) + concatenatedData;
            
            // Hash the input
            m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(inputWithPrefix.c_str()), inputWithPrefix.length());
            
            // Compute the index from the hash digest
            size_t index = 0;
            for (size_t i = 0; i < CryptoPP::SHA256::DIGESTSIZE; ++i) {
                index += digest[i];
            }
           
            index %= m_optimalNumberBits;

            // Compare the local bit array and the received Bloom filter
            if (m_bloomFilter[index] == true && maybePresent) {
                maybePresent = true;
            } else if(m_bloomFilter[index] == false) {
                maybePresent = false;
                m_bloomFilter[index] = true;
            }
        }

        if(!maybePresent){
            m_tuples.push_back(tup);
        }
    
    }

     std::vector<bool>
     RoutingTable::GetBloomFilter(){
        CreateBloomFilter();
        return m_bloomFilter;
     }

     std::vector<bool>
     RoutingTable::GetDynamicBloomFilterIfActive(uint16_t sizeRTSender, bool isForward){
        CreateDynamicBloomFilter(sizeRTSender, isForward);
       
        return m_bloomFilter;
     }

    u_int16_t 
    RoutingTable::GetSizeRoutingTable(){
        return m_tuples.size();
    }
    


    void 
    RoutingTable::CreateBloomFilter() {

    //NS_LOG_DEBUG("Creating Bloom filter Static of size: " << m_optimalNumberBits);

    // First, resize the bloom filter
    m_bloomFilter.resize(m_optimalNumberBits, false);

    // Initialize a new bit array for the bloom filter
    std::vector<bool> bitArray(m_optimalNumberBits, false);

    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    
    for (const auto& t : m_tuples) {
        // Extract elements from the tuple
        Ipv4Address srcAddress = std::get<0>(t);
        Ipv4Address dstAddress = std::get<1>(t);
        uint16_t port1 = std::get<2>(t);
        uint16_t port2 = std::get<3>(t);
        uint16_t port3 = std::get<4>(t);
        uint16_t port4 = std::get<5>(t);
        uint16_t port5 = std::get<6>(t);
        uint16_t port6 = std::get<7>(t);
        /*
        uint8_t port7 = static_cast<uint8_t>(std::get<8>(t));
        uint8_t port8 = static_cast<uint8_t>(std::get<9>(t));
        std::string port9 = std::get<10>(t);
        */

        // Convert tuple elements to strings or bytes (as needed)
        std::ostringstream oss;
        oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
        std::string concatenatedData = oss.str();

         //NS_LOG_DEBUG("tuple: " << concatenatedData << " indexes: ");

        for (uint8_t prefix = 0; prefix < m_num_hash_functions; ++prefix) {
            // Create a new input with the prefix
            std::string inputWithPrefix = std::to_string(prefix) + concatenatedData;

            // Hash the input
            m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(inputWithPrefix.c_str()), inputWithPrefix.length());

            // Compute the index from the hash digest
            size_t index = 0;
            index = (digest[0] | (digest[1] << 8) | (digest[2] << 16) | (digest[3] << 24)) % m_optimalNumberBits; // Use a subset of bytes to calculate index
            

            // Set the corresponding bit in the local bit array
            bitArray[index] = true;
            //NS_LOG_DEBUG("" << index);
        }
    }


    // Assign the generated bit array to the bloom filter
    m_bloomFilter = bitArray;

   /*
           for (bool element : m_bloomFilter) {
            std::cout << element << " ";
          }
             std::cout << std::endl;
             */

}



   void RoutingTable::CreateDynamicBloomFilter(uint16_t sizeRTsender, bool isForward) {
    double partial;
    if(!isForward){
        partial = (m_num_hash_functions * (GetSizeRoutingTable() + sizeRTsender)) / log(1 - pow(m_probabilityFP, 1.0 / m_num_hash_functions));
    }
    else{
        partial = (m_num_hash_functions * (sizeRTsender)) / log(1 - pow(m_probabilityFP, 1.0 / m_num_hash_functions));

    }
    u_int32_t m_optimalNumberBitsDynamic = ceil(partial * (-1));

    //NS_LOG_DEBUG("Creating Dynamic Bloom filter of size: " << m_optimalNumberBitsDynamic);

    // First, resize the bloom filter
    m_bloomFilter.resize(m_optimalNumberBitsDynamic, false);

    // Initialize a new bit array for the bloom filter
    std::vector<bool> bitArray(m_optimalNumberBitsDynamic, false);

    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

    for (const auto& t : m_tuples) {
        // Extract elements from the tuple
        Ipv4Address srcAddress = std::get<0>(t);
        Ipv4Address dstAddress = std::get<1>(t);
        uint16_t port1 = std::get<2>(t);
        uint16_t port2 = std::get<3>(t);
        uint16_t port3 = std::get<4>(t);
        uint16_t port4 = std::get<5>(t);
        uint16_t port5 = std::get<6>(t);
        uint16_t port6 = std::get<7>(t);
        /*
        uint8_t port7 = static_cast<uint8_t>(std::get<8>(t));
        uint8_t port8 = static_cast<uint8_t>(std::get<9>(t));
        std::string port9 = std::get<10>(t);
        */

        // Convert tuple elements to strings or bytes (as needed)
        std::ostringstream oss;
        oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
        std::string concatenatedData = oss.str();

         //NS_LOG_DEBUG("tuple: " << concatenatedData << " indexes: ");

        for (uint8_t prefix = 0; prefix < m_num_hash_functions; ++prefix) {
            // Create a new input with the prefix
            std::string inputWithPrefix = std::to_string(prefix) + concatenatedData;

            // Hash the input
            m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(inputWithPrefix.c_str()), inputWithPrefix.length());

            // Compute the index from the hash digest
            size_t index = 0;
            index = (digest[0] | (digest[1] << 8) | (digest[2] << 16) | (digest[3] << 24)) % m_optimalNumberBitsDynamic; // Use a subset of bytes to calculate index
            
            // Set the corresponding bit in the local bit array
            bitArray[index] = true;
            //NS_LOG_DEBUG("" << index);
        }
    }
    /*
    // Print the bloom filter elements
    std::cout << "bit array elements: ";
    for (bool element : bitArray) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    PrintAll();
    */

    // Assign the generated bit array to the bloom filter
    m_bloomFilter = bitArray;

}




    

    void
    RoutingTable::DeleteAll(){
            
        m_tuples.clear();
    }

    void
    RoutingTable::CheckDifference(){

    }

   
    void RoutingTable::RunDijkstraNew(Ipv4Address myAddress){
    m_shortestPaths.clear();

    std::map<Ipv4Address, uint16_t> distances;
    std::map<Ipv4Address, Ipv4Address> previous;
    std::priority_queue<std::pair<uint16_t, Ipv4Address>, std::vector<std::pair<uint16_t, Ipv4Address>>, std::greater<std::pair<uint16_t, Ipv4Address>>> pq;
    std::map<Ipv4Address, std::vector<std::pair<Ipv4Address, uint16_t>>> topology; // nodeIP, vector of neighbors and cost

    // Initialize distances
    for (const auto& node : m_tuples) {
        Ipv4Address src = std::get<0>(node);
        Ipv4Address dest = std::get<1>(node);
        uint16_t repO = GetNodeReputation(src); 
        uint16_t repH = GetNodeReputation(dest);
        uint16_t GPSO = std::get<4>(node);
        uint16_t GPSH = std::get<5>(node);
        uint16_t batteryO = std::get<6>(node);
        uint16_t batteryH = std::get<7>(node);
        
        // Calcolare il "costo" in base alla reputazione inversa (più alta la reputazione, minore il costo)
        uint16_t cost = (1000 / (repO + 1) + 1000 / (repH + 1)) / 2;  // Incremento 1 per evitare divisione per zero

        topology[src].emplace_back(dest, cost);
        topology[dest].emplace_back(src, cost);

        if (distances.find(src) == distances.end()) {
            distances[src] = (src == myAddress) ? 0 : UINT16_MAX;
            previous[src] = Ipv4Address();
        }
        if (distances.find(dest) == distances.end()) {
            distances[dest] = (dest == myAddress) ? 0 : UINT16_MAX;
            previous[dest] = Ipv4Address();
        }
    }

    pq.emplace(0, myAddress);

    // Dijkstra's algorithm
    while (!pq.empty()) {
        auto [currentDist, u] = pq.top();
        pq.pop();

        if (currentDist > distances[u]) continue;

        for (const auto& [v, weight] : topology[u]) {
            uint16_t newDist = currentDist + weight;
            if (newDist < distances[v]) {
                distances[v] = newDist;
                previous[v] = u;
                pq.emplace(newDist, v);
            }
        }
    }

    // Reconstruct shortest paths
    for (const auto& [dest, dist] : distances) {
        if (dist == UINT16_MAX) {
            continue;
        }

        std::vector<Ipv4Address> path;
        for (Ipv4Address at = dest; at != myAddress; at = previous[at]) {
            path.push_back(at);
        }
        path.push_back(myAddress);
        std::reverse(path.begin(), path.end());
        m_shortestPaths[dest] = path;
    }

    for(const auto& [dest, path] : m_shortestPaths){
        if(checkExistenceOfNegh(dest, myAddress)){
            std::vector<Ipv4Address> path;
            path.push_back(myAddress);
            path.push_back(dest);
            m_shortestPaths[dest] = path;
        }
    }
}



    void
    RoutingTable::RunDijkstra(Ipv4Address myAddress){
        
       
        
        RunDijkstraNew(myAddress);
        
    }

    
    bool
    RoutingTable::checkExistenceOfNegh(Ipv4Address ip1, Ipv4Address ip2){
        for(const auto& t : m_tuples){
            if( (std::get<0>(t)==ip1 and std::get<1>(t)==ip2) || (std::get<1>(t)==ip1 and std::get<0>(t)==ip2)){
                return true;
            }
        }
        return false;
    }

    uint16_t
    RoutingTable::getDist(Ipv4Address ip1, Ipv4Address ip2){
        for(const auto& t : m_tuples){
            if((std::get<0>(t)==ip1 and std::get<1>(t)==ip2)){
                return std::get<3>(t);
            }
            else if((std::get<1>(t)==ip1 and std::get<0>(t)==ip2)){
                return std::get<2>(t);
            }
        }
        return 0;
    }

    // to delete, not used --------
    bool RoutingTable::CheckDubleEntry(Ipv4Address addr){
            for (const auto& t : m_distance) {
                // Check if myIP matches the first element of the current tuple
                if (addr == std::get<0>(t)) {
                    return true; // Found a match, return true
                }
            }
            return false; // IP not found in any tuple
    }    

    Ipv4Address RoutingTable::LookUpAddr(Ipv4Address source, Ipv4Address dest){
            

            // Check if the destination is reachable from the source
            if (m_shortestPaths.find(dest) == m_shortestPaths.end()) {
                // Destination is unreachable
                return Ipv4Address("0.0.0.0"); // Or any appropriate handling for unreachable destination
            }

            // Get the shortest path from source to destination
            const std::vector<Ipv4Address>& path = m_shortestPaths.at(dest);

            // Ensure source is not the destination
            if (path.size() < 2 || path.front() != source) {
                // Unexpected path or source is the destination
                return Ipv4Address("0.0.0.0"); // Or any appropriate handling for unexpected path
            }

            // Return the next hop in the path
            return path[1];
            
          

        }   

   


    /* Since the bloon filter is a probilistic data structure, it is possibile to decide the probability in having false positive
    changing either the lenght of the bloom filter or the number of hashing functions.
    K is the number of hashing functions
    M is the lenght of the bloom filter */
   bool RoutingTable::ProcessSetReconciliation(std::vector<bool> bfs) {
     // Constants
    //NS_LOG_DEBUG("[ProcessSetReconciliationStatic]");

    // Initialize the local bit array to match the size of the received Bloom filter
    std::vector<bool> bitArray(bfs.size(), false);
    m_senderTuplMissing.clear();
    
    //NS_LOG_DEBUG("[ProcessSetReconciliationStatic] BF SIZE " << bfs.size());
    
    // Process each tuple
    for (const auto& t : m_tuples) {
        // Extract elements from the tuple
        Ipv4Address srcAddress = std::get<0>(t);
        Ipv4Address dstAddress = std::get<1>(t);
        uint16_t port1 = std::get<2>(t);
        uint16_t port2 = std::get<3>(t);
        uint16_t port3 = std::get<4>(t);
        uint16_t port4 = std::get<5>(t);
        uint16_t port5 = std::get<6>(t);
        uint16_t port6 = std::get<7>(t);
        /*
        uint8_t port7 = static_cast<uint8_t>(std::get<8>(t));
        uint8_t port8 = static_cast<uint8_t>(std::get<9>(t));
        std::string port9 = std::get<10>(t);
        */

        // Create a concatenated string of all elements in the tuple
        std::ostringstream oss;
        oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
        std::string concatenatedData = oss.str();

        CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

        // Check each hash function with different prefixes
        bool tupleMissing = false;
        //NS_LOG_DEBUG("tuple :" << concatenatedData << " indexes: ");

        for (uint8_t prefix = 0; prefix < m_num_hash_functions; ++prefix) {
            // Create a new input with the prefix
            std::string inputWithPrefix = std::to_string(prefix) + concatenatedData;

            // Hash the input
            m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(inputWithPrefix.c_str()), inputWithPrefix.length());

            // Compute the index from the hash digest
            size_t index = 0;
            u_int32_t bfsSize = static_cast<u_int32_t>(bfs.size());
            index = (digest[0] | (digest[1] << 8) | (digest[2] << 16) | (digest[3] << 24)) % bfsSize; // Use a subset of bytes to calculate index
            //NS_LOG_DEBUG("TO DIVIDE?: " << bfsSize);


            // Set the corresponding bit in the local bit array
            bitArray[index] = true;

            // Compare the local bit array and the received Bloom filter
            if (bfs[index] != bitArray[index]) {
                m_senderTuplMissing.push_back(t);  // Add the tuple to missing tuples
                //NS_LOG_DEBUG("MISSING IN CHILD: " << concatenatedData << " index " << index);
                tupleMissing = true;
                break;  // No need to check further hashing functions if one failed
            }
        }

        if (!tupleMissing) {
            //NS_LOG_DEBUG("Tuple may be present");
        }
    }

    // Log whether there are missing elements or not
    if (m_senderTuplMissing.size() > 0) {
        //NS_LOG_DEBUG("Missing one or more elements in sender set");
    } else if (m_senderTuplMissing.size() == 0 && !m_finalRound) {
        //NS_LOG_DEBUG("Seems not missing elements, final check round");
    } else {
        //NS_LOG_DEBUG("Set reconciliation terminated");
    }

    return true;
}



bool RoutingTable::ProcessSetReconciliationDynamic(std::vector<bool> bfs) {
    // Constants
    //NS_LOG_DEBUG("[ProcessSetReconciliationDynamic]");

    // Initialize the local bit array to match the size of the received Bloom filter
    std::vector<bool> bitArray(bfs.size(), false);
    m_senderTuplMissing.clear();
    
 //NS_LOG_DEBUG("[ProcessSetReconciliationDynamic] BF SIZE " << bfs.size());
    // Process each tuple
    for (const auto& t : m_tuples) {
        // Extract elements from the tuple
        Ipv4Address srcAddress = std::get<0>(t);
        Ipv4Address dstAddress = std::get<1>(t);
        uint16_t port1 = std::get<2>(t);
        uint16_t port2 = std::get<3>(t);
        uint16_t port3 = std::get<4>(t);
        uint16_t port4 = std::get<5>(t);
        uint16_t port5 = std::get<6>(t);
        uint16_t port6 = std::get<7>(t);
        /*
        uint8_t port7 = static_cast<uint8_t>(std::get<8>(t));
        uint8_t port8 = static_cast<uint8_t>(std::get<9>(t));
        std::string port9 = std::get<10>(t);
        */

        // Create a concatenated string of all elements in the tuple
        std::ostringstream oss;
        oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
        std::string concatenatedData = oss.str();

        CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

        // Check each hash function with different prefixes
        bool tupleMissing = false;
        //NS_LOG_DEBUG("tuple :" << concatenatedData << " indexes: ");

        for (uint8_t prefix = 0; prefix < m_num_hash_functions; ++prefix) {
            // Create a new input with the prefix
            std::string inputWithPrefix = std::to_string(prefix) + concatenatedData;

            // Hash the input
            m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(inputWithPrefix.c_str()), inputWithPrefix.length());

            // Compute the index from the hash digest
            size_t index = 0;
            u_int32_t bfsSize = static_cast<u_int32_t>(bfs.size());
            index = (digest[0] | (digest[1] << 8) | (digest[2] << 16) | (digest[3] << 24)) % bfsSize; // Use a subset of bytes to calculate index
            //NS_LOG_DEBUG("TO DIVIDE?: " << bfsSize);

           
            // Set the corresponding bit in the local bit array
            bitArray[index] = true;
            //NS_LOG_DEBUG("" << index);

            // Compare the local bit array and the received Bloom filter
            if (bfs[index] != bitArray[index]) {
                m_senderTuplMissing.push_back(t);  // Add the tuple to missing tuples
                //NS_LOG_DEBUG("MISSING: " << concatenatedData << " index " << index);
                tupleMissing = true;
                break;  // No need to check further hashing functions if one failed
            }
        }

        if (!tupleMissing) {
            //NS_LOG_DEBUG("Tuple may be present");
        }
    }

    // Log whether there are missing elements or not
    if (m_senderTuplMissing.size() > 0) {
        //NS_LOG_DEBUG("Missing one or more elements in sender set");
    } else if (m_senderTuplMissing.size() == 0 && !m_finalRound) {
        //NS_LOG_DEBUG("Seems not missing elements, final check round");
    } else {
        //NS_LOG_DEBUG("Set reconciliation terminated");
    }

    return true;
}

    


    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> 
    RoutingTable::GetMissingTuples(){
        return m_senderTuplMissing;
    }

    void
    RoutingTable::UpdateFileHistory(){
        m_firstIT = false;
       UpdateReputationLocally();
       m_blockchain.addBlock(m_tuples, m_mapIDRep);
       m_blockchain.storeToFile(m_outputFile);
       


    }

    void RoutingTable::UpdateReputationLocally() {
    std::map<Ipv4Address, int> counter;

    for(const auto& it : m_globalVotes) {
        const ns3::sahara::SaharaHeader::VotePacket& votePacket = it.second; 

        for(const auto& vt : votePacket.votes) {
            if(vt.vote == ns3::sahara::SaharaHeader::VoteState::Positive) {
                counter[vt.EvaluatedIP]++;  
            } else if(vt.vote == ns3::sahara::SaharaHeader::VoteState::Negative) {
                counter[vt.EvaluatedIP]--; 
            }
        }
    }

    for(const auto& it : m_mapIDRep){
        if(counter[it.first] < 0) {
            m_mapIDRep[it.first] = m_mapIDRep[it.first] - 250;
            std::cout << "DECREASED ON ROUTING TABLE REPUTATION OF " << it.first << " new: "<< m_mapIDRep[it.first] << std::endl;
        }
        else if(counter[it.first] > 0){
            m_mapIDRep[it.first] = m_mapIDRep[it.first] + 1;
            std::cout << "INCREASE ON ROUTING TABLE REPUTATION OF " << it.first << " new: "<< m_mapIDRep[it.first] << std::endl;
        }
    }
}



    void
    RoutingTable::PrintAll(){
        NS_LOG_DEBUG("Printing routing table");
        
        for (const auto& tuple : m_tuples) {
                // Extract elements from the tuple
            const Ipv4Address& originIP = std::get<0>(tuple);
            const Ipv4Address& hop1IP = std::get<1>(tuple);
            uint16_t repO = std::get<2>(tuple);
            uint16_t repH = std::get<3>(tuple);
            uint16_t GPSO = std::get<4>(tuple);
            uint16_t GPSH = std::get<5>(tuple);
            uint16_t batteryO = std::get<6>(tuple);
            uint16_t batteryH = std::get<7>(tuple);
            /*
            int voteO = static_cast<int>(std::get<8>(tuple));
            int voteH = static_cast<int>(std::get<9>(tuple));
            std::string signature = std::get<10>(tuple);
            */
                
            // Print elements
            NS_LOG_DEBUG("Tuple: " << originIP << ", " << hop1IP << ", " << repO << ", " << repH << ", " << GPSO <<", " << GPSH <<", " << batteryO <<", " << batteryH);
        }
        NS_LOG_DEBUG("End______________________________________");


   
    }

    void 
    RoutingTable::SetAllTupleFalse(){
        
        for(auto& t : m_hashMap){
            t.second.second = false;
        }
    }

    void
    RoutingTable::UpdateRoutingTable(){
        
        auto it = m_hashMap.begin();
        while (it != m_hashMap.end()) {
            // Check if the `bool` value is `false`
            if (!it->second.second) {
                // Erase the current entry and get the next iterator
                it = m_hashMap.erase(it);
            } else {
                // Move to the next iterator
                ++it;
            }
        }
        
        
    }
    void 
    RoutingTable::SetMyNodeID (uint16_t id){
        m_myNodeID = id;
    }

    uint16_t
    RoutingTable::GetMyNodeID(){
        return m_myNodeID;
    }

    void 
    RoutingTable::GenerateRepMap(){

         for (const auto& t : m_tuples) {
            m_mapIDRep[std::get<0>(t)] = std::get<2>(t);
            m_mapIDRep[std::get<1>(t)] = std::get<3>(t);
         }
    }

    uint16_t
    RoutingTable::GetNodeReputation(Ipv4Address nodeIP){
        
        if(m_firstIT){
            return 255; // max at beginning
        }
        else{
            return m_mapIDRep[nodeIP];
        }

    }

    std::map<Ipv4Address, uint16_t> 
    RoutingTable::getMapIDRep(){
        return m_mapIDRep;
    }


    std::vector<Ipv4Address> 
    RoutingTable::GetVectOfNeigByIP(Ipv4Address nodeIP){
         std::vector<Ipv4Address> neighIPs;
    
        for (const auto& tuple : m_tuples) {
            Ipv4Address ip1 = std::get<0>(tuple);
            Ipv4Address ip2 = std::get<1>(tuple);
            
            if (ip1 == nodeIP) {
                neighIPs.push_back(ip2);
            } else if (ip2 == nodeIP) {
                neighIPs.push_back(ip1);
            }
        }
        return neighIPs;
    }



     void RoutingTable::SetVotes(Ipv4Address evaluatorIP, sahara::SaharaHeader::VotePacket votePacket) {
        m_votes = votePacket;
        AddEvaluatorVotes(evaluatorIP, votePacket);
    }


  void RoutingTable::PrintVotes() {
    for (const auto& entry : m_globalVotes) {
        Ipv4Address evaluatorIP = entry.first;
        const ns3::sahara::SaharaHeader::VotePacket& votePacket = entry.second;

        // Print the evaluator's IP
        std::cout << "Evaluator IP: " << evaluatorIP << std::endl;

        // Print each vote in the votes list
        for (const auto& vote : votePacket.votes) {
            std::cout << "  Evaluated IP: " << vote.EvaluatedIP
                      << ", Vote: " << static_cast<int>(vote.vote) << std::endl;
        }

        // Print the signature of the vote packet
        std::cout << "Signature: " << votePacket.signature << std::endl;
    }
}


    std::vector<Ipv4Address> 
    RoutingTable::GetListVotesIP(){
        std::vector<Ipv4Address> keyList;

        for (const auto& pair : m_globalVotes) {
            keyList.push_back(pair.first);
        }

        return keyList;
    }

    std::vector<ns3::sahara::SaharaHeader::VotePacket> 
    RoutingTable::GetMissingVotePackets(std::vector<Ipv4Address> ipReceived) {

    std::vector<ns3::sahara::SaharaHeader::VotePacket> missingForParent;

    for (const auto& globalVotes : m_globalVotes) {
        bool found = false;

        for (const auto& ipRecv : ipReceived) {
            if (ipRecv == globalVotes.first) { 
                found = true;
                break;
            }
        }

        if (!found) {
            missingForParent.push_back(globalVotes.second);  
        }
    }

    return missingForParent;
}


    

    void
    RoutingTable::ResetVariables(){
        m_tuples.clear();

        
        //std::fill(m_bloomFilter.begin(), m_bloomFilter.end(), false);

        // it works only for dynamic
        m_bloomFilter.clear();

        double partial = (m_nTuplesWorstCase * log(m_probabilityFP) ) / pow(log(2),2);
        m_optimalNumberBits = ceil(partial*(-1));
        m_bloomFilter.resize(m_optimalNumberBits);
       

       
        m_distance.clear();
        m_visited.clear();
        m_previous.clear();
        m_routes.clear();
        m_senderTuplMissing.clear();
        m_hashMap.clear();
        m_shortestPaths.clear();
        m_finalRound = true;
        m_globalVotes.clear();
        //m_votes.clear();

    }

   


    }