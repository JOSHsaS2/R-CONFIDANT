#ifndef _MONITOR_H
#define _MONITOR_H
#include "ReputationSystem.h"
#include "Detector.h"
#include "ns3/net-device.h"
#include "ns3/address.h"


namespace ns3
{

namespace confidant
{

typedef std::map <uint64_t, PackForAck> PacketTable;
//using PacketTable = std::map <uint64_t, PackForAck>;

class Monitor: public Object
{

private:
    /*private fields*/
    //const ns3::aodv::RoutingProtocol& aodvRP;
    //IdCache m_alarmCache;

    PacketTable packQueue;
    Ptr<ReputationSystem> repuSys;///change

    double waitAckTimeout;
    bool ifenableL;
    Detector detectors;
    //ns3::aodv::IdCache m_publicCache;
    ns3::aodv::Neighbors* neig_l;///change

    //for source trace -- get node id form ipv4
    Ptr<Ipv4> m_ipv4;
    ns3::TracedCallback <uint32_t, ns3::Time,
        uint32_t> m_TableSizeChangeTrace;

    /*
     *l-confidant code:
     *add last firsthand info update table in monitor part
     */
    std::map<Ipv4Address, Timer> lastUpdate_t;

    /*private functions*/
    void deletePack(uint64_t uid);
    Timer initNewTimer(Ipv4Address ip, uint64_t uid);
    void cancelTimer(PackForAck&);
    void suspendTimer(PackForAck&);
    void resumeTimer(PackForAck&);
    void waitPackExpire(Ipv4Address ip, uint64_t uid);

    //L-CONFIDANT: LAST UPDATE TABLE
    void addItemFromLUT(Ipv4Address ip);
    Timer initNewTimerForLUT(Ipv4Address ip);
    void updateTimerFromLUT(Ipv4Address ip);
    bool ifExpireLUT(Ipv4Address ip);

public:
    /*
    use ipv4 to get macaddress of nexthop ip address before invoking registerPack(),
    to achieve this, add codes in function which could invoke the method, e.g. RouteInput() or
    RouteOutput() etc..
    */
    void registerPack(Ptr<const Packet> p, Address nextH, Ipv4Address dst, Ipv4Address next);

    /*
    preparation work is done!
    before set callback in wifinetdevice, check the protocol_tag user input
    */
    bool handlePassivePack(Ptr<NetDevice> nd, Ptr<const Packet> pack, uint16_t protN,
                           const Address & sender, const Address & receiver, ns3::NetDevice::PacketType packType);

    /*
    before invoked by AODV::RecvAodv(), add code in recvaodv() to set the parameters
    */
    void handlePublicRating(Ipv4Address from, Ptr<Packet> p);

    void registerDetecor(Detector& d)
    {
        detectors = d;
    }

    void deleteItemFromLUT(Ipv4Address ip);/*INVOKE THIS METHOD WHEN LINK BREAK*/
    bool findItemLUT(Ipv4Address ip);
    void setEableL(bool ifenable)
    {
        ifenableL = ifenable;
    }

    /*
    add Monitor class into attribute system in ns3
    */
    static TypeId GetTypeId (void);

    /*
      methods and typedef callbacks for building source trace in Monitor
     */
    uint32_t getNodeId();
    uint32_t getTableSize();
    void notifyTableSizeChange();
    typedef void (* TableSizeChangeTracedCallback) (const uint32_t node_id, const ns3::Time c_time, const uint32_t n_size);

    //Monitor(ReputationSystem& r, ns3::aodv::Neighbors& neig, double wtime, Ptr<Ipv4>& aodv_ipv4);
    void setReputationSystem(Ptr<ReputationSystem> repu);
    void setComponentsFromAodv(ns3::aodv::Neighbors* neig, Ptr<Ipv4> aodv_ipv4);
    /// default c-tor for set TypeId
    Monitor();


};



}
}
#endif // _MONITOR_H
