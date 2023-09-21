#include "filter-element.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/ppp-header.h"

using namespace ns3;

// only filters port number in TCP and UDP
class DestPortNumberFilter : public FilterElement {
    private:
        uint32_t destPortNumber;

    public:
        DestPortNumberFilter(uint32_t portNumber) {
            this->destPortNumber = portNumber;
        }

        bool match(Ptr<Packet> myPacket) override {
            Ptr<Packet> packetReplica = myPacket->Copy();
            
            // remove ppp header
            PppHeader ppp;
            packetReplica->RemoveHeader(ppp);

            // remove ipv4 header
            Ipv4Header ipHeader;
            packetReplica->RemoveHeader(ipHeader);

            uint16_t packetDestPort = 0;

            if (ipHeader.GetProtocol() == TCP_Protocol_Number) {
                TcpHeader tcpHeader;
                packetReplica->RemoveHeader(tcpHeader);
                packetDestPort = tcpHeader.GetDestinationPort();
            } else if (ipHeader.GetProtocol() == UDP_Protocol_Number) {
                UdpHeader udpHeader;
                packetReplica->RemoveHeader(udpHeader);
                packetDestPort = udpHeader.GetDestinationPort();
            }

            return packetDestPort == this->destPortNumber;
        }

        u_int32_t getDestPort() {
            return this->destPortNumber;
        }
};
