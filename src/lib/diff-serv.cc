#include <vector>
#include <cstdint>
#include "traffic-class.h"
#include "filter.h"
#include "ns3/queue.h"
#include "./filter-element/dest-port-filter-element.cc"

using namespace ns3;

class DiffServ : public Queue<Packet> {
    private:
        std::vector<TrafficClass*> q_class;

        bool DoEnqueue(Ptr<Packet> p) {
            //printf("[DiffServ] DoEnqueue queue len %ld\n", q_class.size());
            uint32_t index = Classify(p);
            //printf("[DiffServ] DoEnqueue:\tclass index %d\n", index);    
            return q_class[index]->Enqueue(p);
        }

        Ptr<Packet> DoDequeue() {
            Ptr<Packet> p = nullptr;
            uint32_t count = 0;
            while (p == nullptr && count < q_class.size()) {
                p = Schedule();
                count++;
            }
            return p;
        }

        Ptr<Packet> DoRemove() {
            return Schedule();
        }

        Ptr<const Packet> DoPeek() const{
            return 0;
        }

    protected:

        std::vector<TrafficClass*> getQClass() {
            return this->q_class;
        }

        void setQClass(std::vector<TrafficClass*> q_class) {
            this->q_class = q_class;
        }


    public:
        virtual Ptr<Packet> Schedule() = 0;

        virtual uint32_t Classify(Ptr<Packet> p) = 0;

        bool Enqueue(Ptr<Packet> p) override {
            //printf("[DiffServ] Enqueue\n");
            return DoEnqueue(p);
        }

        Ptr<Packet> Dequeue() override {
            return DoDequeue();
        }

        Ptr<const Packet> Peek() const{
            return DoPeek();
        }
        
        Ptr<Packet> Remove() override {
            return DoRemove();
        }

        



};  