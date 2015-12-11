//
//  jucpp.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 04/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include "jucpp.h"

// STL
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif


namespace jucpp
{
	// String
	
	const String String::EmptyString = {};
	const Variant EmptyVariant = {};
	
	// ThreadJob
	
	void s_job_ThreadFn(void* p)
	{
		ThreadJob* _jobBase = (ThreadJob*)p;
		_jobBase->Execute();
		_jobBase->OnFinish();
	}
	
	ThreadJob::~ThreadJob()
	{
		std::thread* thread = (std::thread*)m_thread;
		delete thread;
		m_thread = nullptr;
	}
	
	void ThreadJob::run()
	{
		m_thread = new std::thread([](ThreadJob* jobBase)
		{
			try
			{
				jobBase->Execute();
				jobBase->OnFinish();
			}
			catch (std::exception &e)
			{
				printf("Fatal exception in Server Thread : %s\n", e.what());
			}
			catch (...)
			{
				printf("Fatal exception in Server Thread\n");
			}
		}, this);
	}
	
	void ThreadJob::wait()
	{
		std::thread* thread = (std::thread*)m_thread;
		if (isMyThread())
			return; // we are on current thread

		if (thread && thread->joinable())
			thread->join();
	}

	bool ThreadJob::isMyThread()
	{
		std::thread* thread = (std::thread*)m_thread;
		if (thread->get_id() == std::this_thread::get_id())
			return true;
		return false;
	}

	void ThreadJob::stop()
	{
		m_bStop = true;
		wait();
	}

	void ThreadJob::terminate()
	{
		std::thread* thread = (std::thread*)m_thread;
		m_bStop = true;

		if (thread)
		{
#ifdef _WIN32
			TerminateThread(thread->native_handle(), 0);
#else
			pthread_cancel(thread->native_handle());
#endif
		}
	}

} // namespace end