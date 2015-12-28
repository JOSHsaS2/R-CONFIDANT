#include "ReputationSystem.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <limits>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("CONFIDANT_REPUTATIONSYSTEM");

namespace confidant {

NS_OBJECT_ENSURE_REGISTERED(ReputationSystem);

ReputationSystem::ReputationSystem() :
		public_t(ns3::Timer::CANCEL_ON_DESTROY), m_alarmCache(
				ns3::Seconds(PUBLICATION_TIMEOUT)), m_publicCache(
				ns3::Seconds(PUBLICATION_TIMEOUT)) {
	m_publicid = 0;
	m_alarmid = 0;
	u = FADING_U;
	d = DEVIATION_T;
	r = REGULAR_T;
	w = SECONDHAND_WEIGHT;
	f_timeout = FADING_TIMEOUT;
	p_timeout = PUBLICATION_TIMEOUT;
	ifenableL = false;
	L_TRADEOFF = false;
}

ReputationSystem::~ReputationSystem() {

	for (RatingTable::iterator i = reputation_t.begin();
			i != reputation_t.end(); i++)
		delete (i->second);
	for (RatingTable::iterator i = firsthandInfo_t.begin();
			i != firsthandInfo_t.end(); i++)
		delete (i->second);

}

TypeId ReputationSystem::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::confidant::ReputationSystem").SetParent<Object>().SetGroupName(
					"CONFIDANT").AddConstructor<ReputationSystem>().AddTraceSource(
					"FTableSizeChange",
					"size of First-hand information table has changed.",
					MakeTraceSourceAccessor(
							&ReputationSystem::mF_TableSizeChangeTrace),
					"ns3::confidant::ReputationSystem::FTableSizeChangeTracedCallback").AddTraceSource(
					"RTableSizeChange",
					"size of Reputation ratings table has changed.",
					MakeTraceSourceAccessor(
							&ReputationSystem::mR_TableSizeChangeTrace),
					"ns3::confidant::ReputationSystem::RTableSizeChangeTracedCallback")
					// etc (more parameters).
					;
	return tid;
}

void ReputationSystem::setComponentsFromAodv(ns3::aodv::Neighbors* neighbors,
		Ptr<Ipv4> aodv_ipv4, Ptr<UniformRandomVariable> unRV,
		std::map<Ptr<Socket>, Ipv4InterfaceAddress>* m_sAdd) {
	m_socketAddresses = m_sAdd;
	m_uniformRandomVariable = unRV;
	m_neighbour = neighbors;
	m_ipv4 = aodv_ipv4;
}

void ReputationSystem::setComponentsFromConfidant(Ptr<TrustManager> tm,
		Ptr<PathManager> pm) {
	trust_m = tm;
	path_m = pm;
}

uint32_t ReputationSystem::getNodeId() {
	return m_ipv4->GetObject<ns3::Node>()->GetId();
}

uint32_t ReputationSystem::getTableSize_F() {
	return firsthandInfo_t.size();
}

uint32_t ReputationSystem::getTableSize_R() {
	return reputation_t.size();
}

void ReputationSystem::notifyTableSizeChange_F() {
	NS_LOG_FUNCTION("["<<getNodeId()<< "] "<< this);
	mF_TableSizeChangeTrace(getNodeId(), ns3::Simulator::Now(),
			getTableSize_F());
}

void ReputationSystem::notifyTableSizeChange_R() {
	mR_TableSizeChangeTrace(getNodeId(), ns3::Simulator::Now(),
			getTableSize_R());
}

bool ReputationSystem::passDeviationTest(Rating& repu, Rating& second) {
	double deviation = fabs(repu.bayeMean() - second.bayeMean());
	return deviation < d ? true : false;
}

bool ReputationSystem::passDeviationTest(Rating& repu, double alpha,
		double beta) {
	double deviation = fabs(repu.bayeMean() - alpha / (alpha + beta));
	return deviation < d ? true : false;
}

bool ReputationSystem::isMalicious(Ipv4Address target) {
	RatingTable::iterator i = reputation_t.find(target);
	if (i == reputation_t.end()) {
		return false;
	}
	double result = (i->second)->bayeMean();
	//return (result > this->r) ? true : false;
	return (result >= (dynamic_cast<Repu_Rating*>(i->second))->getThreshold()) ? true : false;
}

void ReputationSystem::updateByob(Rating& ratingForIp, BEHAVIOR b) {
	//((F_Rating)ratingForIp).updateByob(b);
	(dynamic_cast<F_Rating&>(ratingForIp)).updateByOb(b);
}

