#include <stdint.h>
#include <iostream>
#include "replayTimer.h"

/* WAIT can be one of the following values:
   0	do not wait
   1	wait using the usleep() function
   2	wait using a while-loop that evaluate the clock() function each pass (busywait)
   3	wait using a for-loop
*/
#define WAIT 3

/* If you want to use WAIT 3, you can define the number of cycles the cpu can perform in one micro second.
   Do not define this to let the cycles calculate at program startup.
   Note that this depends on the cpu and the optimization of the compiler. */

//#define CYCLES 24
//#define CYCLES 147
using namespace std;

ReplayTimer::ReplayTimer() : delay_fact(1)
{
	struct timeval cur_real_tv;
#if WAIT == 3
#ifndef CYCLES
	_num_cycles = get_cycles_per_usec();
#else
	_num_cycles = CYCLES;
#endif

	cout<< "Cycles: " << _num_cycles << endl;
	
#endif
	_sim_timestamp = 0;
	gettimeofday( &cur_real_tv, NULL );
	_real_timestamp = cur_real_tv.tv_sec + cur_real_tv.tv_usec * 1000000;
}

ReplayTimer::ReplayTimer(const ReplayTimer& timer) : 
#if WAIT == 3
		_num_cycles(timer._num_cycles),
#endif
		_sim_timestamp(timer._sim_timestamp),
		_real_timestamp(timer._real_timestamp),
		delay_fact(1)
{
}

ReplayTimer& ReplayTimer::operator=(const ReplayTimer& timer)
{
	if (&timer == this) return *this;
	
	_sim_timestamp = timer._sim_timestamp;
	_real_timestamp = timer._real_timestamp;
#if WAIT == 3
	_num_cycles = timer._num_cycles;
#endif
	
	return *this;
}

ReplayTimer::ReplayTimer(uint32_t cycles_per_usec) : delay_fact(1)
{
	struct timeval cur_real_tv;
	_num_cycles = cycles_per_usec;
	_sim_timestamp = 0;
	gettimeofday( &cur_real_tv, NULL );
	_real_timestamp = cur_real_tv.tv_sec + cur_real_tv.tv_usec * 1000000;
}

#if WAIT == 3
int ReplayTimer::get_cycles_per_usec(void) {

	uint64_t num_cycles;
	int max_j = 1000000;
	int max_i = 50;
	int ret = 1;

	clock_t start = clock();
	for(int j=0; j<max_j; j++) {
		for(int i=0; i<max_i; i++) {
			ret = ret + ret % 100;
		}
	}
	clock_t end = clock();
	num_cycles = ((double)max_i * (double)max_j) / (double)(end - start);
	
	return (num_cycles - (ret % ret));
}
#endif

__inline int ReplayTimer::user_wait(int64_t usec) {
	
	int ret = 1;
	if (usec > 0)
	{
		uint64_t towait = delay_fact * usec;
		
		//cout<< " delay " << usec << " usec \n";
	
	#if WAIT == 1
		/* wait suing usleep() */
		//usleep( useconds_t ( usec ) );
		usleep( (unsigned long) towait );
	
	#elif WAIT == 2
		/* busy wait */
		clock_t endwait;
		endwait = clock () + (towait * CLOCKS_PER_SEC / 1000000);
		while (clock() < endwait) {}
	
	#elif WAIT == 3
		/* busy wait by for-loop whose runtime matches 1 usec */
		for(uint64_t j=0; j<towait; j++) {
			for(uint i=0; i < _num_cycles; i++) {
				ret = ret + ret % 100;
			}
		}
	
	#endif
	}
	return ( ret % ret );
}

void ReplayTimer::user_function( uint64_t cur_sim_time )
{
	struct timeval cur_real_tv;
	double usec;
	double sim_diff;
	double real_diff;
	gettimeofday( &cur_real_tv, NULL );
	
	sim_diff = (cur_sim_time - this->_sim_timestamp) / (double) this->_timer_resolution * 1E6;
	real_diff =  (cur_real_tv.tv_sec * 1E6 + cur_real_tv.tv_usec) - this->_real_timestamp;

	if (sim_diff > real_diff)
	{
		usec = sim_diff - real_diff;
		//cout << "Sleeping for " << usec << "us" << endl;
		user_wait( usec );
	}
	
	update_timer( cur_sim_time );
}

void ReplayTimer::update_timer( uint64_t sim_time )
{
	struct timeval cur_real_tv;
	this->_sim_timestamp = sim_time;
	gettimeofday( &cur_real_tv, NULL );
	this->_real_timestamp = cur_real_tv.tv_sec * 1E6 + cur_real_tv.tv_usec;
}

void ReplayTimer::set_timer_resolution(uint64_t resolution)
{
	this->_timer_resolution = resolution;
}

void ReplayTimer::set_delay_factor(float factor)
{
	delay_fact = factor;
}
