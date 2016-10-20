#ifndef REPLAY_DEBUG_H
#define REPLAY_DEBUG_H

#include <string>
#include "replayTimer.h"

using namespace std;

class ReplayLogger{
	private:
		ReplayTimer *timer;
		int rank;
		void write_msg(string msg);
	public:
		void set_timer(ReplayTimer* timer);
		void set_rank(int rank);
		
		void debug(string msg);
		void info(string msg);
		void warn(string msg);
		void error(string msg);

};

#endif /*REPLAY_DEBUG_H*/
