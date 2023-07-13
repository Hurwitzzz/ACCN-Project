# Create this file as such "INC = /path/to/vivado/Vitis_HLS/2023.1/include"
include ./VitisPath

test: testbench.cpp conv.cpp
	g++ $^ -o $@ -I${INC} -g -O3

board: board.cpp
	g++ $^ -o $@ -g -O3 -lpynq -lcma -lpthread

OBJDIR=build
SRCDIR=src
UTILDIR=utils

USRCS=$(wildcard $(UTILDIR)/*.cpp)
OBJS=$(patsubst $(UTILDIR)/%.cpp,$(OBJDIR)/%.o,$(USRCS))

SRCS=$(wildcard $(SRCDIR)/*.cpp)
OBJS += $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))




FLAGS += -I$(UTILDIR) -I$(SRCDIR)  -g -O3


class: $(OBJS)
	$(CXX) $(FLAGS)  $^ -o $@ 


$(OBJDIR)/%.o : $(SRCDIR)/%.cpp 
	$(CXX) $(FLAGS) -c $< -o $@ 



$(OBJDIR)/%.o : $(UTILDIR)/%.cpp 
	$(CXX) $(FLAGS) -c $< -o $@ 


clean:
	$(RM) build/*
	$(RM) -r utils/__pycache__
	$(RM) class
	$(RM) -r data
	$(RM) image.tensor

