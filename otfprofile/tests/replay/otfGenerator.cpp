#include <otf.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include "otfGenerator.h"

OtfGenerator::OtfGenerator(const string& namestub, int id)
{
	this->namestub = namestub;
	this->fileman = OTF_FileManager_open(4);
	if (this->fileman == NULL)
	{
		cerr << "Failed to open OTF_FileManager object. Aborting!" << endl;
		exit(1);
	}
	this->wstream = OTF_WStream_open(namestub.c_str(), id, this->fileman);
	if (this->wstream == NULL)
	{
		cerr << "Failed to open OTF_WStream object. Aborting!" << endl;
		exit(1);
	}
}

OtfGenerator::~OtfGenerator()
{
	OTF_FileManager_close(this->fileman);
	OTF_WStream_close(this->wstream);
}

int OtfGenerator::writeDefFunction(const uint32_t func, const char *name,
		     const uint32_t funcGroup, const uint32_t source)
{
	return 0;
}

int OtfGenerator::writeDefTimerResolution(const uint64_t ticksPerSecond)
{
	return 0;
}

int OtfGenerator::writeDefProcessGroup(const uint32_t procGroup, const char *name, 
			 const uint32_t numberOfProcs, const uint32_t *procs)
{
	return 0;
}


int OtfGenerator::writeDefKeyValue(const uint32_t token, const char *name, const OTF_KeyValueList *list)
{
	
	return 0;
}

int OtfGenerator::writeEnter(const uint64_t time, const uint32_t function, const uint32_t process,
	       const uint32_t source, OTF_KeyValueList *list )
{
	
	return 0;
}

int OtfGenerator::writeLeave(const uint64_t time, const uint32_t function, const uint32_t process,
	       const uint32_t source, OTF_KeyValueList *list)
{
	
	return 0;
}


int OtfGenerator::writeSendMsg (const uint64_t time, const uint32_t sender, const uint32_t receiver,
		  const uint32_t group, const uint32_t type, const uint32_t length, 
    		  const uint32_t source, OTF_KeyValueList *list)
{
	
	return 0;
}

int OtfGenerator::writeRecvMsg (const uint64_t time, const uint32_t recvProc, const uint32_t sendProc,
		  const uint32_t group, const uint32_t type, const uint32_t length, const uint32_t source, OTF_KeyValueList *list)
{
	
	return 0;
}

int OtfGenerator::writeBeginCollectiveOperation(const uint64_t time, const uint32_t process,
				  const uint32_t collOp, const uint64_t matchingId, const uint32_t procGroup,
      				  const uint32_t rootProc, const uint64_t sent, const uint64_t received,
      				  const uint32_t scltoken, OTF_KeyValueList *list)
{
	
	return 0;
}

int OtfGenerator::writeEndCollectiveOperation(const uint64_t time, const uint32_t process, const uint64_t matchingId, OTF_KeyValueList *list)
{
	
	return 0;
}

