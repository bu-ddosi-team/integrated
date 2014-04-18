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
#include "top.h"

int MAX_LINE=1024;

int numSamples=4;
float minFreq=50;
float maxFreq=100;
int numSteps=2;
float sweepDelay=0;
float gain=1;


int loadSavedSettings(char *fileName, Control& settings);

int readFromAddress(int new_s, char addrloc, int *iVal, double *dVal, int *type, Control& settings){
//we'll be simulating reading from the zynq with reading from a txt file. If we are using xml for settings data, we'll need to add code to handle it but it will be much more convient for accessing data.
	char reply[MAX_LINE];
	char controlFilename[] = "Control_Settings.txt";
	sprintf( reply, "%c", 'f');
	send( new_s, reply, strlen(reply), 0);
// 	loadSavedSettings("Control_Settings.txt", settings);
 	loadSavedSettings(controlFilename, settings);

	char *sendf= new char[MAX_LINE];
	switch(addrloc)
	{
		case 'a':	//nsweep
	  	  *iVal = settings.nSweep;
	  	  printf("Sweep number is %d \n", *iVal);
		  sprintf( sendf, "%c%d", 'b', *iVal);
		  *type = 1;
		  break;
		case 'b':	//nstep
	  	  *iVal = settings.nStep;
	  	  printf("Step Number is %d \n", *iVal);
		  sprintf( sendf, "%c%d", 'b',*iVal);
		  *type = 2;
		  numSteps=*iVal;
		  break;
		case 'c':	//nSample
	  	  *iVal = settings.nSample;
	  	  printf("Sample Number %d \n", *iVal);
	  	  sprintf( sendf, "%c%d", 'b',*iVal);
		  *type = 3;
		  numSamples=*iVal;
		  break;
		case 'd':	//dsweeep
	  	  *dVal = settings.dSweep;
	  	  printf("Sweep Delay is %f \n", *dVal);
	  	  sprintf( sendf, "%c%f", 'b',*dVal);
		  *type = 4;
		  sweepDelay=*dVal;
		  break;
		case 'e':	//minF
	  	  *dVal = settings.minF;
	  	  printf("Min Frequency is %f \n", *dVal);
	  	  sprintf( sendf, "%c%f", 'b',*dVal);
		  minFreq=*dVal;
		  *type = 5;
		  break;
		case 'f':	//maxF
	  	  *dVal = settings.maxF;
	  	  printf("Max Frequency is %f \n", *dVal);
	  	  sprintf( sendf, "%c%f", 'b',*dVal);
		  maxFreq=*dVal;
		  *type = 6;
		  break;
		case 'g':	//gain
	  	  *dVal = settings.maxF;
	  	  printf("Gain is %f \n", *dVal);
	  	  sprintf( sendf, "%c%f", 'b',*dVal);
		  gain=*dVal;
		  *type = 6;
		  break;
		default:
		  fprintf(stderr, "%s \n", "Not a valid address");
		  sprintf( sendf, "%c%s", 'e', "Not a valid address");
		  send( new_s, sendf,strlen( sendf), 0);
		  return 0;
	}
	//Server code to send back response? or do something else?
	send( new_s, sendf, strlen( sendf), 0);
	sprintf( sendf, "%c", 'f');
	send( new_s, sendf, strlen( sendf), 0);
return 1;
}

int loadSavedSettings(char *fileName, Control& settings){
	int buf_size = 128;
	char pset1[128];	char pset4[128];
	char pset2[128];	char pset5[128];
	char pset3[128];	char pset6[128];
	char pset7[128];
	fprintf(stderr, "opening %s \n", fileName);
	std::ifstream ifile (fileName, std::ifstream::in );
		ifile.getline(pset1, 128);
		ifile.getline(pset2, 128);
		ifile.getline(pset3, 128);
		ifile.getline(pset4, 128);
		ifile.getline(pset5, 128);
		ifile.getline(pset6, 128);
		ifile.getline(pset7, 128);
	ifile.close(); 
	
	 settings.C_sweep  = pset1;
	 settings.C_step   = pset2;
	 settings.C_sample = pset3;
	 settings.C_delay  = pset4;
	 settings.C_min    = pset5;
	 settings.C_max    = pset6;
	 settings.C_gain   = pset7;
	 
	 settings.nSweep   = atoi(pset1); 
	 settings.nStep    = atoi(pset2);
	 settings.nSample  = atoi(pset3);	
	 settings.dSweep   = atof(pset4);
	 settings.minF 	   = atof(pset5);
	 settings.maxF     = atof(pset6);
	 settings.gain     = atof(pset7);

}

