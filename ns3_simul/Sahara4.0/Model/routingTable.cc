

    #include "routingTable.h"


    #include <vector>
    #include <tuple>
    #include <cmath>

    namespace ns3
    {
        NS_LOG_COMPONENT_DEFINE("routingTable");

        RoutingTable::RoutingTable(){}
        
        
        bool
        RoutingTable::AddTuple(Ipv4Address originIP, Ipv4Address hop1IP, uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O,
     uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H){
            tuples.push_back(std::make_tuple(originIP, hop1IP, reputation_O, reputation_H, GPS_O, GPS_H, battery_O, battery_H));
            
            return true;
        
        }
        
        std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>
        RoutingTable::getLastTupleTest(){

            if (tuples.empty()) {
                
                NS_LOG_DEBUG("Routing table empty");
                return std::make_tuple(Ipv4Address("0.0.0.0"), Ipv4Address("0.0.0.0"), 0, 0, 0,0,0,0);
            }
            else {
                auto lastTuple = tuples.back();
                return lastTuple;
            }
        }


        bool
        RoutingTable::CheckDuplicate(Ipv4Address originIP, Ipv4Address hop1IP){
                
                // Iterate through each tuple in the vector
            for (const auto& tuple : tuples) {
                
                // Check if myIP matches the first element of the current tuple
                if ((std::get<0>(tuple) == originIP and std::get<1>(tuple) == hop1IP) or (std::get<1>(tuple) == originIP and std::get<0>(tuple) == hop1IP)) {
                    return true; // Found a match, return true
                }
            }
            return false; // IP not found in any tuple

        }

        size_t
        RoutingTable::GetSize(){
            
            size_t tuplesSize = tuples.size();
            NS_LOG_DEBUG("size of routing table: " << tuplesSize);
            return tuplesSize;
        
        }


        void
        RoutingTable::PrintAll(){
            NS_LOG_DEBUG("Printing routing table node");
            for (const auto& tuple : tuples) {
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
            
            tuples.clear();
        }

        void
        RoutingTable::CheckDifference(){

        }


        void RoutingTable::RunDijkstra(Ipv4Address myAddress){
            
            NS_LOG_DEBUG("[RunDijkstra - Routing Table] -> " << myAddress);

            // from this node to all other nodes
            distance.clear();
            visited.clear();
            routes.clear();
            previous.clear();

            // fill this list, at the beginning distance is max and all nodes are unvisited
            for (const auto& tup : tuples) {
                if(!CheckDubleEntry(std::get<0>(tup))){
                    distance[std::get<0>(tup)] = 255;
                    visited[std::get<0>(tup)] = false;
                    previous[std::get<0>(tup)] = Ipv4Address("0.0.0.0");
                }
            }
            distance[myAddress] = 0;

            
            Ipv4Address ind;
            
           for(int i = 0; i < tuples.size(); i++){
                uint16_t min = 255;

                for(const auto& tup : distance){
                    if(visited[tup.first]==false and tup.second<min){
                        min = tup.second; //distance[tup];
                        ind = tup.first;    
                    }   
                }

                visited[ind] = true;

                for(const auto& tup : distance){
                    // updating the distance of neighbouring vertex
                    if(!visited[tup.first] and checkExistenceOfNegh(ind,tup.first) and distance[ind]!=255 && distance[ind] + getDist(ind, tup.first) < distance[tup.first]){
                            
                            distance[tup.first]=distance[ind]+getDist(ind, tup.first);
                            previous[tup.first] = ind;
                            
                    }
                }

           }

            for (const auto& dest : distance) {
                Ipv4Address current = dest.first;
                std::vector<Ipv4Address> path;
                while (current != myAddress) {
                    path.push_back(current);
                    current = previous[current];
                }
                std::reverse(path.begin(), path.end());
                routes[dest.first] = path;
            }


           /*
           if(previousHop!=0){
                                routes.set(previousHop, tup.first, ind);
                            }else{
                                routes.set(ind, ind, ind);
                            }*/

            NS_LOG_DEBUG("Printing visited vector...");
            for (const auto& tu : visited) {
                
                // Print elements
                NS_LOG_DEBUG("Visited: " << tu.first << ", " << tu.second);
             }
             NS_LOG_DEBUG("End______________________________________");
           

           for (const auto& tu : distance) {
               
                NS_LOG_DEBUG("Distance: " << tu.first << ", " << tu.second);
             }
             NS_LOG_DEBUG("End______________________________________");


            for (const auto& e : routes) {
                    NS_LOG_DEBUG("Dest: " << e.first);
                for (const auto& hop : e.second) {
                        NS_LOG_DEBUG(hop);
                    }
            }
            NS_LOG_DEBUG("End______________________________________");

            // DeleteAll(); removed to test set reconc
        }


        bool RoutingTable::checkExistenceOfNegh(Ipv4Address ip1, Ipv4Address ip2){
            for(const auto& t : tuples){
                if( (std::get<0>(t)==ip1 and std::get<1>(t)==ip2) or (std::get<1>(t)==ip1 and std::get<0>(t)==ip2)){
                    return true;
                }
            }
            return false;
        }

        uint16_t RoutingTable::getDist(Ipv4Address ip1, Ipv4Address ip2){
            for(const auto& t : tuples){
                if((std::get<0>(t)==ip1 and std::get<1>(t)==ip2)){
                    return std::get<3>(t);
                }
                else if((std::get<1>(t)==ip1 and std::get<0>(t)==ip2)){
                    return std::get<2>(t);
                }
            }
            return 0;
        }


        bool RoutingTable::CheckDubleEntry(Ipv4Address addr){
            for (const auto& t : distance) {
                // Check if myIP matches the first element of the current tuple
                if (addr == std::get<0>(t)) {
                    return true; // Found a match, return true
                }
            }
            return false; // IP not found in any tuple
        }

        Ipv4Address RoutingTable::LookUpAddr(Ipv4Address source, Ipv4Address dest){
            
            // NS_LOG_DEBUG("[RTABLE, LookUpAddr], DEST -> " <<  dest);
            auto it = routes.find(dest);
            if (it == routes.end()) {
                return Ipv4Address("0.0.0.0"); // Return an invalid address if route doesn't exist
            }

            // dest route
            const std::vector<Ipv4Address>& route = it->second;

            // Find the next hop in the route
            auto nextHopIt = std::find(route.begin(), route.end(), source);

            if (nextHopIt != route.end()){
                return  *(nextHopIt + 1);
            } 
                else {
                //it's the last hop
                return route.front();
            }

        }



        std::vector<bool> 
        RoutingTable::CreateBloomFilter(){
            
            uint32_t obtimalNumberBits = ceil((GetSize() * num_hash_functions) / log(2));
            std::vector<bool> bitArray(obtimalNumberBits, false);

            CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

            for(const auto& t : tuples){
                // Extract elements from the tuple
                Ipv4Address srcAddress = std::get<0>(t);
                Ipv4Address dstAddress = std::get<1>(t);
                uint16_t port1 = std::get<2>(t);
                uint16_t port2 = std::get<3>(t);
                uint16_t port3 = std::get<4>(t);
                uint16_t port4 = std::get<5>(t);
                uint16_t port5 = std::get<6>(t);
                uint16_t port6 = std::get<7>(t);

                 // Convert tuple elements to strings or bytes (as needed) and hash
                std::ostringstream oss;
                oss << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
                std::string concatenatedData = oss.str();

                hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(concatenatedData.c_str()), concatenatedData.length());
                
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

        bool
        RoutingTable::ProcessSetReconciliation(std::vector<bool> bfs){
            // std::vector<bool> myBloomFilter;
            // myBloomFilter = CreateBloomFilter();

            
            // starting checking tuple by tuple
            uint32_t obtimalNumberBits = ceil((GetSize() * num_hash_functions) / log(2));
            std::vector<bool> bitArray(obtimalNumberBits, false);

            CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
            
            NS_LOG_DEBUG("BF own size -> " << obtimalNumberBits << ", BF received size-> " << bfs.size());

            std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> senderTuplMissing;

            for(const auto& t : tuples){
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

                
                hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(concatenatedData.c_str()), concatenatedData.length());

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
                    senderTuplMissing.push_back(t);
                }
            }

            if(senderTuplMissing.size() > 0){
                NS_LOG_DEBUG("Missing Elements in sender set");
            }
            else if(senderTuplMissing.size() == 0 and !finalRound){
                NS_LOG_DEBUG("Seems not missing elements, final check round");
            }
            else{
                NS_LOG_DEBUG("Set reconciliation terminated");
            }


            return true;
        }

    }