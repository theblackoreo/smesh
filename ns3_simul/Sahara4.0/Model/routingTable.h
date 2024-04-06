
#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include <iostream>
#include <vector>
#include <tuple>
#include "ns3/ipv4-address.h"
#include <map>

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>


namespace ns3{


class RoutingTable{

public:
	
	RoutingTable();
	
	bool AddTuple(Ipv4Address originIP, Ipv4Address hop1IP, uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O,
     uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H);
	
    std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t> getLastTupleTest();

	bool CheckDuplicate(Ipv4Address originIP, Ipv4Address hop1IP);

    size_t GetSize();

    void PrintAll();

    void DeleteAll();
    void CheckDifference();
    void RunDijkstra(Ipv4Address myAddress);
    bool CheckDubleEntry(Ipv4Address addr);
	bool checkExistenceOfNegh(Ipv4Address ip1, Ipv4Address ip2);
    uint16_t getDist(Ipv4Address ip1, Ipv4Address ip2);
    Ipv4Address LookUpAddr(Ipv4Address source, Ipv4Address dest); 
    std::vector<bool> CreateBloomFilter();
    bool ProcessSetReconciliation(std::vector<bool> bfs);


private:
	std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> tuples;
    //std::vector<std::tuple<Ipv4Address,uint16_t>> distance;
    //std::vector<Ipv4Address> unvisited;

    std::map<Ipv4Address, uint16_t> distance;
    std::map<Ipv4Address, bool> visited;
    //std::map<std::pair<Ipv4Address, Ipv4Address>, Ipv4Address> routes;
    std::map<Ipv4Address, Ipv4Address> previous;
    std::map<Ipv4Address, std::vector<Ipv4Address>> routes;
    //std::vector<std::vector<Ipv4Address>> routes;
    int num_hash_functions = 1;
    bool finalRound = false;

    // create crypto++ hash object
    CryptoPP::SHA256 hash; 
	

};
}

#endif


