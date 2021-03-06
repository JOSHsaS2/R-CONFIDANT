/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of Kansas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Justin Rohrer <rohrej@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

/*
 * This example program allows one to run ns-3 DSDV, AODV, or OLSR under
 * a typical random waypoint mobility model.
 *
 * By default, the simulation runs for 200 simulated seconds, of which
 * the first 50 are used for start-up time.  The number of nodes is 50.
 * Nodes move according to RandomWaypointMobilityModel with a speed of
 * 20 m/s and no pause time within a 300x1500 m region.  The WiFi is
 * in ad hoc mode with a 2 Mb/s rate (802.11b) and a Friis loss model.
 * The transmit power is set to 7.5 dBm.
 *
 * It is possible to change the mobility and density of the network by
 * directly modifying the speed and the number of nodes.  It is also
 * possible to change the characteristics of the network by changing
 * the transmit power (as power increases, the impact of mobility
 * decreases and the effective density increases).
 *
 * By default, OLSR is used, but specifying a value of 2 for the protocol
 * will cause AODV to be used, and specifying a value of 3 will cause
 * DSDV to be used.
 *
 * By default, there are 10 source/sink data pairs sending UDP data
 * at an application rate of 2.048 Kb/s each.    This is typically done
 * at a rate of 4 64-byte packets per second.  Application data is
 * started at a random time between 50 and 51 seconds and continues
 * to the end of the simulation.
 *
 * The program outputs a few items:
 * - packet receptions are notified to stdout such as:
 *   <timestamp> <node-id> received one packet from <src-address>
 * - each second, the data reception statistics are tabulated and output
 *   to a comma-separated value (csv) file
 * - some tracing and flow monitor configuration that used to work is
 *   left commented inline in the program
 */
#include "manet-routing-compare.h"

static inline std::string PrintReceivedPacket(Ptr<Socket> socket,
		Ptr<Packet> packet) {
	SocketAddressTag tag;
	bool found;
	found = packet->PeekPacketTag(tag);
	std::ostringstream oss;

	oss << Simulator::Now().GetSeconds() << " " << socket->GetNode()->GetId();

	if (found) {
		InetSocketAddress addr = InetSocketAddress::ConvertFrom(
				tag.GetAddress());
		oss << " received one packet from " << addr.GetIpv4();
	} else {
		oss << " received one packet!";
	}
	return oss.str();
}

void RoutingExperiment::ReceivePacket(Ptr<Socket> socket) {
	Ptr<Packet> packet;
	while ((packet = socket->Recv())) {
		bytesTotal += packet->GetSize();
		packetsReceived += 1;
		NS_LOG_UNCOND(PrintReceivedPacket(socket, packet));
	}
}

void RoutingExperiment::CheckThroughput() {
	double kbs = (bytesTotal * 8.0) / 1000;
	bytesTotal = 0;

	std::ofstream out(m_CSVfileName.c_str(), std::ios::app);

	out << (Simulator::Now()).GetSeconds() << "," << kbs << ","
			<< packetsReceived << "," << m_nSinks << "," << m_protocolName
			<< "," << m_txp << "" << std::endl;

	std::cout << "Total number of packets recieved successfully = "
			<< packetsReceived << std::endl;
	out.close();
	packetsReceived = 0;
	Simulator::Schedule(Seconds(1.0), &RoutingExperiment::CheckThroughput,
			this);
}

//traced callback methods for trace source in CONFIDANT and Energy remaining

void RoutingExperiment::TableSizeChange_LUT(const uint32_t node_id,
		const ns3::Time c_time, const uint32_t n_size) {
	out_LUT.open(monitor_LUT.c_str(), std::ios::app);

	out_LUT << node_id << "," << c_time.GetMicroSeconds() / 1000000 << "."
			<< c_time.GetMicroSeconds() % 1000000 << "," << n_size << ""
			<< std::endl;
	out_LUT.close();
}

void RoutingExperiment::TableSizeChange_FirstHand(const uint32_t node_id,
		const ns3::Time c_time, const uint32_t n_size) {
	NS_LOG_FUNCTION(this << node_id << c_time << n_size);
	out_First.open(repu_First.c_str(), std::ios::app);

	out_First << node_id << "," << c_time.GetMicroSeconds() / 1000000 << "."
			<< c_time.GetMicroSeconds() % 1000000 << "," << n_size << ""
			<< std::endl;
	out_First.close();
}

