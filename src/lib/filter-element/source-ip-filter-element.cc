#include "filter-element.h"
#include "ns3/ipv4-header.h"

using namespace ns3;

class SourceIPAddressFilter : public FilterElement {
    private:
        Ipv4Address sourceAddr;

    public:
        SourceIPAddressFilter(Ipv4Address sourceIP) {
            this->sourceAddr = sourceIP;
        }

        bool match(Ptr<Packet> myPacket) override {
            Ipv4Header header;
            myPacket->PeekHeader(header);
            
            return header.GetSource() == this->sourceAddr;
        }
};
