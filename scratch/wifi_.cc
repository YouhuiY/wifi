//build frame work
//throughput
//delay
// one AP contains two devices corresponding to two type of stations 




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

int v;
std::ofstream outData;
// define the trace function
void DC_Delay (struct results *DCR, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  //outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/delay_0.dat", std::ios::app);
  DCR->times = DCR->times * (double)DCR->packets + (double)temp_new;
  DCR->packets ++;
  DCR->times = DCR->times / (double)DCR->packets;
  /*std::ofstream outData;
      outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/wifi-1.dat", std::ios::app);
      if (!outData)
      {
        std::cout <<"can not open the file"<< std::endl;
      }
      else
      {
        outData  <<temp_new <<" " << DCR->times <<" " << DCR-> packets<<std::endl;
        outData.close();
      }
  //std::cout 
  system("PAUSE");*/
  
}

void DT_Delay (struct results *DTR, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  //outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/delay_0.dat", std::ios::app);
  DTR->times = DTR->times *(double) DTR->packets + (double)temp_new;
  DTR->packets ++;
  DTR->times = DTR->times / (double)DTR->packets;
  //std::cout <<temp_new <<std::endl;
  
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

  for(v = 1; v <= DC_nsta; v+=2)
  {
   for( int u = 0; u < 3; u ++)
    {

	    NodeContainer DCStaNodes;
	    DCStaNodes.Create (v);
	    NodeContainer DTStaNodes;
	    DTStaNodes.Create (v);
	    NodeContainer wifiApNode;
	    wifiApNode.Create (1);
	    std::cout << "MCS value: " << mcs << "Channel width: " << channelWidth << "short GI: " << gi << "number of delay-critical devices: " << v << "number of delay-tolerant devices: " << v << std::endl;


	    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
        phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
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
	    //wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
	    

        NetDeviceContainer DC_staDevices;
        NetDeviceContainer DT_staDevices;
        NetDeviceContainer apDevices;



//for DC devices
	    Ssid ssid_DC = Ssid ("ns3-80211ac_edca_DC");
	    phy.Set ("ChannelNumber", UintegerValue(50));
        mac.SetType ("ns3::StaWifiMac",
                     "QosSupported", BooleanValue (true),
                     "Ssid", SsidValue (ssid_DC));
        DC_staDevices = wifi.Install (phy, mac, DCStaNodes);

        mac.SetType ("ns3::ApWifiMac",
                     "QosSupported", BooleanValue (true),
                     "Ssid", SsidValue (ssid_DC),
                     "BeaconGeneration", BooleanValue (true));
        apDevices.Add (wifi.Install (phy, mac, wifiApNode.Get(0)));


//for DT devices
	    Ssid ssid_DT = Ssid ("ns3-80211ac_edca_DT");
	    phy.Set ("ChannelNumber", UintegerValue(114));
        mac.SetType ("ns3::StaWifiMac",
                     "QosSupported", BooleanValue (true),
                     "Ssid", SsidValue (ssid_DT));
        DT_staDevices = wifi.Install (phy, mac, DTStaNodes);

        mac.SetType ("ns3::ApWifiMac",
                     "QosSupported", BooleanValue (true),
                     "Ssid", SsidValue (ssid_DT),
                     "BeaconGeneration", BooleanValue (true));
        apDevices.Add  (wifi.Install (phy, mac, wifiApNode.Get(0)));


	    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	      
	    MobilityHelper mobility;
	    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	    positionAlloc->Add (Vector (distance, 0.0, 0.0));
	    positionAlloc->Add (Vector (distance, 0.0, 0.0));
	    

	    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	    mobility.Install (wifiApNode);
	    mobility.Install (DCStaNodes);
	    mobility.Install (DTStaNodes);

	    InternetStackHelper stack;
	    stack.Install (wifiApNode);
	    stack.Install (DCStaNodes);
	    stack.Install (DTStaNodes);

	    Ipv4AddressHelper address;
	    address.SetBase ("192.168.0.0", "255.255.0.0");
	    
	    Ipv4InterfaceContainer apNodeInterface;
	    Ipv4InterfaceContainer DCstaNodeInterface;
	    Ipv4InterfaceContainer DTstaNodeInterface;

	    apNodeInterface.Add( address.Assign (apDevices.Get(0)));
	    apNodeInterface.Add( address.Assign (apDevices.Get(1)));

	    //uint32_t  apint = apNodeInterface.GetAddress(0)//.ConvertTo().m_data();
	   // std::string st = apNodeInterface.GetAddress(0).toString();
	   // std::cout <<"the ap's address is: "<<st<< std::endl;
	   
	    int idx = 0;
	    for (int s = 0; s < v; s++)
	    { 
	      DCstaNodeInterface.Add( address.Assign (DC_staDevices.Get (idx)));
	      idx ++;
	    }
	    idx = 0;
	    for (int s = 0; s < v; s++)
	    {
	      DTstaNodeInterface.Add( address.Assign (DT_staDevices.Get (idx)));
	      idx ++;
	    }
	//uint32_t ni = apNodeInterface.GetAddress(0).ptr;
	/*  ******************** test *********************

	std::cout <<apNodeInterface.GetAddress(0).m_address<< std::endl;
	std::cout <<apNodeInterface.GetAddress(0)<< std::endl;
	std::cout <<DCstaNodeInterface.GetAddress(0).m_address<< std::endl;
	std::cout <<DCstaNodeInterface.GetAddress(0)<< std::endl;

	*/
	   //ap_one = apNodeInterface.GetAddress(0).m_address;
	   DC_down = DCstaNodeInterface.GetAddress(0).m_address;
	   DC_up = DCstaNodeInterface.GetAddress(v - 1).m_address;
	   DT_down = DTstaNodeInterface.GetAddress(0).m_address;
	   DT_up = DTstaNodeInterface.GetAddress(v - 1).m_address;
	   //std::cout <<DT_up<< std::endl;

	    // ApplicationContainer serverApp, DC_clientApps, DT_clientApps;
	    idx = 0;

	    ApplicationContainer serverApps;

	    UdpServerHelper DCServer (9);
	    serverApps.Add (DCServer.Install (wifiApNode.Get (0)));
	    UdpServerHelper DTServer (10);
	    serverApps.Add (DTServer.Install (wifiApNode.Get (0)));
	    serverApps.Start (Seconds (0.0));
	    serverApps.Stop (Seconds (simulationTime + 1));

            InetSocketAddress destDC (apNodeInterface.GetAddress (0), 9);
            destDC.SetTos(0xc0);
	    OnOffHelper DCClient ("ns3::UdpSocketFactory", destDC);
            DCClient.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
            DCClient.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
            DCClient.SetAttribute ("DataRate", StringValue("1177600kb/s"));
            DCClient.SetAttribute ("PacketSize", UintegerValue(payloadSize));
       // DCClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));


            InetSocketAddress destDT (apNodeInterface.GetAddress (1), 10);
            destDT.SetTos(0x40);
            OnOffHelper DTClient ("ns3::UdpSocketFactory", destDT);
            DTClient.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
            DTClient.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
            DTClient.SetAttribute ("DataRate", StringValue("1177600kb/s"));
            DTClient.SetAttribute ("PacketSize", UintegerValue(payloadSize));

	    ApplicationContainer clientApps;

	    clientApps.Add (DCClient.Install (DCStaNodes));
	    clientApps.Add (DTClient.Install (DTStaNodes));
	    clientApps.Start (Seconds (1.0));
	    clientApps.Stop (Seconds (simulationTime + 1));
	      

	  
	    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	    // tracer callback function
	    Ptr<UdpServer> server_DC = DynamicCast<UdpServer> (serverApps.Get (0));
	    server_DC->TraceConnectWithoutContext ("Delay_DC", MakeBoundCallback (&DC_Delay, &DCR));

	    Ptr<UdpServer> server_DT = DynamicCast<UdpServer> (serverApps.Get (1));
	    server_DT->TraceConnectWithoutContext ("Delay_DT", MakeBoundCallback (&DT_Delay, &DTR));

	    
	    //Ptr<UdpServer> server = DynamicCast<UdpServer> (serverApp.Get(0));
	    //server->TraceConnectWithoutContext ("Delay", MakeBoundCallback (&TraceArray));
	    
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

            double T_DT = 0.0;
	     // uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
	    //std::cout<<totalPacketsThrough<<std::endl;
	    uint64_t m_DT = DTR.packets * 8 * payloadSize;
	    //std::cout<<m<<std::endl;
	    uint32_t n_DT = simulationTime*1000000.0; 
	    //std::cout<<n<<std::endl;
	    T_DT = (double)m_DT / (double)n_DT; //Mbit/s

	    std::ofstream outData;
	    outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/wifi_edca.dat", std::ios::app);
	    if (!outData)
	    {
	      std::cout <<"can not open the file"<< std::endl;
	    }
	    else
	    {
	      outData  << v << " " << T_DC << " " << T_DT << " " << DCR.times << " " <<DTR.times << std::endl;
	      outData.close();
	    }
 
	    DCR.times = 0;
	    DCR.packets = 0;
	    DTR.times = 0;
	    DTR.packets = 0;
  }
 
}
    
  return 0;
}
