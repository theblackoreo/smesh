

#include "saharaQueue.h"

#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/socket.h"

#include <algorithm>
#include <functional>


// copied from DSDV packet queue (Copyright (c) 2010 Hemanth Narra)

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("saharaQueue");

namespace sahara
{
uint32_t
PacketQueue::GetSize()
{
    Purge();
    return m_queue.size();
}

bool
PacketQueue::Enqueue(QueueEntry& entry)
{
    NS_LOG_FUNCTION("Enqueuing packet destined for" << entry.GetIpv4Header().GetDestination());
    Purge();
    uint32_t numPacketswithdst;
    for (auto i = m_queue.begin(); i != m_queue.end(); ++i)
    {
        if ((i->GetPacket()->GetUid() == entry.GetPacket()->GetUid()) &&
            (i->GetIpv4Header().GetDestination() == entry.GetIpv4Header().GetDestination()))
        {
            return false;
        }
    }
    numPacketswithdst = GetCountForPacketsWithDst(entry.GetIpv4Header().GetDestination());
    NS_LOG_DEBUG("Number of packets with this destination: " << numPacketswithdst);
    /** For Brock Paper comparison*/
    if (numPacketswithdst >= m_maxLenPerDst || m_queue.size() >= m_maxLen)
    {
        NS_LOG_DEBUG("Max packets reached for this destination. Not queuing any further packets");
        return false;
    }
    else
    {
        // NS_LOG_DEBUG("Packet size while enqueuing "<<entry.GetPacket()->GetSize());
        entry.SetExpireTime(m_queueTimeout);
        m_queue.push_back(entry);
        return true;
    }
}

void
PacketQueue::DropPacketWithDst(Ipv4Address dst)
{
    NS_LOG_FUNCTION("Dropping packet to " << dst);
    Purge();
    for (auto i = m_queue.begin(); i != m_queue.end(); ++i)
    {
        if (i->GetIpv4Header().GetDestination() == dst)
        {
            Drop(*i, "DropPacketWithDst ");
        }
    }
    auto new_end = std::remove_if(m_queue.begin(), m_queue.end(), [&](const QueueEntry& en) {
        return en.GetIpv4Header().GetDestination() == dst;
    });
    m_queue.erase(new_end, m_queue.end());
}

bool
PacketQueue::Dequeue(Ipv4Address dst, QueueEntry& entry)
{
    NS_LOG_FUNCTION("Dequeueing packet destined for" << dst);
    Purge();
    for (auto i = m_queue.begin(); i != m_queue.end(); ++i)
    {
        if (i->GetIpv4Header().GetDestination() == dst)
        {
            entry = *i;
            m_queue.erase(i);
            return true;
        }
    }
    return false;
}

bool
PacketQueue::Find(Ipv4Address dst)
{
    for (auto i = m_queue.begin(); i != m_queue.end(); ++i)
    {
        if (i->GetIpv4Header().GetDestination() == dst)
        {
            NS_LOG_DEBUG("Find");
            return true;
        }
    }
    return false;
}

uint32_t
PacketQueue::GetCountForPacketsWithDst(Ipv4Address dst)
{
    uint32_t count = 0;
    for (auto i = m_queue.begin(); i != m_queue.end(); ++i)
    {
        if (i->GetIpv4Header().GetDestination() == dst)
        {
            count++;
        }
    }
    return count;
}


std::vector<Ipv4Address>
PacketQueue::GetAllPacketsDest(){
    std::vector<Ipv4Address> allDest;
    
    for (auto i = m_queue.begin(); i != m_queue.end(); ++i)
    {
        allDest.push_back(i->GetIpv4Header().GetDestination());
    }

    return allDest;

}


/**
 * IsExpired structure
 */
struct IsExpired
{
    /**
     * \brief Check for expired entry
     * \param e QueueEntry to check
     * \return true if expired
     */
    bool operator()(const QueueEntry& e) const
    {
        // NS_LOG_DEBUG("Expire time for packet in req queue: "<<e.GetExpireTime ());
        return (e.GetExpireTime() < Seconds(0));
    }
};

void
PacketQueue::Purge()
{
    // NS_LOG_DEBUG("Purging Queue");
    IsExpired pred;
    for (auto i = m_queue.begin(); i != m_queue.end(); ++i)
    {
        if (pred(*i))
        {
            NS_LOG_DEBUG("Dropping outdated Packets");
            Drop(*i, "Drop outdated packet ");
        }
    }
    m_queue.erase(std::remove_if(m_queue.begin(), m_queue.end(), pred), m_queue.end());
}

void
PacketQueue::Drop(QueueEntry en, std::string reason)
{
    NS_LOG_LOGIC(reason << en.GetPacket()->GetUid() << " " << en.GetIpv4Header().GetDestination());
    // en.GetErrorCallback () (en.GetPacket (), en.GetIpv4Header (),
    //   Socket::ERROR_NOROUTETOHOST);
}

} // namespace sahara
} // namespace ns3