void ReputationSystem::updateReputation(Rating& r, double a, double b,
		double weight) {
	//((Repu_Rating)r).updateReputation(a, b, this.w);
	(dynamic_cast<Repu_Rating&>(r)).updateReputation(a, b, weight);
}

Rating*
ReputationSystem::initNewR_Rating(double fading, double r_t) {
	return new Repu_Rating(r_t, fading);
}

Rating*
ReputationSystem::initNewF_Rating(double fading, Ipv4Address ip) {
	return new F_Rating(fading, ip);
}

void ReputationSystem::cancelTimer(Rating& r) {
	(dynamic_cast<F_Rating&>(r)).cancelTimer();
}

void ReputationSystem::rescheduleTimer(Rating& r) {
	(dynamic_cast<F_Rating&>(r)).reScheduleTimer();
}

void ReputationSystem::handleFirsthandInfo(bool isNeighbor,
		Ipv4Address neighbor, BEHAVIOR behavior) {
	NS_LOG_FUNCTION(
			"["<<getNodeId()<< "] "<< this<< isNeighbor << neighbor << behavior);
	if (ifenableL && !isNeighbor) {
		NS_LOG_LOGIC(
				getNodeId()<< ", Ignore first-hand infor due to non neighbor");
		return;
	}
	if (firsthandInfo_t.find(neighbor) == firsthandInfo_t.end()) {
		std::pair<RatingTable::iterator, bool> i_first;
		std::pair<RatingTable::iterator, bool> i_repu;
		RatingTable::iterator item_repu = reputation_t.find(neighbor);
		//i_first = firsthandInfo_t.insert(std::pair<Ipv4Address, Rating*>(neighbor, initNewF_Rating(this.u, neighbor)));
		i_first = addFirsthandItem(neighbor);
		(dynamic_cast<F_Rating*>((i_first.first)->second))->setClassifyCallback(
				MakeCallback(&ReputationSystem::handleReputationFading, this));

		//IMPORTANT!! always check whether the item of first-hand or reputation rating exist or not
		if (item_repu == reputation_t.end()) {
			//i_repu = reputation_t.insert(std::pair<Ipv4Address, Rating*>(neighbor, initNewR_Rating(u, r)));
			i_repu = addReputationItem(neighbor);
			(dynamic_cast<F_Rating*>((i_first.first)->second))->setRepu(
					dynamic_cast<Repu_Rating*>((i_repu.first)->second));
		} else {
			(dynamic_cast<F_Rating*>((i_first.first)->second))->setRepu(
					dynamic_cast<Repu_Rating*>(item_repu->second));
		}
		updateByob(*((i_first.first)->second), behavior);
		checkClassification(neighbor);
		//updateByob(*((i_repu.first)->second), behavior);
		return;
	}
	RatingTable::iterator it_f, it_r;
	it_f = firsthandInfo_t.find(neighbor);
	//it_r = reputation_t.find(neighbor);
	cancelTimer(*(it_f->second));
	updateByob(*(it_f->second), behavior);
	checkClassification(neighbor);
	// updateByob(*(it_r -> second), behavior);
	if (findFirsthandItem(neighbor))
		rescheduleTimer(*(it_f->second));
}

void ReputationSystem::decreaseR(Rating& ratingForIp) {
	double c_thres = (dynamic_cast<Repu_Rating&>(ratingForIp)).getThreshold();
	(dynamic_cast<Repu_Rating&>(ratingForIp)).setThreshold(
	SECONDARY_RESPONSE * c_thres);
}

int ReputationSystem::compRating(Ipv4Address pre, Ipv4Address next) {
	double v_pre = 0.5;
	double v_next = 0.5;
	RatingTable::iterator i_p = reputation_t.find(pre);
	RatingTable::iterator i_n = reputation_t.find(next);
	if (i_p != reputation_t.end())
		v_pre = (i_p->second)->bayeMean();
	if (i_n != reputation_t.end())
		v_next = (i_n->second)->bayeMean();
	if (v_pre < v_next)
		return -1;
	else if (fabs(v_pre - v_next) < pow(10, -6))
		return 0;
	else
		return 1;
}

void ReputationSystem::handlePublicExpire() {
	exchangeFirsthandInfo();
	//NS_LOG_DEBUG("Broadcast first-hand information at time:" +
	//NumberToString(Simulator::Now().ToDouble(ns3::Time::MS)));
	public_t.Cancel();
	public_t.Schedule(Time(Seconds(p_timeout)));
}

