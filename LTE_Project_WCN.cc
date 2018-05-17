
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/lte-fr-strict-algorithm.h"
//#include "ns3/lte-fr-soft-algorithm.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include <ns3/spectrum-module.h>
#include <ns3/log.h>
#include "ns3/netanim-module.h"
#include <ns3/buildings-helper.h>
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/lte-global-pathloss-database.h"
#include <iomanip>
#include <string>
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include <fstream>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LTE_Project");

void
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti,
                       uint16_t targetCellId)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

void
PrintGnuplottableUeListToFile (std::string filename)
{
	std::ofstream outFile;
	outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
	if (!outFile.is_open ())
	{
		NS_LOG_ERROR ("Can't open file " << filename);
		return;
	}
	for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
	{
		Ptr<Node> node = *it;
		int nDevs = node->GetNDevices ();
		for (int j = 0; j < nDevs; j++)
		{
			Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
			if (uedev)
			{
				Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
				outFile << "set label \"" << uedev->GetImsi ()
                    								  << "\" at " << pos.x << "," << pos.y << " left font \"Helvetica,4\" textcolor rgb \"grey\" front point pt 1 ps 0.3 lc rgb \"grey\" offset 0,0"
													  << std::endl;
			}
		}
	}
}

void
PrintGnuplottableEnbListToFile (std::string filename)
{
	std::ofstream outFile;
	outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
	if (!outFile.is_open ())
	{
		NS_LOG_ERROR ("Can't open file " << filename);
		return;
	}
	for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
	{
		Ptr<Node> node = *it;
		int nDevs = node->GetNDevices ();
		for (int j = 0; j < nDevs; j++)
		{
			Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
			if (enbdev)
			{
				Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
				outFile << "set label \"" << enbdev->GetCellId ()
                    								  << "\" at " << pos.x << "," << pos.y
													  << " left font \"Helvetica,4\" textcolor rgb \"white\" front  point pt 2 ps 0.3 lc rgb \"white\" offset 0,0"
													  << std::endl;
			}
		}
	}
}

int main (int argc, char *argv[])
{
/*	  LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);

	   LogComponentEnable ("LteHelper", logLevel);
	   LogComponentEnable ("EpcHelper", logLevel);
	   LogComponentEnable ("EpcEnbApplication", logLevel);
	   LogComponentEnable ("EpcX2", logLevel);
	   LogComponentEnable ("EpcSgwPgwApplication", logLevel);

	  LogComponentEnable ("LteEnbRrc", logLevel);
	  LogComponentEnable ("LteEnbNetDevice", logLevel);
	  LogComponentEnable ("LteUeRrc", logLevel);
	  LogComponentEnable ("LteUeNetDevice", logLevel);
*/


	Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (true));
	Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (true));
	Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));
	Config::SetDefault ("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue (true));
	Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
	Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.0001));

//=======================================================Uplink Power COntrol===================================================================//
	Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (true));
	Config::SetDefault ("ns3::LteUePowerControl::ClosedLoop", BooleanValue (true));
	Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (true));

//=======================================================Variable Declarations===================================================================//
	uint32_t runId = 3;
	uint16_t numberOfRandomUes = 3;
	 uint32_t numUes = 3;
	double simTime = 3.00;
	bool generateSpectrumTrace = true;
	bool generateRem = false;
	int32_t remRbId = -1;
	uint8_t bandwidth = 50	;
	double distance = 1000;
	double interPacketInterval = 100;
	double enbDist = 500.0;

	Box macroUeBox = Box (-distance * 0.1, distance * 1.5, -distance * 0.1, distance * 1.5, 1.5, 1.5);

//=======================================================Command Line Arguments===================================================================//
	CommandLine cmd;
	cmd.AddValue ("numberOfUes", "Number of random UEs", numberOfRandomUes);
	cmd.AddValue ("simTime", "Total duration of the simulation (in seconds)", simTime);
	cmd.AddValue ("generateSpectrumTrace", "if true, will generate a Spectrum Analyzer trace", generateSpectrumTrace);
	cmd.AddValue ("generateRem", "if true, will generate a REM and then abort the simulation", generateRem);
	cmd.AddValue ("remRbId", "Resource Block Id, for which REM will be generated,"
			"default value is -1, what means REM will be averaged from all RBs", remRbId);
	cmd.AddValue ("runId", "runId", runId);
	cmd.Parse (argc, argv);

	RngSeedManager::SetSeed (1);
	RngSeedManager::SetRun (runId);

	Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
	Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
	lteHelper->SetEpcHelper (epcHelper);
	lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm");

	 //ConfigStore inputConfig;
	// inputConfig.ConfigureDefaults();

	Ptr<Node> pgw = epcHelper->GetPgwNode ();

