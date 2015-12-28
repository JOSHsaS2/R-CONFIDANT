#ifndef CONFIDANTPACKET_H
#define CONFIDANTPACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/ipv4-address.h"
#include <map>
#include <utility>

#define MULTIFACTOR 10000.0

namespace ns3
{
namespace confidant
{

/**
* \ingroup confidant
* \brief   public rating Message Format
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |       public rating ID        |   Hop Count   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Neighvour IP Address 1                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  first-hand rating 1                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Neighvour IP Address 2                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  rating 2                                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/

class PratingHeader : public Header
{
public:
    /// c-tor
    PratingHeader (uint16_t id = 0);

    // Header serialization/deserialization
    static TypeId GetTypeId ();
    TypeId GetInstanceTypeId () const;
    uint32_t GetSerializedSize () const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);
    void Print (std::ostream &os) const;

    // Fields
    void SetId (uint16_t id)
    {
        r_id = id;
    }
    uint16_t GetId () const
    {
        return r_id;
    }


    bool addRating (Ipv4Address neig, double, double);
    bool removeRating (std::pair<Ipv4Address, std::pair<double,double> >& nr);
    void clearRatings();
    uint8_t getRatingCount() const
    {
        return (uint8_t) prating_list.size();
    }

    bool operator== (PratingHeader const & o) const;
private:
    uint16_t        r_id;
    std::map<Ipv4Address, std::pair<double,double> > prating_list;
};

std::ostream & operator<< (std::ostream & os, PratingHeader const &);

/**
* \ingroup confidant
* \brief Alarm Message Format
  \verbatim

  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |  packet id (16 bit)     |    flag(8 bit)      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     selfish node IP address                   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     first hand rating                         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     sender IP address                         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  \endverbatim
*/
class AlarmHeader : public Header
{
public:
    /// c-tor
    AlarmHeader()
    {
    }
    AlarmHeader (uint16_t a_id, uint8_t flag =0, Ipv4Address m_dst = Ipv4Address (), Ipv4Address origin =
                     Ipv4Address (), double alpha = 1.0, double beta = 1.0);
    // Header serialization/deserialization
    static TypeId GetTypeId ();
    TypeId GetInstanceTypeId () const;
    uint32_t GetSerializedSize () const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);
    void Print (std::ostream &os) const;

    // Fields
    void SetAlarmId (uint16_t id)
    {
        a_id = id;
    }
    uint16_t GetAlarmId  () const
    {
        return a_id;
    }
    uint8_t GetFlag()
    {
        return broadcast_flag;
    }
    void SetFlag(uint8_t f)
    {
        broadcast_flag = f;
    }

    void SetDst (Ipv4Address a)
    {
        m_dst = a;
    }
    Ipv4Address GetDst () const
    {
        return m_dst;
    }
    void SetOrigin (Ipv4Address a)
    {
        m_origin = a;
    }
    Ipv4Address GetOrigin () const
    {
        return m_origin;
    }
    void SetAlpha(double a)
    {
        alpha = a;
    }
    double GetAlpha () const
    {
        return alpha;
    }
    void SetBeta(double b)
    {
        beta = b;
    }
    double GetBeta () const
    {
        return beta;
    }

    bool operator== (AlarmHeader const & o) const;
private:
    uint16_t       a_id;
    Ipv4Address    m_dst;              ///< Destination IP Address
    Ipv4Address    m_origin;           ///< Source IP Address
    double         alpha;
    double         beta;
    //tag 1 bit
    uint8_t broadcast_flag;
};

std::ostream & operator<< (std::ostream & os, AlarmHeader const &);
}
}
#endif /* CONFIDANTPACKET_H */
