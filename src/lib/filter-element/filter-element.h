#ifndef FILTER_ELEMENT_H
#define FILTER_ELEMENT_H

#include "ns3/packet.h"
#include "ns3/ptr.h"

using namespace ns3;

    const uint8_t TCP_Protocol_Number = 6;
    const uint8_t UDP_Protocol_Number = 17;

class FilterElement {
    public: 
        virtual bool match(Ptr<ns3::Packet> myPackets) = 0;
};



#endif
