OBJDIR=build
SRCDIR=src
UTILDIR=utils

USRCS=$(wildcard $(UTILDIR)/*.cpp)
OBJS=$(patsubst $(UTILDIR)/%.cpp,$(OBJDIR)/%.o,$(USRCS))

SRCS=$(wildcard $(SRCDIR)/*.cpp)
OBJS += $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))



# Compiler settings #
CC = g++
CXXFLAGS = -std=c++11 -Wall -g
ldflags = 

# LDFLAGS = -lm -lprofiler

FLAGS += -I$(UTILDIR) -I$(SRCDIR) -g -O3
TRG=lab3.bin

$(TRG): 

$(TRG): $(OBJS)
	$(CXX) $(FLAGS) $^ -o $@ $(LDFLAGS)


$(OBJDIR)/%.o : $(SRCDIR)/%.cpp 
	$(CXX) $(FLAGS) -c $< -o $@ 



$(OBJDIR)/%.o : $(UTILDIR)/%.cpp
	$(CXX) $(FLAGS) -c $< -o $@ -I$(UTILDIR)





clean:
	$(RM) build/*
	$(RM) -r utils/__pycache__
	$(RM) $(TRG)
	$(RM) -r data





# OBJDIR=build: Sets the directory for object files to be stored.
# SRCDIR=src: Sets the directory for source files to be stored.
# UTILDIR=utils: Sets the directory for utility files to be stored.
# USRCS=$(wildcard $(UTILDIR)/*.cpp): Finds all the .cpp files in the UTILDIR directory and stores them in the USRCS variable.
# OBJS=$(patsubst $(UTILDIR)/%.cpp,$(OBJDIR)/%.o,$(USRCS)): Uses the patsubst function to replace the UTILDIR directory with the OBJDIR directory in the USRCS variable and stores the result in the OBJS variable.
# SRCS=$(wildcard $(SRCDIR)/*.cpp): Finds all the .cpp files in the SRCDIR directory and stores them in the SRCS variable.
# OBJS += $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS)): Uses the patsubst function to replace the SRCDIR directory with the OBJDIR directory in the SRCS variable and appends the result to the OBJS variable.
# CC = g++: Sets the compiler to g++.
# CXXFLAGS = -std=c++11 -Wall -g: Sets the compiler flags to use C++11, enable all warnings, and include debugging symbols.
# ldflags = : Sets the linker flags to nothing.
# FLAGS += -I$(UTILDIR) -I$(SRCDIR) -g -O3: Appends the compiler flags to include the UTILDIR and SRCDIR directories, enable debugging symbols, and optimize the code.
# TRG=lab3.bin: Sets the target executable name to lab3.bin.
# $(TRG): : Specifies the target executable.
# $(TRG): $(OBJS): Specifies that the target executable depends on all the object files.
# $(CXX) $(FLAGS) $^ -o $@ $(LDFLAGS): Compiles the object files and links them to create the target executable.
# $(OBJDIR)/%.o : $(SRCDIR)/%.cpp: Specifies a rule to compile a .cpp file in the SRCDIR directory to an object file in the OBJDIR directory.
# $(CXX) $(FLAGS) -c $< -o $@ : Compiles the .cpp file to an object file.
# $(OBJDIR)/%.o : $(UTILDIR)/%.cpp: Specifies a rule to compile a .cpp file in the UTILDIR directory to an object file in the OBJDIR directory.
# $(CXX) $(FLAGS) -c $< -o $@ -I$(UTILDIR): Compiles the .cpp file to an object file and includes the UTILDIR directory.
# clean: Specifies a target to clean up the project.
# $(RM) build/*: Removes all files in the build directory.
# $(RM) -r utils/pycache: Removes the pycache directory in the utils directory.
# $(RM) $(TRG): Removes the target executable.
# $(RM) -r data: Removes the data directory.




