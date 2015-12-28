#ifndef _REPUTATIONSYSTEM_H
#define _REPUTATIONSYSTEM_H
#include "TrustManager.h"
#include "PathManager.h"
#include "Confidant-packet.h"
#include <math.h>
#include "ns3/socket.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/aodv-id-cache.h"
#include "ns3/aodv-neighbor.h"
#include "ns3/aodv-rtable.h"
#include "ns3/aodv-packet.h"
#include "ns3/aodv-rqueue.h"
#include "ns3/aodv-neighbor.h"
#include "ns3/tag.h"
#include "ns3/tag-buffer.h"


namespace ns3
{
namespace confidant
{

class ReputationSystem: public Object
{

private:
    /*private data member*/
    //double bayeMean(double alpha, double beta);
    //void fading(Rating& ratingForIp);
    // change it to the prpameter of member functions of CONFIDANT
    //ns3::aodv::RoutingProtocol& aodvRP;

    RatingTable firsthandInfo_t;
    RatingTable reputation_t;
    double u,d,r,w,f_timeout, p_timeout;
    Ptr<TrustManager> trust_m;///change
    Ptr<PathManager> path_m;///change
    Timer public_t;
    bool ifenableL;

    //switch for the trade-off between detection and storage usage
    bool L_TRADEOFF;

    //id counter for second-hand infor and alarms
    uint16_t m_publicid;
    uint16_t m_alarmid;

    //callbacks from function of aodv instance to avoid reference to aodv::routingprotocol
    Callback<void, Ptr<Socket>, Ptr<Packet>, Ipv4Address> callbackSendto;
    std::map <Ptr<Socket>, Ipv4InterfaceAddress>* m_socketAddresses;///change
    Ptr< UniformRandomVariable > m_uniformRandomVariable;
    ns3::aodv::Neighbors* m_neighbour;///change
    Callback<Ptr<Socket>, Ipv4InterfaceAddress> callbackFindSocketIp;
    Callback<void, Ipv4Address> callbackSendRerrWhenBreaksLinkToNextHop;
    Callback<void, Ipv4Address, uint32_t, Ipv4Address> callbackSendRerrWhenNoRouteToForward;
    Callback<void, Ipv4Address> callbackSendRequest;
    Callback<void, bool> callbackIsNeighborFlag;

    //callback for access last update table in Monitor: find item!
    Callback<bool, Ipv4Address> callbackLUT;

    //duplicate check
    ns3::aodv::IdCache m_alarmCache;

    //for source trace -- get node id form ipv4
    Ptr<Ipv4> m_ipv4;///change

    //trace source for fist-hand information table
    ns3::TracedCallback <uint32_t, ns3::Time,
        uint32_t> mF_TableSizeChangeTrace;

    //trace source for reputation ratings table
    ns3::TracedCallback <uint32_t, ns3::Time,
        uint32_t> mR_TableSizeChangeTrace;

    /*private functions*/
    bool passDeviationTest(Rating& repu, Rating& second);
    bool passDeviationTest(Rating& repu, double alpha, double beta);
    void exchangeFirsthandInfo(/*RatingTable& ratings*/); //duplicated packets check
    void updateByob(Rating& ratingForIp, BEHAVIOR b);
    void updateReputation(Rating& r, double a, double b, double);
    Rating* initNewR_Rating(double fading, double r_t);
    Rating* initNewF_Rating(double fading, Ipv4Address);
    void decreaseR(Rating& ratingForIp);
    void cancelTimer(Rating& r);
    void rescheduleTimer(Rating& r);
    void handlePublicExpire();
    void checkClassification(Ipv4Address targer_ip);
    void SendTo(Ptr<Socket>, Ptr<Packet>, Ipv4Address);

    //encapsulate the logic of how to use second-hand and alarm in L-CONFIDANT
    void handlePublicRatingL(Ipv4Address ip,RatingTable::iterator item_r, double a, double b);

    /*functions for update routingtable when receiving RREP*/
    int compRating(Ipv4Address pre, Ipv4Address next);
    uint8_t getPriority(Ipv4Address);
    int compPriority(Ipv4Address pre, Ipv4Address newEntry);

    //callback function for set ErrorCallback for QueueEntry parameter.
    void handleErrorForAlarmWithoutRoute(Ptr<const Packet>, const Ipv4Header &, Socket::SocketErrno);

    /*public interfaces*/
public:
    ns3::aodv::IdCache m_publicCache;
    bool isMalicious(Ipv4Address target);
    void handleFirsthandInfo(bool, Ipv4Address neighbor, BEHAVIOR behavior);
    void handleSecondhandInfo(ns3::aodv::Neighbors& neighbors, Ipv4Address from, std::map<Ipv4Address, std::pair<double,double> >&);

