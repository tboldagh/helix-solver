CC := g++
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

SRC := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJ := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRC:.$(SRCEXT)=.o))

INC := -I $(INCDIR) -I $(INTDIR)

TESTSRC := $(shell find $(TESTSRCDIR) -type f -name *.$(SRCEXT))
TESTOBJ := $(patsubst $(TESTSRCDIR)/%,$(TESTBUILDDIR)/%,$(TESTSRC:.$(SRCEXT)=.o))

GTESTINC := -I libs/googletest/googletest/include -I libs/googletest/googlemock/include
GTESTLIBPATH := $(LIBS)/build-gtest/lib
GTESTLIBSLINK := -lgtest -lgmock -lgtest_main

run: $(TARGET)
	./$<

$(TARGET): $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) $< -c -o $@ $(WFLAGS)

test: $(TESTTARGET)
	./$<

$(TESTTARGET): $(TESTOBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -L$(GTESTLIBPATH) $(GTESTLIBSLINK) $^ -o $@

$(TESTBUILDDIR)/%.o: $(TESTSRCDIR)/%.$(SRCEXT)
	mkdir -p $(TESTBUILDDIR)
	$(CC) $(CFLAGS) $(INC) $(GTESTINC) $< -c -o $@ $(WFLAGS)

build_gtest:
	mkdir -p $(LIBS)/build-gtest
	cmake -S $(LIBS)/googletest -B $(LIBS)/build-gtest
	make -C $(LIBS)/build-gtest

clean:
	rm -rf $(BUILDDIR) $(BINDIR) $(TESTBUILDDIR)

clean_external:
	rm -rf libs/build-gtest
.PHONY: clean, run, test, build_gtest, clean_external
