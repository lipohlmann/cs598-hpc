CXX      = mpicxx
CXXFLAGS = -O2 -Wall -Wextra -std=c++17 -Iinclude
LDFLAGS  = -lm

include ../common.mk

# ---- Assignment-specific settings ----
TARGET  = hw4
SRCS    = $(wildcard src/*.cc)
OBJS    = $(SRCS:.cc=.o)
NP     ?= 4

REPORT  = report/report.tex

.PHONY: all code pdf clean run

all: code pdf

code: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

pdf:
	$(LATEXMK) $(LATEXOPTS) -cd $(REPORT)

run: code
	mpirun -n $(NP) ./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
	$(LATEXMK) -C -cd $(REPORT)