//=======================================================Create a Remote Host===================================================================//
	NodeContainer remoteHostContainer;
	remoteHostContainer.Create (1);
	Ptr<Node> remoteHost = remoteHostContainer.Get (0);
	InternetStackHelper internet;
	internet.Install (remoteHostContainer);

//=======================================================Create the Internet===================================================================//
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
	NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
	Ipv4AddressHelper ipv4h;
	ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
	// interface 0 is localhost, 1 is the p2p device
	Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

//=======================================================Routing of Internet Host Towards LTE Network===================================================================//
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
	remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

//=======================================================Set Fading And Path Loss Model for Network===================================================================//

	lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
	lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));

	std::ifstream ifTraceFile;
	ifTraceFile.open ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad", std::ifstream::in);
	if (ifTraceFile.good ())
	{
		// script launched by test.py
		lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
	}
	else
	{
		lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
	}

	lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
	lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
	lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
	lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));

//=======================================================Create Nodes and EnB's===================================================================//
	NodeContainer enbNodes;
	NodeContainer centerUeNodes;
	NodeContainer edgeUeNodes;
	NodeContainer randomUeNodes;
	enbNodes.Create (3);
	centerUeNodes.Create (numUes);
	edgeUeNodes.Create (numUes);
	randomUeNodes.Create (numberOfRandomUes);

//=======================================================Install Mobility Model===================================================================//

	Ptr<ListPositionAllocator> remotehostPositionAlloc = CreateObject<ListPositionAllocator> ();
	remotehostPositionAlloc->Add (Vector (0.0, 0.0, 0.0));
	remotehostPositionAlloc->Add (Vector (distance,  0.0, 0.0));
	remotehostPositionAlloc->Add (Vector (distance * 0.5, distance * 0.866, 0.0));
	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.SetPositionAllocator (remotehostPositionAlloc);
	mobility.Install (remoteHostContainer);

	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
	enbPositionAlloc->Add (Vector (0.0, 0.0, 0.0));                       // eNB1
	enbPositionAlloc->Add (Vector (enbDist,  0.0, 0.0));                 // eNB2
	enbPositionAlloc->Add (Vector (enbDist * 0.5, distance * 0.866, 0.0));   // eNB3
	//MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.SetPositionAllocator (enbPositionAlloc);
	mobility.Install (enbNodes);