void ReputationSystem::checkClassification(Ipv4Address target_ip) {
	RatingTable::iterator i = reputation_t.find(target_ip);
	if (ifenableL) {
		if (((i->second)->bayeMean()) < LO_T) {
			NS_LOG_LOGIC(
					getNodeId()<<", Delete item in repu_t& first_t when repu < 0.5, " << target_ip);
			double t = (dynamic_cast<Repu_Rating*>(i->second))->getThreshold();
			if (t >= REGULAR_T) {
				NS_LOG_LOGIC(
						getNodeId()<<", delete item in repu_t due to non selfish node");
				rmReputationItem(target_ip);
			}
			rmFirsthandItem(target_ip);
		}
	}
	if (!findReputationItem(target_ip)) {
		return;
	}
	if (isMalicious(target_ip)) {
		path_m->addMnode(target_ip);
		decreaseR(*(i->second));
		//add send Rerr packet
		callbackIsNeighborFlag(true);
		callbackSendRerrWhenBreaksLinkToNextHop(target_ip);
		///path_m->deleteAllRoutes(target_ip);
	} else {
		path_m->rmMnode(target_ip);
	}
}

void ReputationSystem::handleReputationFading(Ipv4Address ip, F_Rating* f,
		Repu_Rating* r) {
	NS_LOG_FUNCTION(this << getNodeId() << ip << r->getAlpha() << r->getBeta());
	if ((r->getAlpha() < FADING_T) && (r->getBeta() < FADING_T)) {
		if (!isMalicious(ip)) {
			if (ifenableL && !L_TRADEOFF) {
				rmFirsthandItem(ip);
				return;
			}
			if (ifenableL && L_TRADEOFF) {
				if (!(m_neighbour->IsNeighbor(ip))) {
					if (r->getThreshold() >= REGULAR_T)
						rmReputationItem(ip);
				}
				rmFirsthandItem(ip);
			}
		} else {
			path_m->rmMnode(ip);
			if (ifenableL)
				rmFirsthandItem(ip);
		}
	}
}

void ReputationSystem::exchangeFirsthandInfo(/*RatingTable& ratings*/) //duplicated packets check
{
    NS_LOG_FUNCTION(this << getNodeId() << "exchange first-hand info");
	//Add ReputationSystem as friend class in AODV-ROUTING-PROTOCOL
	if (firsthandInfo_t.size() == 0)
		return;
	m_publicid++;
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses->begin(); j != m_socketAddresses->end(); ++j) {
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		PratingHeader prHeader(m_publicid);
		for (RatingTable::iterator i = firsthandInfo_t.begin();
				i != firsthandInfo_t.end(); i++) {
			///prHeader.addRating(i->first, std::pair<double, double>((i->second)->getAlpha(),(i->second)->getBeta()));
			prHeader.addRating(i->first, (i->second)->getAlpha(),
					(i->second)->getBeta());
		}
		Ptr<Packet> packet = Create<Packet>();
		packet->AddHeader(prHeader);
		ns3::aodv::TypeHeader tHeader(ns3::aodv::AODVTYPE_PRATING);
		packet->AddHeader(tHeader);
		// Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
		Ipv4Address destination;
		if (iface.GetMask() == Ipv4Mask::GetOnes()) {
			destination = Ipv4Address("255.255.255.255");
		} else {
			destination = iface.GetBroadcast();
		}
		Time jitter = Time(
				MilliSeconds(m_uniformRandomVariable->GetInteger(0, 10)));
		//m_publicid++;
		// Do not forget to count public rating and Alarms to process duplicate check
		// code: add packet to idcache
		m_publicCache.IsDuplicate(iface.GetLocal(), m_publicid);
		Simulator::Schedule(jitter, &ns3::confidant::ReputationSystem::SendTo,
				this, socket, packet, destination);
		NS_LOG_LOGIC(getNodeId()<< ", schedule to send public rating, " << packet->GetUid() << ", dst "<< destination);
	}
}

void ReputationSystem::SendTo(Ptr<Socket> ps, Ptr<Packet> pp,
		Ipv4Address des_ip) {
	NS_LOG_FUNCTION(this << ps << pp->GetUid() << des_ip);
	callbackSendto(ps, pp, des_ip);
}

