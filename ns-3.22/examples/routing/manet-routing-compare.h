#include <fstream>
#include <iostream>
#include <vector>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/aodv-module.h"// add confidant components into wscript file ?
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/energy-module.h"
#include <string>
#include "ns3/netanim-module.h"
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("manet-routing-compare");
bool pcap(true);

template <typename T>
std::string NumberToString ( T Number )
{
	std::stringstream ss;
	ss << Number;
	return ss.str();
}


class RoutingExperiment
{

public:
    RoutingExperiment ();
    void Run (int nSinks, double txp, std::string CSVfileName);

    //static void SetMACParam (ns3::NetDeviceContainer & devices,
    //                                 int slotDistance);

    std::string CommandSetup (int argc, char **argv);

    //traced callback methods for trace source in CONFIDANT and Energy remaining
    void TableSizeChange_LUT (const uint32_t node_id, const ns3::Time c_time, const uint32_t n_size);
    void TableSizeChange_FirstHand (const uint32_t node_id, const ns3::Time c_time, const uint32_t n_size);
    void TableSizeChange_Rating (const uint32_t node_id, const ns3::Time c_time, const uint32_t n_size);
    void AddSelfishNode (const uint32_t node_id, const ns3::Time d_time, const Ipv4Address selfishnode, bool);
    void RemainingPowerChange (std::string context, const double oldValue, const double newValue);
    void TableSizeChange_Trust (const uint32_t node_id, const ns3::Time c_time, const uint32_t n_size);
    void initialize_files();

    //AODV-CONFIDANT, SET FOR PASS PARAMETERS FOR METHOD Run(...)
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
    int m_nSinks; // Number of different destinations for all packets sent
    std::string m_protocolName;
    double m_txp; //dBM
    bool m_traceMobility;
     uint32_t m_protocol; // protocol types


    //AODV-CONFDIANT
    int     uofa_totalNumberOfNodes;           // 30, 50, 100
    int     uofa_numberOfMaliciousNodes;       // 10%, 20% and 30%

    int     uofa_nodeMovementSpeedMax;         // 20 m/s
    int     uofa_nodeMovementSpeedMin;         // 5 m/s

    std::string  uofa_simu_areaSizeX;              // Width of Simulation Area: 1000 m
    std::string  uofa_simu_areaSizeY;              // Height of Simulation Area: 1000 m

    double  uofa_simu_totalSimulationTime;    // seconds: 1200 s
    double  uofa_simu_nodeStartupTime_Min;    // determines random time before nodes begin sending packets
    double  uofa_simu_nodeStartupTime_Max;    // determines random time before nodes begin sending packets
    int  nodePauseTime;                        // nodes pause timer [0, 900]

    //AODV-CONFIDANT
    bool enableC;
    bool enableL;
    bool enableT;

    //Set filestreams for record traced output data
    std::string monitor_LUT;
    std::string repu_First;
    std::string repu_Rate;
    std::string detection;
    std::string energy;
    std::string trust;
    std::string dictory_name;

    std::ofstream out_LUT;
    std::ofstream out_First;
    std::ofstream out_Rating;
    std::ofstream out_Dtection;
    std::ofstream out_energy;
    std::ofstream out_Trust;

    NodeContainer* nc;
    NetDeviceContainer* ndc;
    Ipv4InterfaceContainer* ipic;
    void setAllContainer(NodeContainer*, NetDeviceContainer*, Ipv4InterfaceContainer*);
    uint32_t getNodeId(Ipv4Address);

};

RoutingExperiment::RoutingExperiment ()
    :
    uofa_numberOfSinks (10),
    uofa_simu_transmissionPower (7.5),

    port (9),
    bytesTotal (0),
    packetsReceived (0),
    m_CSVfileName ("manet-routing.output.csv"),
    m_traceMobility (false),

    //CONFIDANT
    m_protocol (0), // Default value: use AODV only without any version of CONFIDANT

    // AODV default values
    uofa_totalNumberOfNodes (30),
    uofa_numberOfMaliciousNodes (3),
    uofa_nodeMovementSpeedMax (20),
    uofa_nodeMovementSpeedMin (5),

    uofa_simu_areaSizeX ("1000.0"),
    uofa_simu_areaSizeY ("1000.0"),

    uofa_simu_totalSimulationTime (1200.0),
    uofa_simu_nodeStartupTime_Min (20.0),
    uofa_simu_nodeStartupTime_Max (30.0),
    nodePauseTime (0),
    enableC (false),
    enableL (false),
    enableT (false)
{
    // uofa_maliciousNodeIDs[] = {};
     monitor_LUT = "LUT_SIZE.csv";
   repu_First = "Firsthand_SIZE.csv";
    repu_Rate = "Rating_SIZE.csv";
    detection = "SelfishNode_DetectionTime.csv";
    energy = "Remaining_Power.csv";
     trust = "Trust_SIZE.csv";
}



