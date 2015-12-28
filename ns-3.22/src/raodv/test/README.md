HOW-TO-RUN

1. Enter the < .../ns-allinone-3.22/ns-3.22/examples/routing > directory. 
2. Edit the wscript file [LINE 37] to add ['raodv'] to the list of modules to build.
*note: the [manet-routing-compare.cc] file is copied automatically via the x_module_build script

Run the manet-routing-compare file via the waf command:
```
./waf --run "manet-routing-compare --protocol=2 --totalNodes=25"
```
Where you can add/change parameters as you like. 

The parameters are:

| Parameter     | Description 																		| Default Values |
|:-------------:|-------------------------------------------------|:--------------:|
| --protocol		|	Set to 2 to use AODV. Set to 5 to use RAODV 		|5							 |
| --totalNodes	|	Number of total Nodes in the simulation					|10							 |
| --sinks				|	Number of sinks(destinations) to simulate		 		|5							 |
| --malicious		|	Number of malicious nodes (randomly assigned) 	|0							 |
| --mvtSpeed		|	Movement Speed factor (in m/s) of every node		|20.0						 |
| --sizeX				|	Width of Simulation Environment									|700.0					 |
| --sizeY				|	Height of Simulation Environment								|500.0					 |
| --totalTime		|	Total Number of Seconds to run the Simulation		|40.0						 |
| --power				|	Transmission Power Factor (in dBM) of each node |7.5						 |
| --startupMin	|	Startup Time Range minimum (seconds)						|5.0						 |
| --startupMax	|	Startup Time Range maximum (seconds)						|11.0						 |