void ReputationSystem::handleSecondhandInfo(ns3::aodv::Neighbors& neighbors,
		Ipv4Address from,
		std::map<Ipv4Address, std::pair<double, double> >& secondHandInfo) {
	NS_LOG_FUNCTION("["<<getNodeId()<< "] "<< this<<from);
	if (!ifenableL) {
		bool ifTrustworthy = trust_m->isTrustworthy(from);
		if (ifTrustworthy) {
			for (std::map<Ipv4Address, std::pair<double, double> >::iterator i =
					secondHandInfo.begin(); i != secondHandInfo.end(); i++) {
				RatingTable::iterator item_r = reputation_t.find(i->first);
				if (item_r == reputation_t.end()) {
					std::pair<RatingTable::iterator, bool> i_repu;
					//i_repu = reputation_t.insert(std::pair<Ipv4Address, Rating*>(i->first, initNewR_Rating(u, r)));
					i_repu = addReputationItem(i->first);
					updateReputation(*((i_repu.first)->second),
							(i->second).first, (i->second).second, this->w);
				} else {
					updateReputation(*(item_r->second), (i->second).first,
							(i->second).second, this->w);
				}
				checkClassification(i->first);
			}
		}
		for (std::map<Ipv4Address, std::pair<double, double> >::iterator i =
				secondHandInfo.begin(); i != secondHandInfo.end(); i++) {
			RatingTable::iterator item_r = reputation_t.find(i->first);
			if (item_r == reputation_t.end()) {
				std::pair<RatingTable::iterator, bool> i_repu;
				//i_repu = reputation_t.insert(std::pair<Ipv4Address, Rating*>(i->first, initNewR_Rating(u, r)));
				i_repu = addReputationItem(i->first);
				item_r = i_repu.first;
			}
			ns3::confidant::DEVIATIONTEST d_result;
			if (passDeviationTest(*(item_r->second), (i->second).first,
					(i->second).second)) {
				d_result = compatiable;
				if (!ifTrustworthy) {
					updateReputation(*(item_r->second), (i->second).first,
							(i->second).second, this->w);
					checkClassification(i->first);
				}

			} else {
				d_result = noncompatiable;
			}
			trust_m->updateTrust(i->first, d_result);
		}
	} else {
		for (std::map<Ipv4Address, std::pair<double, double> >::iterator i =
				secondHandInfo.begin(); i != secondHandInfo.end(); i++) {
			if (!neighbors.IsNeighbor(i->first))
				continue;
			RatingTable::iterator item_r = reputation_t.find(i->first);
			NS_LOG_LOGIC(
					getNodeId()<<"handle second_infor through l-confidant LOGIC");
			handlePublicRatingL(i->first, item_r, (i->second).first, (i->second).second);
		}
	}
}

bool ReputationSystem::findReputationItem(Ipv4Address ip) {
	RatingTable::iterator i = reputation_t.find(ip);
	if (i != reputation_t.end())
		return true;
	else
		return false;
}

bool ReputationSystem::findFirsthandItem(Ipv4Address ip) {
	RatingTable::iterator i = firsthandInfo_t.find(ip);
	if (i != firsthandInfo_t.end())
		return true;
	else
		return false;
}

void ReputationSystem::rmFirsthandItem(Ipv4Address ip) {
	NS_LOG_FUNCTION("["<<getNodeId()<< "] "<< this << ip);
	RatingTable::iterator i = firsthandInfo_t.find(ip);
	if (i != firsthandInfo_t.end()) {
		delete (i->second);
		firsthandInfo_t.erase(ip);
		NS_LOG_DEBUG(
				"["<<getNodeId()<< "] "<< "remove First-hand: call notify for trace");
		notifyTableSizeChange_F();
	}
}

void ReputationSystem::rmReputationItem(Ipv4Address ip) {
	RatingTable::iterator i = reputation_t.find(ip);
	if (i != reputation_t.end()) {
		delete (i->second);
		reputation_t.erase(ip);
		notifyTableSizeChange_R();
	}
}

std::pair<RatingTable::iterator, bool> ReputationSystem::addFirsthandItem(
		Ipv4Address neighbor) {
	NS_LOG_FUNCTION("["<<getNodeId()<< "] "<< this << neighbor);
	std::pair<RatingTable::iterator, bool> i;
	i = firsthandInfo_t.insert(
			std::pair<Ipv4Address, Rating*>(neighbor,
					initNewF_Rating(this->u, neighbor)));
	NS_LOG_DEBUG(
			"["<<getNodeId()<< "] "<< "add FIRST-HAND: call notify for trace");
	notifyTableSizeChange_F();
	return i;
}

