//
//  rest.h
//  jucpp
//
//  Created by Igor Mladenovic on 05/08/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#ifndef __jucpp__rest__
#define __jucpp__rest__

#include "jucpp.h"
#include "http.h"

namespace jucpp { namespace rest {

	class RestServer : public jucpp::http::Server
	{
	public:
		RestServer(jucpp::http::Server::ServerFn fn, const String& documentRoot = String::EmptyString) : jucpp::http::Server(fn, documentRoot)
		{}
		
		static RestServer createServer(jucpp::http::Server::ServerFn fn, const String& documentRoot = String::EmptyString)
		{
			RestServer serv(fn, documentRoot);
			return serv;
		}
		
		void GET(String cp, jucpp::http::Server::ServerFn fn)
		{
			m_get_functions[cp] = fn;
		}

		
	protected:
		virtual int EventHandler(jucpp::http::Request &req, jucpp::http::Response &res) override;
		
	private:
		using FunctionList = std::map<String, jucpp::http::Server::ServerFn>;
		FunctionList m_get_functions;

	};
	
// namespace end
}}


#endif /* defined(__jucpp__rest__) */
