#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include <assert.h> 
#include "ns3/aux.h"

using namespace ns3;

struct results
{
   double times = 0;
   uint64_t packets = 0;  
};
/*uint32_t DCCOLLI;
uint32_t DTCOLLI;*/

struct results DCR;
//struct results DTR;
int n;
//TracedCollisionDC
void DC_Delay (struct results *R, Time oldValue, Time newValue)
{
  n++;
  uint64_t temp_new = newValue.GetMilliSeconds ();
  R->times = R->times * (double)R->packets + (double)temp_new;
  R->packets ++;
  R->times = (double)R->times / (double)R->packets;
 // std::cout << " simulation time: " <<Simulator::Now().GetMilliSeconds ()<< " packet number: "<< n << " delay for this packet " << temp_new<< " average delay from so far " <<R->times<<std::endl;
}


/*void DT_Delay (struct results *R, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  R->times = R->times *(double) R->packets + (double)temp_new;
  R->packets ++;
  R->times = R->times / (double)R->packets;
  
}*/

NS_LOG_COMPONENT_DEFINE ("vht-wifi-network");

int main (int argc, char *argv[])
{

    for(int v =40; v < 1100 ; )
    {
        for(int i = 0; i < 2; i++) 
        {

          uint32_t payloadSize_DC = 1472;
         // uint32_t payloadSize_DT = 1472;
          double simulationTime = 60; //seconds
          double distance = 1.0; //meters

          uint32_t gi = 0;
          uint32_t mcs = 9;
          uint32_t channelWidth = 160;
          //int v = 17;
          NodeContainer wifiStaNode;
          wifiStaNode.Create (v);
          NodeContainer wifiApNode;
          wifiApNode.Create (1);
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
                  
          Ssid ssid = Ssid ("ns3-80211ac");

          
          mac.SetType ("ns3::ApWifiMac",
                       "QosSupported", BooleanValue (true),
                       "Ssid", SsidValue (ssid),
                       "BeaconGeneration", BooleanValue (true));
          
          NetDeviceContainer apDevice;
          apDevice = wifi.Install (phy, mac, wifiApNode); 
       

          mac.SetType ("ns3::StaWifiMac",
                       "QosSupported", BooleanValue (true),
                       "Ssid", SsidValue (ssid));

          NetDeviceContainer staDevice;
          staDevice = wifi.Install (phy, mac, wifiStaNode);


          // Set channel width
          Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", 
                       UintegerValue (channelWidth));

                // mobility.
          MobilityHelper mobility;
          Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

          positionAlloc->Add (Vector (0.0, 0.0, 0.0));
          positionAlloc->Add (Vector (distance, 0.0, 0.0));
          mobility.SetPositionAllocator (positionAlloc);

          mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

          mobility.Install (wifiApNode);
          mobility.Install (wifiStaNode);

          InternetStackHelper stack;
          stack.Install (wifiApNode);
          stack.Install (wifiStaNode);

          Ipv4AddressHelper address;

          address.SetBase ("192.168.0.0", "255.255.0.0");
          Ipv4InterfaceContainer staNodeInterface;
          Ipv4InterfaceContainer apNodeInterface;

          staNodeInterface = address.Assign (staDevice);
          apNodeInterface = address.Assign (apDevice);

        //  DC_up = staNodeInterface.GetAddress(0).m_address;
         // DC_down = staNodeInterface.GetAddress(v - 1).m_address;

          ApplicationContainer serverApp, sinkApp;
          UdpServerHelper Server_DC (9);
          //UdpServerHelper Server_DT (10);
          serverApp.Add (Server_DC.Install (wifiApNode.Get (0)));
          //serverApp.Add (Server_DT.Install (wifiApNode.Get (0)));
          serverApp.Start (Seconds (0.0));
          serverApp.Stop (Seconds (simulationTime + 1));
 
 //delay critical onoff
          InetSocketAddress dest_DC (apNodeInterface.GetAddress (0), 9);
          dest_DC.SetTos (0x40); //AC_VO
	  OnOffHelper Client_DC ("ns3::UdpSocketFactory", dest_DC);
	  Client_DC.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=2]"));
	  Client_DC.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=4]"));
	  Client_DC.SetAttribute ("DataRate", StringValue("117kb/s"));
	  Client_DC.SetAttribute ("PacketSize", UintegerValue(payloadSize_DC));


//delay tolerant udpclient
          /*InetSocketAddress dest_DT (apNodeInterface.GetAddress (0), 10);
          dest_DT.SetTos (0x70); //AC_BE
          UdpClientHelper Client_DT (dest_DT);
          Client_DT.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
          Client_DT.SetAttribute ("Interval", TimeValue (Time ("0.01"))); //packets/s
          Client_DT.SetAttribute ("PacketSize", UintegerValue (payloadSize_DT));*/

          ApplicationContainer clientApp;
          clientApp.Add (Client_DC.Install (wifiStaNode));
         // clientApp.Add (Client_DT.Install (wifiStaNode));
          clientApp.Start (Seconds (1.0));
          clientApp.Stop (Seconds (simulationTime + 1));

          Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

          Ptr<UdpServer> server_DC = DynamicCast<UdpServer> (serverApp.Get (0));
          server_DC->TraceConnectWithoutContext ("Delay_DC", MakeBoundCallback (&DC_Delay, &DCR));
         /* Ptr<UdpServer> server_DT = DynamicCast<UdpServer> (serverApp.Get (1));
          server_DT->TraceConnectWithoutContext ("Delay_DT", MakeBoundCallback (&DT_Delay, &DTR));*/

          


          Simulator::Stop (Seconds (simulationTime + 1));
          Simulator::Run ();
          Simulator::Destroy ();

          double throughput_DC = 0;
         // double throughput_DT = 0;
                
          uint32_t totalPacketsThrough_DC = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
          throughput_DC = totalPacketsThrough_DC * payloadSize_DC * 8 / (simulationTime * 1000000.0); //Mbit/s
         // uint32_t totalPacketsThrough_DT = DynamicCast<UdpServer> (serverApp.Get (1))->GetReceived ();
          // throughput_DT = totalPacketsThrough_DT * payloadSize_DT * 8 / (simulationTime * 1000000.0); //Mbit/s

          
          std::ofstream outData;
          outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/singleQueue_NoSaturate_BK.dat", std::ios::app);
         
          if (!outData)
          {
            std::cout <<"can not open the file"<< std::endl;
          }
         else
         {
            std::cout  << v << " " << throughput_DC <<" " <<DCR.times<<" "<< C_DC/simulationTime <<" " <<DCR.packets/(simulationTime/6*10*2*v) << std::endl;
            outData << v << " " << throughput_DC <<" " <<DCR.times<<" "<< C_DC/simulationTime <<" " <<DCR.packets/(simulationTime/6*2*10*v) << std::endl;
           // outData  << v << " " << throughput_DC <<" " << throughput_DT <<" "<<DCR.times<<" "<<DTR.times << " " << C_DC << " " << C_DT<< std::endl;
            outData.close();
         };
         DCR.times = 0;
         DCR .packets = 0;
        // DTR.times = 0;
        // DTR .packets = 0;
         C_DC = 0;
        // C_DT = 0;
         tempCollisionDC = 0;
	 collisionDCStore = 0;
	 lastTime = 0;
	 beforeLastTime = 0;
      }
 v = v + 20;
 }
  return 0;
}