void RoutingExperiment::TableSizeChange_Rating(const uint32_t node_id,
		const ns3::Time c_time, const uint32_t n_size) {
	out_Rating.open(repu_Rate.c_str(), std::ios::app);

	out_Rating << node_id << "," << c_time.GetMicroSeconds() / 1000000 << "."
			<< c_time.GetMicroSeconds() % 1000000 << "," << n_size << ""
			<< std::endl;
	out_Rating.close();
}

void RoutingExperiment::TableSizeChange_Trust(const uint32_t node_id,
		const ns3::Time c_time, const uint32_t n_size) {
	out_Trust.open(trust.c_str(), std::ios::app);

	out_Trust << node_id << "," << c_time.GetMicroSeconds() / 1000000 << "."
			<< c_time.GetMicroSeconds() % 1000000 << "," << n_size << ""
			<< std::endl;
	out_Trust.close();
}

void RoutingExperiment::AddSelfishNode(const uint32_t node_id,
		const ns3::Time d_time, const Ipv4Address selfishnode, bool add) {
	NS_LOG_FUNCTION(this << node_id << selfishnode << add);
	uint32_t nodeId = getNodeId(selfishnode);
	std::string flag = "+";
	if(!add)
		flag = "-";
	out_Dtection.open(detection.c_str(), std::ios::app);

	out_Dtection << node_id << "," << d_time.GetMicroSeconds() / 1000000 << "."
			<< d_time.GetMicroSeconds() % 1000000 << "," << flag << nodeId << ""
			<< std::endl;
	out_Dtection.close();
}

void RoutingExperiment::RemainingPowerChange(std::string context,
		const double oldValue, const double newValue) {
	out_energy.open(energy.c_str(), std::ios::app);
	std::string node_id = context;
	// get the context string and extract node id;
	out_energy << node_id << ","
			<< (Simulator::Now()).GetMicroSeconds() / 1000000 << "."
			<< (Simulator::Now()).GetMicroSeconds() % 1000000 << "," << newValue
			<< "" << std::endl;
	out_energy.close();
}

Ptr<Socket> RoutingExperiment::SetupPacketReceive(Ipv4Address addr,
		Ptr<Node> node) {
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	Ptr<Socket> sink = Socket::CreateSocket(node, tid);
	InetSocketAddress local = InetSocketAddress(addr, port);
	sink->Bind(local);
	sink->SetRecvCallback(
			MakeCallback(&RoutingExperiment::ReceivePacket, this));

	return sink;
}

void RoutingExperiment::setAllContainer(NodeContainer* nc,
		NetDeviceContainer* ndc, Ipv4InterfaceContainer* ipic) {
	this->nc = nc;
	this->ndc = ndc;
	this->ipic = ipic;
}

uint32_t RoutingExperiment::getNodeId(Ipv4Address node_ip) {
	for (uint32_t i = 0; i < ipic->GetN(); i++) {
		if (ipic->GetAddress(i) == node_ip) {
			Ptr<NetDevice> n_p = ndc->Get(i);
			return (n_p->GetNode())->GetId();
		}
	}
	NS_LOG_ERROR("No node with ip found!");
	NS_ASSERT(false);
	return -1;
}

void RoutingExperiment::initialize_files() {
	out_LUT.open(monitor_LUT.c_str());
	out_LUT << "NodeId," << "Time," << "Size" << std::endl;
	out_LUT.close();

	out_First.open(repu_First.c_str());
	out_First << "NodeId," << "Time," << "Size" << std::endl;
	out_First.close();

	out_Rating.open(repu_Rate.c_str());
	out_Rating << "NodeId," << "Time," << "Size" << std::endl;
	out_Rating.close();

	out_Dtection.open(detection.c_str());
	out_Dtection << "NodeId," << "DetectionTime," << "SelfishNodeId"
			<< std::endl;
	out_Dtection.close();

	out_energy.open(energy.c_str());
	out_energy << "NodeId," << "Time," << "RemainingPower" << std::endl;
	out_energy.close();

	out_Trust.open(trust.c_str());
	out_Trust << "NodeId," << "Time," << "Size" << std::endl;
	out_Trust.close();
}

