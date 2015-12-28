#include "TrustManager.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("CONFIDANT_TRUSTMANAGER");

namespace confidant
{

NS_OBJECT_ENSURE_REGISTERED (TrustManager);


/*TrustManager::TrustManager(Ptr<Ipv4>& aodv_ipv4):
    m_ipv4(aodv_ipv4)
{
    t = TRUSTWORTHY_T;
    v = FADING_V;
    f_timeout = FADING_TIMEOUT;
    //ifenableL = false;
}
*/

TrustManager::TrustManager()
{
  t = TRUSTWORTHY_T;
    v = FADING_V;
    f_timeout = FADING_TIMEOUT;
    //ifenableL = false;
}

TrustManager::~TrustManager()
{
for(RatingTable::iterator i = trust_t.begin(); i!= trust_t.end(); i++)
delete (i->second);
}
void
TrustManager::setComponentsFromAodv(Ptr<Ipv4> aodv_ipv4)
{
m_ipv4 = aodv_ipv4;
}

TypeId
TrustManager::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::confidant::TrustManager")
                        .SetParent<Object> ()
                        .SetGroupName ("CONFIDANT")
                        .AddConstructor<TrustManager> ()
                        .AddTraceSource ("TrustTalbleSizeChange", "New item added into Trust table.",
                                         MakeTraceSourceAccessor (&TrustManager::m_TableSizeChangeTrace),
                                         "ns3::confidant::TrustManager::TableSizeChangeTracedCallback")
                        // etc (more parameters).
                        ;
    return tid;
}

uint32_t
TrustManager::getNodeId()
{
    return m_ipv4->GetObject<ns3::Node> ()->GetId ();
}

uint32_t
TrustManager::getTableSize()
{
    return trust_t.size();
}

void
TrustManager::notifyTableSizeChange()
{
    m_TableSizeChangeTrace(getNodeId(), ns3::Simulator::Now(), getTableSize());
}

Rating*
TrustManager::initNewTrust(double fading)
{
    Tru_Rating* r_p = new Tru_Rating(fading);
    return r_p;
}

void
TrustManager::cancelTimer(Rating& r)
{
    (dynamic_cast<Tru_Rating&>(r)).cancelTimer();
}

void
TrustManager::rescheduleTimer(Rating& r)
{
    (dynamic_cast<Tru_Rating&>(r)).reScheduleTimer();
}

void
TrustManager::updateTrust(Ipv4Address neigh, DEVIATIONTEST result)
{
    RatingTable::iterator i = trust_t.find(neigh);
    if(i == trust_t.end())
    {
        i = (trust_t.insert(std::pair<Ipv4Address, Rating*>(neigh, initNewTrust(v)))).first;
        notifyTableSizeChange();
    }
    cancelTimer(*(i->second));
    (dynamic_cast<Tru_Rating*>(i->second)) -> updateByOb(result);
    rescheduleTimer(*((*i).second));
}

bool
TrustManager::isTrustworthy(Ipv4Address neigh)
{
    RatingTable::iterator i = trust_t.find(neigh);
    if(i == trust_t.end())
    {
        return false;
    }
    double result = ((*i).second) -> bayeMean();
    return (result < t)? true: false;
}

}
}
