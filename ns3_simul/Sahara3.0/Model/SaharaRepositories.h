#ifndef SAHARA_REPOSITORIES_H
#define SAHARA_REPOSITORIES_H

#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#include <iostream>
#include <set>
#include <vector>


namespace ns3
{
namespace olsr
{

    /// \ingroup olsr
/// An Interface Association Tuple.
struct IfaceAssocTuple
{
    /// Interface address of a node.
    Ipv4Address ifaceAddr;
    /// Main address of the node.
    Ipv4Address mainAddr;
    /// Time at which this tuple expires and must be removed.
    Time time;
};

struct IfaceAssocTuple
{
    /// Interface address of a node.
    Ipv4Address ifaceAddr;
    /// Main address of the node.
    Ipv4Address mainAddr;
    /// Time at which this tuple expires and must be removed.
    Time time;
};

inline bool
operator==(const IfaceAssocTuple& a, const IfaceAssocTuple& b)
{
    return (a.ifaceAddr == b.ifaceAddr && a.mainAddr == b.mainAddr);
}

inline std::ostream&
operator<<(std::ostream& os, const IfaceAssocTuple& tuple)
{
    os << "IfaceAssocTuple(ifaceAddr=" << tuple.ifaceAddr << ", mainAddr=" << tuple.mainAddr
       << ", time=" << tuple.time << ")";
    return os;
}

/// \ingroup olsr
/// A Link Tuple.
struct LinkTuple
{
    /// Interface address of the local node.
    Ipv4Address localIfaceAddr;
    /// Interface address of the neighbor node.
    Ipv4Address neighborIfaceAddr;
    /// The link is considered bidirectional until this time.
    Time symTime;
    /// The link is considered unidirectional until this time.
    Time asymTime;
    /// Time at which this tuple expires and must be removed.
    Time time;
};

inline bool
operator==(const LinkTuple& a, const LinkTuple& b)
{
    return (a.localIfaceAddr == b.localIfaceAddr && a.neighborIfaceAddr == b.neighborIfaceAddr);
}

inline std::ostream&
operator<<(std::ostream& os, const LinkTuple& tuple)
{
    os << "LinkTuple(localIfaceAddr=" << tuple.localIfaceAddr
       << ", neighborIfaceAddr=" << tuple.neighborIfaceAddr << ", symTime=" << tuple.symTime
       << ", asymTime=" << tuple.asymTime << ", expTime=" << tuple.time << ")";
    return os;
}

/// \ingroup olsr
/// Association
struct Association
{
    Ipv4Address networkAddr; //!< IPv4 Network address.
    Ipv4Mask netmask;        //!< IPv4 Network mask.
};
inline bool
operator==(const Association& a, const Association& b)
{
    return (a.networkAddr == b.networkAddr && a.netmask == b.netmask);
}

inline std::ostream&
operator<<(std::ostream& os, const Association& tuple)
{
    os << "Association(networkAddr=" << tuple.networkAddr << ", netmask=" << tuple.netmask << ")";
    return os;
}

/// An Association Tuple
struct AssociationTuple
{
    /// Main address of the gateway.
    Ipv4Address gatewayAddr;
    /// Network Address of network reachable through gatewayAddr
    Ipv4Address networkAddr;
    /// Netmask of network reachable through gatewayAddr
    Ipv4Mask netmask;
    /// Time at which this tuple expires and must be removed
    Time expirationTime;
};

inline bool
operator==(const AssociationTuple& a, const AssociationTuple& b)
{
    return (a.gatewayAddr == b.gatewayAddr && a.networkAddr == b.networkAddr &&
            a.netmask == b.netmask);
}

inline std::ostream&
operator<<(std::ostream& os, const AssociationTuple& tuple)
{
    os << "AssociationTuple(gatewayAddr=" << tuple.gatewayAddr
       << ", networkAddr=" << tuple.networkAddr << ", netmask=" << tuple.netmask
       << ", expirationTime=" << tuple.expirationTime << ")";
    return os;
}


typedef std::set<Ipv4Address> MprSet;                       //!< MPR Set type.
typedef std::vector<MprSelectorTuple> MprSelectorSet;       //!< MPR Selector Set type.
typedef std::vector<LinkTuple> LinkSet;                     //!< Link Set type.
typedef std::vector<NeighborTuple> NeighborSet;             //!< Neighbor Set type.
typedef std::vector<TwoHopNeighborTuple> TwoHopNeighborSet; //!< 2-hop Neighbor Set type.
typedef std::vector<TopologyTuple> TopologySet;             //!< Topology Set type.
typedef std::vector<DuplicateTuple> DuplicateSet;           //!< Duplicate Set type.
typedef std::vector<IfaceAssocTuple> IfaceAssocSet;         //!< Interface Association Set type.
typedef std::vector<AssociationTuple> AssociationSet;       //!< Association Set type.
typedef std::vector<Association> Associations;              //!< Association Set type.

}
}
#endif


