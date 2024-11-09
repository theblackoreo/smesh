
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
#include "saharaPacket.h"

#include <cstdint>



namespace ns3{


 

class RoutingTable{

public:
/*
struct Vote {
        Ipv4Address EvaluatedIP; // evaluated node IP
        sahara::SaharaHeader::VoteState vote;          
        std::string signature;   // signature of the evaluator
    };
*/

	
	RoutingTable();
    ~RoutingTable();

 

    RoutingTable& operator=(const RoutingTable& other);
	
	bool AddTuple(Ipv4Address originIP, Ipv4Address hop1IP, uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O,
     uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H);

    bool AddTupleFlooding(Ipv4Address originIP, Ipv4Address hop1IP, uint16_t reputation_O, uint16_t reputation_H, uint16_t GPS_O,
     uint16_t GPS_H, uint16_t battery_O, uint16_t battery_H);
	
    std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t> getLastTupleTest();

    //     void AddVote(const std::string& EvaluatedIP, const std::string& EvaluatorIP, int voto, const std::string& signature) {

    void AddEvaluatorVotes(const Ipv4Address& EvaluatorIP, ns3::sahara::SaharaHeader::VotePacket votePacket);



	bool CheckDuplicate(Ipv4Address originIP, Ipv4Address hop1IP);

    size_t GetSize();

    void PrintAll();
    void PrintVotes();

    void DeleteAll();
    void CheckDifference();
    void RunDijkstra(Ipv4Address myAddress);
    bool CheckDubleEntry(Ipv4Address addr);
	bool checkExistenceOfNegh(Ipv4Address ip1, Ipv4Address ip2);
    uint16_t getDist(Ipv4Address ip1, Ipv4Address ip2);
    Ipv4Address LookUpAddr(Ipv4Address source, Ipv4Address dest); 
    void CreateBloomFilter();
    bool ProcessSetReconciliation(std::vector<bool> bfs);
    bool ProcessSetReconciliationDynamic(std::vector<bool> bfs);
    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> GetMissingTuples();

    void SetFile(std::string fileName);
    void UpdateFileHistory();
    void SetAllTupleFalse();
    void UpdateRoutingTable();
    std::vector<bool> GetBloomFilter();

    void CreateDynamicBloomFilter(uint16_t sizeRTSender, bool isForward);
    std::vector<bool> GetDynamicBloomFilterIfActive(uint16_t sizeRTSender, bool isForward);

    // to test
    void RunDijkstraNew(Ipv4Address myAddress);
    void ResetVariables();
    
    u_int16_t GetSizeRoutingTable();

    uint16_t m_myNodeID;
    void SetMyNodeID(uint16_t id);
    uint16_t GetMyNodeID();

    uint16_t GetNodeReputation(Ipv4Address nodeIP);

    // map rep into ID's and not ips
    std::map<Ipv4Address, uint16_t> m_mapIDRep;
    void GenerateRepMap();
    std::map<Ipv4Address, uint16_t> getMapIDRep();

   void SetVotes(Ipv4Address evaluatorIP, sahara::SaharaHeader::VotePacket votePacket);
    std::vector<Ipv4Address> GetListVotesIP();

     std::vector<ns3::sahara::SaharaHeader::VotePacket> GetMissingVotePackets(std::vector<Ipv4Address> ipReceived);
      void AddReceivedMissingVotePackets(std::vector<ns3::sahara::SaharaHeader::VotePacket> listVotePackets);

    std::vector<Ipv4Address> GetVectOfNeigByIP(Ipv4Address nodeIP);
    void UpdateReputationLocally();
    
    bool IsIPInNeigh(Ipv4Address nodeIP1, Ipv4Address nodeIP2);
    std::vector<Ipv4Address> GetPathFromSourceToDestination(Ipv4Address source, Ipv4Address dest);


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
    u_int32_t m_nTuplesWorstCase = 6000;
    bool m_sr_dynamic_ON = false;

    double m_probabilityFP = 0.01;
    std::vector<bool> m_bloomFilter;
    u_int32_t m_optimalNumberBits;
    Blockchain m_blockchain;
    bool m_firstIT = true;

    ns3::sahara::SaharaHeader::VotePacket m_votes; 


    // id is the evaluator
   std::map<Ipv4Address, ns3::sahara::SaharaHeader::VotePacket> m_globalVotes;

    // new disj
    std::map<Ipv4Address, std::vector<Ipv4Address>> m_shortestPaths;


    // find another path varibles used in the function
    std::map<Ipv4Address, std::vector<Ipv4Address>> m_shortestPaths_d;

private:
 // updates
    void HashTuple(TupleType& tup);


};
}

#endif


