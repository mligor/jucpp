#ifndef _JUCPP_H_
#define _JUCPP_H_

#include <stdio.h>

namespace jucpp
{
	class JobBase
	{
	public:
		// Virtual methods to implemented
		virtual void Execute() = 0;
		virtual void OnFinish() {};
		
	protected:
		friend class Job;
		virtual void run() = 0;
		virtual void wait() = 0;
		virtual void stop() = 0;
	};
	
	class ThreadJob : public JobBase
	{
	protected:
		friend void s_Job_ThreadFn(void* p);
		
	protected:
		virtual void run();
		virtual void wait();
		virtual void stop();
		
		virtual bool stopThread() { return false; }

	private:
		void* m_thread;
	};
	
	class Job
	{
	public:
		Job(JobBase* pJob) : m_pJob(pJob) {};
		
		void run() { m_pJob->run(); }
		void wait() { m_pJob->wait(); }
		void stop() { m_pJob->stop(); }

	private:
		JobBase* m_pJob;
	};
}

#endif // _JUCPP_H_
