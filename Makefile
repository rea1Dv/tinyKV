CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = main

OBJS = ./src/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ./build/$(TARGET)  -pthread 