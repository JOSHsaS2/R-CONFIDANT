#include <list>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <ctime>

#include "ns3/simulator.h"
#include "ns3/log.h"

#include "RankBuffer.h"
#include "raodv-rtable.h"

using namespace std;

namespace ns3{
namespace raodv{

RankBuffer::RankBuffer() {}
RankBuffer::~RankBuffer() {}

//ACCESSORS
unsigned int RankBuffer::
search_pos(Ipv4Address input) {
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		if((*iter).first == input) {
			return i;
		}
		iter++;
	}
	return -1;
}

RoutingTable* RankBuffer::
search(Ipv4Address destination) {
	
	// TODO: SEARCH THROUGH RB FROM TOP AND RETURN FIRST FOUND APPLICABLE ROUTE
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
  RoutingTableEntry temp_entry;
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		 RoutingTable* temp_rt = (*iter).second;
		 if (temp_rt->LookupRoute(destination, temp_entry)){
			 return temp_rt;
		 }
		iter++;
	}
	return NULL;
}

//SETTERS
void RankBuffer::		// NOTE: THIS FUNCTION MAY BE IRRELEVANT IF THE AddRoute() FUNCTION WORKS FINE
add_new(Ipv4Address input_name,  RoutingTable* rtable_ptr) {		// adds item into first place if container is empty, else, adds into 2nd place

	// TODO:: CHECK IF TABLE EXISTS FIRST. ELSE DO THE BELOW

	pair<Ipv4Address, RoutingTable*> entry = make_pair(input_name, rtable_ptr);

	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	if(p_ranked_list.empty()) {
		// cerr << "list is empty" << endl;
		p_ranked_list.insert(iter, entry);
	} else {
		p_ranked_list.insert(iter, entry);

		iter++;
	}
}

void RankBuffer::
deRank(Ipv4Address target) {	// forces target to the bottom of the ranking
	pair<Ipv4Address, RoutingTable*> entry;
	// search for target (1-hop neighbour)
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	while( iter != p_ranked_list.end() ) {
		// cerr << "scanning " << iter->first
		if( iter->first == target ) {
			// entry.first = (*iter).first;
			// entry.second = (*iter).second;
			break;
		}

		iter++;
	}

	p_ranked_list.splice(p_ranked_list.end(), p_ranked_list, iter);		// cuts out item and places at at end (no invalidation errors)
	// p_ranked_list.remove(*iter);
	// p_ranked_list.insert(p_ranked_list.end(), entry);
}

//DEBUG
void RankBuffer::
printRankings() {
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		cerr << i++ << " \t" << (*iter).first << " \t" << (*iter).second << endl;

		iter++;
	}
}

//============================================================================== REDIRECTS
/* 		THE FOLLOWING SET OF FUNCTIONS MOVE THROUGH 
			THE RANKBUFFER AND REDIRECTS THE CALL TO 
			EACH ROUTINGTABLE
			*/

bool RankBuffer::
LookupRoute (Ipv4Address dst, RoutingTableEntry & rt) {
	bool result = false;
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		result = (*iter).second->LookupRoute(dst, rt);
		if(result == true) {
			return result;
		}

		iter++;
	}
	return result;
}

bool RankBuffer::
LookupValidRoute (Ipv4Address dst, RoutingTableEntry & rt) {
	bool result = false;
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		result = (*iter).second->LookupValidRoute(dst, rt);
		if(result == true) {
			return result;
		}

		iter++;
	}
	return result;
}
//===========================================
bool RankBuffer::
DeleteRoute (Ipv4Address dst) {
	bool result = false;
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		result = (*iter).second->DeleteRoute(dst);
		if(result == true) {
			return result;
		}

		iter++;
	}
	return result;
}

bool RankBuffer::
AddRoute (RoutingTableEntry & r) {

	//bool result = false;
	// RoutingTable* RT_to_use;

	//TODO: search for matching 1st-Hop, then redirect to paired rtable
	// Ipv4Address route_head = r.GetNextHop ();

	/* 
	1. Search p_ranked_list for route_head;

	2. if not found, create a new pair<Ipv4Address, RoutingTable*>
		-	RT_to_use  <--  make sure the Routing Table is dynamically created with "new"

	3. if the RankBuffer entry is found, 
		- RT_to_use  <--  set to the found entry's RoutingTable* 
	*/

	//REDIRECT AddRoute() TO THE DEFINED ROUTING TABLE
	// result = RT_to_use->AddRoute(r);

	Time DeleteTime (Time (50*4));//we must set delete timer for each routing table entry 
	RoutingTable* RT_to_use = new RoutingTable(DeleteTime);
	RT_to_use->AddRoute(r);
	Ipv4Address route_head = r.GetNextHop ();

	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		if((*iter).first == route_head){
			return (*iter).second->AddRoute(r);
		}
		iter++;
	}
	
	std::pair<Ipv4Address, RoutingTable*> temp;
	temp = std::make_pair (route_head,RT_to_use);
	p_ranked_list.push_back(temp);
	return true;
}

bool RankBuffer::
Update (RoutingTableEntry & rt) {
	bool result = false;
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		result = (*iter).second->Update(rt);
		if(result == true) {
			return result;
		}

		iter++;
	}
	return result;
}
//===========================================
void RankBuffer::
GetListOfDestinationWithNextHop (Ipv4Address nextHop, std::map<Ipv4Address, uint32_t> & unreachable) {
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		(*iter).second->GetListOfDestinationWithNextHop(nextHop, unreachable);

		iter++;
	}
}

void RankBuffer::
InvalidateRoutesWithDst (Ipv4Address abstract_src, std::map<Ipv4Address, uint32_t> const & unreachable) {
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		//mod 
		if((*iter).first == abstract_src) {
			(*iter).second->InvalidateRoutesWithDst(unreachable);
		}

		iter++;
	}
}
//===========================================
bool RankBuffer::
MarkLinkAsUnidirectional (Ipv4Address neighbor, Time blacklistTimeout) {
	bool result = false;
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		result = (*iter).second->MarkLinkAsUnidirectional(neighbor, blacklistTimeout);
		if(result == true) {
			return result;
		}

		iter++;
	}
	return result;
}

//===========================================
void RankBuffer::
DeleteAllRoutesFromInterface (Ipv4InterfaceAddress iface) {
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		(*iter).second->DeleteAllRoutesFromInterface(iface);
		iter++;
	}
}

//===========================================
void RankBuffer::
Clear() {
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		(*iter).second->Clear();
		iter++;
	}
}

void RankBuffer::
Purge() {
	list< pair<Ipv4Address, RoutingTable*> >::iterator iter = p_ranked_list.begin();
	for(unsigned int i=0; i<p_ranked_list.size(); i++) {
		(*iter).second->Purge();
		iter++;
	}
}

}
}
