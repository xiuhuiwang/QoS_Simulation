#include "filter-element.h"
#include "ns3/ipv4-header.h"

using namespace ns3;

class ProtocolNumberFilter : public FilterElement {
    private:
        uint32_t protocolNumber;

    public:
        ProtocolNumberFilter(uint32_t pn) {
            this->protocolNumber = pn;
        }

        bool match(Ptr<Packet> myPacket) override {
            Ipv4Header header;
            myPacket->PeekHeader(header);
            
            return header.GetProtocol() == this->protocolNumber;
        }
};
