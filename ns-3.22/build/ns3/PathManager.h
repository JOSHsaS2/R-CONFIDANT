#ifndef _PATHMANAGER_H
#define _PATHMANAGER_H
#include "ns3/ipv4-address.h"
#include "ns3/aodv-rtable.h"
#include "ns3/ptr.h"
#include "ns3/ipv4.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include <vector>
#include <string>
#include <sstream>

namespace ns3
{
namespace confidant
{

std::string ConvertIpToString(Ipv4Address);


class PathManager: public Object
{

private:
    std::vector<Ipv4Address> m_list;
    ns3::aodv::RoutingTable* aodvRtable;///change
    //for source trace -- get node id form ipv4
    Ptr<Ipv4> m_ipv4;
    ns3::TracedCallback <uint32_t, ns3::Time,
        Ipv4Address, bool> m_AddSelfishNodeTrace;

    //bool ifenableL;
    //ns3::aodv::RoutingProtocol& aodvRP;
    //ns3::confidant::ReputationSystem repSys;

public:
    void addMnode(Ipv4Address m_node);
    void rmMnode(Ipv4Address m_node);
    bool isNodeSafe(Ipv4Address m_node);
    void deleteAllRoutes(Ipv4Address m_node);
    /*
    add Path Manager class into attribute system in ns3
    */
    static TypeId GetTypeId (void);

    /*
      methods and typedef callbacks for building source trace in PathManager
     */
    uint32_t getNodeId();
    void notifyDetectSelfish(Ipv4Address m_node, bool);
    typedef void (* AddSelfishNodeTracedCallback) (const uint32_t node_id, const ns3::Time d_time,
    		const Ipv4Address selfishnode, bool add);
    //PathManager(ns3::aodv::RoutingTable& rt_ins, Ptr<Ipv4>& aodv_ipv4);
    void setComponentsFromAodv(ns3::aodv::RoutingTable* rt_ins, Ptr<Ipv4> aodv_ipv4);
    /// default c-tor for set TypeId
    PathManager();
    /*
    modified CONFIDANT methods


    */

};
}
}

#endif // _PATHMANAGER_H
