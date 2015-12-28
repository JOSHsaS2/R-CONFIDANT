#include "Monitor.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/node.h"
#include "ns3/simulator.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("CONFIDANT_MONITOR");

namespace confidant {

NS_OBJECT_ENSURE_REGISTERED(Monitor);

/*
 Monitor::Monitor(ReputationSystem& r, ns3::aodv::Neighbors& neig, double wtime, Ptr<Ipv4>& aodv_ipv4):
 repuSys(r),
 neig_l(neig),
 waitAckTimeout(wtime),
 m_ipv4(aodv_ipv4)
 {
 ifenableL = false;
 repuSys->setCallbackLUT(MakeCallback(&Monitor::findItemLUT, this));
 }
 */

Monitor::Monitor() {
	ifenableL = false;
	waitAckTimeout = PACK_TIMEOUT;

}

void Monitor::setReputationSystem(Ptr<ReputationSystem> repu) {
	repuSys = repu;
}

void Monitor::setComponentsFromAodv(ns3::aodv::Neighbors* neig,
		Ptr<Ipv4> aodv_ipv4) {
	neig_l = neig;
	m_ipv4 = aodv_ipv4;
}

TypeId Monitor::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::confidant::Monitor").SetParent<Object>().SetGroupName(
					"CONFIDANT").AddConstructor<Monitor>().AddTraceSource(
					"LUTSizeChange",
					"Last update table in Monitor has changed.",
					MakeTraceSourceAccessor(&Monitor::m_TableSizeChangeTrace),
					"ns3::confidant::Monitor::TableSizeChangeTracedCallback")
					// etc (more parameters).
					;
	return tid;
}

uint32_t Monitor::getNodeId() {
	return m_ipv4->GetObject<ns3::Node>()->GetId();
}

uint32_t Monitor::getTableSize() {
	return lastUpdate_t.size();
}

void Monitor::notifyTableSizeChange() {
	NS_LOG_DEBUG("["<<getNodeId()<< "] "<<"Monitor LUT size change!");
	m_TableSizeChangeTrace(getNodeId(), ns3::Simulator::Now(), getTableSize());
}

void Monitor::deletePack(uint64_t uid) {
	if (packQueue.find(uid) != packQueue.end()) {
		packQueue.erase(uid);
	}
}

Timer Monitor::initNewTimer(Ipv4Address ip, uint64_t uid) {
	Timer ntimer(ns3::Timer::CANCEL_ON_DESTROY);
	ntimer.SetFunction(&Monitor::waitPackExpire, this);
	ntimer.SetArguments(ip, uid);
	ntimer.Schedule(Time(Seconds(waitAckTimeout)));
	return ntimer;
}

//L-CONFIDANT: LAST UPDATE TABLE
void Monitor::deleteItemFromLUT(Ipv4Address ip) {
	if (lastUpdate_t.find(ip) != lastUpdate_t.end()) {
		lastUpdate_t.erase(ip);
		notifyTableSizeChange();
	}
}
void Monitor::addItemFromLUT(Ipv4Address ip) {
	//ip is not a neighbor, ingore it
	//add Monitor as friend class
	if (neig_l->IsNeighbor(ip)) {
		NS_LOG_LOGIC(getNodeId()<<", Add new item in LUT: " << ip);
		ns3::Timer nTimer(ns3::Timer::CANCEL_ON_DESTROY);
		std::pair<std::map<Ipv4Address, Timer>::iterator, bool> i =
				lastUpdate_t.insert(std::pair<Ipv4Address, Timer>(ip, nTimer));
		((i.first)->second).SetFunction(&Monitor::deleteItemFromLUT, this);
		((i.first)->second).SetArguments(ip);
		((i.first)->second).Schedule(Time(Seconds(LASTUPDATE_TIMEOUT)));
		notifyTableSizeChange();
	}

}
Timer Monitor::initNewTimerForLUT(Ipv4Address ip) {
	Timer ntimer(ns3::Timer::CANCEL_ON_DESTROY);
	return ntimer;
}
void Monitor::updateTimerFromLUT(Ipv4Address ip) {
	NS_LOG_LOGIC(getNodeId()<< ", Update timer in LUT: "<< ip);
	std::map<Ipv4Address, Timer>::iterator i = lastUpdate_t.find(ip);
	(i->second).Cancel();
	(i->second).Schedule(Time(Seconds(LASTUPDATE_TIMEOUT)));
}

bool Monitor::ifExpireLUT(Ipv4Address ip) {
	std::map<Ipv4Address, Timer>::iterator i = lastUpdate_t.find(ip);
	if ((i->second).IsExpired())
		return true;
	else
		return false;
}

bool Monitor::findItemLUT(Ipv4Address ip) {
	if (lastUpdate_t.find(ip) != lastUpdate_t.end())
		return true;
	else
		return false;
}
void Monitor::cancelTimer(PackForAck& p) {
	p.cancelTimer();
}

void Monitor::suspendTimer(PackForAck& p) {
	p.suspendTimer();
}

void Monitor::resumeTimer(PackForAck& p) {
	p.resumeTimer();
}

