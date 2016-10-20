MPICXX=mpicxx
MPICXXLINK=vtcxx -vt:cxx mpicxx
#MPICXXLINK=$(MPICXX)

MPICC=vtcc -vt:mpi 

#CXXFLAGS=-O2 -Wall -W -pipe -ansi -DNDEBUG
CXXFLAGS=-O2 -Wall -W -pipe -ansi -Wno-unused-parameter
CCFLAGS=$(CXX_FLAGS)

#LIBS=-L../../otflib/.libs/ -lopen-trace-format -lz
LIBS=../../otflib/.libs/libopen-trace-format.a -lz
INC=-I../../otflib/ -DVTRACE -I/home/tschoo/bin/vtrace_kv/include/vampirtrace/
OBJ=replay.o replayHandler.o handler.o replayBufferHandler.o replayTimer.o replayLogger.o otfGenerator.o
HEADER=handler.h replayHandler.h otfMap.h replayBufferHandler.h replayTimer.h replayLogger.h otfGenerator.h

.Phony: all clean 

all: otf_replay replayExample otf_strip

replayExample: replayExample.c
	$(MPICC) $(CCFLAGS) -o $@ replayExample.c

generate_replay_example: generate_replay_example.cpp Makefile
	$(MPICXX) -Wall -g -o generate_replay_example generate_replay_example.cpp $(INC) $(LIBS)
	
otf_replay: $(OBJ) $(HEADER) Makefile
	$(MPICXXLINK) $(CXXFLAGS) -o otf_replay $(OBJ) $(LIBS)

otf_strip: otfStrip.cpp Makefile
	g++ -O2 -Wall -o otf_strip otfStrip.cpp $(INC) $(LIBS)

%.o: %.cpp $(HEADER) Makefile
	$(MPICXX) $(CXXFLAGS) -o $@ -c $< $(INC)

clean:
	rm -f generate_replay_example otf_replay otf_strip $(OBJ)
