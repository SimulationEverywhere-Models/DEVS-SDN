//Cadmium Simulator headers
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>

//Time class header
#include <NDTime.hpp>

//Messages structures
#include "../data_structures/message.hpp"

//Atomic model headers
#include <cadmium/basic_model/pdevs/iestream.hpp> //Atomic model for inputs
#include "../atomic/switch.hpp"

//C++ libraries
#include <iostream>
#include <string>

using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

/***** Define input port for coupled models *****/

/***** Define output ports for coupled model *****/
struct top_out_to_server: public out_port<Message_t>{};
struct top_out_to_controller: public out_port<Message_t>{};
struct top_out_to_client: public out_port<Message_t>{};

/****** Input Reader atomic model declaration *******************/
template<typename T>
class InputReader_Message_t : public iestream_input<Message_t,T> {
public:
    InputReader_Message_t () = default;
    InputReader_Message_t (const char* file_path) : iestream_input<Message_t,T>(file_path) {}
};

int main(int argc, char ** argv) {

    if (argc < 2) {
        cout << "Program used with wrong parameters. The program must be invoked as follow:";
        cout << argv[0] << " path to the input file " << endl;
        return 1;
    }

    /****** Input Reader atomic model instantiation *******************/
    string input = argv[1];
    const char * i_input_data = input.c_str();
    shared_ptr<dynamic::modeling::model> input_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_Message_t, TIME, const char*>("input_reader", move(i_input_data));

    /****** Switch atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> swth = dynamic::translate::make_dynamic_atomic_model<Switch, TIME>("switch");

    /*******TOP MODEL********/
    dynamic::modeling::Ports iports_TOP;
    iports_TOP = {};
    dynamic::modeling::Ports oports_TOP;
    oports_TOP = {typeid(top_out_to_server), typeid(top_out_to_controller), typeid(top_out_to_client)};
    dynamic::modeling::Models submodels_TOP;
    submodels_TOP = {input_reader, swth};
    dynamic::modeling::EICs eics_TOP;
    eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP;
    eocs_TOP = {
            dynamic::translate::make_EOC<Switch_defs::outToServer,top_out_to_server>("switch"),
            dynamic::translate::make_EOC<Switch_defs::outToController,top_out_to_controller>("switch"),
            dynamic::translate::make_EOC<Switch_defs::outToClient,top_out_to_client>("switch"),
    };
    dynamic::modeling::ICs ics_TOP;
    ics_TOP = {
            dynamic::translate::make_IC<iestream_input_defs<Message_t>::out,Switch_defs::inFromClient>("input_reader","switch")
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
            "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP
    );

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/switch_client_input_output_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/switch_client_input_output_state.txt");
    struct oss_sink_state{
        static ostream& sink(){
            return out_state;
        }
    };

    using state=logger::logger<logger::logger_state, dynamic::logger::formatter<TIME>, oss_sink_state>;
    using log_messages=logger::logger<logger::logger_messages, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_mes=logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_sta=logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_state>;

    using logger_top=logger::multilogger<state, log_messages, global_time_mes, global_time_sta>;

    /************** Runner call ************************/
    dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    r.run_until(NDTime("04:00:00:000"));
    return 0;
}
