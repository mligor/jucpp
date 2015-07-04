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
	
	void s_Job_ThreadFn(void* p)
	{
		
	}
	
	void Job::run()
	{
		m_thread = new tthread::thread(s_Job_ThreadFn, this);
	}
	
	void Job::wait()
	{
		
		
	}

} // namespace end