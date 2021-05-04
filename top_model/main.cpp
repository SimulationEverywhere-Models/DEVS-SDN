/**
 * MAIN SDN
 * written By AmirHoseein Ghorab
 * OCT 2020
*/


//Cadmium Simulator headers
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>
//Json exporter header
//#include "../../../CadmiumModelJSONExporter/include/dynamic_json_exporter.hpp"

#include <NDTime.hpp>

#include "../data_structures/message.hpp"

#include <cadmium/basic_model/pdevs/iestream.hpp> //Atomic model for inputs
#include "../atomic/sdnController.hpp"
#include "../atomic/server.hpp"
#include "../atomic/switch.hpp"
#include "../atomic/client.hpp"


#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>


using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;


/***** Define ports for coupled model *****/
struct inputToSwitchFromClient : public in_port<Message_t>{};
struct outputFromSwitchToClient : public out_port<Message_t>{};

struct inputToSwitchFromServer : public in_port<Message_t>{};
struct outputFromSwitchToServer : public out_port<Message_t>{};

/***** Define ports for reader *****/
struct inputFromReaderToClient : public in_port<Message_t>{};


/****** Input Reader atomic model declaration *******************/
template<typename T>
class InputReader_Int : public iestream_input<Message_t,T> {
public:
    InputReader_Int() = default;
    InputReader_Int(const char* file_path) : iestream_input<Message_t,T>(file_path) {}
};

int main(int argc, char ** argv) {

    if (argc < 2) {
        cout << "Program used with wrong parameters. The program must be invoked as follow:";
        cout << argv[0] << " path to the input file " << endl;
        return 1; 
    }
    /****** Input Reader atomic model instantiation *******************/
    string input = argv[1];
    const char * i_input = input.c_str();
    shared_ptr<dynamic::modeling::model> input_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_Int, TIME, const char* >("input_reader" , move(i_input));

    /****** Client atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> client = dynamic::translate::make_dynamic_atomic_model<Client, TIME>("client");

    /****** Server atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> server = dynamic::translate::make_dynamic_atomic_model<Server, TIME>("server");

    /****** 2-layer controller (SDN Controller & Switch) atomic models instantiation *******************/
    shared_ptr<dynamic::modeling::model> sdnController = dynamic::translate::make_dynamic_atomic_model<SdnController, TIME>("sdnController");
    shared_ptr<dynamic::modeling::model> swth = dynamic::translate::make_dynamic_atomic_model<Switch, TIME>("switch");

    /******* CONTROLLER (SDN Controller & Switch) COUPLED MODEL********/
    dynamic::modeling::Ports iports_Controller = {typeid(inputToSwitchFromClient),typeid(inputToSwitchFromServer)};
    dynamic::modeling::Ports oports_Controller = {typeid(outputFromSwitchToClient),typeid(outputFromSwitchToServer)};
    dynamic::modeling::Models submodels_Controller = {sdnController, swth};
    dynamic::modeling::EICs eics_Controller = {
        dynamic::translate::make_EIC<inputToSwitchFromClient, Switch_defs::inFromClient>("switch"),
         dynamic::translate::make_EIC<inputToSwitchFromServer, Switch_defs::inFromServer>("switch")
    };
    dynamic::modeling::EOCs eocs_Controller = {
        dynamic::translate::make_EOC<Switch_defs::outToClient,outputFromSwitchToClient>("switch"),
        dynamic::translate::make_EOC<Switch_defs::outToServer,outputFromSwitchToServer>("switch")
    };
    dynamic::modeling::ICs ics_Controller = {
            dynamic::translate::make_IC<Switch_defs::outToController, SdnController_defs::inFromSwitch>("switch","sdnController"),
            dynamic::translate::make_IC<SdnController_defs::outToSwitch, Switch_defs::inFromController>("sdnController","switch"),
    };

    shared_ptr<dynamic::modeling::coupled<TIME>> CONTROLLER;
    CONTROLLER = make_shared<dynamic::modeling::coupled<TIME>>(
        "Controller", submodels_Controller, iports_Controller, oports_Controller, eics_Controller, eocs_Controller, ics_Controller
    );

    /******* SDN SIMULATOR COUPLED MODEL********/
    dynamic::modeling::Ports iports_SDN = {typeid(inputFromReaderToClient)};
    dynamic::modeling::Ports oports_SDN = {};
    dynamic::modeling::Models submodels_SDN = {client, server, CONTROLLER};
    dynamic::modeling::EICs eics_SDN = {
        cadmium::dynamic::translate::make_EIC<inputFromReaderToClient, Client_defs::inFromReader>("client")
    };
    dynamic::modeling::EOCs eocs_SDN = {};

    dynamic::modeling::ICs ics_SDN = {
        dynamic::translate::make_IC<Client_defs::outToSwitch, inputToSwitchFromClient>("client","Controller"),
        dynamic::translate::make_IC<Server_defs::outToSwitch, inputToSwitchFromServer>("server","Controller"),

        dynamic::translate::make_IC<outputFromSwitchToClient, Client_defs::inFromSwitch>("Controller","client"),
        dynamic::translate::make_IC<outputFromSwitchToServer, Server_defs::inFromSwitch>("Controller","server")
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> SDN;
    SDN = make_shared<dynamic::modeling::coupled<TIME>>(
        "SDN", submodels_SDN, iports_SDN, oports_SDN, eics_SDN, eocs_SDN, ics_SDN
    );


    /*******TOP COUPLED MODEL********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {};
    dynamic::modeling::Models submodels_TOP = {input_reader, SDN};
    dynamic::modeling::EICs eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP = {};
    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<Message_t>::out, inputFromReaderToClient>("input_reader","SDN")
    };
    shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP 
    );

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/SDN_output_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){          
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/SDN_output_state.txt");
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
    r.run_until_passivate();

    /************* JSON Exporter call ************************/
    /****** UN-COMMENT these lines for Graphical representation output ********/
//    static ofstream out_JSON("SDN.json");
//    dynamic_export_model_to_json(out_JSON, TOP);
    /**********/

    return 0;
}