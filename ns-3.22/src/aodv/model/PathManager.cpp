#include "PathManager.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include <map>
#include <algorithm>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("CONFIDANT_PATHMANAGER");

namespace confidant
{

NS_OBJECT_ENSURE_REGISTERED (PathManager);


/*PathManager::PathManager(ns3::aodv::RoutingTable& rt_ins, Ptr<Ipv4>& aodv_ipv4):
    aodvRtable(rt_ins),
    m_ipv4(aodv_ipv4)
{
    //ifenableL = false;
}
*/

PathManager::PathManager()
{}

void
PathManager::setComponentsFromAodv(ns3::aodv::RoutingTable* rt_ins, Ptr<Ipv4> aodv_ipv4)
{
aodvRtable = rt_ins;
m_ipv4 = aodv_ipv4;
}

TypeId
PathManager::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::confidant::PathManager")
                        .SetParent<Object> ()
                        .SetGroupName ("CONFIDANT")
                        .AddConstructor<PathManager> ()
                        .AddTraceSource ("DetectedSelfishNode", "Add or remove new selfish node in list.",
                                         MakeTraceSourceAccessor (&PathManager::m_AddSelfishNodeTrace),
                                         "ns3::confidant::PathManager::AddSelfishNodeTracedCallback")
                        // etc (more parameters).
                        ;
    return tid;
}

void
PathManager::notifyDetectSelfish(Ipv4Address m_node, bool add)
{
    m_AddSelfishNodeTrace(getNodeId(), ns3::Simulator::Now(), m_node, add);
}

uint32_t
PathManager::getNodeId()
{
    return m_ipv4->GetObject<ns3::Node> ()->GetId ();
}


void
PathManager::addMnode(Ipv4Address m_node)
{
    if(std::find(m_list.begin(),m_list.end(),m_node)!= m_list.end())
        return;
    NS_LOG_DEBUG("["<<getNodeId()<< "] "<< "Add Selfish Node:" + ConvertIpToString(m_node));
    m_list.push_back(m_node);
    notifyDetectSelfish(m_node, true);
}

void
PathManager::rmMnode(Ipv4Address m_node)
{
    std::vector<Ipv4Address>::iterator position;
    position = std::find(m_list.begin(),m_list.end(),m_node);
    if(position != m_list.end())
    {
    	NS_LOG_DEBUG("["<<getNodeId()<< "] "<< "Remove Selfish Node:" + ConvertIpToString(m_node));
    	m_list.erase(position);
    	notifyDetectSelfish(m_node, false);
        return;
    }
}

bool
PathManager::isNodeSafe(Ipv4Address m_node)
{
    return (std::find(m_list.begin(),m_list.end(),m_node)!= m_list.end())? false:true;
}

void
PathManager::deleteAllRoutes(Ipv4Address m_node)
{
    std::map<Ipv4Address, uint32_t> unreachable;
    aodvRtable->GetListOfDestinationWithNextHop(m_node, unreachable);
    for(std::map<Ipv4Address, uint32_t>::iterator i = unreachable.begin(); i!= unreachable.end(); i++)
    {
        aodvRtable->DeleteRoute(i -> first);
    }
}

std::string ConvertIpToString(Ipv4Address ip)
{
	std::ostringstream oss;
	ip.Print(oss);
	return oss.str();
}

/* bool
 * PathManager::compRating(Ipv4Address pre, Ipv4Address next)
 * {
 *     return repSys.compRating(pre, next);
 * }
 */
/*
modified CONFIDANT methods


*/

}
}
