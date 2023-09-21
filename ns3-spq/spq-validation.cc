// Network Topology
//
//        10.1.1.0  255.255.255.0
// n0 -------------- r1 -------------- n1
//    point-to-point    point-to-point
//

#include <cstdint>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "ns3/log.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/pcap-file-wrapper.h"
#include "ns3/pcap-file.h"
#include "ns3/vector.h"

#include "./model/strict-priority-queue.h"
#include "ns3/drop-tail-queue.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SPQSimulation");

Ptr<PointToPointNetDevice> NetDeviceDynamicCast (Ptr<NetDevice> const&p)
{
    return Ptr<PointToPointNetDevice> (dynamic_cast<PointToPointNetDevice *> (PeekPointer (p)));
}

void parse(std::vector<std::pair<uint32_t, uint16_t>>& pairs, const std::string& filename);

int main (int argc, char *argv[]) 
{   
    CommandLine cmd;  

    std::string filename;

    cmd.AddValue("filename", "Name of the config file", filename);
    cmd.Parse(argc, argv);

    std::vector<std::pair<uint32_t, uint16_t>> priority_port;
    parse(priority_port, filename);

    uint32_t queueNumber = 2;
    u_int32_t PriorityA = 0;
    u_int32_t PriorityB = 1;

    uint16_t node1PortA = 5001;     // high priority estination port
    uint16_t node1PortB = 5002;     // low priority Destination port

    uint32_t queueMaxPackets = 1500;   // max packet number allowed queueing in mid node


    double DEFAULT_START_TIME = 0.0;
    double DEFAULT_END_TIME = 40.0;

    double appAStartTime = DEFAULT_START_TIME + 12.0;      // start later than B A: 12-30, B: 0-40
    double appAEndTime = DEFAULT_END_TIME - 10.0;
    double appBStartTime = DEFAULT_START_TIME;
    double appBEndTime = DEFAULT_END_TIME ;

    // if user doesn't provide a config file
    if (priority_port.size() == 0) {
      priority_port.emplace_back(PriorityA, node1PortA);
      priority_port.emplace_back(PriorityB, node1PortB);
    }


    // Create vector of TrafficClass*
    std::vector<TrafficClass*> tc_vector;
    for (u_int32_t i = 0; i < queueNumber; i++) {
        DestPortNumberFilter* destPortFilter1 = new DestPortNumberFilter(priority_port[i].second);
        // Create Filter
        Filter* filter1 = new Filter();
        filter1->elements.push_back(destPortFilter1);  
        // Create TrafficClass
        TrafficClass* tc = new TrafficClass();
        tc->setPriorityLevel(priority_port[i].first);
        if (priority_port[i].first == 1) {
            tc->setDefault(true);   // set priority 1 queue as default
        }
        tc->setMaxPackets(queueMaxPackets);
        tc->filters.push_back(filter1);
        tc_vector.push_back(tc);
    }

    NodeContainer nodes;
    nodes.Create(3);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("4Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer devices1 = p2p.Install(nodes.Get(0), nodes.Get(1));

    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer devices2 = p2p.Install(nodes.Get(1), nodes.Get(2));

    // create spq
    ObjectFactory m_spqFactory;
    m_spqFactory.SetTypeId("StrictPriorityQueue");
    //m_spqFactory.Set("QueueNumber", UintegerValue(queueNumber));

    // Install SPQ on router1
    Ptr<StrictPriorityQueue> spq = m_spqFactory.Create<StrictPriorityQueue>();
    spq->setQ_Class(tc_vector);
    Ptr<PointToPointNetDevice> devR1 = NetDeviceDynamicCast(devices2.Get(0));
    devR1->SetQueue(spq);

    // Create the Internet stacks
    InternetStackHelper stack;
    stack.Install(nodes);

    // Create addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface1 = address.Assign(devices1);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interface2 = address.Assign(devices2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create applciation A on node 0
    UdpClientHelper clientA (interface2.GetAddress(1), priority_port[0].second);
    clientA.SetAttribute("Interval", TimeValue(Seconds(0.0003)));
    clientA.SetAttribute("PacketSize", UintegerValue(512));
    clientA.SetAttribute("MaxPackets", UintegerValue(0));
    ApplicationContainer appContA = clientA.Install(nodes.Get(0));
    appContA.Start(Seconds(appAStartTime));  
    appContA.Stop(Seconds(appAEndTime)); 


    // Create applciation B on node 0
    UdpClientHelper clientB (interface2.GetAddress(1), priority_port[1].second);
    clientB.SetAttribute("Interval", TimeValue(Seconds(0.0003)));
    clientB.SetAttribute("PacketSize", UintegerValue(512));
    clientB.SetAttribute("MaxPackets", UintegerValue(0));
    ApplicationContainer appContB = clientB.Install(nodes.Get(0));
    appContB.Start(Seconds(appBStartTime));  
    appContB.Stop(Seconds(appBEndTime));  

    // Create the first UdpServerHelper instance
    UdpServerHelper serverA (priority_port[0].second);
    ApplicationContainer serverAppsA = serverA.Install (nodes.Get(2));
    serverAppsA.Start (Seconds(DEFAULT_START_TIME));
    serverAppsA.Stop (Seconds(DEFAULT_END_TIME));

    // Create the second UdpServerHelper instance
    UdpServerHelper serverB (priority_port[1].second);
    ApplicationContainer serverAppsB = serverB.Install (nodes.Get(2));
    serverAppsB.Start (Seconds(DEFAULT_START_TIME));
    serverAppsB.Stop (Seconds(DEFAULT_END_TIME));


    p2p.EnablePcapAll("dvc", true);

    Simulator::Run();
    Simulator::Destroy();
    return 0;

}

void parse(std::vector<std::pair<uint32_t, uint16_t>>& pairs, const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "No config file or Error opening file " << filename << std::endl;
        return;
    }

    int n;
    file >> n;

    std::string line;
    getline(file, line); // read the newline after n
    for (int i = 0; i < n; i++) {
        getline(file, line);
        std::istringstream iss(line);
        uint32_t x;
        uint16_t y;
        char comma;
        iss >> x >> comma >> y;
        pairs.emplace_back(x, y);
    }

}

