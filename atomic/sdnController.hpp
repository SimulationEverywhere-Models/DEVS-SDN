/**
 * SDN Controller Atomic Model
 * written By AmirHoseein Ghorab
 * OCT 2020
*/

#ifndef _SDNCONTROLLER_HPP__
#define _SDNCONTROLLER_HPP__

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
struct SdnController_defs {
    struct outToSwitch : public out_port<Message_t> {
    };
    struct inFromSwitch : public in_port<Message_t> {
    };
};

template<typename TIME>
class SdnController {
public:
    // ports definition
    using input_ports = tuple<typename SdnController_defs::inFromSwitch>;
    using output_ports = tuple<typename SdnController_defs::outToSwitch>;
    // state definition
    struct state_type {
        bool transmitting;
        Message_t data;
        int index;
    };
    state_type state;

    // default constructor
    SdnController() {
        state.transmitting = false;
        state.index = 0;
    }

    // internal transition
    void internal_transition() {
        state.transmitting = false;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        vector <Message_t> bag_port_in;
        bag_port_in = get_messages<typename SdnController_defs::inFromSwitch>(mbs);
        if (bag_port_in.size() > 1) assert(false && "Should be one message at a time");
        state.index++;
        state.data = bag_port_in[0];
        //Always transmit the data
        state.transmitting = true;
    }

    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        vector <Message_t> bag_port_out;
        bag_port_out.push_back(state.data);
        get_messages<typename SdnController_defs::outToSwitch>(bags) = bag_port_out;
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        TIME next_internal;
        if (state.transmitting) {
            // Delay to indicate that the controller is processing the shortest path
            next_internal = TIME("00:00:05:000");
        } else {
            next_internal = numeric_limits<TIME>::infinity();
        }
        return next_internal;
    }

    friend ostringstream &operator<<(ostringstream &os, const typename SdnController<TIME>::state_type &i) {
        /****** COMMENT these lines for Graphical representation output *******************/
        if (i.transmitting) {
            os << ": Adding flow rule to Switch's flow table for packet flow to destination: " << i.data.dstIP;
        } else {
            os << ": Passive";
        };
        /************/

        /****** UN-COMMENT this line for Graphical representation output *******************/
//        os << "<" << i.data.dstIP << ">";

        return os;
    }
};

#endif // _SDNCONTROLLER_HPP__




















