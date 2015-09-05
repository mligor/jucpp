//
//  http.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 04/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include "http.h"

#include <libs/mongoose/mongoose.h>
#include <libs/json/json.h>

#include <string.h>
#include <string>
#include <vector>
#include <regex>

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
			Server::ResponseStatus s = Server::Skipped;
			try
			{
				_this->EventHandler(req, res, s);
			}
			catch (std::exception* e)
			{
				mg_send_status(conn, 501);
				mg_printf_data(conn, "%s", e->what());
				return MG_TRUE;
			}
			
			if (s == Server::ServerStaticFile)
				return MG_FALSE; // Mongoose will do it for us
			
			if (s == Server::Skipped)
			{
				// set status not found
				mg_send_status(conn, 404);
				mg_printf_data(conn, "Not found");
				return MG_TRUE;
			}
			
			mg_send_status(conn, res.getStatus());
			
			//TODO: add settings to include CORS headers or not
			_this->addCORSHeaders(req, res);
			
			const StringStringMap& headers = res.getHeaders();
			for (StringStringMap::const_iterator it = headers.begin(); it != headers.end(); ++it)
				mg_send_header(conn, (*it).first.c_str(), (*it).second.c_str());
			
			mg_printf_data((mg_connection*)conn, res.getContent().c_str(), res.getContent().size());
			return MG_TRUE;
		}
		return MG_FALSE;  // Rest of the events are not processed
	}
	
	void Server::EventHandler(Request &req, Response &res, ResponseStatus& s)
	{
		const auto& l = m_functions.find(req.Method());
		
		if (l != m_functions.end())
		{
			// write response
			const auto& it_all = (*l).second.find(""); // ALL Match
			
			if (it_all != (*l).second.end())
			{
				(*it_all).second(req, res, s);
				if (s != Skipped)
					return;
			}
			
			const auto& it = (*l).second.find(req.Url());
			
			if (it != (*l).second.end())
			{
				s = Processed;
				(*it).second(req, res, s);
				if (s != Skipped)
					return;
			}
			
			// find in param
			for (const auto &f : (*l).second)
			{
				std::regex regex_part(":[^\\/]*");
				if (std::regex_search(f.first, regex_part, std::regex_constants::match_any))
				{
					// TODO: escape other special characters
					// TODO: build regex_name and catche it when function is inserted
					
					std::string regex_name = std::regex_replace(f.first, std::regex("/"), std::string("\\/"));
					
					regex_name = std::regex_replace(regex_name, regex_part, std::string("([^\\/]*?)"));
					regex_name.append("$");
					
					std::smatch match_name;
					if (std::regex_match(req.Url(), match_name, std::regex(regex_name)))
					{
						std::sregex_iterator next(f.first.begin(), f.first.end(), regex_part);
						std::sregex_iterator end;
						size_t counter = 0;
						while (next != end)
						{
							if (counter >= match_name.size())
								break;
							std::smatch match = *next;
							String key = match.str().substr(1);
							String value = match_name[counter+1].str();
							req.setGet(key, value);
							next++;
							counter++;
						}
						s = Processed;
						f.second(req, res, s);
						if (s != Skipped)
							return;
					}
				}
			}
		}
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
	
	void Server::addCORSHeaders(const Request &req, Response &res)
	{
		//TODO: add configuration to allow just specific Origins
		
		if (req.Header("Access-Control-Request-Headers").length())
			res.addHeader("Access-Control-Allow-Headers", req.Header("Access-Control-Request-Headers"));

		if (req.Header("Access-Control-Request-Method").length())
			res.addHeader("Access-Control-Allow-Methods", req.Header("Access-Control-Request-Method"));

		if (req.Header("Origin").length())
			res.addHeader("Access-Control-Allow-Origin", req.Header("Origin"));
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
		
		for (int i = 0; i < pConn->num_headers; ++i)
			m_headers[pConn->http_headers[i].name] = pConn->http_headers[i].value;

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
