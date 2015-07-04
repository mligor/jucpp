//
//  jucpp.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 04/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include "jucpp.h"

#include <libs/tthread/source/tinythread.h>

namespace jucpp
{
	// ThreadJob
	
	void s_Job_ThreadFn(void* p)
	{
		ThreadJob* _jobBase = (ThreadJob*)p;
		_jobBase->Execute();
		_jobBase->OnFinish();
	}
	
	void ThreadJob::run()
	{
		m_thread = new tthread::thread(s_Job_ThreadFn, this);
	}
	
	void ThreadJob::wait()
	{
		tthread::thread* pThread = (tthread::thread*)m_thread;
		pThread->join();
	}

	void ThreadJob::stop()
	{
		tthread::thread* pThread = (tthread::thread*)m_thread;
		if (stopThread())
			wait();
		else
			delete pThread; // destroy thread object
		
	}

} // namespace end