// file: global_data.h
#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include "ns3/ipv4-address.h"
namespace  ns3{

namespace GlobalData {
   extern uint16_t m_rep;
   extern uint16_t m_gps;
   extern uint16_t m_bat;

   
   //extern std::vector<ns3::Ipv4Address> m_allowedIPs;
   extern std::vector<ns3::Ipv4Address> m_allowedIPs;


   bool WriteDataToFile(uint16_t m_intNodeID, uint16_t rep, uint16_t gps, uint16_t bat);
   bool ReadDataFromFile(uint16_t m_intNodeID, uint16_t& rep, uint16_t& gps, uint16_t& bat);
   bool IpAllowed(Ipv4Address ip);
   void UpdateAllowedIPs();

}
}
#endif