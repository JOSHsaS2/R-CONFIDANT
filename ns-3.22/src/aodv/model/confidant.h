#ifndef _CONFIDANT_H
#define _CONFIDANT_H
//Final Common header files for all components in CONFIDANT
#include "ns3/timer.h"
#include "ns3/mac48-address.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include <map>
#include <utility>
#include <sstream>


/*CONSTANTS FOR MONITOR*/
#define PACK_TIMEOUT 0.1
#define LASTUPDATE_TIMEOUT 60//? need change reputation redemption, important factor which can influence packet drop rate
//and selfish node detection accuracy.

/*CONSTANTS FOR REPUTATION SYSTEM*/
#define FADING_U 0.9
#define DEVIATION_T 0.5
#define REGULAR_T 0.75
#define SECONDHAND_WEIGHT 0.1
#define PUBLICATION_TIMEOUT 10
#define FADING_TIMEOUT 10
#define D_ALPHA 1.0
#define D_BETA 1.0
#define SECONDARY_RESPONSE 0.75
#define HOPCOUNT_LIMIT 2 //important factor which can influence packet drop rate and selfish node detection accuracy.

//threshold for reputation fading when during inactivity period without enough second-hand info
#define FADING_T 0.5//? need change reputation redemption

/*CONSTANTS FOR TRUST MANAGER*/
#define FADING_V 0.9
#define TRUSTWORTHY_T 0.25

/*CONSTANTS FOR THRESHOLD ABOUT DELETING ITEMS IN REPUTATION TABLE*/
#define LO_T 0.50 //adaptive factors. NEW!

namespace ns3
{

namespace confidant
{

template <typename T>
std::string NumberToString ( T Number )
{
	std::stringstream ss;
	ss << Number;
	return ss.str();
}


enum BEHAVIOR
{
    regular,
    misbehave,
};

enum DEVIATIONTEST
{
    compatiable,
    noncompatiable,
};

class PackForAck
{

private:
    Ptr<const Packet> p;
    Timer timeout;
    Address macOfNextHop;
    Ipv4Address ipOfNextHop;
public:
    void cancelTimer();
    void suspendTimer();
    void resumeTimer();
    Ptr<const Packet> getPacket()
    {
        return p;
    }
    const Address getMacOfNextHop()
    {
        return macOfNextHop;
    }
    const Ipv4Address getIpOfNextHop()
    {
        return ipOfNextHop;
    }
    Timer& getTimer(){
    	return timeout;
    }
    PackForAck(Ptr<const Packet> pack, Address mach, Ipv4Address iph);
    //~PackForAck();
};

class Rating
{
protected:
    double alpha;
    double beta;
    double f_factor;

public:

    Rating(double fading_f):
        f_factor(fading_f)
    {
       alpha = D_ALPHA;
    beta = D_BETA;
    }

    //fields
    void fadeRating();
    void setFadingFactor(double f)
    {
        f_factor = f;
    }

    double getAlpha()
    {
        return alpha;
    }

    double getBeta()
    {
        return beta;
    }

    double bayeMean();
    virtual ~Rating()
    {}

};

class Repu_Rating: public Rating
{

private:
    double threshold; // threshold r for classify regular or selfish, which can be decreased by behavious
public:
    double getThreshold();
    void setThreshold(double thres);
    Repu_Rating(double thres, double fading_f);
    void updateByOb(BEHAVIOR behavior);
    void updateReputation(double salpha,double sbeta, double w);
    //void updateReputation(Rating* p_rating, double w);
};

class F_Rating : public Rating
{
private:
    Timer f_timer;
    Repu_Rating* repu;
    Callback<bool, Ipv4Address, F_Rating*, Repu_Rating*> classifyNode;
public:
    F_Rating(double fading_f, Ipv4Address f_ip);

    //fields
    Repu_Rating* getRepu();
    void setRepu(Repu_Rating* r);
    void cancelTimer();
    void reScheduleTimer();
    void updateByOb(BEHAVIOR behavior);
    void handleFadingTimeoutExpire(Ipv4Address f_ip);
    void setClassifyCallback(Callback<bool, Ipv4Address, F_Rating*, Repu_Rating*> cb);
};


class Tru_Rating: public Rating
{
private:
    Timer f_timer;
public:
    Tru_Rating(double fading_f);
    void cancelTimer();
    void reScheduleTimer();
    void handleFadingTimeoutExpire();
    void updateByOb(DEVIATIONTEST result);
};

//using RatingTable = std::map <Ipv4Address, Rating*>;
typedef std::map <Ipv4Address, Rating*> RatingTable;

}
}
#endif // _CONFIDANT_H
