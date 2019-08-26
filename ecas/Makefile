
HOSTNAME = $(shell hostname)
USERNAME = $(shell whoami)

PROGRAM = ecas

DEBUG = 1

CPLEX_VERSION = 12.9
CPLEX_DIR = ../cplex/CPLEX_Studio129

SRCDIR = src
OBJDIR = obj

# 32bit: x86, 64bit: x86-64
ARCH = x86-64

GPP = g++

CPPFLAGS = -DIL_STD \
	-isystem $(CPLEX_DIR)/cpoptimizer/include \
	-isystem $(CPLEX_DIR)/concert/include

CXXFLAGS += -Wall -Wno-non-virtual-dtor -pipe -std=c++11

LDFLAGS = -L$(CPLEX_DIR)/cpoptimizer/lib/$(ARCH)_linux/static_pic \
	-L$(CPLEX_DIR)/concert/lib/$(ARCH)_linux/static_pic \
	-L$(CPLEX_DIR)/cplex/lib/$(ARCH)_linux/static_pic

LDFLAGS += -lilocplex -lcp -lcplex -lconcert -lpthread -ldl

ifeq ($(DEBUG), 1)
  CXXFLAGS += -g -p
else
  CXXFLAGS += -O3
endif

STARTUP_SOURCE = $(SRCDIR)/Main.cpp

CPP_SOURCES = \
	src/ECAS.cpp \
	src/Tools.cpp \
	src/Instance.cpp \
	src/Task.cpp \
	src/Machine.cpp


# $< the name of the related file that caused the action.
# $* the prefix shared by target and dependent files.
# $? is the names of the changed dependents.
# $@ is the name of the file to be made.
# $^ dependencies above


# ----- object files ---------------------------------------------------------------

OBJ_FILES = $(addprefix $(OBJDIR)/, $(patsubst %.cpp,%.o, \
	$(patsubst src/%, %, $(CPP_SOURCES) ) ) )
STARTUP_OBJ = $(addprefix $(OBJDIR)/, $(patsubst %.cpp,%.o, \
	$(patsubst src/%, %,$(STARTUP_SOURCE) ) ) )


all: ecas

depend:
	@echo 
	@echo "creating dependencies ..."
	$(GPP) -MM $(CPPFLAGS) $(CPP_SOURCES) $(SINGLE_FILE_SOURCES) \
	$(STARTUP_SOURCE) $(LD_FLAGS) \
	| sed -e "s/.*:/$(OBJDIR)\/&/" > depend.in

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/%.h
	@echo
	@echo "compiling $<"
	$(GPP) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $< 


# for startup (no header file available)
$(OBJDIR)/Main.o: $(SRCDIR)/Main.cpp
	@echo 
	@echo "compiling $<"
	$(GPP) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $< 

# ----- linking --------------------------------------------------------------------


ecas: $(STARTUP_OBJ) $(OBJ_FILES)	
	@echo 
	@echo "linking ..."
	@echo
	$(GPP) $(CPPFLAGS) $(CXXFLAGS) -o $(PROGRAM) $(OBJ_FILES) $(STARTUP_OBJ) $(LDFLAGS)


# ----- debugging and profiling ----------------------------------------------------

clean:
	rm -rf obj/*.o $(PROGRAM) gmon.out

run: $(PROGRAM)
	./$(PROGRAM) ../data/sample01.json
	
# ------ include dependency file ---------------------------------------------------
include depend.in
