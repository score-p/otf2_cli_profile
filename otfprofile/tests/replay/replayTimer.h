#include <stdint.h>
#include <sys/time.h>


#ifndef REPLAY_TIMER_H
#define REPLAY_TIMER_H

class ReplayTimer
{
	private:
		uint32_t _num_cycles;
		uint64_t _timer_resolution;
		
		/* timestamp of the otf trace file */
		uint64_t _sim_timestamp;
		/* timestamp of the real execution */
		uint64_t _real_timestamp;
		
		float delay_fact;
		
		int get_cycles_per_usec(void);
		int user_wait(int64_t usec);

	public:
		ReplayTimer();
		ReplayTimer(const ReplayTimer& timer);
		ReplayTimer(uint32_t cycles_per_usec);
		ReplayTimer& operator=(const ReplayTimer& timer);
		void user_function( uint64_t cur_sim_time );
		void update_timer( uint64_t sim_time );
		void set_timer_resolution(uint64_t resolution);
		void set_delay_factor(float factor);
		uint64_t get_sim_timestamp(){ return _sim_timestamp; }
		uint64_t get_real_timestamp(){ return _real_timestamp; }
};

#endif /* REPLAY_TIMER_H */
