//
//  http.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 04/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

// disable error when using sprintf_s
#define _CRT_SECURE_NO_WARNINGS 

#include "http.h"

#include <libs/mongoose/mongoose.h>
#include <libs/json/json.h>

#include <string.h>
#include <string>
#include <vector>

#include <stdio.h>


namespace jucpp { namespace http {

	
	int s_Server_EventHandler(void *conn, int ev)
	{
		if (ev == MG_AUTH)
		{
			return MG_TRUE;   // Authorize all requests
			
		}
		else if (ev == MG_REQUEST)
		{
			mg_connection* pConn = (mg_connection*)conn;
			Server* _this = (Server*)pConn->server_param;
			Request req(pConn);
			Response res;
			int ret = _this->EventHandler(req, res);
			
			mg_send_status((mg_connection*)conn, res.getStatus());
			
			const StringStringMap& headers = res.getHeaders();
			for (StringStringMap::const_iterator it = headers.begin(); it != headers.end(); ++it)
				mg_send_header((mg_connection*)conn, (*it).first.c_str(), (*it).second.c_str());
			
			mg_printf_data((mg_connection*)conn, "%s", res.getContent().c_str());
			
			return ret;
		}
		return MG_FALSE;  // Rest of the events are not processed
	}
	
	int Server::EventHandler(const Request &req, Response &res)
	{
		if (m_fn)
		{
			m_fn(req, res);
			return MG_TRUE;
		}
		return MG_FALSE;
	}
	
	Job Server::listen(int port)
	{
		mg_server* server = mg_create_server(this, (mg_handler_t)s_Server_EventHandler);
		mg_set_option(server, "document_root", ".");
		
		char buffer[33];
		sprintf(buffer, "%d", port);

		mg_set_option(server, "listening_port", buffer);
	
		Job job(new ServerJob(server));
		job.run();
		return job;
	}
	
	
	// ServerJob
	
	void ServerJob::Execute()
	{
		mg_server* mgserver = (mg_server*)m_server;
		for (;;)
		{
			if (m_bStop)
				break;
			mg_poll_server(mgserver, 1000);  // Infinite loop, Ctrl-C to stop
		}
		
	}
	
	void ServerJob::OnFinish()
	{
		mg_server* mgserver = (mg_server*)m_server;
		mg_destroy_server(&mgserver);
	}
	
	// Request
	
	Request::Request(void* connection)
	{
		mg_connection* pConn = (mg_connection*)connection;
		m_method = pConn->request_method;
		m_url = pConn->uri;
		if (pConn->query_string)
		{
			m_url.append("?");
			size_t queryStringLen = strlen(pConn->query_string);
			char* buff = new char[queryStringLen + 1];
			//mg_url_decode(ri->uri, n, (char *) ri->uri, n + 1, 0);
			mg_url_decode(pConn->query_string, queryStringLen, buff, queryStringLen+1, 0);
			m_url.append(buff);
			delete [] buff;
		}
		
		m_httpVersion = pConn->http_version;

		//TODO: parse headers and body
		
		
		//m_rawHeaders = pConn->content;
		
		m_content = String(pConn->content, pConn->content_len);
	}
	
	const Variant& Request::ContentAsJson() const
	{
		if (m_jsonContentParsed)
			return m_jsonContent;
		
		Json::Reader reader;
		reader.parse(Content(), m_jsonContent);

		return m_jsonContent;
	}
	
	const String& Request::Headers(const String& name) const
	{
		StringStringMap::const_iterator it = m_headers.find(name);
		if (it == m_headers.end())
			return String::EmptyString;
		
		return (*it).second;
	}
	
	// Response

	void Response::write(const Json::Value& v)
	{
		write(Json::FastWriter().write(v).c_str());
	}
	
	

}} // namespaces