std::string RoutingExperiment::CommandSetup(int argc, char **argv) {
	CommandLine cmd;
	cmd.AddValue("CSVfileName", "The name of the CSV output file name",
			m_CSVfileName);
	cmd.AddValue("pcap", "Write PCAP traces.", pcap);
	cmd.AddValue("traceMobility", "Enable m obility tracing", m_traceMobility);

	// AODV-CONFIDANT VARIED PARAMETERS
	cmd.AddValue("totalNodes", "Total Nodes",
			uofa_totalNumberOfNodes);
	cmd.AddValue("sinks",
			"Total Number of Sinks. Notes: must less than half of total number of nodes",
			uofa_numberOfSinks);
	cmd.AddValue("malicious", "Number of Malicious Nodes",
			uofa_numberOfMaliciousNodes);
	cmd.AddValue("power", "Node Transmission Power",
			uofa_simu_transmissionPower);
	cmd.AddValue("protocol", "0=AODV; 1=CONFIDANT; 2=L-CONFIDANT; 3=TradeOff",
			m_protocol);
	cmd.AddValue("pauseTime", "range: from 0 to 900 s", nodePauseTime);

	cmd.AddValue("mvtSpeedMax", "Max Node Movement Speed",
			uofa_nodeMovementSpeedMax);
	cmd.AddValue("mvtSpeedMin", "Min Node Movement Speed",
			uofa_nodeMovementSpeedMin);
	cmd.AddValue("sizeX", "Simulation Area X-size", uofa_simu_areaSizeX);
	cmd.AddValue("sizeY", "Simulation Area Y-size", uofa_simu_areaSizeY);
	cmd.AddValue("totalTime", "Total Simulation Time",
			uofa_simu_totalSimulationTime);
	cmd.AddValue("startupMin", "Node Startup Min Time",
			uofa_simu_nodeStartupTime_Min);
	cmd.AddValue("startupMax", "Node Startup Max Time",
			uofa_simu_nodeStartupTime_Max);

	cmd.Parse(argc, argv);

	dictory_name = "T" + NumberToString(uofa_totalNumberOfNodes) + "_M"
			+ NumberToString(uofa_numberOfMaliciousNodes) + "_PT"
			+ NumberToString(nodePauseTime) + "_S"
			+ NumberToString(uofa_numberOfSinks) + "_";

	return m_CSVfileName;
}

int main(int argc, char *argv[]) {
	RoutingExperiment experiment;

	//CONFIDANT: invoke command setup method
	std::string CSVfileName = experiment.CommandSetup(argc, argv);
	//experiment.initialize_files();

	//blank out the last output file and write the column headers
	/*
	 std::ofstream out (CSVfileName.c_str ());
	 out << "SimulationSecond," <<
	 "ReceiveRate," <<
	 "PacketsReceived," <<
	 "NumberOfSinks," <<
	 "RoutingProtocol," <<
	 "TransmissionPower" <<
	 std::endl;
	 out.close ();
	 */
	int nSinks = experiment.uofa_numberOfSinks;
	double txp = experiment.uofa_simu_transmissionPower;

	experiment.Run(nSinks, txp, CSVfileName);
}


void RoutingExperiment::Run(int nSinks, double txp, std::string CSVfileName) {
	Packet::EnablePrinting();
	m_nSinks = nSinks;
	m_txp = txp;
	m_CSVfileName = CSVfileName;

	int nWifis = uofa_totalNumberOfNodes;

	double TotalTime = uofa_simu_totalSimulationTime;
	std::string rate("2048bps");
	std::string phyMode("DsssRate11Mbps");
	std::string tr_name;
	//CONFIDANT: CHANGE TO A RANGE
	int nodeSpeedMax = uofa_nodeMovementSpeedMax; //in m/s
	int nodeSpeedMin = uofa_nodeMovementSpeedMin;
	int nodePause = nodePauseTime; //in s
	m_protocolName = "protocol";

	Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue("64"));
	Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(rate));

	//Set Non-unicastMode rate to unicast mode
	Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
			StringValue(phyMode));

	//LogComponentEnable("AodvRoutingProtocol", LOG_LOGIC);
	LogComponentEnable("CONFIDANT_MONITOR", LOG_LEVEL_LOGIC);
	LogComponentEnable("CONFIDANT_REPUTATIONSYSTEM", LOG_LEVEL_LOGIC);
	LogComponentEnable("CONFIDANT_TRUSTMANAGER", LOG_LEVEL_FUNCTION);
	LogComponentEnable("CONFIDANT_PATHMANAGER", LOG_LEVEL_FUNCTION);
	LogComponentEnable("CONFIDANT_confidant", LOG_LEVEL_FUNCTION);

