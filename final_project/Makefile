# Create this file as such "INC = /path/to/vivado/Vitis_HLS/2023.1/include"
include ./VitisPath

test: testbench.cpp conv.cpp
	g++ $^ -o $@ -I${INC} -g -O3

board: board.cpp
	g++ $^ -o $@ -g -O3 -lpynq -lcma -lpthread