std::pair<RatingTable::iterator, bool> ReputationSystem::addReputationItem(
		Ipv4Address neighbor) {
	std::pair<RatingTable::iterator, bool> i;
	i = reputation_t.insert(
			std::pair<Ipv4Address, Rating*>(neighbor, initNewR_Rating(u, r)));
	notifyTableSizeChange_R();
	return i;
}

void ReputationSystem::SendAlarm(Ipv4Address selfish_ip,
		ns3::aodv::RoutingTable& m_routingTable,
		ns3::aodv::RequestQueue& m_queue, const uint32_t port) {
	NS_LOG_FUNCTION(this<< getNodeId() << "ip of selfish node "<<selfish_ip);
	// check if the node is selfish before invoke it.
	if (isMalicious(selfish_ip)
			&& (firsthandInfo_t.find(selfish_ip) != firsthandInfo_t.end())) {
		NS_LOG_LOGIC(getNodeId() <<", neighbor "<< selfish_ip << "is selfish");
		RatingTable::iterator i = firsthandInfo_t.find(selfish_ip);
		double s_alpha = (i->second)->getAlpha();
		double s_beta = (i->second)->getBeta();
		//remove item from first-hand, reputation_rating, selfish node list
		// add interface of remove item from tables
		if (!L_TRADEOFF) {
			path_m->rmMnode(selfish_ip);
			rmFirsthandItem(selfish_ip);
			rmReputationItem(selfish_ip);
		}

		ns3::aodv::RoutingTableEntry toSelfish;
		if (m_routingTable.LookupValidRoute(selfish_ip, toSelfish)) {
			NS_LOG_LOGIC(getNodeId() << "have valid route to selfish node: " << selfish_ip);
			//check whether hop count of the route is less than limit
			if (toSelfish.GetHop() > HOPCOUNT_LIMIT) {
				NS_LOG_LOGIC(
						getNodeId()<< ", send ALARM when an route exists & hop count < limit");
				Ipv4Address alarm_sender = toSelfish.GetInterface().GetLocal();
				Ptr<Socket> socket = callbackFindSocketIp(
						toSelfish.GetInterface());
				AlarmHeader alarm_h(m_alarmid++, 0, selfish_ip, alarm_sender,
						s_alpha, s_beta);   // remember add field tag in c-tor!!
				Ptr<Packet> packet = Create<Packet>();
				packet->AddHeader(alarm_h);
				ns3::aodv::TypeHeader tHeader(ns3::aodv::AODVTYPE_ALARM);
				packet->AddHeader(tHeader);
				socket->SendTo(packet, 0,
						InetSocketAddress(toSelfish.GetNextHop(), port));
				NS_LOG_LOGIC(getNodeId() << ", send alarm to next hop: " << toSelfish.GetNextHop() << " pid " << packet->GetUid());
				//duplicate check -> add packet into m_alarmCache
				m_alarmCache.IsDuplicate(alarm_sender, m_alarmid);
			}
			else{
				NS_LOG_LOGIC(getNodeId() << ", do not send alarm due to hop count: " << toSelfish.GetHop());
			}
		} else {
			NS_LOG_LOGIC(getNodeId() << "defer alarm due to no route");
			//ADD member function for callback when the route is found!!
			AlarmHeader alarm_h;        // remember add field tag in c-tor!!
			alarm_h.SetDst(selfish_ip);
			alarm_h.SetAlpha(s_alpha);
			alarm_h.SetBeta(s_beta);
			//add 8-bit flag to 0
			alarm_h.SetFlag(0);
			Ptr<Packet> packet = Create<Packet>();
			packet->AddHeader(alarm_h);
			DeferredAlarmTag tag;
			packet->AddPacketTag(tag);
			Ipv4Header trick_header;
			trick_header.SetDestination(selfish_ip);
			NS_LOG_LOGIC(
					getNodeId()<< "Add alarm in RREQ queue" << packet->GetUid() << trick_header);
			ns3::aodv::QueueEntry newEntry(packet, trick_header);
			newEntry.SetErrorCallback(
					MakeCallback(
							&ReputationSystem::handleErrorForAlarmWithoutRoute,
							this));
			bool result = m_queue.Enqueue(newEntry);
			if (result) {
				NS_LOG_LOGIC(
						"Add packet " << packet->GetUid () << " to RREQqueue. Protocol " << (uint16_t) trick_header.GetProtocol ());
				ns3::aodv::RoutingTableEntry rt;
				bool result = m_routingTable.LookupRoute(
						trick_header.GetDestination(), rt);
				if (!result
						|| ((rt.GetFlag() != ns3::aodv::IN_SEARCH) && result)) {
					NS_LOG_LOGIC(
							"Send new RREQ for outbound packet to " <<trick_header.GetDestination ());
					callbackSendRequest(trick_header.GetDestination());
				}
			}
		}
	} else {
		//remove item from first-hand, reputation_rating, selfish node list
		NS_LOG_LOGIC(getNodeId() <<", neighbor "<< selfish_ip << "is not selfish anymore");
		if (!L_TRADEOFF) {
			path_m->rmMnode(selfish_ip);
			rmFirsthandItem(selfish_ip);
			rmReputationItem(selfish_ip);
		}
	}
}

