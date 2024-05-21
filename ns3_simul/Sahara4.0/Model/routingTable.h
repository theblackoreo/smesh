
#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include <iostream>
#include <vector>
#include <tuple>
#include "ns3/ipv4-address.h"
#include <map>

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <string>
#include <limits>
#include <fstream>
#include "saharaBlock.h"



namespace ns3{


class RoutingTable{

public:
	
	RoutingTable();
    ~RoutingTable();
    RoutingTable& operator=(const RoutingTable& other);
	
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
    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> GetMissingTuples();

    void SetFile(std::string fileName);
    void UpdateFileHistory();
    void SetAllTupleFalse();
    void UpdateRoutingTable();
    std::vector<bool> GetBloomFilter();
   

private:
    using TupleType = std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>;
	std::vector<TupleType> m_tuples;

    std::map<Ipv4Address, uint16_t> m_distance;
    std::map<Ipv4Address, bool> m_visited;
    std::map<Ipv4Address, Ipv4Address> m_previous;
    std::map<Ipv4Address, std::vector<Ipv4Address>> m_routes;
    int m_num_hash_functions = 3;
    bool m_finalRound = false;


    // create crypto++ hash object
    CryptoPP::SHA256 m_hash; 

    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> m_senderTuplMissing;
	
    // store file
    std::ofstream m_outputFile;

    // to manage set reconciliation map: hash is the key and then tuple and bool. bool is the flag
    using MapType = std::unordered_map<std::string, std::pair<std::shared_ptr<TupleType>, bool>>;

    MapType m_hashMap;
    u_int32_t m_nTuplesWorstCase = 450;
    double m_probabilityFP = 0.1;
    std::vector<bool> m_bloomFilter;
    u_int32_t m_optimalNumberBits;
    Blockchain m_blockchain;
   

private:
 // updates
    void HashTuple(TupleType& tup);


};
}

#endif