//------------------------------------------------------------------------------------------------------------------------------------

	// Create separate containers for normal and blackhole
	NodeContainer adhocNodes;
	NodeContainer normalNodesContainer;
	NodeContainer maliciousNodesContainer;

	//ADD MALICIOUS NODES SELECTIVELY BASED ON GENERATED IDs
	std::vector<int> mal; //(uofa_maliciousNodeIDs, uofa_maliciousNodeIDs + sizeof(uofa_maliciousNodeIDs) / sizeof(int));
	std::vector<int>::iterator mal_it;

	for (int i = 0; i < uofa_numberOfMaliciousNodes; i++) {
		int r;
		do {
			r = rand() % nWifis;
			mal_it = find(mal.begin(), mal.begin() + i, r);
		} while (mal_it != mal.end());
		mal.push_back(r);
	}

	std::cout << "Malicious Node IDs:" << std::endl;
	for (unsigned int i = 0; i < mal.size(); i++) {
		std::cout << mal[i] << std::endl;
	}

	// Add normal and blackhole nodes to respective containers
	adhocNodes.Create(nWifis);

	for (int i = 0; i < nWifis; i++) {
		mal_it = find(mal.begin(), mal.end(), i);
		if (mal_it != mal.end()) {
			maliciousNodesContainer.Add(adhocNodes.Get(i));
		} else {
			normalNodesContainer.Add(adhocNodes.Get(i));
		}
	}

///------------------------------------------------------------------------------------------------------------------------------------

	// setting up wifi phy and channel using helpers
	WifiHelper wifi;
	wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
	wifiPhy.SetChannel(wifiChannel.Create());

	// Add a non-QoS upper mac, and disable rate control
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
			StringValue(phyMode), "ControlMode", StringValue(phyMode));

	wifiPhy.Set("TxPowerStart", DoubleValue(txp));
	wifiPhy.Set("TxPowerEnd", DoubleValue(txp));

	wifiMac.SetType("ns3::AdhocWifiMac");
	NetDeviceContainer adhocDevices = wifi.Install(wifiPhy, wifiMac,
			adhocNodes);

///--------------------------------------------------------------------------------------------------------------------------------------

	MobilityHelper mobilityAdhoc;
	int64_t streamIndex = 0; // used to get consistent mobility across scenarios

	ObjectFactory pos;
	pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
	pos.Set("X",
			StringValue(
					"ns3::UniformRandomVariable[Min=0.0|Max="
							+ uofa_simu_areaSizeX + "]")); // CHANGE POSITIONS HERE
	pos.Set("Y",
			StringValue(
					"ns3::UniformRandomVariable[Min=0.0|Max="
							+ uofa_simu_areaSizeY + "]")); // CHANGE POSITIONS HERE

	Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<
			PositionAllocator>();
	streamIndex += taPositionAlloc->AssignStreams(streamIndex);

	std::stringstream ssSpeed;
	ssSpeed << "ns3::UniformRandomVariable[Min=" << nodeSpeedMin << "|Max="
			<< nodeSpeedMax << "]";

	std::stringstream ssPause;
	ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";

	mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel", "Speed",
			StringValue(ssSpeed.str()), "Pause", StringValue(ssPause.str()),
			"PositionAllocator", PointerValue(taPositionAlloc));
	mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
	mobilityAdhoc.Install(adhocNodes);
	streamIndex += mobilityAdhoc.AssignStreams(adhocNodes, streamIndex);

	/** Energy Model **/
	/***************************************************************************/
	/* energy source */
	BasicEnergySourceHelper basicSourceHelper;
	// configure energy source
	basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(280));
	basicSourceHelper.Set("PeriodicEnergyUpdateInterval",
			TimeValue(Seconds(50.0)));
	EnergySourceContainer sources = basicSourceHelper.Install(adhocNodes);
	/* device energy model */
	WifiRadioEnergyModelHelper radioEnergyHelper;
	// configure radio energy model
	//radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0174));
	// install device model
	DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(
			adhocDevices, sources);
	/***************************************************************************/

