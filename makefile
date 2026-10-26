SCAS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
SCAS_SRCS := $(SCAS_DIR)tree.cpp $(SCAS_DIR)tree_stack.cpp $(SCAS_DIR)parser.cpp $(SCAS_DIR)serialize.cpp \
	$(SCAS_DIR)checkpoint.cpp $(SCAS_DIR)integer.cpp $(SCAS_DIR)rational.cpp \
	$(SCAS_DIR)projection.cpp $(SCAS_DIR)beautification.cpp $(SCAS_DIR)simple_reduction.cpp
SCAS_CXXFLAGS := -I$(SCAS_DIR)
