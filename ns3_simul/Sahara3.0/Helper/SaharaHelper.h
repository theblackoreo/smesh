/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef SAHARA_HELPER_H
#define SAHARA_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

#include <map>
#include <set>

namespace ns3
{

/**
 * \ingroup Sahara
 *
 * \brief Helper class that adds Sahara routing to nodes.
 *
 * This class is expected to be used in conjunction with
 * ns3::InternetStackHelper::SetRoutingHelper
 */
class SaharaHelper : public Ipv4RoutingHelper
{
  public:
    /**
     * Create an SaharaHelper that makes life easier for people who want to install
     * Sahara routing to nodes.
     */
    SaharaHelper();

    /**
     * \brief Construct an SaharaHelper from another previously initialized instance
     * (Copy Constructor).
     *
     * \param o object to copy
     */
    SaharaHelper(const SaharaHelper& o);

    // Delete assignment operator to avoid misuse
    SaharaHelper& operator=(const SaharaHelper&) = delete;

    /**
     * \returns pointer to clone of this SaharaHelper
     *
     * This method is mainly for internal use by the other helpers;
     * clients are expected to free the dynamic memory allocated by this method
     */
    SaharaHelper* Copy() const override;

    /**
     * \param node the node for which an exception is to be defined
     * \param interface an interface of node on which Sahara is not to be installed
     *
     * This method allows the user to specify an interface on which Sahara is not to be installed on
     */
    void ExcludeInterface(Ptr<Node> node, uint32_t interface);

    /**
     * \param node the node on which the routing protocol will run
     * \returns a newly-created routing protocol
     *
     * This method will be called by ns3::InternetStackHelper::Install
     */
    Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

    /**
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set.
     *
     * This method controls the attributes of ns3::Sahara::RoutingProtocol
     */
    void Set(std::string name, const AttributeValue& value);

    /**
     * Assign a fixed random variable stream number to the random variables
     * used by this model.  Return the number of streams (possibly zero) that
     * have been assigned.  The Install() method of the InternetStackHelper
     * should have previously been called by the user.
     *
     * \param stream first stream index to use
     * \param c NodeContainer of the set of nodes for which the SaharaRoutingProtocol
     *          should be modified to use a fixed stream
     * \return the number of stream indices assigned by this helper
     */
    int64_t AssignStreams(NodeContainer c, int64_t stream);

  private:
    ObjectFactory m_agentFactory; //!< Object factory

    std::map<Ptr<Node>, std::set<uint32_t>>
        m_interfaceExclusions; //!< container of interfaces excluded from Sahara operations
};

} // namespace ns3

#endif /* Sahara_HELPER_H */