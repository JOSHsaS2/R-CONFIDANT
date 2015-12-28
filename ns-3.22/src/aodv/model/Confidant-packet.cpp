#include "Confidant-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/assert.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("CONFIDANT_PACKET_HEADERS");

namespace confidant
{

PratingHeader::PratingHeader(uint16_t id):
    r_id(id)
{}

TypeId
PratingHeader::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::confidant::PratingHeader")
                        .SetParent<Header> ()
                        .AddConstructor<PratingHeader> ()
                        ;
    return tid;
}

TypeId
PratingHeader::GetInstanceTypeId () const
{
    return GetTypeId ();
}

uint32_t
PratingHeader::GetSerializedSize () const
{
    return (3 + 12 * getRatingCount());
}

void
PratingHeader::Serialize (Buffer::Iterator i ) const
{
    i.WriteU16 (r_id);
    i.WriteU8 (getRatingCount());
    std::map<Ipv4Address, std::pair<double,double> >::const_iterator j;
    for (j = prating_list.begin (); j != prating_list.end (); ++j)
    {
        WriteTo (i, (*j).first);
        i.WriteHtonU32 ((uint32_t)((*j).second).first * MULTIFACTOR);
        i.WriteHtonU32 ((uint32_t)((*j).second).second * MULTIFACTOR);
    }
}

uint32_t
PratingHeader::Deserialize (Buffer::Iterator start )
{
    Buffer::Iterator i = start;
    r_id = i.ReadU16 ();
    uint8_t dest = i.ReadU8 ();
    prating_list.clear ();
    Ipv4Address address;
    double alpha, beta;
    for (uint8_t k = 0; k < dest; ++k)
    {
        ReadFrom (i, address);
        alpha = i.ReadNtohU32 ()/MULTIFACTOR;
        beta = i.ReadNtohU32 ()/MULTIFACTOR;
        prating_list.insert (std::make_pair (address, std::make_pair(alpha,beta)));
    }

    uint32_t dist = i.GetDistanceFrom (start);
    NS_ASSERT (dist == GetSerializedSize ());
    return dist;
}

void
PratingHeader::Print (std::ostream &os ) const
{
    os << "Total number: " << getRatingCount() << std::endl << "Raing list (ipv4 address, double alpha, double beta):";
    std::map<Ipv4Address, std::pair<double,double> >::const_iterator j;
    for (j = prating_list.begin (); j != prating_list.end (); ++j)
    {
        os << (*j).first << ", " << (j->second).first <<", " <<(j->second).second << std::endl;
    }
}

bool
PratingHeader::addRating (Ipv4Address neig, double a, double b)
{
    if(prating_list.find(neig)!= prating_list.end())
        return false;
    std::pair<double,double> rating(a, b);
    prating_list.insert(std::make_pair(neig,rating));
    return true;
}

bool
PratingHeader::removeRating (std::pair<Ipv4Address, std::pair<double,double> >& nr)
{
    if(prating_list.empty())
        return false;
    std::map<Ipv4Address, std::pair<double,double> > ::iterator i = prating_list.begin();
    nr = *i;
    prating_list.erase(i);
    return true;
}

void
PratingHeader::clearRatings()
{
    prating_list.clear();
}

bool
PratingHeader::operator== (PratingHeader const & o) const
{
    if(o.r_id!=this->r_id || o.getRatingCount()!= getRatingCount())
        return false;
    std::map<Ipv4Address, std::pair<double,double> >::const_iterator i = o.prating_list.begin();
    std::map<Ipv4Address, std::pair<double,double> >::const_iterator j = prating_list.begin();
    for(int k =0; k< getRatingCount(); k++)
    {
        if(i->first != j->first || (j->second).first!= (i->second).first || (j->second).second!= (i->second).second)
            return false;
        i++;
        j++;
    }
    return true;
}

std::ostream &
operator<< (std::ostream & os, PratingHeader const & h)
{
    h.Print(os);
    return os;
}

AlarmHeader::AlarmHeader (uint16_t a_id, uint8_t flag, Ipv4Address m_dst, Ipv4Address origin, double alpha, double beta):
    a_id(a_id),
    m_dst(m_dst),
    m_origin(origin),
    alpha(alpha),
    beta(beta),
    broadcast_flag(flag)
{
}
// Header serialization/deserialization
TypeId
AlarmHeader::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::confidant::AlarmHeader")
                        .SetParent<Header> ()
                        .AddConstructor<AlarmHeader> ()
                        ;
    return tid;
}

TypeId
AlarmHeader::GetInstanceTypeId () const
{
    return GetTypeId ();
}

uint32_t
AlarmHeader::GetSerializedSize () const
{
    return 19;
}

void
AlarmHeader::Serialize (Buffer::Iterator i ) const
{
    i.WriteU16 (a_id);
    i.WriteU8(broadcast_flag);
    WriteTo(i, m_dst);
    i.WriteHtonU32((uint32_t)(alpha * MULTIFACTOR));
    i.WriteHtonU32 ((uint32_t)(beta * MULTIFACTOR));
    WriteTo(i, m_origin);
}

uint32_t
AlarmHeader::Deserialize (Buffer::Iterator start )
{
    Buffer::Iterator i = start;
    a_id = i.ReadU16 ();
    broadcast_flag = i.ReadU8();
    ReadFrom (i, m_dst);
    alpha = i.ReadNtohU32 ()/MULTIFACTOR;
    beta = i.ReadNtohU32 ()/MULTIFACTOR;
    ReadFrom (i, m_origin);
    uint32_t dist = i.GetDistanceFrom (start);
    NS_ASSERT (dist == GetSerializedSize ());
    return dist;
}

void
AlarmHeader::Print (std::ostream &os ) const
{
    os << "Alarm packet id: " << a_id << "from IP address: " << m_origin << std::endl <<"(Malicious node address, double alpha, double beta):" << std::endl;
    os << m_dst << ", " << alpha <<", " << beta;
    os << std::endl << "flag: " << broadcast_flag << std::endl;
}

bool
AlarmHeader::operator== (AlarmHeader const & o) const
{
    if(o.a_id!=this->a_id || o.broadcast_flag != this->broadcast_flag || o.m_dst != this->m_dst || o.m_origin != this->m_origin || o.alpha != alpha || o.beta!= beta)
        return false;
    else
        return true;
}

std::ostream &
operator<< (std::ostream & os, AlarmHeader const & h)
{
    h.Print(os);
    return os;
}


}

}


