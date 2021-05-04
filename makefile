CC=g++
CFLAGS=-std=c++17

INCLUDECADMIUM=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../DESTimes/include

#CREATE BIN AND BUILD FOLDERS TO SAVE THE COMPILED FILES DURING RUNTIME
bin_folder := $(shell mkdir -p bin)
build_folder := $(shell mkdir -p build)
results_folder := $(shell mkdir -p simulation_results)

#TARGET TO COMPILE ALL THE TESTS TOGETHER (NOT SIMULATOR)
message.o: data_structures/message.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) data_structures/message.cpp -o build/message.o
	
main_switch_client_input_test.o: test/main_switch_client_input_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_switch_client_input_test.cpp -o build/main_switch_client_input_test.o

main_sdnController_test.o: test/main_sdnController_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_sdnController_test.cpp -o build/main_sdnController_test.o

main_server_test.o: test/main_server_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_server_test.cpp -o build/main_server_test.o

main_client_test.o: test/main_client_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_client_test.cpp -o build/main_client_test.o

main_top.o: top_model/main.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) top_model/main.cpp -o build/main_top.o

tests: main_switch_client_input_test.o message.o main_sdnController_test.o main_server_test.o main_client_test.o
		$(CC) -g -o bin/SWITCH_CLIENT_INPUT_TEST build/main_switch_client_input_test.o build/message.o
		$(CC) -g -o bin/SDN_CONTROLLER_INPUT_TEST build/main_sdnController_test.o build/message.o
		$(CC) -g -o bin/SERVER_INPUT_TEST build/main_server_test.o build/message.o
		$(CC) -g -o bin/CLIENT_INPUT_TEST build/main_client_test.o build/message.o


#TARGET TO COMPILE ONLY ABP SIMULATOR
simulator: main_top.o message.o
	$(CC) -g -o bin/SDN build/main_top.o build/message.o

	
#TARGET TO COMPILE EVERYTHING (ABP SIMULATOR + TESTS TOGETHER)
all: simulator tests

#CLEAN COMMANDS
clean: 
	rm -f bin/* build/*