SCAS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
SCAS_SRCS := $(SCAS_DIR)tree.cpp $(SCAS_DIR)tree_stack.cpp $(SCAS_DIR)parser.cpp
SCAS_CXXFLAGS := -I$(SCAS_DIR)
