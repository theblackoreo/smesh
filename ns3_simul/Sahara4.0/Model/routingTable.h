
#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include <iostream>
#include <vector>
#include <tuple>
#include "ns3/ipv4-address.h"


namespace ns3{


class RoutingTable{

public:
	
	RoutingTable();
	
	bool AddTuple(Ipv4Address originIP, Ipv4Address hop1IP, uint8_t reputation, uint8_t GPS, uint8_t battery);
	
    std::tuple<Ipv4Address,Ipv4Address,uint8_t,uint8_t,uint8_t> getLastTupleTest();

	bool CheckDuplicate(Ipv4Address originIP, Ipv4Address hop1IP);

    size_t GetSize();

    void PrintAll();
	
private:
	std::vector<std::tuple<Ipv4Address,Ipv4Address,uint8_t,uint8_t,uint8_t>> tuples;
	

};
}

#endif


