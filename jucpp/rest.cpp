//
//  rest.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 05/08/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include "rest.h"

namespace jucpp { namespace rest {
	
	int RestServer::EventHandler(const jucpp::http::Request &req, jucpp::http::Response &res)
	{
		int ret = jucpp::http::Server::EventHandler(req, res);
		
		if (ret == res.ServeStaticFile())
			return ret;
		
		// write response
		auto it = m_get_functions.find(req.Url());
		if (it == m_get_functions.end())
		{
			//TODO: find functions with parameter
			
			/*
				1) convert all URLs in regular expressions : e.g. /feed/:id -> /feed/([^/]*?)
				2) match every regular expression with current URL and return the first
			 */
			
			return false; // TODO: wrong !!!
		}
		
		(*it).second(req, res);
		
		return true; //TODO: wrong;
		
	}
	
	// namespace end
}}
