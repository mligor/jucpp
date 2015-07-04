#ifndef _JUCPP_H_
#define _JUCPP_H_

#include <stdio.h>

namespace jucpp
{
	class Job
	{
	public:
		typedef void (*RunFn)();
		typedef void (*FinishFn)();

		Job(RunFn fnRun, FinishFn fnFinish = nullptr) : m_run(fnRun), m_finish(fnFinish) {}

		void run();
		void wait();
		
	private:
		RunFn m_run;
		FinishFn m_finish;
		
		void* m_thread;
	};

}

#endif // _JUCPP_H_