///-------------------------------------------------------------------------------------------------------------------------------------
	AodvHelper aodv;
	AodvHelper malicious_aodv;

	Ipv4ListRoutingHelper list;
	InternetStackHelper internet;

	/*
	 codes to install internet stack and AODV on both regualr and selfish nodes
	 */
	switch (m_protocol) {
	case 0:
		enableC = false;
		enableL = false;
		m_protocolName = "AODV_";
		break;
	case 1:
		enableC = true;
		enableL = false;
		m_protocolName = "AODV_CONFIDANT_";
		break;
	case 2:
		enableC = true;
		enableL = true;
		m_protocolName = "AODV_L-CONFIDANT_";
		break;
	case 3:
		enableC = true;
		enableL = true;
		enableT = true;
		m_protocolName = "AODV_L-CONFIDANT_TradeOff_";
		break;
	default:
		NS_FATAL_ERROR("No such protocol:" << m_protocol);
	}

	internet.SetRoutingHelper(aodv);
	internet.Install(normalNodesContainer);

	for (NodeContainer::Iterator i = normalNodesContainer.Begin();
			i != normalNodesContainer.End(); ++i) {
		Ptr<ns3::aodv::RoutingProtocol> p_aodv =
				(*i)->GetObject<Ipv4>()->GetObject<ns3::aodv::RoutingProtocol>();
		p_aodv->setC(enableC);
		p_aodv->setL(enableL);
		p_aodv->setT(enableT);
		if (enableC) {
			Ptr<NetDevice> p_device = (*i)->GetDevice(0);
			p_device->SetPromiscReceiveCallback(
					MakeCallback(&ns3::confidant::Monitor::handlePassivePack,
							p_aodv->getMonitor()));
		}

	}

	malicious_aodv.Set("IsMalicious", BooleanValue(true));
	internet.SetRoutingHelper(malicious_aodv);
	internet.Install(maliciousNodesContainer);

	for (NodeContainer::Iterator i = maliciousNodesContainer.Begin();
			i != maliciousNodesContainer.End(); ++i) {
		Ptr<ns3::aodv::RoutingProtocol> p_aodv =
				(*i)->GetObject<Ipv4>()->GetObject<ns3::aodv::RoutingProtocol>();
		p_aodv->setC(enableC);
		p_aodv->setL(enableL);
		p_aodv->setT(enableT);
		if (enableC) {
			Ptr<NetDevice> p_device = (*i)->GetDevice(0);
			p_device->SetPromiscReceiveCallback(
					MakeCallback(&ns3::confidant::Monitor::handlePassivePack,
							p_aodv->getMonitor()));
		}

	}

///--------------------------------------------------------------------------------------------------------------------------------------

	NS_LOG_INFO("assigning ip address");

	Ipv4AddressHelper addressAdhoc;
	addressAdhoc.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer adhocInterfaces;
	adhocInterfaces = addressAdhoc.Assign(adhocDevices);
	//set node, netdevice, Ipv4Interface container
	setAllContainer(&adhocNodes, &adhocDevices, &adhocInterfaces);

///-------------------------------------------------------------------------------------------------------------------------------------

	OnOffHelper onoff1("ns3::UdpSocketFactory", Address());
	onoff1.SetAttribute("OnTime",
			StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
	onoff1.SetAttribute("OffTime",
			StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));

	for (int i = 0; i < nSinks; i++) {
		//Ptr<Socket> sink = SetupPacketReceive (adhocInterfaces.GetAddress (i), adhocNodes.Get (i));

		AddressValue remoteAddress(
				InetSocketAddress(adhocInterfaces.GetAddress(i), port));
		onoff1.SetAttribute("Remote", remoteAddress);

		Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
		ApplicationContainer temp = onoff1.Install(adhocNodes.Get(i + nSinks));
		temp.Start(
				Seconds(
						var->GetValue(uofa_simu_nodeStartupTime_Min,
								uofa_simu_nodeStartupTime_Max)));
		temp.Stop(Seconds(TotalTime));
	}