void Monitor::waitPackExpire(Ipv4Address ip, uint64_t uid) {
	deletePack(uid);
	if (ifenableL) {
		if (lastUpdate_t.find(ip) == lastUpdate_t.end()) {
			//NS_LOG_LOGIC("Add new item in LUT: " << ip);
			addItemFromLUT(ip);
		} else {
			//NS_LOG_LOGIC("Update timer in LUT: "<< ip);
			updateTimerFromLUT(ip);
		}
	}
	NS_LOG_DEBUG(
			"["<<getNodeId()<< "] "<< "Wait timeout observe as one selfish behavior !");
	repuSys->handleFirsthandInfo(neig_l->IsNeighbor(ip), ip,
			ns3::confidant::misbehave);
}

void Monitor::registerPack(Ptr<const Packet> p, Address nextH, Ipv4Address dst,
		Ipv4Address next) {
	NS_LOG_FUNCTION(
			getNodeId()<< this << p->GetUid() << nextH << dst << next);
	uint64_t uid = p->GetUid();
	if (packQueue.find(uid) == packQueue.end() && dst != next) {
		NS_LOG_DEBUG(getNodeId() << ", insert new item in pack queue!");
		//ns3::Timer nt = initNewTimer(next, uid);
		packQueue.insert(
				std::pair<uint64_t, PackForAck>(uid,
						PackForAck(p, nextH, next)));
		PacketTable::iterator i = packQueue.find(uid);
		(i->second).getTimer().SetFunction(&Monitor::waitPackExpire, this);
		(i->second).getTimer().SetArguments(next, uid);
		(i->second).getTimer().Schedule(Time(Seconds(waitAckTimeout)));
	}
}

bool Monitor::handlePassivePack(Ptr<NetDevice> nd, Ptr<const Packet> pack,
		uint16_t protN, const Address & sender, const Address & receiver,
		ns3::NetDevice::PacketType packType) {
	if (packType != NetDevice::PACKET_OTHERHOST)
		return false;
	NS_LOG_FUNCTION(
			getNodeId() << this << pack->GetUid() << sender << receiver << packType);
	PacketTable::iterator pack_i = packQueue.find(pack->GetUid());
	if (pack_i != packQueue.end()) {
		cancelTimer(pack_i->second);

		//suspendTimer(pack_i->second);

		NS_LOG_DEBUG(
				"["<<getNodeId()<< "] with mac address: "<< m_ipv4->GetNetDevice(0)->GetAddress() <<"\n Test assigning MAC address");

		Address tem_mac = (pack_i->second).getMacOfNextHop();
		Ipv4Address tem_ip = (pack_i->second).getIpOfNextHop();

		deletePack(pack->GetUid());

		//invoke Detector virtual functions
		NS_LOG_DEBUG(
				"["<<getNodeId()<< "] "<< "Test whether passive pack has same mac address");
		NS_LOG_DEBUG("["<<getNodeId()<< "] "<< sender << std::endl <<tem_mac);

		BEHAVIOR b = (tem_mac == sender) ? regular : misbehave;
		/*
		 if (tem_mac == sender) {
		 b = regular;
		 deletePack(pack->GetUid());
		 } else {
		 resumeTimer(pack_i->second);
		 return true;
		 }
		 */
		if (ifenableL) {
			if (lastUpdate_t.find(tem_ip) == lastUpdate_t.end())
				addItemFromLUT(tem_ip);
			else
				updateTimerFromLUT(tem_ip);

			if (repuSys->findReputationItem(tem_ip)
					&& (repuSys->reputationOfIp(tem_ip) >= LO_T)) {
				NS_LOG_LOGIC(
						getNodeId()<<", handle first-info when item sin in repu_t and repu > 0.5, " << tem_ip);
				repuSys->handleFirsthandInfo(neig_l->IsNeighbor(tem_ip), tem_ip,
						b);
			} else if (b == misbehave) {
				NS_LOG_LOGIC(
						getNodeId()<<", handle first_info when no item in repu_t and misbehaviour observed, "<< tem_ip);
				repuSys->handleFirsthandInfo(neig_l->IsNeighbor(tem_ip), tem_ip,
						b);
			}
		} else
			repuSys->handleFirsthandInfo(neig_l->IsNeighbor(tem_ip), tem_ip, b);

		return true;
	}
	return false;
}

void Monitor::handlePublicRating(Ipv4Address from, Ptr<Packet> p) {
	NS_LOG_FUNCTION("["<<getNodeId()<< "] "<< this << p << from);
	PratingHeader r_header;
	p->RemoveHeader(r_header);
	//duplicated packets check
	//only can work in the situation with one Ipv4InterfaceAddress
	if ((repuSys->m_publicCache).IsDuplicate(from, r_header.GetId()))
		return;
	std::map<Ipv4Address, std::pair<double, double> > tmp_rt;
	std::pair<Ipv4Address, std::pair<double, double> > tmp_r;
	for (int i = 0; i < r_header.getRatingCount(); i++) {
		r_header.removeRating(tmp_r);
		tmp_rt.insert(tmp_r);
	}
	repuSys->handleSecondhandInfo(*neig_l, from, tmp_rt);
}
//!< handleAlarm(); //duplicated packets check
}
}

