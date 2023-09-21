#ifndef DEFICIT_ROUND_ROBIN_H
#define DEFICIT_ROUND_ROBIN_H

#include <cstdint>
#include <iostream>
#include <vector>
#include "ns3/type-id.h"
#include "../../src/lib/filter.h"
#include "../../src/lib/filter-element/filter-element.h"
#include "../../src/lib/traffic-class.h"

#include "../../src/lib/diff-serv.cc"

using namespace ns3;

class DeficitRoundRobin : public DiffServ {
    public:
        static TypeId GetTypeId(void) {
            static TypeId tid = 
                TypeId("DeficitRoundRobin")
                .SetParent<DiffServ>()
                .AddConstructor<DeficitRoundRobin>()

                .AddAttribute("QueueNumber",
                        "Queue number.",
                        UintegerValue(3),
                        MakeUintegerAccessor(&DeficitRoundRobin::m_queueNumber),
                        MakeUintegerChecker<uint32_t>());

            return tid;
        }

        DeficitRoundRobin() {
            
        }

        uint32_t Classify(Ptr<Packet> p) override {
            uint32_t classIndex = 1;
            std::vector<TrafficClass*> q_class = DiffServ::getQClass();
            for (uint32_t i = 0; i < q_class.size(); i++) {
                if (q_class[i]->match(p)) {
                    classIndex = i;
                    break;
                }
            }
            ++m_numPackets;

            return classIndex;
        }

        Ptr<Packet> Schedule() override {
            Ptr<Packet> p = nullptr;
            if (m_numPackets == 0) {
                return nullptr;
            }

            if (refresh) {
                RefreshDeficit();
                
                refresh = false;
            }

            std::vector<TrafficClass*> q_class = DiffServ::getQClass();
            
            //Serve current queue
            if (!q_class[m_currentTCIndex]->isEmpty() && 
                    q_class[m_currentTCIndex]->DoPeek()->GetSize() <= m_curDeficit[m_currentTCIndex]) {
                m_curDeficit[m_currentTCIndex] -= q_class[m_currentTCIndex]->DoPeek()->GetSize();
                p = q_class[m_currentTCIndex]->Dequeue();
                --m_numPackets;
            }

            if (q_class[m_currentTCIndex]->isEmpty()) {
                m_curDeficit[m_currentTCIndex] = 0;
            }
            
            //Current queue can no longer be served
            if (q_class[m_currentTCIndex]->isEmpty() ||
                    q_class[m_currentTCIndex]->DoPeek()->GetSize() > m_curDeficit[m_currentTCIndex]) {
                m_currentTCIndex = (m_currentTCIndex + 1) % m_queueNumber;
                if (m_currentTCIndex == 0) {
                    refresh = true;
                }
            }

           
            return p;
        }

        void setQ_Class(std::vector<TrafficClass *> q_class) {
            DiffServ::setQClass(q_class);
        }

    private:
        uint32_t m_currentTCIndex = 0;
        uint32_t m_numPackets = 0;

        uint32_t m_queueNumber;
        std::vector<double_t> m_curDeficit = {0, 0, 0};
        uint16_t m_firstPort;
        uint16_t m_secondPort;
        uint16_t m_thirdPort;

        bool m_isInitialized = false;
        bool refresh = true;


        Ptr<Packet> DoPeek() {  
            if (m_numPackets == 0) {
                return nullptr;
            }
            std::vector<TrafficClass*> q_class = DiffServ::getQClass();
            if (!q_class[m_currentTCIndex]->isEmpty()) {
                return q_class[m_currentTCIndex]->DoPeek();
            }

        }

        void RefreshDeficit() {
            std::vector<TrafficClass*> q_class = DiffServ::getQClass();
            for (size_t i = 0; i < m_curDeficit.size(); ++i) {
                m_curDeficit[i] += q_class[i]->getWeight();
            }
        }

};

 NS_OBJECT_ENSURE_REGISTERED(DeficitRoundRobin);

#endif
