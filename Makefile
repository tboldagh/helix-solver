CC := dpcpp
CFLAGS := -std=c++17
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
JSON_SINGLE_INCLUDE := $(LIBS)/json/single_include

SRC := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJ := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRC:.$(SRCEXT)=.o))

INC := -I $(INCDIR) -I $(JSON_SINGLE_INCLUDE)

TESTSRC := $(shell find $(TESTSRCDIR) -type f -name *.$(SRCEXT))
TESTOBJ := $(patsubst $(TESTSRCDIR)/%,$(TESTBUILDDIR)/%,$(TESTSRC:.$(SRCEXT)=.o))

GTESTINC := -I libs/googletest/googletest/include -I libs/googletest/googlemock/include
GTESTLIBPATH := $(LIBS)/build-gtest/lib

report:
	$(CC) $(CFLAGS) -fintelfpga -fsycl-link -Xshardware $(INC) $(LINKBOOST) $(SRCDIR)/*.cpp -o bin/report.a

run: $(TARGET)
	./$< $(INPUT_ARGS)

build: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -fintelfpga -DFPGA_EMULATOR $(LINKBOOST) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -fintelfpga -DFPGA_EMULATOR $(INC) $< -c -o $@ $(WFLAGS)

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

clean_external:
	rm -rf $(LIBS)/build-gtest
	rm -rf $(LIBS)/gtest-obj

.PHONY: clean, run, build, test, build_gtest, clean_external, report
