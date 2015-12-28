#ifndef _TRUSTMANAGER_H
#define _TRUSTMANAGER_H
#include "confidant.h"
#include "ns3/ipv4.h"
#include "ns3/nstime.h"
#include "ns3/node.h"
#include "ns3/traced-callback.h"


namespace ns3
{
namespace confidant
{

class TrustManager: public Object
{

private:
    //bool ifenableL;

    RatingTable trust_t;
    double t,v,f_timeout;
    Rating* initNewTrust(double fading);
    void cancelTimer(Rating& r);
    void rescheduleTimer(Rating& r);
    //for source trace -- get node id form ipv4
    Ptr<Ipv4> m_ipv4;
    ns3::TracedCallback <uint32_t, ns3::Time, uint32_t> m_TableSizeChangeTrace;

public:
    void updateTrust(Ipv4Address neigh, DEVIATIONTEST result);
    bool isTrustworthy(Ipv4Address neigh);
    void setT(double nt)
    {
        t = nt;
    }
    void setEableL(bool ifenable)
    {
        //ifenableL = ifenable;
    }

    /*
    add Trust manager class into attribute system in ns3
    */
    static TypeId GetTypeId (void);
    /*
     methods and typedef callbacks for building source trace in TrustManager
    */
    uint32_t getNodeId();
    uint32_t getTableSize();
    void notifyTableSizeChange();
    typedef void (* TableSizeChangeTracedCallback) (const uint32_t node_id, const ns3::Time c_time, const uint32_t n_size);

    /// default c-tor for set TypeId
    //TrustManager(Ptr<Ipv4>& aodv_ipv4);
    void setComponentsFromAodv(Ptr<Ipv4> aodv_ipv4);

    TrustManager();
    ~TrustManager();
    /*
    modified CONFIDANT methods

    */



};

}
}

#endif // _TRUSTMANAGER_H
