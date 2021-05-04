/**
 * Server Atomic Model
 * written By AmirHoseein Ghorab
 * OCT 2020
*/

#ifndef _SERVER_HPP__
#define _SERVER_HPP__

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
struct Server_defs {
    struct outToSwitch : public out_port<Message_t> {
    };
    struct inFromSwitch : public in_port<Message_t> {
    };
};

template<typename TIME>
class Server {
public:
    // ports definition
    using input_ports = tuple<typename Server_defs::inFromSwitch>;
    using output_ports = tuple<typename Server_defs::outToSwitch>;
    // state definition
    struct state_type {
        bool transmitting;
        Message_t data;
        int index;
    };
    state_type state;

    // default constructor
    Server() {

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
        bag_port_in = get_messages<typename Server_defs::inFromSwitch>(mbs);
        if (bag_port_in.size() > 1) assert(false && "one message per time unit time");
        state.index++;
        state.data = bag_port_in[0];
        //Always transmit the data without dropping
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
        get_messages<typename Server_defs::outToSwitch>(bags) = bag_port_out;
        return bags;
    }

    // time_advance function
    TIME time_advance() const {

        TIME next_internal;
        if (state.transmitting) {

            // Delay for sending ACK
            next_internal = TIME("00:00:01:000");
        } else {
            next_internal = numeric_limits<TIME>::infinity();
        }
        return next_internal;
    }

    friend ostringstream &operator<<(ostringstream &os, const typename Server<TIME>::state_type &i) {
        /****** COMMENT these lines for Graphical representation output *******************/
        if(i.transmitting)
            os << ": Send back Ack for packet sequence #" << i.data.packetSec << " From Destination: " << i.data.dstIP;
        else
            os << ": Passive";
        /************/

        /****** UN-COMMENT this line for Graphical representation output *******************/
//        os << "<" << i.data.packetSec << ", " << i.data.dstIP << ">";
        return os;
    }
};

#endif // _SERVER_HPP__




















