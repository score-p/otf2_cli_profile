
#include <iostream>
#include <stdlib.h>
#include "replayLogger.h"
#include "replayTimer.h"

using namespace std;

void ReplayLogger::write_msg(string msg)
{
	cerr 	<< "[" << this->rank << "] " 
		<< msg 
		<< "[" << timer->get_sim_timestamp() << "]" 
		<< endl;
}


void ReplayLogger::debug(string msg)
{
	this->write_msg(msg.insert(0, "DEBUG: "));
}

void ReplayLogger::info(string msg)
{
	this->write_msg(msg);
}

void ReplayLogger::warn(string msg)
{
	this->write_msg(msg.insert(0, "WARN: "));
}

//void ReplayLogger::error(string& msg)
void ReplayLogger::error(string msg)
{
	this->write_msg(msg.insert(0, "ERROR: "));
	exit(EXIT_FAILURE);
}

void ReplayLogger::set_timer(ReplayTimer* timer)
{
	this->timer = timer;
}

void ReplayLogger::set_rank(int rank)
{
	this->rank = rank;
}