void ReputationSystem::SendAlarmFromQueue(Ptr<Packet> alarm,
		const ns3::aodv::RoutingTableEntry& rt, uint32_t port) {
	NS_LOG_FUNCTION(this << getNodeId() << alarm->GetUid());
	if (rt.GetHop() > HOPCOUNT_LIMIT) {
		AlarmHeader header;
		alarm->RemoveHeader(header);
		header.SetAlarmId(m_alarmid++);
		Ipv4Address alarm_sender = rt.GetInterface().GetLocal();
		Ptr<Socket> socket = callbackFindSocketIp(rt.GetInterface());
		Ptr<Packet> packet = Create<Packet>();
		packet->AddHeader(header);
		ns3::aodv::TypeHeader tHeader(ns3::aodv::AODVTYPE_ALARM);
		packet->AddHeader(tHeader);
		socket->SendTo(packet, 0, InetSocketAddress(rt.GetNextHop(), port));
		NS_LOG_LOGIC(getNodeId() << ", send alarm to next hop: " << rt.GetNextHop() << " PID: " << packet->GetUid());
		// add it into idcache
		m_alarmCache.IsDuplicate(alarm_sender, header.GetAlarmId());
	}
	else{
		NS_LOG_LOGIC(getNodeId() << ", do not send alarm from queue due to hop count: " << rt.GetHop());
	}

}

void ReputationSystem::handlePublicRatingL(Ipv4Address ip, RatingTable::iterator item_r,
		double a, double b) {
	NS_LOG_FUNCTION(this << getNodeId() << ip << a << b);
	if (item_r != reputation_t.end()) {
		NS_LOG_LOGIC(
				getNodeId()<< ", l-confidant handle second-info: item is in repu_t");
		//deviation test
		if (passDeviationTest(*(item_r->second), a, b)) {
			NS_LOG_LOGIC(
					getNodeId() << ", accept public rating due to passing deviation test");
			updateReputation(*(item_r->second), a, b, this->w);
			checkClassification(item_r->first);
		}
	} else if (!callbackLUT(ip))/*i->first NOT in the first hand update table*/
	{
		std::pair<RatingTable::iterator, bool> i_repu;
		//i_repu = reputation_t.insert(std::pair<Ipv4Address, Rating*>(item_r->first, initNewR_Rating(u, r)));
		i_repu = addReputationItem(ip);
		item_r = i_repu.first;
		if (passDeviationTest(*(item_r->second), a, b)) {
			NS_LOG_LOGIC(
					getNodeId()<< ", l-confidant handle second-info: item is not in repu_t& LUT");
			updateReputation(*(item_r->second), a, b, this->w);
			checkClassification(item_r->first);
		}

	}
}

