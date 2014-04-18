GPP=arm-xilinx-linux-gnueabi-g++
INCLUDES=-I./includes

all: testing

testing: SNC.cpp odscpp.cpp bitbang-spi.cpp
	${GPP} ${INCLUDES} -static -o $@ SNC.cpp odscpp.cpp bitbang-spi.cpp

clean:
	rm -f *.0 *~ *.so *.a *.o $(PROGS)
	rm testing
