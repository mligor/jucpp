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
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif


namespace jucpp
{
	// String
	
	const String String::EmptyString = {};
	const Variant EmptyVariant = {};
	
	StringList String::split(const String delimiter, int nMax) const
	{
		StringList strList;
		
		if (delimiter.empty())
		{
			strList.push_back(*this);
			return strList;
		}
		
		String::size_type i = 0;
		String::size_type j = 0;
		
		for (;;)
		{
			j = find(delimiter, i);
			if (j == String::npos || (nMax > 0 && ((int)strList.size() >= (nMax - 1))))
			{
				strList.push_back(substr(i));
				break;
			}
			
			strList.push_back(substr(i, j - i));
			i = j + delimiter.size();
			
			if (i == size())
			{
				strList.push_back(String());
				break;
			}
		}
		return strList;
	}
	
	String String::lowercase() const
	{
		String s(*this);
		
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		return s;
	}


	
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