void ReputationSystem::RecvAlarm(Ptr<Packet> alarm,
		ns3::aodv::Neighbors& neighbors,
		ns3::aodv::RoutingTable& m_routingTable, const uint32_t port) {
	NS_LOG_FUNCTION(this << getNodeId() <<alarm->GetUid());
	//do not forget to check classification
	AlarmHeader header;
	alarm->RemoveHeader(header);
	//duplicated packets check
	if (m_alarmCache.IsDuplicate(header.GetOrigin(), header.GetAlarmId())) {
		NS_LOG_LOGIC("DROP ALARM due to duplicate process");
		return;
	}
	if (header.GetFlag() == 1)/*tag check*/
	{
		//l-confidant logic to decide whether to accept the shared rating information
		if (!neighbors.IsNeighbor(header.GetDst()))
			return;
		RatingTable::iterator item_r = reputation_t.find(header.GetDst());
		NS_LOG_LOGIC("handle received alarm to update rating when flag =1");
		handlePublicRatingL(header.GetDst(), item_r, header.GetAlpha(), header.GetBeta());
	} else if (neighbors.IsNeighbor(header.GetDst()))/*is the neig of target node*/
	{
		//set flag as 1 and broadcast to neighbors
		header.SetFlag(1);
		//update rating about selfish node
		RatingTable::iterator item_r = reputation_t.find(header.GetDst());
		NS_LOG_LOGIC(
				"update rating from received alarm when flag = 0 & dst is neighbor");
		handlePublicRatingL(header.GetDst(), item_r, header.GetAlpha(),
				header.GetBeta()/*, last update table first-hand*/);
		NS_LOG_LOGIC("Broadcast received alarm to neighbors");
		for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
				m_socketAddresses->begin(); j != m_socketAddresses->end();
				++j) {
			Ptr<Socket> socket = j->first;
			Ipv4InterfaceAddress iface = j->second;
			Ptr<Packet> brocastAlarm = Create<Packet>();
			brocastAlarm->AddHeader(header);
			ns3::aodv::TypeHeader tHeader(ns3::aodv::AODVTYPE_ALARM);
			brocastAlarm->AddHeader(tHeader);
			// Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
			Ipv4Address destination;
			if (iface.GetMask() == Ipv4Mask::GetOnes()) {
				destination = Ipv4Address("255.255.255.255");
			} else {
				destination = iface.GetBroadcast();
			}
			Time jitter = Time(
					MilliSeconds(m_uniformRandomVariable->GetInteger(0, 10)));
			Simulator::Schedule(jitter,
					&ns3::confidant::ReputationSystem::SendTo, this, socket,
					brocastAlarm, destination);
		}
	} else {
		//not the neig & forward alarms, /*exist route to target node*/
		Ptr<Packet> newAlarm = Create<Packet>();//Create new packet to avoid repeated adding tag with same typeid
		ns3::aodv::RoutingTableEntry toSelfish;
		if (m_routingTable.LookupRoute(header.GetDst(), toSelfish)) {
			NS_LOG_LOGIC("Forward alarm when dst is not a neighbor");
			//Ipv4Address alarm_sender = toSelfish.GetInterface().GetLocal();
			if (toSelfish.GetFlag() == ns3::aodv::VALID) {
				Ptr<Socket> socket = callbackFindSocketIp(
						toSelfish.GetInterface());
				newAlarm->AddHeader(header);
				ns3::aodv::TypeHeader tHeader(ns3::aodv::AODVTYPE_ALARM);
				newAlarm->AddHeader(tHeader);
				socket->SendTo(newAlarm, 0,
						InetSocketAddress(toSelfish.GetNextHop(), port));
			} else {
				NS_LOG_LOGIC(
						getNodeId() << ", no valid route to forward ALARM: " << newAlarm->GetUid());
				callbackSendRerrWhenNoRouteToForward(header.GetDst(),
						toSelfish.GetSeqNo(), header.GetOrigin());
				return;
			}
		} else {
			NS_LOG_LOGIC(
					getNodeId() << ", no route to forward ALARM: " << newAlarm->GetUid());
			callbackSendRerrWhenNoRouteToForward(header.GetDst(), 0,
					header.GetOrigin());
			return;
		}
	}

}

void ReputationSystem::handleErrorForAlarmWithoutRoute(Ptr<const Packet> p,
		const Ipv4Header & h, Socket::SocketErrno flag) {
	NS_LOG_FUNCTION(this << getNodeId() << p << h << flag);
	NS_LOG_DEBUG(getNodeId() <<", No route to send ALARM");
}

void ReputationSystem::updateRouteTableFromRREPC(
		ns3::aodv::RoutingTable& m_routingTable, ns3::aodv::RrepHeader& header,
		ns3::aodv::RoutingTableEntry& toDst,
		ns3::aodv::RoutingTableEntry& newEntry, Ipv4Address sender) {
	NS_LOG_FUNCTION(
			this<< getNodeId() <<"dst "<<toDst.GetNextHop() << "sender " << sender);
	if ((header.GetDstSeqno() == toDst.GetSeqNo())
			&& (compRating(toDst.GetNextHop(), sender) == -1)) {
		NS_LOG_LOGIC(
				"C: update route entry to dst: "<< toDst.GetNextHop() << std::endl << "new next hop is " << sender << " higher rating");
		m_routingTable.Update(newEntry);
	} else if ((header.GetDstSeqno() == toDst.GetSeqNo())
			&& (compRating(toDst.GetNextHop(), sender) == 0)
			&& (header.GetHopCount() < toDst.GetHop())) {
		NS_LOG_LOGIC(
				"C: update route entry to dst: "<< toDst.GetNextHop() << std::endl << "new next hop is " << sender << " equal rating & less hop");
		m_routingTable.Update(newEntry);
	}
}