/*	MobilityHelper ueMobility;
	Ptr<ListPositionAllocator> centerUePositionAlloc = CreateObject<ListPositionAllocator> ();
	ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
	mobility.SetPositionAllocator (centerUePositionAlloc);
	ueMobility.Install (centerUeNodes);
	centerUeNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, yForUe, 0));
	centerUeNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (s, 0, 0));
	centerUeNodes.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (10, yForUe+50, 0));
	centerUeNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (s, 0, 0));
	centerUeNodes.Get (2)->GetObject<MobilityModel> ()->SetPosition (Vector (15, yForUe+60, 0));
	centerUeNodes.Get (2)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (s, 0, 0));

//	MobilityHelper ueMobility;
	Ptr<ListPositionAllocator> edgeUePositionAlloc = CreateObject<ListPositionAllocator> ();
	ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
	mobility.SetPositionAllocator (edgeUePositionAlloc);
	ueMobility.Install (edgeUeNodes);
	edgeUeNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, yForUe, 0));
	edgeUeNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (s, 0, 0));
	edgeUeNodes.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (10, yForUe+30, 0));
	edgeUeNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (s, 0, 0));
	edgeUeNodes.Get (2)->GetObject<MobilityModel> ()->SetPosition (Vector (15, yForUe+55, 0));
	edgeUeNodes.Get (2)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (s, 0, 0));
*/

	Ptr<ListPositionAllocator> edgeUePositionAlloc = CreateObject<ListPositionAllocator> ();
	edgeUePositionAlloc->Add (Vector (distance * 0.5, distance * 0.28867, 0.0));  // edgeUE1
	edgeUePositionAlloc->Add (Vector (distance * 0.5, distance * 0.28867, 0.0));  // edgeUE2
	edgeUePositionAlloc->Add (Vector (distance * 0.5, distance * 0.28867, 0.0));  // edgeUE3
	mobility.SetPositionAllocator (edgeUePositionAlloc);
	mobility.Install (edgeUeNodes);

	Ptr<ListPositionAllocator> centerUePositionAlloc = CreateObject<ListPositionAllocator> ();
	centerUePositionAlloc->Add (Vector (0.0, 0.0, 0.0));                                      // centerUE1
	centerUePositionAlloc->Add (Vector (distance,  0.0, 0.0));                            // centerUE2
	centerUePositionAlloc->Add (Vector (distance * 0.5, distance * 0.866, 0.0));      // centerUE3
	mobility.SetPositionAllocator (centerUePositionAlloc);
	mobility.Install (centerUeNodes);

	Ptr<RandomBoxPositionAllocator> randomUePositionAlloc = CreateObject<RandomBoxPositionAllocator> ();
	Ptr<UniformRandomVariable> xVal = CreateObject<UniformRandomVariable> ();
	xVal->SetAttribute ("Min", DoubleValue (macroUeBox.xMin));
	xVal->SetAttribute ("Max", DoubleValue (macroUeBox.xMax));
	randomUePositionAlloc->SetAttribute ("X", PointerValue (xVal));
	Ptr<UniformRandomVariable> yVal = CreateObject<UniformRandomVariable> ();
	yVal->SetAttribute ("Min", DoubleValue (macroUeBox.yMin));
	yVal->SetAttribute ("Max", DoubleValue (macroUeBox.yMax));
	randomUePositionAlloc->SetAttribute ("Y", PointerValue (yVal));
	Ptr<UniformRandomVariable> zVal = CreateObject<UniformRandomVariable> ();
	zVal->SetAttribute ("Min", DoubleValue (macroUeBox.zMin));
	zVal->SetAttribute ("Max", DoubleValue (macroUeBox.zMax));
	randomUePositionAlloc->SetAttribute ("Z", PointerValue (zVal));
	mobility.SetPositionAllocator (randomUePositionAlloc);
	mobility.Install (randomUeNodes);

//=======================================================Create and Install LTE over Devices and ENB's===================================================================//
	NetDeviceContainer enbDevs;
	NetDeviceContainer edgeUeDevs;
	NetDeviceContainer centerUeDevs;
	NetDeviceContainer randomUeDevs;
/*
//=======================================================PfFfMacScheduler===================================================================//
	lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
	lteHelper->SetSchedulerAttribute ("UlCqiFilter", EnumValue (FfMacScheduler::PUSCH_UL_CQI));
	lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth));
	lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth));
*/
//=======================================================TdMtFfMacScheduler===================================================================//
	lteHelper->SetSchedulerType ("ns3::TdMtFfMacScheduler");    // TD-MT scheduler
	lteHelper->SetSchedulerAttribute ("UlCqiFilter", EnumValue (TdMtFfMacScheduler::PUSCH_UL_CQI));
	lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth));
	lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth));
