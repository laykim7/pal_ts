#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>

#include <dbDefs.h>
#include <registryFunction.h>
#include <subRecord.h>
#include <waveformRecord.h>
#include <stringinRecord.h>
#include <epicsExport.h>
#include <epicsThread.h>
#include <dbAccess.h>
#include <link.h>
#include <dbAddr.h>

using namespace std;

static long initStrToValue(subRecord *precord)
{
  long rtVal;
	DBADDR addr;
	
	rtVal=dbNameToAddr(precord->name, &addr);

	if (rtVal) {
    printf("ERROR!!! Record '%s' not found\n", precord->name);
		return rtVal;
	}

  printf("initStrToValue( %s ) !!!!!!!!!!!!!!\r\n", precord->name);









  // DBADDR *pdbAddr = dbGetPdbAddrFromLink(&precord->inpa);
	// long initvalue = precord->b;
  // printf("initStrToValue( %s ): %d !!!!!!!!!!!!!!\r\n", precord->name, initvalue);
	return (0);
}

static long procStrToValue(subRecord *precord)
{
	DBADDR *pdbAddr = dbGetPdbAddrFromLink(&precord->inpa);

	long initvalue = precord->b;
	long initcount = precord->c;
	long initinc   = precord->d;

	double *pfieldLink = (double *)pdbAddr->pfield;

	if(initvalue > 2047 ) return (-1);

	// printf("Type(%ld, 0x%x), Elements(%ld, 0x%x)\n", record,record, NOEL, NOEL);
	printf("procStrToValue : %ld-%ld-%ld\r\n", initvalue, initcount, initinc);

	waveformRecord *precordLink = (waveformRecord *)pdbAddr->precord;

	for(int i = 0; i < initcount; i++)
	{
		pfieldLink[i] = (long)initvalue+(i+1)*initinc;
		//if(pfieldLink[i]>2047) pfieldLink[i] = 2047; 
		if(i>2047) break; 
	}

	precordLink->nord = initcount;
	dbProcess((dbCommon*)precordLink);

	printf("*********** %ld-%ld-%ld\r\n", initvalue, initcount, initinc);
	return (0);
}

epicsRegisterFunction(initStrToValue);
epicsRegisterFunction(procStrToValue);