void ReputationSystem::updateRouteTableFromRREPL(
		ns3::aodv::RoutingTable& m_routingTable, ns3::aodv::RrepHeader& header,
		ns3::aodv::RoutingTableEntry& toDst,
		ns3::aodv::RoutingTableEntry& newEntry, Ipv4Address sender) {
	NS_LOG_FUNCTION(
			this<< getNodeId() << "dst " << toDst.GetNextHop() << "sender " << sender);
	if ((header.GetDstSeqno() == toDst.GetSeqNo())
			&& (compPriority(toDst.GetNextHop(), sender) == -1)) {
		NS_LOG_LOGIC(
				"L: update route entry to dst: "<< toDst.GetNextHop() << std::endl << "new next hop is " << sender << " higher priority");
		m_routingTable.Update(newEntry);
	} else if ((header.GetDstSeqno() == toDst.GetSeqNo())
			&& (compPriority(toDst.GetNextHop(), sender) == 0)
			&& (header.GetHopCount() < toDst.GetHop())) {
		NS_LOG_LOGIC(
				"L: update route entry to dst: "<< toDst.GetNextHop() << std::endl << "new next hop is " << sender << " equal pri & less hop");
		m_routingTable.Update(newEntry);
	}
}
uint8_t ReputationSystem::getPriority(Ipv4Address ip) {
	/*
	 *First priority: 0 -- nodes which are not in reputation_t but in lastupdate_t.
	 *Second priority: 1 -- nodes which are in neither reputation_t nor lastupdate_t.
	 *Third priority: 2 -- nodes which are in reputation_t, so that compare reputation ratings
	 */
	bool r = findReputationItem(ip);
	bool l = callbackLUT(ip);
	if (r)
		return 2;
	else if (l)
		return 0;
	else
		return 1;
}

int ReputationSystem::compPriority(Ipv4Address pre, Ipv4Address newEntry) {
	uint8_t p_pre = getPriority(pre);
	uint8_t p_new = getPriority(newEntry);
	if (p_pre < p_new)
		return -1;
	else if (p_pre > p_new)
		return 1;
	else {
		if (p_pre == 2)
			return compRating(pre, newEntry);
		else
			return 0;
	}
}

/*set fields and callbacks method*/
void ReputationSystem::setPublicTimer() {
	///public_t(Timer::CANCEL_ON_DESTROY);
	public_t.SetFunction(&ReputationSystem::handlePublicExpire, this);
	public_t.Schedule(Seconds(m_uniformRandomVariable->GetValue(22.0, 24.0)));
	//NS_LOG_DEBUG("Set public timer for share first-hand information");
}

void ReputationSystem::setWeight(double nWeight) {
	w = nWeight;
}

void ReputationSystem::setEableL(bool ifenable) {
	ifenableL = ifenable;
}

/*set callbacks*/
void ReputationSystem::setCallbackSendTo(
		Callback<void, Ptr<Socket>, Ptr<Packet>, Ipv4Address> cb) {
	callbackSendto = cb;
}

void ReputationSystem::setCallbackFindSocketIp(
		Callback<Ptr<Socket>, Ipv4InterfaceAddress> cb) {
	callbackFindSocketIp = cb;
}

void ReputationSystem::setCallbackSendRerrWhenBreaksLinkToNextHop(
		Callback<void, Ipv4Address> cb) {
	callbackSendRerrWhenBreaksLinkToNextHop = cb;
}

void ReputationSystem::setCallbackSendRerrWhenNoRouteToForward(
		Callback<void, Ipv4Address, uint32_t, Ipv4Address> cb) {
	callbackSendRerrWhenNoRouteToForward = cb;
}

void ReputationSystem::setcallbackSendRequest(Callback<void, Ipv4Address> cb) {
	callbackSendRequest = cb;
}

void ReputationSystem::setcallbackNeighborFlag(Callback<void, bool> cb) {
	callbackIsNeighborFlag = cb;
}

/*L-weight CONFIDANT methods*/

double ReputationSystem::reputationOfIp(Ipv4Address ip) {
	return (reputation_t.find(ip)->second)->bayeMean();
}

void ReputationSystem::setTradeoffSwitch(bool t) {
	L_TRADEOFF = t;
}

bool ReputationSystem::getTradeoffSwitch() {
	return L_TRADEOFF;
}

void ReputationSystem::setCallbackLUT(Callback<bool, Ipv4Address> cb) {
	callbackLUT = cb;
}

}
}
