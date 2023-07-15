# Create this file as such "INC = /path/to/vivado/Vitis_HLS/2023.1/include"
include ./VitisPath

test: testbench.cpp conv.cpp
	g++ $^ -o $@ -I${INC} -g -O3

board: board.cpp
	g++ $^ -o $@ -g -O3 -lpynq -lcma -lpthread

boardnet: boardnet.cpp conv.cpp inf_utils.cpp
	g++ $^ -o $@ -g -O3 -lpthread -I ../../pynq_api/src/

