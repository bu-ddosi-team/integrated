#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <fstream>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <ctime>
#include "getSettings.cpp"
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>

#include "ddosi/bitbang-spi.h"
#include "ddosi/ddosi-constants.h"

//#define MAX_LINE 1024
extern int MAX_LINE;

extern int numSamples;
extern float minFreq;
extern float maxFreq;
extern int numSteps;
extern float sweepDelay;
extern float gain;

int stepSize;
int curFreq;

#define MAP_SIZE 4096UL
unsigned long int rstAddr=0x81200000;
unsigned long int sampleSizeAddr=0x81220000;
unsigned long int CDMA=0x7E200000;
unsigned long int RAM=0x10000000;
unsigned long int FILESIZE=4096;

void *rst; //board reset
void *samples; //number of samples, ready is also on this port
void *cdma; //cdma controller
void *ram; //RAM address (Where the data is)
inline void resetBoard();

int startCollecting(int new_s, Control& settings, dds_bbspi_dev *dds_dev)
{
	char f = 'f';
	char reply[MAX_LINE];
 	int len;

	printf("starting start.\n");
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	rst = mmap(0, MAP_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,rstAddr);
	samples = mmap(0, MAP_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,sampleSizeAddr);
	cdma = mmap(0, MAP_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,CDMA);
	ram = mmap(0, MAP_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,RAM);

	//Error checking!
	if (minFreq < 50)
		minFreq = 50;
	if (maxFreq > 450)
		maxFreq = 450;
	if (numSteps > 400)
		numSteps = 400;
	if (numSteps < 1)
		numSteps = 1;
	if (sweepDelay < 0)
		sweepDelay = 0;

	printf("in here bitch.\n");
	curFreq = (int)minFreq;
	stepSize = (int)(maxFreq - minFreq) / numSteps;
	if (stepSize < 1)
		stepSize = 1;
	//4KB of data per 1K samples
	FILESIZE=4096*numSamples;

	printf("I think these things are true:\n");
	printf("minF: %d, maxF: %d, numSteps: %d, delay: %d, stepSize: %d\n",minFreq,maxFreq,numSteps,sweepDelay,stepSize);

	printf("pre-reset.\n");
	//Reset board to start.
	resetBoard();
	//Tells the board what the sample size is.
	*(volatile unsigned int *)(samples) = ((unsigned int)stepSize);
	//Need to reset to board again!
	resetBoard();
	printf("post sample taking\n");

	//This sets the CDMA controller configuration
	*(volatile unsigned int *)(cdma+0x00) = ((unsigned int)0);
	//Source Pointer - location of BRAM
	*(volatile unsigned int *)(cdma+0x18) = ((unsigned int)0xC0000000);
	//Destination Pointer - location in RAM
	*(volatile unsigned int *)(cdma+0x20) = ((unsigned int)0x10000000);

	//At this point everything is set, so we need to check for
	//the "ready" signal from the RAM controller to signal
	//that everything is recorded.
	//Ready signal is on the same port as where we set samples
	int ready = 0;
	//wait until we're ready to go.
	printf("Waiting!\n");
	while(!ready) {
		ready = *(volatile int *)(samples + 0x08);
	}
	printf("Done waiting.\n");

	printf("Beginning CDMA transfer.\n");
	//Bytes to Transfer register. Starts the transfer.
	*(volatile unsigned int *)(cdma+0x28) = ((unsigned int)FILESIZE);

	//Check the status register on the DMA controller for idle
	while (*(volatile unsigned int *)(cdma+0x4)&(0x1)==0);
	printf("CDMA transfer done!\n");

	sprintf(reply, "%c", f);
	len = strlen( reply);
	send(new_s, reply, len, 0);
	memset(reply, 0, MAX_LINE);

	printf("Some of that data:\n");

	for (int i=0; i<20; i++)
		printf("%d,",((volatile int*)ram)[i]);
	printf("\n");

	printf("Sending 4K packets now.\n");
	//Send the data in 4K packets
	for (int i=0; i<numSteps; i++) {
		send(new_s, ((int *)((volatile int *)(ram+4096*i))), sizeof(int)*1024, 0);
	}
	printf("Sent %d 4K packets\n",numSteps);
	printf("Done with that now!\n");
	char *sendf= new char[MAX_LINE];
	sprintf( sendf, "%c", f);
	len = strlen(sendf);

	//Send a reply.
	uint8_t sendfchar = 102;
	sprintf(reply, "%c%c%c%c%c", f,f,f,f,f);
	len = strlen(reply);
	send(new_s, reply, len, 0);
	return 0;
}

inline void resetBoard() {
	*(volatile unsigned int *)(rst) = ((unsigned int)0);
	*(volatile unsigned int *)(rst) = ((unsigned int)1);
	*(volatile unsigned int *)(rst) = ((unsigned int)0);
}
