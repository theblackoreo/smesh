#include "globalData.h"

#include "ns3/ipv4-address.h"


#include <fstream>
#include <cstdint>
#include <string>

namespace ns3{
namespace GlobalData {

  
   
   std::vector<ns3::Ipv4Address> m_allowedIPs;
   bool 
   WriteDataToFile(uint16_t m_intNodeID, uint16_t rep, uint16_t gps, uint16_t bat) {
    std::string filename = "data_10.1.1."+std::to_string(m_intNodeID)+".txt";
    std::ofstream file(filename, std::ios::out | std::ios::trunc);
    if (file.is_open()) {
        file << std::to_string(rep) << "\n";
        file << std::to_string(gps) << "\n";
        file << std::to_string(bat) << "\n";
        file.close();
        return true;
    } else {
        
        return false;
    }
}

bool 
ReadDataFromFile(uint16_t m_intNodeID, uint16_t& rep, uint16_t& gps, uint16_t& bat) {
    std::string filename = "data_10.1.1."+std::to_string(m_intNodeID)+".txt";
    std::ifstream file(filename, std::ios::in);

    if (file.is_open()) {
         std::string rep_str, gps_str, bat_str;
        std::getline(file, rep_str);
        std::getline(file, gps_str);
        std::getline(file, bat_str);
        file.close();

        rep = static_cast<uint16_t>(std::stoul(rep_str));
        gps = static_cast<uint16_t>(std::stoul(gps_str));
        bat = static_cast<uint16_t>(std::stoul(bat_str));
         return true;
    } else {
        // Handle file opening error
        return false;
    }
}

bool
IpAllowed(ns3::Ipv4Address ip){
      for(const auto& i : m_allowedIPs){
        if(i == ip){
          
          return true;
        }
      }
      return false;
      
}

void UpdateAllowedIPs(){
    // Code to read IP addresses from the file and store them in a vector
    std::ifstream ipFile("ips.txt");

    if (ipFile.is_open()) {
        Ipv4Address ipAddress;
        while (ipFile >> ipAddress) {
            m_allowedIPs.push_back(ipAddress);
        }
        ipFile.close();
    }

}
}

}