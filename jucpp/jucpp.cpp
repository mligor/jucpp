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

namespace jucpp
{
	// String
	
	const String String::EmptyString = {};
	const Variant EmptyVariant = {};
	
	// ThreadJob
	
	void s_Job_ThreadFn(void* p)
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
			jobBase->Execute();
			jobBase->OnFinish();
		}, this);
	}
	
	void ThreadJob::wait()
	{
		std::thread* thread = (std::thread*)m_thread;
		if (thread->get_id() == std::this_thread::get_id())
			return; // we are on current thread

		if (thread)
			thread->join();
	}

	void ThreadJob::stop()
	{
		std::thread* thread = (std::thread*)m_thread;
		if (stopThread())
			wait();
		else
			delete thread, m_thread = nullptr; // destroy thread object
	}

} // namespace end