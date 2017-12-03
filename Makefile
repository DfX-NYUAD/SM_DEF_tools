APP := DEF-RT

#=============================================================================#
# Define Compiler Executable:
#=============================================================================#
COMPILER	= clang++
#COMPILER	= /opt/clang+llvm-3.2-x86-linux-ubuntu-12.04/bin/clang++
#COMPILER	= g++
# Intel compiler; required for proper analysis w/ Intel VTune Amplifier XE
#COMPILER	= /opt/intel/bin/icpc

#=============================================================================#
# Compiler Options:
#=============================================================================#
# warnings
CXXFLAGS := $(CXXFLAGS) -Wall -Wextra -Wconversion
# C++11
CXXFLAGS := $(CXXFLAGS) -std=c++11
##-fopenmp
## gcc std::thread requires this threading support
#CXXFLAGS := $(CXXFLAGS) -pthread
# native tuning, since gcc 4.2
CXXFLAGS := $(CXXFLAGS) -march=native
# include directories
CXXFLAGS := $(CXXFLAGS) \
           -Isi2_parser/lef/include -Isi2_parser/def/include

# enable debug symbols
CXX_DEBUG_FLAGS := -g
# optimization and no-debug-symbol
CXX_RELEASE_FLAGS := -O3 -DNDEBUG

LDFLAGS :=
LD_DEBUG_FLAGS := $(LDFLAGS) -g
LD_RELEASE_FLAGS := $(LDFLAGS)

LDLIBS := -Lsi2_parser/lef/lib/ -Lsi2_parser/def/lib \
          -llef -ldef

#=============================================================================#
# Printing On/Off Options:
#=============================================================================#
PRINT_COMP := 

#=============================================================================#
# Variables:
#=============================================================================#
BUILD_DIR := build
DEBUG_DIR := $(BUILD_DIR)/debug_obj
RELEASE_DIR := $(BUILD_DIR)/release_obj
SRC_DIR := src
# derive related variables
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRC:%.cpp=%.o)
OBJS := $(notdir $(OBJS))
DEBUG_OBJS   := $(addprefix $(DEBUG_DIR)/,$(OBJS))
RELEASE_OBJS := $(addprefix $(RELEASE_DIR)/,$(OBJS))
DEBUG_DEPS   := $(DEBUG_OBJS:%.o=%.d)
RELEASE_DEPS := $(RELEASE_OBJS:%.o=%.d)

NUMCPUS := 2
#automatically check CPUs number (with 8 cores you need more then 5 GB RAM)
#NUMCPUS=`grep -c '^processor' /proc/cpuinfo`

#=============================================================================#
# Link Main Executable
#=============================================================================#
all: debug

debug: CXXFLAGS += $(CXX_DEBUG_FLAGS)
debug: si2_parser $(DEBUG_DIR)/$(APP)
	@$(MAKE) -s copy_debug_exe

profiling: CXXFLAGS += $(CXX_DEBUG_FLAGS) -g
profiling: LDFLAGS += -pg
profiling: si2_parser $(DEBUG_DIR)/$(APP)
	@$(MAKE) -s copy_debug_exe

release: CXXFLAGS += $(CXX_RELEASE_FLAGS)
release: si2_parser $(RELEASE_DIR)/$(APP)
	@$(MAKE) -s copy_release_exe

# I don't know how to combine these two targets...
$(DEBUG_DIR)/$(APP): $(DEBUG_OBJS)
	@echo
	@echo "=== linking main binary (DEBUG): $@"
	$(PRINT_COMP) $(COMPILER) $(LD_DEBUG_FLAGS) $(DEBUG_OBJS) -o $@ $(LDLIBS)

$(RELEASE_DIR)/$(APP): $(RELEASE_OBJS)
	@echo
	@echo "=== linking main binary (RELEASE): $@"
	$(PRINT_COMP) $(COMPILER) $(LD_RELEASE_FLAGS) $(RELEASE_OBJS) -o $@ $(LDLIBS)

