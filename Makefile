CC := dpcpp
CFLAGS := -std=c++17 -g -O0 -Og
WFLAGS := -Wall -Wextra

INPUT_ARGS := config.json

LIBS := libs
SRCDIR := src
BUILDDIR := build
TESTBUILDDIR := testbuild
DEVCLOUD_EXEC := exec
INCDIR := include
BINDIR := bin
EXEC := HelixSolver.out
TESTSRCDIR := test
TESTEXEC := Test.out
TARGET := $(BINDIR)/$(EXEC)
TESTTARGET := $(BINDIR)/$(TESTEXEC)
SRCEXT := cpp

LINKBOOST := -lboost_program_options
JSON_SINGLE_INCLUDE := $(LIBS)/root/include
ROOT_CONFIG := $(shell root-config --glibs --libs)

SRC := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJ := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRC:.$(SRCEXT)=.o))

INC := -I $(INCDIR) -I $(JSON_SINGLE_INCLUDE)
# INC := -I $(INCDIR) -I $(JSON_SINGLE_INCLUDE) -I $(shell root-config --incdir)

TESTSRC := $(shell find $(TESTSRCDIR) -type f -name *.$(SRCEXT))
TESTOBJ := $(patsubst $(TESTSRCDIR)/%,$(TESTBUILDDIR)/%,$(TESTSRC:.$(SRCEXT)=.o))

GTESTINC := -I libs/googletest/googletest/include -I libs/googletest/googlemock/include
GTESTLIBPATH := $(LIBS)/build-gtest/lib

ARRIA_FLAGS_FOR_REPORT := -fintelfpga -Xshardware -fsycl-link=early -Xsboard=intel_a10gx_pac:pac_a10
STRATIX_FLAGS_FOR_REPORT := -fintelfpga -Xshardware -fsycl-link=early -Xsboard=intel_s10sx_pac:pac_s10

ARRIA_FLAGS := -fintelfpga -Xshardware -Xsboard=intel_a10gx_pac:pac_a10
STRATIX_FLAGS := -fintelfpga -Xshardware -Xsboard=intel_s10sx_pac:pac_s10

FPGA_FLAGS := -fintelfpga -DFPGA_EMULATOR
CPU_FLAGS := 
GPU_FLAGS :=
PLATFOMR_FLAGS := $(CPU_FLAGS)
# PLATFOMR_FLAGS := $(FPGA_FLAGS)
# PLATFOMR_FLAGS := $(GPU_FLAGS)

fpga_hw:
	$(CC) $(CFLAGS) $(STRATIX_FLAGS) $(INC) $(SRCDIR)/*.cpp -o bin/fpga_out.fpga

report:
	$(CC) $(CFLAGS) $(STRATIX_FLAGS_FOR_REPORT) $(INC) $(SRCDIR)/*.cpp -o bin/fpga_compile_report.a

run: $(TARGET)
	./$< $(INPUT_ARGS)

build: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(PLATFOMR_FLAGS) $(ROOT_CONFIG) $(INC) $(LINKBOOST) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(PLATFOMR_FLAGS) $(INC) $< -c -o $@ $(WFLAGS)

test: $(TESTTARGET)
	./$<

$(TESTTARGET): $(TESTOBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -pthread $(LIBS)/gtest-obj/*.o $^ -o $@

$(TESTBUILDDIR)/%.o: $(TESTSRCDIR)/%.$(SRCEXT)
	mkdir -p $(TESTBUILDDIR)
	$(CC) $(CFLAGS) $(INC) $(GTESTINC) $< -c -o $@ $(WFLAGS)

build_gtest:
	mkdir -p $(LIBS)/build-gtest
	mkdir -p $(LIBS)/gtest-obj
	cd $(LIBS)/build-gtest && cmake ../googletest
	make -C $(LIBS)/build-gtest
	cd $(LIBS)/gtest-obj && ar xv ../build-gtest/lib/libgmock.a
	cd $(LIBS)/gtest-obj && ar xv ../build-gtest/lib/libgtest.a
	cd $(LIBS)/gtest-obj && ar xv ../build-gtest/lib/libgtest_main.a

clean:
	rm -rf $(BUILDDIR) $(BINDIR) $(TESTBUILDDIR)
	rm -rf $(DEVCLOUD_EXEC)/build.sh.*
	rm -rf $(DEVCLOUD_EXEC)/run.sh.*
	rm -rf *.sh.o*
	rm -rf *.sh.e*
	rm -rf *.png
	rm -rf detected-circles.txt

clean_external:
	rm -rf $(LIBS)/build-gtest
	rm -rf $(LIBS)/gtest-obj

.PHONY: clean, run, build, test, build_gtest, clean_external, report, fpga_hw