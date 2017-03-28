//add the head files and the delay function
//add the basic parameter for the program  

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include <assert.h> 
#include <set>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>

//my includes
#include "/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/includes/aux.hh"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("before_change"); 

struct results
{
   double times = 0;
   uint64_t packets = 0;  
};
struct results DCR;
struct results DTR;

//int v;

void DC_Delay (struct results *DCR, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  DCR->times = DCR->times * (double)DCR->packets + (double)temp_new;
  DCR->packets ++;
  DCR->times = DCR->times / (double)DCR->packets;
  std::cout << Simulator::Now() << std::endl;
  
}

void DT_Delay (struct results *DTR, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  DTR->times = DTR->times *(double) DTR->packets + (double)temp_new;
  DTR->packets ++;
  DTR->times = DTR->times / (double)DTR->packets;
  
}

int main (int argc, char *argv[])
{
  double simulationTime = 5; //seconds
  int mcs = 9;
  int channelWidth = 160;
  int gi = 1; //Guard Interval

 //////////////////////////

  uint32_t payloadSize = 1472; 
  double distance = 1.0; //meters

  CommandLine cmd;

  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);

  cmd.AddValue ("mcs", "Modulation and Coding scheme", mcs);
  cmd.AddValue ("channelWidth", "WiFi Channel width", channelWidth);
  cmd.AddValue ("gi", "Guard Interval", gi);

  cmd.AddValue ("DC_nsta", "Number of delay-critical stations per AP", DC_nsta);
  cmd.AddValue ("DT_nsta", "Number of delay-tolerant stations per AP", DT_nsta);

  cmd.AddValue ("payloadSize", "the size of payload", payloadSize);

  cmd.Parse (argc,argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("before_change", LOG_LEVEL_INFO);

  if (simulationTime <= 0)
  {

    simulationTime = 10;
    std::cout<<" Invalid input, simulationTime set to 10 seconds"<<std::endl;
  }
  if (mcs > 9 || mcs < 0)
  {
    mcs = 5;
    std::cout<<" Invalid input, mcs set to 5"<<std::endl;
  }

  std::set<int> cw;
  std::set<int>::iterator it_cw;
  for (int i = 1; i <= 4; i++)
    cw.insert (pow(2,i) * 10);
  it_cw = cw.find (channelWidth);
  if (it_cw == cw.end())
  {
    channelWidth = 80;
    std::cout<<" Invalid input, channelWidth set to 80MHZ"<<std::endl;
  }

  if (gi != 0 && gi != 1)
  {
    gi = 1;
    std::cout<<" Invalid input, gi set to 1"<<std::endl;
  }
    
  if (DC_nsta <= 0)
  {
    DC_nsta = 64;
    std::cout<<" Invalid input, DC_nsta set to 10"<<std::endl;
  }
  if(DT_nsta <= 0)
  {
    DT_nsta = 64;
    std::cout<<" Invalid input, DT_nsta set to 100"<<std::endl;
  }

  if(payloadSize <= 0)
  {
    payloadSize = 1427;
    std::cout << " invalid input, payloadSize set to 1427" << std::endl;

  }

  NodeContainer wifiApNode;
  wifiApNode.Create (1);
  NodeContainer DCStaNodes;
  DCStaNodes.Create (1);
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
	    
  // Set guard interval
  phy.Set ("ShortGuardEnabled", BooleanValue (gi));

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  WifiMacHelper mac;
		          
  std::ostringstream oss;
  oss << "VhtMcs" << mcs;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
                                            "ControlMode", StringValue (oss.str ()));

  
  NetDeviceContainer DC_staDevices;
  NetDeviceContainer apDevice;


  Ssid ssid_DC = Ssid ("ns3-80211ac_edca");
  mac.SetType ("ns3::StaWifiMac",
               "QosSupported", BooleanValue (true),
               "Ssid", SsidValue (ssid_DC));
  DC_staDevices = wifi.Install (phy, mac, DCStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "QosSupported", BooleanValue (true),
               "Ssid", SsidValue (ssid_DC),
               "BeaconGeneration", BooleanValue (true));
  apDevice.Add (wifi.Install (phy, mac, wifiApNode.Get(0)));


  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	      
  MobilityHelper mobility;
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (distance, 0.0, 0.0));
  positionAlloc->Add (Vector (distance, 0.0, 0.0));
	    

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (wifiApNode);
  mobility.Install (DCStaNodes);

  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (DCStaNodes);
  
 


  Ipv4AddressHelper address;
  address.SetBase ("192.168.0.0", "255.255.0.0");
	    
  Ipv4InterfaceContainer apNodeInterface;
  Ipv4InterfaceContainer DCstaNodeInterface;

  apNodeInterface.Add( address.Assign (apDevice.Get(0)));
  DCstaNodeInterface.Add( address.Assign (DC_staDevices.Get (0)));

  DC_down = DCstaNodeInterface.GetAddress(0).m_address;
  DC_up = DCstaNodeInterface.GetAddress(0).m_address;


  ApplicationContainer serverApps;

  UdpServerHelper DCServer (9);
  serverApps.Add (DCServer.Install (wifiApNode.Get (0)));

  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (simulationTime + 1));

  InetSocketAddress destDC (apNodeInterface.GetAddress (0), 9);
  destDC.SetTos(0xc0); 
  UdpClientHelper DCClient (destDC);
  DCClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  DCClient.SetAttribute ("Interval", TimeValue (Time ("0.0000001"))); //packets/s
  DCClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  ApplicationContainer clientApps;
  clientApps.Add (DCClient.Install (DCStaNodes));
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (simulationTime + 1));
	      

	  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  // tracer callback function
  Ptr<UdpServer> server_DC = DynamicCast<UdpServer> (serverApps.Get (0));
  server_DC->TraceConnectWithoutContext ("Delay_DC", MakeBoundCallback (&DC_Delay, &DCR));

   Simulator::Stop (Seconds (simulationTime + 1));
   Simulator::Run ();
   Simulator::Destroy ();

   double T_DC = 0.0;
   // uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
   //std::cout<<totalPacketsThrough<<std::endl;
   uint64_t m_DC = DCR.packets * 8 * payloadSize;
   //std::cout<<m<<std::endl;
   uint32_t n_DC = simulationTime*1000000.0; 
   //std::cout<<n<<std::endl;
   T_DC = (double)m_DC / (double)n_DC; //Mbit/s
   std::cout  <<  T_DC << " " << DCR.times << std::endl;

   return 0;
}