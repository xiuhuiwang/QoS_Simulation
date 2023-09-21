#include "filter-element.h"
#include "ns3/ipv4-header.h"

using namespace ns3;

class DestIPAddressFilter : public FilterElement {
    private:
        Ipv4Address destAddr;

    public:
        DestIPAddressFilter(Ipv4Address destIP) {
            this->destAddr = destIP;
        }

        bool match(Ptr<Packet> myPacket) override {
            Ipv4Header header;
            myPacket->PeekHeader(header);
            
            return header.GetDestination() == this->destAddr;
        }
};