# Targets to the right of the pipe symbol are order-only prerequisites.
# They are required, but do not trigger rebuilding the object files, if their
# timestamp changes.
$(DEBUG_OBJS): | $(DEBUG_DIR)
$(RELEASE_OBJS): | $(RELEASE_DIR)

$(DEBUG_DIR) $(RELEASE_DIR):
	@echo
	@echo "=== create build directories"
	mkdir -p $(DEBUG_DIR)
	mkdir -p $(RELEASE_DIR)

copy_debug_exe:
	@echo "=== copy debug binary to root directory"
	@cp -f $(DEBUG_DIR)/$(APP) $(APP)

copy_release_exe:
	@echo "=== copy release binary to root directory"
	@cp -f $(RELEASE_DIR)/$(APP) $(APP)

#=============================================================================#
# Compile Source Code to Object Files
#=============================================================================#

# pull in dependencies for compiled objects
# -include performs include w/o warnings, aborts for non-existent files
ifeq ($(MAKECMDGOALS),release)
-include $(RELEASE_DEPS)
else
-include $(DEBUG_DEPS)
endif

$(DEBUG_DIR)/%.o $(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo
	@echo "=== compile $<"
	$(PRINT_COMP) $(COMPILER) -c $(CXXFLAGS) $< -o $@
	$(PRINT_COMP) $(COMPILER) -MM $(CXXFLAGS) $< | sed 's/$(basename $(@F))\.o/$(subst /,\/,$@)/g' > $(@:%.o=%.d)

#=============================================================================#
# si2_parser build
#=============================================================================#
si2_parser: si2_parser/def/include/defrReader.hpp si2_parser/lef/include/lefrReader.hpp

si2_parser/def/include/defrReader.hpp:
	cd si2_parser/def && make clean && make

si2_parser/lef/include/lefrReader.hpp:
	cd si2_parser/lef && make clean && make

#=============================================================================#
# Cleanup build
#=============================================================================#
clean:
	@echo "=== removing: $(DEBUG_DIR)/* $(RELEASE_DIR)/* $(APP)"
	@rm -f $(DEBUG_DIR)/* $(RELEASE_DIR)/* $(APP)
	
#=============================================================================#
# Purge build
#=============================================================================#
purge: clean

#=============================================================================#
# Some debugging output
#=============================================================================#
debug_info:
	@echo "-----------------------------------------------------------------------"
	@echo "- COMPILER    : $(COMPILER)"
	@echo "-----------------------------------------------------------------------"
	@echo "- APP         : $(APP)"
	@echo "-----------------------------------------------------------------------"
	@echo "- SRC         : $(SRC)"
	@echo "-----------------------------------------------------------------------"
	@echo "- DEBUG_OBJS   : $(DEBUG_OBJS)"
	@echo "-----------------------------------------------------------------------"
	@echo "- RELEASE_OBJS : $(RELEASE_OBJS)"
	@echo "-----------------------------------------------------------------------"

#=============================================================================#
# Make without printing of warnings
#=============================================================================#
nowarn: CXXFLAGS += -w
nowarn: PRINT_COMP = @
nowarn: all

littlewarn: CXXFLAGS += -Wno-sign-compare -Wno-switch -Wno-unused-variable -Wno-unused-parameter -Wno-comment
littlewarn: all

# to specify number of threads: make parallelnowarn N=2 (Default: N=4)
parallelnowarn: ts := `/bin/date "+%Y-%m-%d---%H-%M-%S"`
parallelnowarn:
	@$(MAKE) -s nowarn -j$(NUMCPUS)
	@echo === $(ts)
	
# to specify number of threads: make parallelnowarn N=2 (Default: N=4)
parallel: ts := `/bin/date "+%Y-%m-%d---%H-%M-%S"`
parallel:
	@$(MAKE) -s all -j$(NUMCPUS)
	@echo === $(ts)
