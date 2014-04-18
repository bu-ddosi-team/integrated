G++ = arm-xilinx-linux-gnueabi-g++

all: testing

testing: SNC.cpp odscpp.cpp
	arm-xilinx-linux-gnueabi-g++ -static -o $@ SNC.cpp odscpp.cpp 

clean:
	rm -f *.0 *~ *.so *.a *.o $(PROGS)
	rm testing
