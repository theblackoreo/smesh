#include "routingTable.h"
#include <vector>
#include <tuple>
#include <cmath>

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("routingTable");

    RoutingTable::RoutingTable(){
    }

    RoutingTable::~RoutingTable(){
         m_outputFile.close();
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
    // Check if the tuple already exists in the vector
    for (const auto& tuple : m_tuples) {
        
        if (std::get<0>(tuple) == originIP && std::get<1>(tuple) == hop1IP) {
            // Tuple already exists, return false
            return false;
        }
    }

    // Tuple doesn't exist, add it to the vector
    if (originIP.Get() > hop1IP.Get()) {
        m_tuples.push_back(std::make_tuple(hop1IP, originIP, reputation_H, reputation_O, GPS_H, GPS_O, battery_H, battery_O));
    } else {
        m_tuples.push_back(std::make_tuple(originIP, hop1IP, reputation_O, reputation_H, GPS_O, GPS_H, battery_O, battery_H));
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
     NS_LOG_DEBUG("[RunDijkstra - Routing Table] -> " << myAddress << " first loop passed ");

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
            
            for (const auto& e : m_routes) {
            NS_LOG_DEBUG("Dest: " << e.first);
            for (const auto& hop : e.second) {
                NS_LOG_DEBUG(hop);
            }
        }
        NS_LOG_DEBUG("End______________________________________");

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

    std::vector<bool> 
    RoutingTable::CreateBloomFilter(){
            
            uint32_t nWorstCaseTuple = 270;

            uint32_t obtimalNumberBits = ceil((nWorstCaseTuple * m_num_hash_functions) / log(2));
            std::vector<bool> bitArray(obtimalNumberBits, false);

            CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

            for(const auto& t : m_tuples){
                // Extract elements from the tuple
                Ipv4Address srcAddress = std::get<0>(t);
                Ipv4Address dstAddress = std::get<1>(t);
                uint16_t port1 = std::get<2>(t);
                uint16_t port2 = std::get<3>(t);
                uint16_t port3 = std::get<4>(t);
                uint16_t port4 = std::get<5>(t);
                uint16_t port5 = std::get<6>(t);
                uint16_t port6 = std::get<7>(t);

                 // Convert tuple elements to strings or bytes (as needed) and m_hash
                std::ostringstream oss;
                oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
                std::string concatenatedData = oss.str();

                m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(concatenatedData.c_str()), concatenatedData.length());
                
                size_t index = 0;
                for (size_t i = 0; i < CryptoPP::SHA256::DIGESTSIZE; ++i) {
                    index += digest[i];
                }
                index %= obtimalNumberBits;

                // Set the corresponding bit to 1 in the bit array
                bitArray[index] = true;

                std::string hexDigest;
                CryptoPP::HexEncoder encoder;
                encoder.Attach(new CryptoPP::StringSink(hexDigest));
                encoder.Put(digest, sizeof(digest));
                encoder.MessageEnd();


                NS_LOG_DEBUG("Tuple: " << srcAddress << ", " <<  dstAddress << " Digest -> " << hexDigest);
                
            }

            std::string toPrintTosee;
            for (int i = 0; i < obtimalNumberBits; ++i) {
                toPrintTosee += std::to_string(bitArray[i]);
            }
            
            NS_LOG_DEBUG("Obtimal number of bits: " << obtimalNumberBits);
            NS_LOG_DEBUG("Bloom filter " << toPrintTosee);

            return bitArray;
    }


    /* Since the bloon filter is a probilistic data structure, it is possibile to decide the probability in having false positive
    changing either the lenght of the bloom filter or the number of hashing functions.
    K is the number of hashing functions
    M is the lenght of the bloom filter */
    bool
        RoutingTable::ProcessSetReconciliation(std::vector<bool> bfs){
           
            // only to test -> K = 1, M = max dimension routing table in the worst case scenaio -> se 6 nodi è 1700?

            uint32_t nWorstCaseTuple = 270;

            // starting checking tuple by tuple
            uint32_t obtimalNumberBits = ceil((nWorstCaseTuple * m_num_hash_functions) / log(2));
            std::vector<bool> bitArray(obtimalNumberBits, false);

            CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
            
            NS_LOG_DEBUG("BF own size -> " << obtimalNumberBits << ", BF received size-> " << bfs.size());

            
            for(const auto& t : m_tuples){
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

                
                m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(concatenatedData.c_str()), concatenatedData.length());

                size_t index = 0;
                for (size_t i = 0; i < CryptoPP::SHA256::DIGESTSIZE; ++i) {
                    index += digest[i];
                }
                index %= bfs.size();

                bitArray[index] = true;
                
                if(bfs[index] == bitArray[index]){
                    NS_LOG_DEBUG("Tuple maybe present");
                }
                else{
                    NS_LOG_DEBUG("Tuple missing in sender -> " << srcAddress << ", " << dstAddress);
                    m_senderTuplMissing.push_back(t);
                }
            }

            if(m_senderTuplMissing.size() > 0){
                NS_LOG_DEBUG("Missing Elements in sender set");
            }
            else if(m_senderTuplMissing.size() == 0 and !m_finalRound){
                NS_LOG_DEBUG("Seems not missing elements, final check round");
            }
            else{
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

    }
    


    }