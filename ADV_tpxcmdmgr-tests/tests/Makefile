CC = gcc
CXX = g++

PIXET_DIR ?= /home/martin/repos/Pixet/
PIXET_OUT_DIR = $(PIXET_DIR)/_build/Linux_x64_Debug
PIXET_HWLIB_SRC_DIR = $(PIXET_DIR)/src/hwlibs
PIXET_HWLIB_OBJ_DIR = $(PIXET_DIR)/_build/Linux_x64_Debug_obj/hwlibs
PIXET_HWLIB_OUT_DIR = $(PIXET_OUT_DIR)/hwlibs

MINIPIX_SRC_DIR = $(PIXET_HWLIB_SRC_DIR)/minipix
MINIPIX_INCLUDE_PATHS = . ../../interfaces ../../shared ../common ../../interfaces ../../shared/files
MINIPIX_INCLUDE_DIRS = $(addprefix $(MINIPIX_SRC_DIR)/,$(MINIPIX_INCLUDE_PATHS))
MINIPIX_CPPFLAGS = $(addprefix -I,$(MINIPIX_INCLUDE_DIRS))

# ---- Googletest configuration ----
GTEST_INC_DIR ?= /usr/local/include
GTEST_LIB_DIR ?= /usr/local/lib

CPPFLAGS = $(MINIPIX_CPPFLAGS) -I$(GTEST_INC_DIR)
CXXFLAGS = --std=c++20 -g3 -pthread

# Library linking setup
HWLIBS = minipix
LDLIBS = $(addprefix $(PIXET_HWLIB_OUT_DIR)/,$(addsuffix .so,$(HWLIBS)))

# Link against GTest/GMock and provide main() via gtest_main
LDFLAGS += -L$(GTEST_LIB_DIR)
LDLIBS  += -lgtest -lgmock -lgtest_main -pthread

OBJS = main.o
TARGET = main

.PHONY: build-libs

all: $(TARGET) compile_commands.json

clean:
	rm -f $(OBJS) $(TARGET) compile_commands.json

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(PIXET_HWLIB_OUT_DIR)/$(HWLIBS):
	export OUTDIR=$(PIXET_OUT_DIR); \
	$(MAKE) -C $(PIXET_HWLIB_OBJ_DIR)/$(HWLIBS)

# One binary that runs all tests
$(TARGET): $(OBJS) $(PIXET_HWLIB_OUT_DIR)/$(HWLIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

# compile_commands.json (so IDEs/language servers get proper flags)
define COMPILE_COMMAND_TEMPLATE
{
  "directory": "$(CURDIR)",
  "command": "$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $(1) -o $(1:.cpp=.o)",
  "file": "$(1)"
}
endef

SOURCES := $(wildcard *.cpp)

compile_commands.json: Makefile
	@echo "Generating $@"
	$(file > $@,[)
	$(file >> $@,$(call COMPILE_COMMAND_TEMPLATE,$(firstword $(SOURCES))))
	@$(foreach src,$(wordlist 2,$(words $(SOURCES)),$(SOURCES)), \
		$(file >> $@,,) \
		$(file >> $@,$(call COMPILE_COMMAND_TEMPLATE,$(src))))
	$(file >> $@,])