/*
//=======================================================TdBetFfMacScheduler===================================================================//
	lteHelper->SetSchedulerType ("ns3::TdBetFfMacScheduler");   // TD-BET scheduler
	lteHelper->SetSchedulerAttribute ("UlCqiFilter", EnumValue (TdBetFfMacScheduler::PUSCH_UL_CQI));
	lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth));
	lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth));
*/
	 //Any of the MAC Scheduler can be used by commenting out the others//

	std::string frAlgorithmType = lteHelper->GetFfrAlgorithmType ();
	NS_LOG_DEBUG ("FrAlgorithmType: " << frAlgorithmType);

	if (frAlgorithmType == "ns3::LteFrHardAlgorithm")
	{

		//Nothing to configure here in automatic mode

	}
	else if (frAlgorithmType == "ns3::LteFrStrictAlgorithm")
	{

		lteHelper->SetFfrAlgorithmAttribute ("RsrqThreshold", UintegerValue (32));
		lteHelper->SetFfrAlgorithmAttribute ("CenterPowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB_6));
		lteHelper->SetFfrAlgorithmAttribute ("EdgePowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB3));
		lteHelper->SetFfrAlgorithmAttribute ("CenterAreaTpc", UintegerValue (0));
		lteHelper->SetFfrAlgorithmAttribute ("EdgeAreaTpc", UintegerValue (3));

		//ns3::LteFrStrictAlgorithm works with Absolute Mode Uplink Power Control
		Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (false));

	}
	else if (frAlgorithmType == "ns3::LteFrSoftAlgorithm")
	{

		lteHelper->SetFfrAlgorithmAttribute ("AllowCenterUeUseEdgeSubBand", BooleanValue (true));
		lteHelper->SetFfrAlgorithmAttribute ("RsrqThreshold", UintegerValue (25));
		lteHelper->SetFfrAlgorithmAttribute ("CenterPowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB_6));
		lteHelper->SetFfrAlgorithmAttribute ("EdgePowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB3));
		lteHelper->SetFfrAlgorithmAttribute ("CenterAreaTpc", UintegerValue (0));
		lteHelper->SetFfrAlgorithmAttribute ("EdgeAreaTpc", UintegerValue (3));

		//ns3::LteFrSoftAlgorithm works with Absolute Mode Uplink Power Control
		Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (false));

	}
	else if (frAlgorithmType == "ns3::LteFfrSoftAlgorithm")
	{

		lteHelper->SetFfrAlgorithmAttribute ("CenterRsrqThreshold", UintegerValue (30));
		lteHelper->SetFfrAlgorithmAttribute ("EdgeRsrqThreshold", UintegerValue (25));
		lteHelper->SetFfrAlgorithmAttribute ("CenterAreaPowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB_6));
		lteHelper->SetFfrAlgorithmAttribute ("MediumAreaPowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB_1dot77));
		lteHelper->SetFfrAlgorithmAttribute ("EdgeAreaPowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB3));
		lteHelper->SetFfrAlgorithmAttribute ("CenterAreaTpc", UintegerValue (1));
		lteHelper->SetFfrAlgorithmAttribute ("MediumAreaTpc", UintegerValue (2));
		lteHelper->SetFfrAlgorithmAttribute ("EdgeAreaTpc", UintegerValue (3));

		//ns3::LteFfrSoftAlgorithm works with Absolute Mode Uplink Power Control
		Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (false));

	}
	else if (frAlgorithmType == "ns3::LteFfrEnhancedAlgorithm")
	{

		lteHelper->SetFfrAlgorithmAttribute ("RsrqThreshold", UintegerValue (25));
		lteHelper->SetFfrAlgorithmAttribute ("DlCqiThreshold", UintegerValue (10));
		lteHelper->SetFfrAlgorithmAttribute ("UlCqiThreshold", UintegerValue (10));
		lteHelper->SetFfrAlgorithmAttribute ("CenterAreaPowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB_6));
		lteHelper->SetFfrAlgorithmAttribute ("EdgeAreaPowerOffset",
				UintegerValue (LteRrcSap::PdschConfigDedicated::dB3));
		lteHelper->SetFfrAlgorithmAttribute ("CenterAreaTpc", UintegerValue (0));
		lteHelper->SetFfrAlgorithmAttribute ("EdgeAreaTpc", UintegerValue (3));

		//ns3::LteFfrEnhancedAlgorithm works with Absolute Mode Uplink Power Control
		Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (false));

	}
	else if (frAlgorithmType == "ns3::LteFfrDistributedAlgorithm")
	{

		NS_FATAL_ERROR ("ns3::LteFfrDistributedAlgorithm not supported in this example. Please run lena-distributed-ffr");

	}
	else
	{
		lteHelper->SetFfrAlgorithmType ("ns3::LteFrNoOpAlgorithm");
	}

	lteHelper->SetFfrAlgorithmAttribute ("FrCellTypeId", UintegerValue (1));
	enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (0)));

	lteHelper->SetFfrAlgorithmAttribute ("FrCellTypeId", UintegerValue (2));
	enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (1)));

	lteHelper->SetFfrAlgorithmAttribute ("FrCellTypeId", UintegerValue (3));
	enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (2)));

	//FR algorithm reconfiguration if needed
	PointerValue tmp;
	enbDevs.Get (0)->GetAttribute ("LteFfrAlgorithm", tmp);
	Ptr<LteFfrAlgorithm> ffrAlgorithm = DynamicCast<LteFfrAlgorithm> (tmp.GetObject ());
	ffrAlgorithm->SetAttribute ("FrCellTypeId", UintegerValue (1));