//--------------------------------------------CONFIG TRACED OUTPUT------------------------------------------------

	std::stringstream ss;
	ss << nWifis;
	std::string nodes = ss.str();

	std::stringstream ss2;
	ss2 << uofa_numberOfMaliciousNodes;
	std::string nSelfish = ss2.str();

	std::stringstream ss3;
	ss3 << nodePause;
	std::string sNodePause = ss3.str();

	NS_LOG_INFO("Configure Tracing.");
	tr_name = m_protocolName + this->dictory_name;

	if (pcap)
		wifiPhy.EnablePcapAll(tr_name, false);

	AsciiTraceHelper ascii;
	MobilityHelper::EnableAsciiAll(ascii.CreateFileStream(tr_name + ".mob"));

	// Ptr<FlowMonitor> flowmon;
	// FlowMonitorHelper flowmonHelper;
	// flowmon = flowmonHelper.InstallAll ();

	//AsciiTraceHelper ascii;
	//Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (tr_name + ".tr").c_str());
	//wifiPhy.EnableAsciiAll (osw);

	NS_LOG_INFO("Configure Tracing of Memory Overhead.");

	Config::ConnectWithoutContext(
			"/NodeList/*/$ns3::aodv::RoutingProtocol/$ns3::confidant::Monitor/LUTSizeChange",
			MakeCallback(&RoutingExperiment::TableSizeChange_LUT, this));

	Config::ConnectWithoutContext(
			"/NodeList/*/$ns3::aodv::RoutingProtocol/$ns3::confidant::ReputationSystem/FTableSizeChange",
			MakeCallback(&RoutingExperiment::TableSizeChange_FirstHand, this));

	Config::ConnectWithoutContext(
			"/NodeList/*/$ns3::aodv::RoutingProtocol/$ns3::confidant::ReputationSystem/RTableSizeChange",
			MakeCallback(&RoutingExperiment::TableSizeChange_Rating, this));

	Config::ConnectWithoutContext(
			"/NodeList/*/$ns3::aodv::RoutingProtocol/$ns3::confidant::PathManager/DetectedSelfishNode",
			MakeCallback(&RoutingExperiment::AddSelfishNode, this));

	Config::ConnectWithoutContext(
			"/NodeList/*/$ns3::aodv::RoutingProtocol/$ns3::confidant::TrustManager/TrustTalbleSizeChange",
			MakeCallback(&RoutingExperiment::TableSizeChange_Trust, this));

	NS_LOG_INFO("Configure Tracing of Remaining Energy of source.");

	for (EnergySourceContainer::Iterator i = sources.Begin();
			i != sources.End(); i++) {
		Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource>(
				*i);
		uint32_t id = ((*i)->GetNode())->GetId();
		basicSourcePtr->TraceConnect("RemainingEnergy", NumberToString(id),
				MakeCallback(&RoutingExperiment::RemainingPowerChange, this));
	}

	//enable Packet printing
	Packet::EnablePrinting();
	dictory_name = m_protocolName + dictory_name;
	monitor_LUT = dictory_name + monitor_LUT;
	repu_First = dictory_name + repu_First;
	repu_Rate = dictory_name + repu_Rate;
	detection = dictory_name + detection;
	energy = dictory_name + energy;
	trust = dictory_name + trust;
	initialize_files();

	std::ofstream out((dictory_name + "selfish_node record.csv").c_str());
	out << "NodeId" << std::endl;
	out.close();

	out.open((dictory_name + "selfish_node record.csv").c_str(), std::ios::app);
	for (unsigned int i = 0; i < mal.size(); i++) {
		out << mal[i] << "" << std::endl;
	}
	out.close();

	NS_LOG_INFO("Run Simulation.");

	//CheckThroughput ();

	Simulator::Stop(Seconds(TotalTime));

	//AnimationInterface anim("manet-routing-compare.xml"); // FOR GENERATING NETANIM XML FILE

	Simulator::Run();

	// flowmon->SerializeToXmlFile ((tr_name+"_flowmon.xml").c_str(), true, true);

	Simulator::Destroy();
}
