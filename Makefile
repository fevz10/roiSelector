CC = g++
CFLAGS = -g -Wall -std=c++11
SRCS = roiSelector.cpp
PROG = roiSelector

OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
