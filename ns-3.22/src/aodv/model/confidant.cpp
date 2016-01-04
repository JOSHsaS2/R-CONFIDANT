#include "confidant.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/node.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("CONFIDANT_confidant");

namespace confidant {

void PackForAck::cancelTimer() {
	//NS_LOG_DEBUG("Observed packet: Cancel Timer");
	timeout.Cancel();
}

void PackForAck::suspendTimer() {
	timeout.Suspend();
}

void PackForAck::resumeTimer() {
	timeout.Resume();
}

PackForAck::PackForAck(Ptr<const Packet> pack, Address mach, Ipv4Address iph) :
		p(pack), timeout(ns3::Timer::CANCEL_ON_DESTROY), macOfNextHop(mach), ipOfNextHop(
				iph) {
}

void Rating::fadeRating() {
	alpha *= f_factor;
	beta *= f_factor;
}

double Rating::bayeMean() {
	return alpha / (alpha + beta);
}

double Repu_Rating::getThreshold() {
	return threshold;
}

void Repu_Rating::setThreshold(double thres) {
	threshold = thres;
}

Repu_Rating::Repu_Rating(double thres, double fading_f) :
		Rating(fading_f) {
	threshold = thres;
}

void Repu_Rating::updateByOb(BEHAVIOR behavior) {
	fadeRating();
	int s = ((behavior == regular) ? 0 : 1);
	alpha += s;
	beta += (1 - s);
}

void Repu_Rating::updateReputation(double salpha, double sbeta, double w) {
	alpha += w * salpha;
	beta += w * sbeta;
}

/*
 void
 Repu_Rating::updateReputation(Rating* p_rating, double w)
 {
 alpha += w * p_rating->alpha;
 beta += w * p_rating->beta;
 }
 */

F_Rating::F_Rating(double fading_f, Ipv4Address f_ip) :
		Rating(fading_f), f_timer(Timer::CANCEL_ON_DESTROY) {
	f_timer.SetFunction(&F_Rating::handleFadingTimeoutExpire, this);
	f_timer.SetArguments(f_ip);
	f_timer.Schedule(Time(Seconds(FADING_TIMEOUT)));
	//NS_LOG_DEBUG("Initiate timer for first-hand rating item!");
}

//fields

Repu_Rating*
F_Rating::getRepu() {
	return repu;
}

void F_Rating::setRepu(Repu_Rating* r) {
	repu = r;
}

void F_Rating::cancelTimer() {
	f_timer.Cancel();
}

void F_Rating::reScheduleTimer() {
	f_timer.Schedule(Time(Seconds(FADING_TIMEOUT)));
	//NS_LOG_DEBUG("Reschedule timer for first-hand rating item!");
}

void F_Rating::updateByOb(BEHAVIOR behavior) {
	fadeRating();
	int s = ((behavior == regular) ? 0 : 1);
	alpha += s;
	beta += (1 - s);
	repu->updateByOb(behavior);
}

void F_Rating::handleFadingTimeoutExpire(Ipv4Address f_ip) {
	fadeRating();
	repu->fadeRating(); ///?
	//NS_LOG_DEBUG("Fading rating item for first and reputation!");
	//reputation redemption mechanism here
	bool f_exist = classifyNode(f_ip, this, repu);
	//f_timer.Cancel();
	if (f_exist)
		reScheduleTimer();
}

void F_Rating::setClassifyCallback(
		Callback<bool, Ipv4Address, F_Rating*, Repu_Rating*> cb) {
	classifyNode = cb;
}

Tru_Rating::Tru_Rating(double fading_f) :
		Rating(fading_f), f_timer(Timer::CANCEL_ON_DESTROY) {
	NS_LOG_FUNCTION(this);
	f_timer.SetFunction(&Tru_Rating::handleFadingTimeoutExpire, this);
	//f_timer.SetArguments(f_factor);
	f_timer.Schedule(Time(Seconds(FADING_TIMEOUT)));
	NS_LOG_DEBUG(
			"Initiate timer for trust rating item at time: "
					+ NumberToString(Simulator::Now().ToDouble(ns3::Time::MS)));
}

void Tru_Rating::cancelTimer() {
	f_timer.Cancel();
}

void Tru_Rating::reScheduleTimer() {
	f_timer.Schedule(Time(Seconds(FADING_TIMEOUT)));
}

void Tru_Rating::handleFadingTimeoutExpire() {
	fadeRating();
	//f_timer.Cancel();
	reScheduleTimer();
	NS_LOG_DEBUG(
			"Reschedule new timer for trust rating item at time: "
					+ NumberToString(Simulator::Now().ToDouble(ns3::Time::MS)));
}

void Tru_Rating::updateByOb(DEVIATIONTEST result) {
	fadeRating();
	int s = ((result == compatiable) ? 0 : 1);
	alpha += s;
	beta += (1 - s);
}

}
}
