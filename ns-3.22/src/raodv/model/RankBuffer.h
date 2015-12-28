#ifndef RANKBUFFER_H
#define RANKBUFFER_H

#include <list>
#include <string>
#include <utility>
#include <map>
#include <sys/types.h>

#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/timer.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"

#include "raodv-rtable.h"


using namespace std;

namespace ns3{
namespace raodv{

// typedef string Ipv4Address;
// typedef struct {
// 	Ipv4Address route;
// } RoutingTable;

class RankBuffer {
private:
	list< pair<Ipv4Address, RoutingTable*> > p_ranked_list;
public:
	RankBuffer();
	~RankBuffer();

	//ACCESSORS
	unsigned int search_pos(Ipv4Address input);
	RoutingTable* search(Ipv4Address destination);

	//SETTERS
	void add_new(Ipv4Address input_name, RoutingTable* rtable_ptr);
	void deRank(Ipv4Address target);

	//DEBUG
	void printRankings();


	/// REDIRECT TO RTABLES
	bool LookupRoute (Ipv4Address dst, RoutingTableEntry & rt);
  bool LookupValidRoute (Ipv4Address dst, RoutingTableEntry & rt);
  bool Update (RoutingTableEntry & rt);
  bool AddRoute (RoutingTableEntry & r);
  bool DeleteRoute (Ipv4Address dst);
  void GetListOfDestinationWithNextHop (Ipv4Address nextHop, std::map<Ipv4Address, uint32_t> & unreachable);
  void InvalidateRoutesWithDst (Ipv4Address abstract_src, std::map<Ipv4Address, uint32_t> const & unreachable);	//mod 
  bool MarkLinkAsUnidirectional (Ipv4Address neighbor, Time blacklistTimeout);

  void DeleteAllRoutesFromInterface (Ipv4InterfaceAddress iface);

  void Clear();
  void Purge();
};

}
}
#endif