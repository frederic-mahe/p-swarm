INCLUDES = -I ./include
COMPILEOPT=-Wall -mssse3 -mtune=core2 -Icityhash
#CXX = gcc -g -v -lstdc++ -lpthread -fprofile-arcs -ftest-coverage
CXX = g++ -g  
FLAGS = -c $(INCLUDES)
CXXFLAGS=$(COMPILEOPT) $(COMMON) -O3
OBJS=main.o matrix.o cpu_info.o qgram.o cityhash/city.o\
     util.o db.o scan.o search8.o search16.o property.o cluster.o\
     parallel.o clusterresult.o merger.o search.o
LINKFLAGS=-g
LIBS=-lpthread

DEPS=Makefile main.h cityhash/config.h cityhash/city.h


.SUFFIXES:.o .cc

%.o : %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.s : %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) -c -S -o $@ $<

all : main

main :  $(OBJS)
	$(CXX) $(LINKFLAGS) -o $@ $(OBJS) $(LIBS)

clean :
	rm -f *.o *~ $(PROG) gmon.out cityhash/*.o
