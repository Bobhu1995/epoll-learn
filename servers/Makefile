cc = gcc
cxx = g++

TARGET = gatewaysvr 
OBJS = Main.o FrameWork.o
SRC = Config.h FrameWork.h FrameWork.cpp ServerTypes.h Main.cpp

gatewaysvr : $(OBJS)
	$(cxx) -o gatewaysvr Main.o FrameWork.o

FrameWork.o : Config.h FrameWork.h ServerTypes.h FrameWork.cpp
	$(cxx) -o FrameWork.o -c -g FrameWork.cpp

Main.o : $(SRC) 
	$(cxx) -o Main.o -c -g Main.cpp

clean :
	rm -f Main.o FrameWork.o gatewaysvr
