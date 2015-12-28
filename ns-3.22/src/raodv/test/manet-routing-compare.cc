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

#include <fstream>
#include <iostream>
#include <vector>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/raodv-module.h"   // RAODV
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/netanim-module.h"

using namespace ns3;
using namespace dsr;

NS_LOG_COMPONENT_DEFINE ("manet-routing-compare");
bool pcap(true);

class RoutingExperiment
{
public:
  RoutingExperiment ();
  void Run (int nSinks, double txp, std::string CSVfileName);
  //static void SetMACParam (ns3::NetDeviceContainer & devices,
  //                                 int slotDistance);
  std::string CommandSetup (int argc, char **argv);

  //RAODV
  int     uofa_numberOfSinks;               // Number of different destinations for all packets sent
  double  uofa_simu_transmissionPower;      // dBm

private:
  Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);
  void ReceivePacket (Ptr<Socket> socket);
  void CheckThroughput ();

  uint32_t port;
  uint32_t bytesTotal;
  uint32_t packetsReceived;

  std::string m_CSVfileName;
  int m_nSinks;
  std::string m_protocolName;
  double m_txp;
  bool m_traceMobility;
  uint32_t m_protocol;

  //RAODV
  int     uofa_totalNumberOfNodes;
  int     uofa_numberOfMaliciousNodes;
  int     uofa_nodeMovementSpeed;           // m/s

  string  uofa_simu_areaSizeX;              // Width of Simulation Area
  string  uofa_simu_areaSizeY;              // Height of Simulation Area

  double  uofa_simu_totalSimulationTime;    // seconds
  double  uofa_simu_nodeStartupTime_Min;    // determines random time before nodes begin sending packets
  double  uofa_simu_nodeStartupTime_Max;    // determines random time before nodes begin sending packets

};

RoutingExperiment::RoutingExperiment ()
  : 
    uofa_numberOfSinks (5),
    uofa_simu_transmissionPower (7.5),


    port (9),
    bytesTotal (0),
    packetsReceived (0),
    m_CSVfileName ("manet-routing.output.csv"),
    m_traceMobility (false),
    m_protocol (5), // AODV  // TODO:change to use RAODV (5)

    // RAODV default values
    uofa_totalNumberOfNodes (10),
    uofa_numberOfMaliciousNodes (0) ,
    uofa_nodeMovementSpeed (20),

    uofa_simu_areaSizeX ("700.0"),
    uofa_simu_areaSizeY ("500.0"),

    uofa_simu_totalSimulationTime (40.0),     // seconds
    uofa_simu_nodeStartupTime_Min (5.0),
    uofa_simu_nodeStartupTime_Max (11.0)
{
    // uofa_maliciousNodeIDs[] = {};
}

static inline std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet)
{
  SocketAddressTag tag;
  bool found;
  found = packet->PeekPacketTag (tag);
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (found)
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (tag.GetAddress ());
      oss << " received one packet from " << addr.GetIpv4 ();
    }
  else
    {
      oss << " received one packet!";
    }
  return oss.str ();
}

void
RoutingExperiment::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while ((packet = socket->Recv ()))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
      NS_LOG_UNCOND (PrintReceivedPacket (socket, packet));
    }
}

void
RoutingExperiment::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << ","
      << kbs << ","
      << packetsReceived << ","
      << m_nSinks << ","
      << m_protocolName << ","
      << m_txp << ""
      << std::endl;


  cout << "Total number of packets recieved successfully = " << packetsReceived << endl;
  

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &RoutingExperiment::CheckThroughput, this);
}

Ptr<Socket>
RoutingExperiment::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));

  return sink;
}

