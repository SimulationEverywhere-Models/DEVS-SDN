# Modeling Software Defined Network Using Cadmium
A simplified SDN network environment will be modeled and simulated in this project. The system
contains four different components.
- SDN controller
- SDN enabled witch
- Webserver
- Client (User)

The client asks for a service (e.g., Http service) from the web server by sending a request. This request
is received by the switch in the middle of the way. Unlike the traditional switch that only forwards the
packets based on its rule sets in the switch table, the SDN enabled switch buffers the packet and
compares its destination IP with the switch flow table entry. If the destination is found in the flow table,
the packet will be sent to the destined server. Otherwise, the packet will be sent to the controller over a
secure connection asking for the action that needs to be done on the packet. The SDN controller analyzes
the packet and sends the packet back to the switch setting the packet's destination into the switch's flow
table. Then the switch forwards the packet to the web server. The webserver receives the packet and
sends an ACK packet to the client through the switch. This procedure will be repeated for each new
type of flow with a new destination.

## Requirements
- Cadmium 
http://www.sce.carleton.ca/courses/sysc-5104/lib/exe/fetch.php?media=cadmium.pdf

## To *Run the Project* : 
- make clean
- make all

## To run the tests:
```
~# cd Scripts
~# bash client_test.sh
~# bash SDN.sh
~# bash sdnController_test.sh
~# bash server_input_test.sh
~# bash switch_client_test.sh
```


## To visualize the result on ARSLabDEVS Web Viewer
1. Load “Diagram.svg”, “Messages.log”, “Options.json”, “Structure.json” from **SDN_WebViewer** folder to the web
viewer
2. Click on the “Load simulation” on web viewer page