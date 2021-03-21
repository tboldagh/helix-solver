CC := dpcpp
CFLAGS := -std=c++17
WFLAGS := -Wall -Wextra

LIBS := libs
SRCDIR := src
BUILDDIR := build
TESTBUILDDIR := testbuild
INCDIR := include
INTDIR := interfaces
BINDIR := bin
EXEC := HelixSolver.out
TESTSRCDIR := test
TESTEXEC := Test.out
TARGET := $(BINDIR)/$(EXEC)
TESTTARGET := $(BINDIR)/$(TESTEXEC)
SRCEXT := cpp

LINKBOOST = -lboost_program_options

SRC := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJ := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRC:.$(SRCEXT)=.o))

INC := -I $(INCDIR) -I $(INTDIR)

TESTSRC := $(shell find $(TESTSRCDIR) -type f -name *.$(SRCEXT))
TESTOBJ := $(patsubst $(TESTSRCDIR)/%,$(TESTBUILDDIR)/%,$(TESTSRC:.$(SRCEXT)=.o))

GTESTINC := -I libs/googletest/googletest/include -I libs/googletest/googlemock/include
GTESTLIBPATH := $(LIBS)/build-gtest/lib

run: $(TARGET)
	./$<

$(TARGET): $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(LINKBOOST) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) $< -c -o $@ $(WFLAGS)

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

clean_external:
	rm -rf $(LIBS)/build-gtest
	rm -rf $(LIBS)/gtest-obj

.PHONY: clean, run, test, build_gtest, clean_external