std::string
RoutingExperiment::CommandSetup (int argc, char **argv)
{
  CommandLine cmd;
  // cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
  cmd.AddValue ("protocol", "1=OLSR;2=AODV;3=DSDV;4=DSR;5=RAODV", m_protocol);

  // RAODV PARAMETERS
  cmd.AddValue ("totalNodes", "Total Simulation Time", uofa_totalNumberOfNodes);  // RAODV
  cmd.AddValue ("sinks", "Total Number of Sinks", uofa_numberOfSinks);  // RAODV
  cmd.AddValue ("malicious", "Number of Malicious Nodes", uofa_numberOfMaliciousNodes);  // RAODV
  cmd.AddValue ("mvtSpeed", "Node Movement Speed", uofa_nodeMovementSpeed);  // RAODV
  
  cmd.AddValue ("sizeX", "Simulation Area X-size", uofa_simu_areaSizeX);  // RAODV
  cmd.AddValue ("sizeY", "Simulation Area Y-size", uofa_simu_areaSizeY);  // RAODV

  cmd.AddValue ("totalTime", "Total Simulation Time", uofa_simu_totalSimulationTime);  // RAODV
  cmd.AddValue ("power", "Node Transmission Power", uofa_simu_transmissionPower);  // RAODV
  cmd.AddValue ("startupMin", "Node Startup Min Time", uofa_simu_nodeStartupTime_Min);  // RAODV
  cmd.AddValue ("startupMax", "Node Startup Max Time", uofa_simu_nodeStartupTime_Max);  // RAODV


  cmd.Parse (argc, argv);
  return m_CSVfileName;
}

int
main (int argc, char *argv[])
{
  RoutingExperiment experiment;
  std::string CSVfileName = experiment.CommandSetup (argc,argv);

  //blank out the last output file and write the column headers
  std::ofstream out (CSVfileName.c_str ());
  out << "SimulationSecond," <<
  "ReceiveRate," <<
  "PacketsReceived," <<
  "NumberOfSinks," <<
  "RoutingProtocol," <<
  "TransmissionPower" <<
  std::endl;
  out.close ();

  int nSinks = experiment.uofa_numberOfSinks;
  double txp = experiment.uofa_simu_transmissionPower;

  experiment.Run (nSinks, txp, CSVfileName);
}

