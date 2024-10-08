# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------

# entry point for the program and target name
MAIN := src/main.cpp

# assignment task file
ASSIGNMENT := src/Cache.hpp src/rahmensprogramm.h src/main.hpp

# target name
TARGET := DataCache

# Path to your systemc installation
SCPATH = /home/amatsukaze/gra/workspace/systemc

# Additional flags for the compiler
CXXFLAGS := -std=c++14 -I$(SCPATH)/include
LDFLAGS := -L$(SCPATH)/lib -lsystemc -lm

# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# Determine if clang or gcc is available
CXX := $(shell command -v g++ || command -v clang++)
CC := $(shell command -v gcc || command -v clang)
ifeq ($(strip $(CXX)),)
    $(error Neither clang++ nor g++ is available. Exiting.)
endif

# Add rpath except for MacOS
UNAME_S := $(shell uname -s)

ifneq ($(UNAME_S), Darwin)
    LDFLAGS += -Wl,-rpath=$(SCPATH)/lib
endif

# Default to release build for both app and library
all: debug

# Debug build
debug: CXXFLAGS += -g
debug: $(TARGET)

# Release build
release: CXXFLAGS += -O2
release: $(TARGET)

# recipe for building the program
$(TARGET): $(MAIN) $(ASSIGNMENT) src/rahmensprogramm.c
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) src/main.hpp src/rahmensprogramm.c $(LDFLAGS)

# clean up
clean:
	rm -f $(TARGET)

.PHONY: all debug release clean