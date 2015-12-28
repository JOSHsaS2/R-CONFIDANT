#ifndef _DETECTOR_H
#define _DETECTOR_H
#include "confidant.h"

namespace ns3
{
namespace confidant
{
class Detector
{
    virtual bool detector(Ptr<Packet> promis, Ptr<Packet> pqueue, const Mac48Address& mac, const Address & sender, const Address & receiver);
};
}
}
#endif // _DETECTOR_H
