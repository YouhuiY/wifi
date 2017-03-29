
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"


#include <assert.h> 
#include "/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/includes/aux.hh"

using namespace ns3;


struct results
{
   double times = 0;
   uint64_t packets = 0;  
};
struct results DCR;
struct results DTR;

void DC_Delay (struct results *R, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  R->times = R->times * (double)R->packets + (double)temp_new;
  R->packets ++;
  R->times = R->times / (double)R->packets;
}

void DT_Delay (struct results *R, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  R->times = R->times *(double) R->packets + (double)temp_new;
  R->packets ++;
  R->times = R->times / (double)R->packets;
  
}

NS_LOG_COMPONENT_DEFINE ("vht-wifi-network");

int main (int argc, char *argv[])
{

    for(int v = 1; v < 64; v++)
    {
        for(int i = 0; i < 3; i++) 
        {


          uint32_t payloadSize = 1472;
          double simulationTime = 10; //seconds
          double distance = 1.0; //meters

          uint32_t gi = 0;
          uint32_t stations = 2;
          uint32_t mcs = 9;
          uint32_t channelWidth = 160;


          NodeContainer wifiStaNode_DC;
          wifiStaNode_DC.Create (v);
          NodeContainer wifiStaNode_DT;
          wifiStaNode_DT.Create(1);
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

          mac.SetType ("ns3::StaWifiMac",
                       "QosSupported", BooleanValue (true),
                       "Ssid", SsidValue (ssid));

          NetDeviceContainer staDevice_DC, staDevice_DT;
          staDevice_DC = wifi.Install (phy, mac, wifiStaNode_DC);
          staDevice_DT = wifi.Install (phy, mac, wifiStaNode_DT);

          mac.SetType ("ns3::ApWifiMac",
                       "QosSupported", BooleanValue (true),
                       "Ssid", SsidValue (ssid),
                       "BeaconGeneration", BooleanValue (true)
                      );

          NetDeviceContainer apDevice;
          apDevice = wifi.Install (phy, mac, wifiApNode);

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
          mobility.Install (wifiStaNode_DC);
          mobility.Install (wifiStaNode_DT);

          /* Internet stack*/
          InternetStackHelper stack;
          stack.Install (wifiApNode);
          stack.Install (wifiStaNode_DC);
          stack.Install (wifiStaNode_DT);

          Ipv4AddressHelper address;

          address.SetBase ("192.168.0.0", "255.255.0.0");
          Ipv4InterfaceContainer DCstaNodeInterface;
          Ipv4InterfaceContainer DTstaNodeInterface;
          Ipv4InterfaceContainer apNodeInterface;

          DCstaNodeInterface = address.Assign (staDevice_DC);
          DTstaNodeInterface = address.Assign (staDevice_DT);
          apNodeInterface = address.Assign (apDevice);




          DC_up = DCstaNodeInterface.GetAddress(0).m_address;
          DC_down = DCstaNodeInterface.GetAddress(0).m_address;
          DT_down = DTstaNodeInterface.GetAddress(0).m_address;
          DT_up = DTstaNodeInterface.GetAddress(0).m_address;

          /* Setting applications */
          ApplicationContainer serverApp, sinkApp;
          UdpServerHelper DCServer (9);
          serverApp.Add (DCServer.Install (wifiApNode.Get (0)));
          UdpServerHelper DTServer (10);
          serverApp.Add (DTServer.Install (wifiApNode.Get (0)));
          serverApp.Start (Seconds (0.0));
          serverApp.Stop (Seconds (simulationTime + 1));


          InetSocketAddress destDC (apNodeInterface.GetAddress (0), 9);
          destDC.SetTos (0xa0); //AC_BE
          UdpClientHelper DCClient (destDC);
          DCClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
          DCClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
          DCClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

          InetSocketAddress destDT (apNodeInterface.GetAddress (0), 10);
          destDT.SetTos (0x78); //AC_BE
          UdpClientHelper DTClient (destDT);
          DTClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
          DTClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
          DTClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

          ApplicationContainer clientApp;
          clientApp.Add (DCClient.Install (wifiStaNode_DC));
          clientApp.Add (DTClient.Install (wifiStaNode_DT));
          clientApp.Start (Seconds (1.0));
          clientApp.Stop (Seconds (simulationTime + 1));

          Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

          Ptr<UdpServer> server_DC = DynamicCast<UdpServer> (serverApp.Get (0));
          server_DC->TraceConnectWithoutContext ("Delay_DC", MakeBoundCallback (&DC_Delay, &DCR));
          Ptr<UdpServer> server_DT = DynamicCast<UdpServer> (serverApp.Get (1));
          server_DT->TraceConnectWithoutContext ("Delay_DT", MakeBoundCallback (&DT_Delay, &DTR));
          
          Simulator::Stop (Seconds (simulationTime + 1));
          Simulator::Run ();
          Simulator::Destroy ();

          double throughput_DC = 0;
          double throughput_DT = 0;
                
          uint32_t totalPacketsThrough_DC = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
          throughput_DC = totalPacketsThrough_DC * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s

          uint32_t totalPacketsThrough_DT = DynamicCast<UdpServer> (serverApp.Get (1))->GetReceived ();
          throughput_DT = totalPacketsThrough_DT * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
          std::ofstream outData;
          outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/wifi_edca.dat", std::ios::app);
          if (!outData)
          {
            std::cout <<"can not open the file"<< std::endl;
          }
         else
         {
            outData  << v << " " << throughput_DC << " " << throughput_DT
                     << " " << DCR.times << " " <<DTR.times << std::endl;
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