void
RoutingExperiment::Run (int nSinks, double txp, std::string CSVfileName)
{
  Packet::EnablePrinting ();
  m_nSinks = nSinks;
  m_txp = txp;
  m_CSVfileName = CSVfileName;

  int nWifis = uofa_totalNumberOfNodes;

  double TotalTime = uofa_simu_totalSimulationTime;
  std::string rate ("2048bps");
  std::string phyMode ("DsssRate11Mbps");
  std::string tr_name ("manet-routing-compare");
  int nodeSpeed = uofa_nodeMovementSpeed; //in m/s
  int nodePause = 0; //in s
  m_protocolName = "protocol";

  Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("64"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));

  //Set Non-unicastMode rate to unicast mode
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));


  // Create separate containers for normal and blackhole
  NodeContainer adhocNodes;
  NodeContainer normalNodesContainer;
  NodeContainer maliciousNodesContainer;

  //ADD MALICIOUS NODES SELECTIVELY BASED ON GENERATED IDs
  std::vector<int> mal;//(uofa_maliciousNodeIDs, uofa_maliciousNodeIDs + sizeof(uofa_maliciousNodeIDs) / sizeof(int));
  std::vector<int>::iterator mal_it;

  for(int i=0; i<uofa_numberOfMaliciousNodes; i++) {
    int r;
    do {
      r = rand() % nWifis;
      mal_it = find(mal.begin(), mal.begin()+i, r);
    } while(mal_it != mal.end());
    mal.push_back(r);
  }

  cout << "Malicious Node IDs:" << endl;
  for(unsigned int i=0; i<mal.size(); i++) {
    cout << mal[i] << endl;
  }


  // Add normal and blackhole nodes to respective containers
  adhocNodes.Create (nWifis);

  for(int i=0; i<nWifis; i++) {
    mal_it = find (mal.begin(), mal.end(), i);
    if(mal_it != mal.end()) {
      maliciousNodesContainer.Add(adhocNodes.Get(i));
    } else {
      normalNodesContainer.Add(adhocNodes.Get(i));
    }
  }



  // setting up wifi phy and channel using helpers
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);

  if (pcap)
    {
      switch (m_protocol) {
        case 2:
          wifiPhy.EnablePcapAll (std::string ("aodv"));
          break;
        case 5:
          wifiPhy.EnablePcapAll (std::string ("raodv"));
          break;
        }
    }

  MobilityHelper mobilityAdhoc;
  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=" + uofa_simu_areaSizeX + "]"));     // CHANGE POSITIONS HERE
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=" + uofa_simu_areaSizeY + "]"));     // CHANGE POSITIONS HERE

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str ()),
                                  "Pause", StringValue (ssPause.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
  mobilityAdhoc.Install (adhocNodes);
  streamIndex += mobilityAdhoc.AssignStreams (adhocNodes, streamIndex);

  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  DsrHelper dsr;
  DsrMainHelper dsrMain;
  
  RaodvHelper raodv;              // RAODV normal
  RaodvHelper malicious_raodv;    // RAODV malicious

  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;

/*
  RAODV NOTE: add 'raodv' to LINE 37 in [examples/routing/wscript]
*/
  switch (m_protocol)
    {
    case 1:
      list.Add (olsr, 100);
      m_protocolName = "OLSR";
      break;
    case 2:
      list.Add (aodv, 100);
      m_protocolName = "AODV";
      break;
    case 3:
      list.Add (dsdv, 100);
      m_protocolName = "DSDV";
      break;
    case 4:
      m_protocolName = "DSR";
      break;
    case 5:                         // RAODV
      list.Add (raodv, 100);        // RAODV
      m_protocolName = "RAODV";     // RAODV
      break;                        // RAODV
    default:
      NS_FATAL_ERROR ("No such protocol:" << m_protocol);
    }

  if ( m_protocol == 5 )  //RAODV
    {
      internet.SetRoutingHelper (raodv);
      internet.Install (normalNodesContainer);

      malicious_raodv.Set("IsMalicious",BooleanValue(true));
      internet.SetRoutingHelper(malicious_raodv);
      internet.Install (maliciousNodesContainer);
    }
  else if (m_protocol < 4)
    {
      internet.SetRoutingHelper (list);
      internet.Install (adhocNodes);
    }
  else if (m_protocol == 4)
    {
      internet.Install (adhocNodes);
      dsrMain.Install (dsr, adhocNodes);
    }

  NS_LOG_INFO ("assigning ip address");

  Ipv4AddressHelper addressAdhoc;
  addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer adhocInterfaces;
  adhocInterfaces = addressAdhoc.Assign (adhocDevices);

  OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address ());
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

  for (int i = 0; i < nSinks; i++)
    {
      Ptr<Socket> sink = SetupPacketReceive (adhocInterfaces.GetAddress (i), adhocNodes.Get (i));

      AddressValue remoteAddress (InetSocketAddress (adhocInterfaces.GetAddress (i), port));
      onoff1.SetAttribute ("Remote", remoteAddress);

      Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
      ApplicationContainer temp = onoff1.Install (adhocNodes.Get (i + nSinks));
      temp.Start (Seconds (var->GetValue (uofa_simu_nodeStartupTime_Min,uofa_simu_nodeStartupTime_Max)));
      temp.Stop (Seconds (TotalTime));
    }

  std::stringstream ss;
  ss << nWifis;
  std::string nodes = ss.str ();

  std::stringstream ss2;
  ss2 << nodeSpeed;
  std::string sNodeSpeed = ss2.str ();

  std::stringstream ss3;
  ss3 << nodePause;
  std::string sNodePause = ss3.str ();

  std::stringstream ss4;
  ss4 << rate;
  std::string sRate = ss4.str ();

  //NS_LOG_INFO ("Configure Tracing.");
  //tr_name = tr_name + "_" + m_protocolName +"_" + nodes + "nodes_" + sNodeSpeed + "speed_" + sNodePause + "pause_" + sRate + "rate";

  //AsciiTraceHelper ascii;
  //Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (tr_name + ".tr").c_str());
  //wifiPhy.EnableAsciiAll (osw);
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (tr_name + ".mob"));

  // Ptr<FlowMonitor> flowmon;
  // FlowMonitorHelper flowmonHelper;
  // flowmon = flowmonHelper.InstallAll ();


  NS_LOG_INFO ("Run Simulation.");


//PINGING
  V4PingHelper ping (adhocInterfaces.GetAddress (nWifis - 1));
  ping.SetAttribute ("Verbose", BooleanValue (true));

  ApplicationContainer p = ping.Install (adhocNodes);
  p.Start (Seconds (0));
  p.Stop (Seconds (100) - Seconds (0.001)); 
//PINGING



  CheckThroughput ();

  Simulator::Stop (Seconds (TotalTime));

  AnimationInterface anim ("manet-routing-compare.xml");  // FOR GENERATING NETANIM XML FILE
  Simulator::Run ();

  // flowmon->SerializeToXmlFile ("manet-routing-compare_flowmon.xml", true, true);

  Simulator::Destroy ();
}