    /*functions for update routingtable when receiving RREP*/
    void updateRouteTableFromRREPC(ns3::aodv::RoutingTable& m_routingTable, ns3::aodv::RrepHeader& header, ns3::aodv::RoutingTableEntry& toDst, ns3::aodv::RoutingTableEntry& newEntry, Ipv4Address sender);
    void updateRouteTableFromRREPL(ns3::aodv::RoutingTable& m_routingTable, ns3::aodv::RrepHeader& header, ns3::aodv::RoutingTableEntry& toDst, ns3::aodv::RoutingTableEntry& newEntry, Ipv4Address sender);

    /*set fields*/
    void setPublicTimer();
    void setWeight(double nWeight);
    void setEableL(bool ifenable);

    /*set callbacks*/
    void setCallbackSendTo(Callback<void, Ptr<Socket>, Ptr<Packet>, Ipv4Address> cb);
    void setCallbackFindSocketIp(Callback<Ptr<Socket>, Ipv4InterfaceAddress> cb);
    void setCallbackSendRerrWhenBreaksLinkToNextHop(Callback<void, Ipv4Address> cb);
    void setCallbackSendRerrWhenNoRouteToForward(Callback<void, Ipv4Address, uint32_t, Ipv4Address> cb);
    void setcallbackSendRequest(Callback<void, Ipv4Address> cb);
    void setcallbackNeighborFlag(Callback<void, bool> cb);

    /* modified CONFIDANT methods */
    ///implement in .cc file
    void setComponentsFromAodv(ns3::aodv::Neighbors* neighbors,
    Ptr<Ipv4> aodv_ipv4, Ptr<UniformRandomVariable> unRV,
    std::map<Ptr<Socket>, Ipv4InterfaceAddress>* m_sAdd);

    void setComponentsFromConfidant(Ptr<TrustManager> tm, Ptr<PathManager> pm);

    void SendAlarm(Ipv4Address selfish_ip, ns3::aodv::RoutingTable& m_routingTable, ns3::aodv::RequestQueue& m_queue, const uint32_t port);
    void RecvAlarm(Ptr<Packet> alarm, ns3::aodv::Neighbors& neighbors, ns3::aodv::RoutingTable& m_routingTable, const uint32_t port);
    void SendAlarmFromQueue(Ptr<Packet> alarm, const ns3::aodv::RoutingTableEntry& rt, uint32_t port);
    void rmFirsthandItem(Ipv4Address);
    void rmReputationItem(Ipv4Address);
    std::pair<RatingTable::iterator, bool> addFirsthandItem(Ipv4Address);
    std::pair<RatingTable::iterator, bool> addReputationItem(Ipv4Address);
    bool findReputationItem(Ipv4Address ip);
    bool findFirsthandItem(Ipv4Address ip);
    double reputationOfIp(Ipv4Address ip);

    void setTradeoffSwitch(bool t);
    bool getTradeoffSwitch();

    void setCallbackLUT(Callback<bool, Ipv4Address> cb);
    void handleReputationFading(Ipv4Address, F_Rating*, Repu_Rating*);
    /* modified CONFIDANT methods -- end*/

    /*
     add Reputation System class into attribute system in ns3
     */
    static TypeId GetTypeId (void);

    /*
      methods and typedef callbacks for building source trace in Monitor
     */
    uint32_t getNodeId();
    uint32_t getTableSize_F();
    uint32_t getTableSize_R();
    void notifyTableSizeChange_F();
    void notifyTableSizeChange_R();
    typedef void (* FTableSizeChangeTracedCallback) (const uint32_t node_id, const ns3::Time c_time, const uint32_t n_size);
    typedef void (* RTableSizeChangeTracedCallback) (const uint32_t node_id, const ns3::Time c_time, const uint32_t n_size);

    /// default c-tor for set TypeId
    ReputationSystem();
    ~ReputationSystem();

};

class DeferredAlarmTag: public Tag
{
public:
    DeferredAlarmTag () : Tag ()
    {
    }

        static TypeId GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::confidant::DeferredAlarmTag")
                            .SetParent<Tag> ().SetParent<Tag>()
                            .AddConstructor<DeferredAlarmTag> ()
                            ;
        return tid;
    }

    TypeId  GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    uint32_t GetSerializedSize () const
    {
       return 0;
    }

    void  Serialize (TagBuffer i) const
    {
    }

    void  Deserialize (TagBuffer i)
    {
    }

    void  Print (std::ostream &os) const
    {
        os << "DeferredAlarmTag";
    }
};


}
}


#endif // _REPUTATIONSYSTEM_H
