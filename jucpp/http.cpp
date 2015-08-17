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

	
	int s_Server_EventHandler(void *_conn, int ev)
	{
		mg_connection* conn = (mg_connection*)_conn;
		if (ev == MG_AUTH)
		{
			return MG_TRUE;   // Authorize all requests
			
		}
		else if (ev == MG_REQUEST)
		{
			Server* _this = (Server*)conn->server_param;
			Request req(conn);
			Response res;
			int ret = _this->EventHandler(req, res);
			
			if (ret != MG_TRUE)
				return ret;
			
			mg_send_status(conn, res.getStatus());
			
			const StringStringMap& headers = res.getHeaders();
			for (StringStringMap::const_iterator it = headers.begin(); it != headers.end(); ++it)
				mg_send_header(conn, (*it).first.c_str(), (*it).second.c_str());
			
			mg_printf_data((mg_connection*)conn, res.getContent().c_str(), res.getContent().size());
			return ret;
		}
		return MG_FALSE;  // Rest of the events are not processed
	}
	
	int Server::EventHandler(Request &req, Response &res)
	{
		if (m_fn)
		{
			m_fn(req, res);
			
			if (res.ServeStaticFile())
				return MG_FALSE;
			
			return MG_TRUE;
		}
		return MG_FALSE;
	}
	
	Job Server::listen(int port)
	{
		mg_server* server = mg_create_server(this, (mg_handler_t)s_Server_EventHandler);
		if (m_documentRoot.size())
			mg_set_option(server, "document_root", m_documentRoot.c_str());
		
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
			//m_url.append("?");
			size_t queryStringLen = strlen(pConn->query_string);
			char* buff = new char[queryStringLen + 1];
			mg_url_decode(pConn->query_string, queryStringLen, buff, queryStringLen+1, 0);
			m_queryString = buff;
			delete [] buff;
		}

		//TODO: parse headers
		m_httpVersion = pConn->http_version;
		m_content = String(pConn->content, pConn->content_len);
		Json::Reader reader;
		reader.parse(m_content, m_jsonContent);
	}
	
	const String& Request::Header(const String& name) const
	{
		StringStringMap::const_iterator it = m_headers.find(name);
		if (it == m_headers.end())
			return String::EmptyString;
		
		return (*it).second;
	}
	
	const String& Request::Get(const String& name) const
	{
		StringStringMap::const_iterator it = m_get.find(name);
		if (it == m_get.end())
			return String::EmptyString;
		
		return (*it).second;
	}
	
	// Response

	void Response::write(const Json::Value& v)
	{
		write(Json::FastWriter().write(v).c_str());
	}
	
	

}} // namespaces