//=======================================================Create and Install LTE over Devices and ENB's===================================================================//
	edgeUeDevs = lteHelper->InstallUeDevice (edgeUeNodes);
	centerUeDevs = lteHelper->InstallUeDevice (centerUeNodes);
	randomUeDevs = lteHelper->InstallUeDevice (randomUeNodes);

//=======================================================Install IP stack on Center UE's===================================================================//
	internet.Install (centerUeNodes);
	Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (centerUeDevs));

//=======================================================Install IP stack on Edge UE's===================================================================//
	internet.Install (edgeUeNodes);
	//Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (edgeUeDevs));

//=======================================================Install IP stack on Random UE's===================================================================//
	internet.Install (randomUeNodes);
	//Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (randomUeDevs));


//=======================================================Attach Edge UE's to EnB's===================================================================//
	for (uint32_t i = 0; i < edgeUeDevs.GetN (); i++)
	{
		lteHelper->Attach (edgeUeDevs.Get (i), enbDevs.Get (i));
	}
//=======================================================Attach Edge UE's to EnB's===================================================================//
	for (uint32_t i = 0; i < centerUeDevs.GetN (); i++)
	{
		lteHelper->Attach (centerUeDevs.Get (i), enbDevs.Get (i));
	}

//=======================================================Attach Random UE's to EnB's===================================================================//
	lteHelper->AttachToClosestEnb (randomUeDevs, enbDevs);

	/*
	// Activate a data radio bearer
	enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
	EpsBearer bearer (q);
	lteHelper->ActivateDataRadioBearer (edgeUeDevs, bearer);
	lteHelper->ActivateDataRadioBearer (centerUeDevs, bearer);
	lteHelper->ActivateDataRadioBearer (randomUeDevs, bearer);
	 */

//=======================================================Spectrum Analyser===================================================================//
	NodeContainer spectrumAnalyzerNodes;
	spectrumAnalyzerNodes.Create (1);
	SpectrumAnalyzerHelper spectrumAnalyzerHelper;

	if (generateSpectrumTrace)
	{
		Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
		//position of Spectrum Analyzer
		positionAlloc->Add (Vector (0.0, 0.0, 0.0));                              // eNB1
		positionAlloc->Add (Vector (distance,  0.0, 0.0));                        // eNB2
		positionAlloc->Add (Vector (distance * 0.5, distance * 0.866, 0.0));          // eNB3

		MobilityHelper mobility;
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.SetPositionAllocator (positionAlloc);
		mobility.Install (spectrumAnalyzerNodes);

		Ptr<LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)->GetObject<LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<LteSpectrumPhy> ();
		Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();

		spectrumAnalyzerHelper.SetChannel (dlChannel);
		Ptr<SpectrumModel> sm = LteSpectrumValueHelper::GetSpectrumModel (100, bandwidth);
		spectrumAnalyzerHelper.SetRxSpectrumModel (sm);
		spectrumAnalyzerHelper.SetPhyAttribute ("Resolution", TimeValue (MicroSeconds (10)));
		spectrumAnalyzerHelper.SetPhyAttribute ("NoisePowerSpectralDensity", DoubleValue (1e-15));     // -120 dBm/Hz
		spectrumAnalyzerHelper.EnableAsciiAll ("spectrum-analyzer-output");
		spectrumAnalyzerHelper.Install (spectrumAnalyzerNodes);
	}

