CC := g++
CFLAGS := -std=c++17
WFLAGS := -Wall -Wextra

SRCDIR := src
BUILDDIR := build
INCDIR := include
INTDIR := interfaces
BINDIR := bin
EXEC := HelixSolver.out
TARGET := $(BINDIR)/$(EXEC)
SRCEXT := cpp

SRC := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJ := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRC:.$(SRCEXT)=.o))

INC := -I $(INCDIR) -I $(INTDIR)

run: $(TARGET)
	./$<

$(TARGET): $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) $< -c -o $@ $(WFLAGS)

clean:
	rm -rf $(BUILDDIR) $(BINDIR)

.PHONY: clean, run
