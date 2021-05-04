/**
 * SDN Switch Atomic Model
 * written By AmirHoseein Ghorab
 * OCT 2020
*/

#ifndef _SWITCH_HPP__
#define _SWITCH_HPP__

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
struct Switch_defs {
    //Connection to Controller
    struct outToController : public out_port<Message_t> {};
    struct inFromController : public in_port<Message_t> {};
    //Connection to Servers
    struct outToServer : public out_port<Message_t> {};
    struct inFromServer : public in_port<Message_t> {};
    //Connection to Clients
    struct outToClient : public out_port<Message_t> {};
    struct inFromClient : public in_port<Message_t> {};
};

enum ModelMood {
    Passive = 0,
    TransmittingToDestination ,
    TransmittingToController,
    TransmittingToClient
};
const char* enum2str[] = {
        "Passive",
        "Transmitting To Destination",
        "Transmitting To Controller",
        "Transmitting To Client",
};


template<typename TIME>
class Switch {
public:

    // ports definition
    using input_ports=std::tuple<typename Switch_defs::inFromClient, typename Switch_defs::inFromServer, typename Switch_defs::inFromController>;
    using output_ports=std::tuple<typename Switch_defs::outToClient, typename Switch_defs::outToServer, typename Switch_defs::outToController>;


    // state definition
    struct state_type {
        ModelMood mood;
        Message_t data;
        vector <string> flowTable;
        int index;
    };
    state_type state;

    // default constructor
    Switch() {
        state.mood = ModelMood::Passive;
        state.index = 0;
    }

    // internal transition
    void internal_transition() {
        state.mood = ModelMood::Passive;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

        if((get_messages<typename Switch_defs::inFromClient>(mbs).size()+get_messages<typename Switch_defs::inFromServer>(mbs).size()+
                get_messages<typename Switch_defs::inFromController>(mbs).size())>1)
            assert(false && "one message per time");
        for(const auto &x : get_messages<typename Switch_defs::inFromClient>(mbs))
        {
            state.index++;
            state.data = x;
            bool isDstAvailableInFlowTable = false;
            for (auto it = begin (state.flowTable); it != end (state.flowTable); ++it) {

                if((*it) == state.data.dstIP){

                    state.mood = ModelMood::TransmittingToDestination;
                    isDstAvailableInFlowTable = true;

                    break;
                }
            }
            if (!isDstAvailableInFlowTable){

                state.mood = ModelMood::TransmittingToController;

            }
        }

        for(const auto &x : get_messages<typename Switch_defs::inFromServer>(mbs)){
            state.index++;
            state.data = x;
            state.mood = ModelMood::TransmittingToClient;
        }

        for(const auto &x : get_messages<typename Switch_defs::inFromController>(mbs)){
            state.index++;
            state.data = x;

            state.flowTable.push_back(state.data.dstIP);
            state.mood = ModelMood::TransmittingToDestination;

        }
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

        if (state.mood == ModelMood::TransmittingToDestination){
            get_messages<typename Switch_defs::outToServer>(bags) = bag_port_out;
            return bags;
        }
        else if(state.mood == ModelMood::TransmittingToController){
            get_messages<typename Switch_defs::outToController>(bags) = bag_port_out;
            return bags;
        }
        else if(state.mood == ModelMood::TransmittingToClient){
            get_messages<typename Switch_defs::outToClient>(bags) = bag_port_out;
            return bags;
        }
        else {
            assert("No valid mood for the switch");
        }

    }

    // time_advance function
    TIME time_advance() const {
        TIME next_internal;
        if (state.mood == ModelMood::TransmittingToDestination) {
            next_internal = TIME("00:00:02:000");
        }
        else if(state.mood == ModelMood::TransmittingToController){
            next_internal = TIME("00:00:05:000");
        }
        else if(state.mood == ModelMood::TransmittingToClient){
            next_internal = TIME("00:00:02:000");
        }
         else {
            next_internal = numeric_limits<TIME>::infinity();
        }
        return next_internal;
    }

    friend ostringstream &operator<<(ostringstream &os, const typename Switch<TIME>::state_type &i) {
        /****** COMMENT these lines for Graphical representation output *******************/
        if(i.mood == 0){
            os << ": Passive";
        }
        else
            os << ": " << enum2str[i.mood] << " --> Packet sequence number #" << i.data.packetSec << " destination IP " << i.data.dstIP ;
        /************/

        /****** UN-COMMENT this line for Graphical representation output *******************/
//        os << "<" << enum2str[i.mood] << ", " << i.data.packetSec << ", " << i.data.dstIP << ">";

        return os;
    }
};

#endif // _SWITCH_HPP__




















