#include "routingTable.h"
#include <vector>
#include <tuple>
#include <cmath>


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

    bool RoutingTable::AddTuple(Ipv4Address originIP, Ipv4Address hop1IP, uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O, uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H) {

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

    
        
    std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>
    RoutingTable::getLastTupleTest(){

        if (m_tuples.empty()) {
                
            NS_LOG_DEBUG("Routing table empty");
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
        NS_LOG_DEBUG("size of routing table: " << tuplesSize);
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
           
    /*
        // Hash the input
        m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(concatenatedData.c_str()), concatenatedData.length());
        
    
         std::ostringstream strDig;
        for (size_t i = 0; i < CryptoPP::SHA256::DIGESTSIZE; i++) {
            strDig << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
        }

       auto it = m_hashMap.find(strDig.str());
        if (it != m_hashMap.end()) {
            bool& checked = it->second.second;
            checked = true;
            NS_LOG_DEBUG("existing " << concatenatedData << ", hash: " << strDig.str());
        } else {
            // Add the TupleType to the map
            m_tuples.push_back(tup);
            auto tuplePtr = std::make_shared<TupleType>(tup);
            m_hashMap[strDig.str()] = std::make_pair(tuplePtr, true);
            NS_LOG_DEBUG("not existing " << concatenatedData << ", hash: " << strDig.str());

           
        }
    */
    }

     std::vector<bool>
     RoutingTable::GetBloomFilter(){
        return m_bloomFilter;
     }
    


    std::vector<bool> RoutingTable::CreateBloomFilter() {
        
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

        // Convert tuple elements to strings or bytes (as needed)
        std::ostringstream oss;
        oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
        std::string concatenatedData = oss.str();

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

            // Set the corresponding bit to 1 in the bit array
            bitArray[index] = true;

        }
    }

    std::string toPrintTosee;
    for (int i = 0; i < m_optimalNumberBits; ++i) {
        toPrintTosee += std::to_string(bitArray[i]);
    }

    NS_LOG_DEBUG("Optimal number of bits: " << m_optimalNumberBits);
   // NS_LOG_DEBUG("Bloom filter " << toPrintTosee);
    PrintAll();


    return bitArray;
}


    

    void
    RoutingTable::DeleteAll(){
            
        m_tuples.clear();
    }

    void
    RoutingTable::CheckDifference(){

    }


    void
    RoutingTable::RunDijkstra(Ipv4Address myAddress){

        
            
        NS_LOG_DEBUG("[RunDijkstra - Routing Table] -> " << myAddress);
    
        // from this node to all other nodes
        m_distance.clear();
        m_visited.clear();
        m_routes.clear();
        m_previous.clear();

         NS_LOG_DEBUG("[RunDijkstra - Routing Table] -> " << myAddress << " Cleared");

        // fill this list, at the beginning m_distance is max and all nodes are unvisited
        for (const auto& tup : m_tuples) {
           
                m_distance[std::get<0>(tup)] = 255;
                m_visited[std::get<0>(tup)] = false;
                m_previous[std::get<0>(tup)] = Ipv4Address("0.0.0.0");

                m_distance[std::get<1>(tup)] = 255;
                m_visited[std::get<1>(tup)] = false;
                m_previous[std::get<1>(tup)] = Ipv4Address("0.0.0.0");
            
            
        }
        
        NS_LOG_DEBUG("[RunDijkstra - Routing Table] -> " << myAddress << " Initilized vectors");

        
        m_distance[myAddress] = 0;
        Ipv4Address ind;
            
        for(int i = 0; i < m_tuples.size(); i++){
            uint16_t min = 255;

            for(const auto& tup : m_distance){
                if(m_visited[tup.first]==false and tup.second<min){
                    min = tup.second; //m_distance[tup];
                    ind = tup.first;    
                }   
            }

            m_visited[ind] = true;
            
            for(const auto& tup : m_distance){
                // updating the m_distance of neighbouring vertex
                if(!m_visited[tup.first] and checkExistenceOfNegh(ind,tup.first) and m_distance[ind]!=255 && m_distance[ind] + getDist(ind, tup.first) < m_distance[tup.first]){
                        
                m_distance[tup.first]=m_distance[ind]+getDist(ind, tup.first);
                m_previous[tup.first] = ind;
                            
                }
            }

        }

    /*
        for (const auto& dest : m_distance) {
            Ipv4Address current = dest.first;
            std::vector<Ipv4Address> path;
            while (current != myAddress) {
                path.push_back(current);
                current = m_previous[current];
            }
            std::reverse(path.begin(), path.end());
            m_routes[dest.first] = path;
        }*/

        for (const auto& dest : m_distance) {
            Ipv4Address current = dest.first;
            std::vector<Ipv4Address> path;
            try {
                while (current != myAddress) {
                    if (m_previous.find(current) == m_previous.end()) {
                        // Handle error: No previous node found for current node
                        NS_LOG_DEBUG("Not found previus -> not existing path");
                       
                        break;
                        //throw std::runtime_error("No previous node found for current node.");

                    }
                    else{
                        path.push_back(current);
                        current = m_previous.at(current);
                    }
                }
                path.push_back(myAddress); // Include myAddress in the path
                m_routes[dest.first] = path;
            } catch (const std::exception& e) {
                // Handle error: Exception occurred during path calculation
                std::cerr << "Error while calculating path: " << e.what() << std::endl;
                // Optionally, you can choose to continue with the next destination
            }
        }

       // m_tuples.clear();


        // PRINTING DIJ RESULTS
        /*
         NS_LOG_DEBUG("[RunDijkstra - Routing Table] -> " << myAddress << " second loop passed ");

        NS_LOG_DEBUG("Printing m_visited vector...");
        for (const auto& tu : m_visited) {            
            
            // Print elements
            NS_LOG_DEBUG("m_visited: " << tu.first << ", " << tu.second);
        }
        NS_LOG_DEBUG("End______________________________________");
           

        for (const auto& tu : m_distance) {
               
            NS_LOG_DEBUG("m_distance: " << tu.first << ", " << tu.second);
        }
        NS_LOG_DEBUG("End______________________________________");


        for (const auto& e : m_routes) {
            NS_LOG_DEBUG("Dest: " << e.first);
            for (const auto& hop : e.second) {
                NS_LOG_DEBUG(hop);
            }
        }
        NS_LOG_DEBUG("End______________________________________");
        */
        // DeleteAll(); removed to test set reconc
    }


    bool
    RoutingTable::checkExistenceOfNegh(Ipv4Address ip1, Ipv4Address ip2){
        for(const auto& t : m_tuples){
            if( (std::get<0>(t)==ip1 and std::get<1>(t)==ip2) or (std::get<1>(t)==ip1 and std::get<0>(t)==ip2)){
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
            
           
            NS_LOG_DEBUG("[RTABLE, LookUpAddr], DEST -> " <<  dest);

            auto it = m_routes.find(dest);
            if (it == m_routes.end()) {
                return Ipv4Address("0.0.0.0"); // Return an invalid address if route doesn't exist
            }

            // dest route
            const std::vector<Ipv4Address>& route = it->second;

            // Find the next hop in the route
            auto nextHopIt = std::find(route.begin(), route.end(), source);
            
            if((nextHopIt-1) == route.begin()){
                 NS_LOG_DEBUG("[RTABLE, LookUpAddr], are neighbors-> " <<  dest);
                return dest;
            }
            else{
                NS_LOG_DEBUG("[RTABLE, LookUpAddr]-> " <<   *(nextHopIt-1));
                return *(nextHopIt-1);
            }
           

            /*
            if (nextHopIt != route.end() &&  (nextHopIt + 1) != route.end()){
                NS_LOG_DEBUG("[RTABLE, LookUpAddr], next hop -> " <<  *(nextHopIt + 1));
                return  *(nextHopIt + 1);
            } 
                else {
                //it's the last hop
                NS_LOG_DEBUG("[RTABLE, LookUpAddr], next hop last hop-> " <<  dest);
                return dest;
            }
            */

        }   

   


    /* Since the bloon filter is a probilistic data structure, it is possibile to decide the probability in having false positive
    changing either the lenght of the bloom filter or the number of hashing functions.
    K is the number of hashing functions
    M is the lenght of the bloom filter */
   bool RoutingTable::ProcessSetReconciliation(std::vector<bool> bfs) {
    // Constants
    
    NS_LOG_DEBUG("[ProcessSetReconciliation]");


    
     std::vector<bool> bitArray(m_optimalNumberBits, false);

    /*
    // Ensure the received Bloom filter has the same size as expected
    if (bfs.size() != m_optimalNumberBits) {
        NS_LOG_DEBUG("Size mismatch between own Bloom filter and received Bloom filter");
        return false;
    }
    */

    NS_LOG_DEBUG("BF own size -> " << m_optimalNumberBits << ", BF received size-> " << bfs.size());

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

        std::ostringstream oss;
        oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
        std::string concatenatedData = oss.str();
        CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

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

            // Set the corresponding bit in the local bit array
            bitArray[index] = true;

            // Compare the local bit array and the received Bloom filter
            if (bfs[index] == bitArray[index]) {
                NS_LOG_DEBUG("Tuple maybe present");
            } else {
                NS_LOG_DEBUG("Tuple missing in sender -> " << srcAddress << ", " << dstAddress);
                m_senderTuplMissing.push_back(t);
                break; // No need to check further hashing functions if one failed
            }
        }
    }

    // Provide appropriate logs based on whether there are missing elements or not
    if (m_senderTuplMissing.size() > 0) {
        NS_LOG_DEBUG("Missing Elements in sender set");
    } else if (m_senderTuplMissing.size() == 0 && !m_finalRound) {
        NS_LOG_DEBUG("Seems not missing elements, final check round");
    } else {
        NS_LOG_DEBUG("Set reconciliation terminated");
    }

    return true;
}


    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> 
    RoutingTable::GetMissingTuples(){
        return m_senderTuplMissing;
    }

    void
    RoutingTable::UpdateFileHistory(){
       m_blockchain.addBlock(m_tuples);
       m_blockchain.storeToFile(m_outputFile);

        /*
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
                
            // write elements
            m_outputFile << originIP << ", " << hop1IP << ", " << repO << ", " << repH << ", " << GPSO <<", " << GPSH <<", " << batteryO <<", " << batteryH << std::endl;
        }
        */

    }

    void
    RoutingTable::PrintAll(){
        NS_LOG_DEBUG("Printing routing table node");
        
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
                
            // Print elements
            NS_LOG_DEBUG("Tuple: " << originIP << ", " << hop1IP << ", " << repO << ", " << repH << ", " << GPSO <<", " << GPSH <<", " << batteryO <<", " << batteryH);
        }
        NS_LOG_DEBUG("End______________________________________");


/*
         for (const auto& entry : m_hashMap) {
        // Key (digest) is a pointer to CryptoPP::byte
        CryptoPP::byte* key = entry.first;

        // Convert the key (digest) from pointer to a printable format (e.g., hex representation)
        std::ostringstream oss;
        oss << "Key (digest): ";
        for (size_t i = 0; i < CryptoPP::SHA256::DIGESTSIZE; i++) {
            oss << std::hex << std::setfill('0') << std::setw(2) << (int)key[i];
        }
        oss << std::dec; // Reset format

        // Value is a pair<TupleType&, bool>
        const auto& value = entry.second;
        TupleType& tuple = value.first;
        bool flag = value.second;

        // Print the tuple elements as needed (you need to provide logic to print the tuple)
        oss << " | Tuple: " << std::get<0>(tuple) << ", " <<std::get<1>(tuple); // Replace this line with specific logic to print tuple elements
        
        // Print the flag
        oss << " | Flag: " << std::boolalpha << flag;

        // End of the current entry
        oss << std::endl;
        NS_LOG_DEBUG(oss.str());
    }
*/
   
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

   


    }