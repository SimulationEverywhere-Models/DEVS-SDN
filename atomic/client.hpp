
#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>
#include <random>

#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

//Port definition
struct Client_defs {
    struct outToSwitch : public out_port<Message_t> {
    };
    struct inFromSwitch : public in_port<Message_t> {
    };
    struct inFromReader : public in_port<Message_t> {
    };
};

template<typename TIME>
class Client {
public:

    TIME preparationTime;
    TIME timeout;

    // default constructor
    Client() noexcept {
        preparationTime = TIME("00:00:10");
        timeout = TIME("00:00:20");
        state.next_internal = std::numeric_limits<TIME>::infinity();
        state.model_active = false;
    }

    // state definition
    struct state_type {
        bool ack;
        int packetNum;
        int totalPacketNum;
        string dstIP;
        bool sending;
        bool model_active;
        TIME next_internal;
    };
    state_type state;
    // ports definition
    using input_ports = std::tuple<typename Client_defs::inFromSwitch, typename Client_defs::inFromReader>;
    using output_ports = std::tuple<typename Client_defs::outToSwitch>;

    // internal transition
    void internal_transition() {
        if (state.ack) {
            if (state.packetNum < state.totalPacketNum) {
                state.packetNum++;
                state.ack = false;
                state.sending = true;
                state.model_active = true;
                state.next_internal = preparationTime;
            } else {
                state.model_active = false;
                state.next_internal = std::numeric_limits<TIME>::infinity();
            }
        } else {
            if (state.sending) {
                state.sending = false;
                state.model_active = true;
                state.next_internal = timeout;
            } else {
                state.sending = true;
                state.model_active = true;
                state.next_internal = preparationTime;
            }
        }
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        if ((get_messages<typename Client_defs::inFromSwitch>(mbs).size() +
             get_messages<typename Client_defs::inFromReader>(mbs).size()) > 1)
            assert(false && "one message per time frame");
        for (const auto &x : get_messages<typename Client_defs::inFromReader>(mbs)) {
            if (state.model_active == false) {
                state.totalPacketNum = x.packetSec;
                state.dstIP = x.dstIP;
                if (state.totalPacketNum > 0) {
                    state.packetNum = 1;
                    state.ack = false;
                    state.sending = true;
                    state.model_active = true;
                    state.next_internal = preparationTime;
                } else {
                    if (state.next_internal != std::numeric_limits<TIME>::infinity()) {
                        state.next_internal = state.next_internal - e;
                    }
                }
            }
        }
        for (const auto &x : get_messages<typename Client_defs::inFromSwitch>(mbs)) {
            if (state.model_active == true) {
                state.dstIP = x.dstIP;
                state.ack = true;
                state.sending = false;
                state.next_internal = TIME("00:00:00");
            }
        }
    }

    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        Message_t out;
        if (state.sending) {
            out.packetSec = state.packetNum;
            out.dstIP = state.dstIP;
            get_messages<typename Client_defs::outToSwitch>(bags).push_back(out);
        }
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        return state.next_internal;
    }

    friend std::ostringstream &operator<<(std::ostringstream &os, const typename Client<TIME>::state_type &i) {

        /****** COMMENT these lines for Graphical representation output *******************/
        if(i.sending){
            os << ": Sending Packet sequence #" << i.packetNum << " to destination " << i.dstIP << " | total packet number: " << i.totalPacketNum;
        }
        if(i.ack) {
            os << ": Receiving ACK for Packet sequence #" << i.packetNum << " from destination " << i.dstIP;
        }

        if((!i.sending && !i.ack) || !i.model_active){
            os << ": Passive";
        }
        /************/

        /****** UN-COMMENT this line for Graphical representation output *******************/
//        os << "<" << i.packetNum << ", " << i.dstIP << ">";
        return os;
    }
};

#endif // __CLIENT_HPP__