//=======================================================Install and Start Applications on UE's and Remote Hosts===================================================================//
	uint16_t dlPort = 1234;
	uint16_t ulPort = 2000;
	uint16_t otherPort = 3000;
	ApplicationContainer clientApps;
	ApplicationContainer serverApps;
	for (uint32_t u = 0; u < centerUeNodes.GetN (); ++u)
	{
		++ulPort;
		++otherPort;
		PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
		PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
		PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
		serverApps.Add (dlPacketSinkHelper.Install (centerUeNodes.Get(u)));
		serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
		serverApps.Add (packetSinkHelper.Install (centerUeNodes.Get(u)));

		UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
		dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

		UdpClientHelper ulClient (remoteHostAddr, ulPort);
		ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

		UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
		client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		client.SetAttribute ("MaxPackets", UintegerValue(1000000));

		clientApps.Add (dlClient.Install (remoteHost));
		clientApps.Add (ulClient.Install (centerUeNodes.Get(u)));
		if (u+1 < centerUeNodes.GetN ())
		{
			clientApps.Add (client.Install (centerUeNodes.Get(u+1)));
		}
		else
		{
			clientApps.Add (client.Install (centerUeNodes.Get(0)));
		}
	}

	for (uint32_t u = 0; u < edgeUeNodes.GetN (); ++u)
	{
		++ulPort;
		++otherPort;
		PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
		PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
		PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
		serverApps.Add (dlPacketSinkHelper.Install (edgeUeNodes.Get(u)));
		serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
		serverApps.Add (packetSinkHelper.Install (edgeUeNodes.Get(u)));

		UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
		dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

		UdpClientHelper ulClient (remoteHostAddr, ulPort);
		ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

		UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
		client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		client.SetAttribute ("MaxPackets", UintegerValue(1000000));

		clientApps.Add (dlClient.Install (remoteHost));
		clientApps.Add (ulClient.Install (edgeUeNodes.Get(u)));
		if (u+1 < edgeUeNodes.GetN ())
		{
			clientApps.Add (client.Install (edgeUeNodes.Get(u+1)));
		}
		else
		{
			clientApps.Add (client.Install (edgeUeNodes.Get(0)));
		}
	}

	for (uint32_t u = 0; u < randomUeNodes.GetN (); ++u)
	{
		++ulPort;
		++otherPort;
		PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
		PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
		PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
		serverApps.Add (dlPacketSinkHelper.Install (randomUeNodes.Get(u)));
		serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
		serverApps.Add (packetSinkHelper.Install (randomUeNodes.Get(u)));

		UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
		dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

		UdpClientHelper ulClient (remoteHostAddr, ulPort);
		ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

		UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
		client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
		client.SetAttribute ("MaxPackets", UintegerValue(1000000));

		clientApps.Add (dlClient.Install (remoteHost));
		clientApps.Add (ulClient.Install (randomUeNodes.Get(u)));
		if (u+1 < randomUeNodes.GetN ())
		{
			clientApps.Add (client.Install (randomUeNodes.Get(u+1)));
		}
		else
		{
			clientApps.Add (client.Install (randomUeNodes.Get(0)));
		}
	}

	serverApps.Start (Seconds (0.01));
	clientApps.Start (Seconds (0.01));

//=======================================================REM===================================================================//
	Ptr<RadioEnvironmentMapHelper> remHelper;
	if (generateRem)
	{
		PrintGnuplottableEnbListToFile ("enbs.txt");
		PrintGnuplottableUeListToFile ("ues.txt");

		remHelper = CreateObject<RadioEnvironmentMapHelper> ();
		remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
		remHelper->SetAttribute ("OutputFile", StringValue ("lena-frequency-reuse.rem"));
		remHelper->SetAttribute ("XMin", DoubleValue (macroUeBox.xMin));
		remHelper->SetAttribute ("XMax", DoubleValue (macroUeBox.xMax));
		remHelper->SetAttribute ("YMin", DoubleValue (macroUeBox.yMin));
		remHelper->SetAttribute ("YMax", DoubleValue (macroUeBox.yMax));
		remHelper->SetAttribute ("Z", DoubleValue (1.5));
		remHelper->SetAttribute ("XRes", UintegerValue (500));
		remHelper->SetAttribute ("YRes", UintegerValue (500));
		if (remRbId >= 0)
		{
			remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));
			remHelper->SetAttribute ("RbId", IntegerValue (remRbId));
		}

		remHelper->Install ();
	}
	else
	{
		Simulator::Stop (Seconds (simTime));
	}

//=======================================================Flow Monitor===================================================================//
  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> flowMonitor;
  flowHelper.SetMonitorAttribute("StartTime", TimeValue(Time(1.0)));
  flowHelper.SetMonitorAttribute("JitterBinWidth", ns3::DoubleValue(0.001));
  flowHelper.SetMonitorAttribute("DelayBinWidth", ns3::DoubleValue(0.001));
  flowHelper.SetMonitorAttribute("PacketSizeBinWidth", DoubleValue(20));
  flowMonitor = flowHelper.InstallAll();

  flowMonitor = flowHelper.Install (enbNodes);
  flowMonitor = flowHelper.Install (centerUeNodes);
  flowMonitor = flowHelper.Install (edgeUeNodes);
  flowMonitor = flowHelper.Install (randomUeNodes);
  flowMonitor = flowHelper.GetMonitor();

 //=======================================================Add X2 Interface and Initiate Handover===================================================================//
	lteHelper->AddX2Interface (enbNodes);

	lteHelper->HandoverRequest (Seconds (0.300), edgeUeDevs.Get (2), enbDevs.Get (2), enbDevs.Get (1));
	lteHelper->HandoverRequest (Seconds (0.400), randomUeDevs.Get (0), enbDevs.Get (2), enbDevs.Get (0));
	lteHelper->HandoverRequest (Seconds (0.500), randomUeDevs.Get (2), enbDevs.Get (1), enbDevs.Get (2));
	lteHelper->HandoverRequest (Seconds (0.600), edgeUeDevs.Get (0), enbDevs.Get (0), enbDevs.Get (1));

	//      lteHelper->HandoverRequest (Seconds (0.600), centerUeDevs.Get (0), enbDevs.Get (0), enbDevs.Get (1));
	//      lteHelper->HandoverRequest (Seconds (0.700), centerUeDevs.Get (1), enbDevs.Get (1), enbDevs.Get (2));
	//      lteHelper->HandoverRequest (Seconds (0.80), centerUeDevs.Get (2), enbDevs.Get (2), enbDevs.Get (1));
	//      lteHelper->HandoverRequest (Seconds (0.90), randomUeDevs.Get (0), enbDevs.Get (2), enbDevs.Get (0));
	//      lteHelper->HandoverRequest (Seconds (1.0), randomUeDevs.Get (2), enbDevs.Get (1), enbDevs.Get (2));



	//lteHelper->EnableTraces ();
	//lteHelper->EnablePhyTraces ();
	//lteHelper->EnableMacTraces ();
	//lteHelper->EnableRlcTraces ();
	// lteHelper->EnablePdcpTraces ();

//=======================================================Connect custom trace sinks for RRC connection establishment and handover notification===================================================================//

	  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
	                   MakeCallback (&NotifyConnectionEstablishedEnb));
	  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
	                   MakeCallback (&NotifyConnectionEstablishedUe));
	  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
	                   MakeCallback (&NotifyHandoverStartEnb));
	  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart",
	                   MakeCallback (&NotifyHandoverStartUe));
	  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
	                   MakeCallback (&NotifyHandoverEndOkEnb));
	  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",
	                   MakeCallback (&NotifyHandoverEndOkUe));


	AnimationInterface anim ("LTE_Project.xml");

	//anim.EnablePacketMetadata (true);

	anim.SetConstantPosition (remoteHostContainer.Get (0), 300.0, 1.0);
	anim.SetConstantPosition(spectrumAnalyzerNodes.Get(0),990.0,1200.0);//Spectrum Analyser Node

	anim.SetConstantPosition(enbNodes.Get(0),1098.15, 366.05);//EnB Node 0
	anim.SetConstantPosition(centerUeNodes.Get(0),950.0,366.05);//3
	anim.SetConstantPosition(edgeUeNodes.Get(0),790.0,700.0);//6
	anim.SetConstantPosition(randomUeNodes.Get(0),1000.0,450.0);//9

	anim.SetConstantPosition(enbNodes.Get(1),950.0,950.0);//EnB Node 1
	anim.SetConstantPosition(centerUeNodes.Get(1),1000.0,900.0);//4
	anim.SetConstantPosition(edgeUeNodes.Get(1),970.0,750.0);//7
	anim.SetConstantPosition(randomUeNodes.Get(1),750.0,770.0);//10

	anim.SetConstantPosition(enbNodes.Get(2),366.05,950.0);//ENB Node 2
	anim.SetConstantPosition(centerUeNodes.Get(2),300.0,900.0);//5
	anim.SetConstantPosition(edgeUeNodes.Get(2),700.0,800.0);//8
	anim.SetConstantPosition(randomUeNodes.Get(2),600.0,750.0);//11

	Simulator::Run ();

	flowMonitor->CheckForLostPackets ();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
	std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats ();
	std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i;
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
	{
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

		NS_LOG_DEBUG ("Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")");

		std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
		std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
		std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
		std::cout << "  Throughput: " << i->second.rxBytes * 8.0/20/1024 << " Mbps\n";
	}

	flowMonitor->SerializeToXmlFile("LTE_Project.xml", true, true);

	Simulator::Destroy ();
	return 0;
}
