//
//  http.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 04/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include "http.h"

#include <libs/mongoose/mongoose.h>
#include <libs/json/json.h>

#include <string.h>
#include <string>
#include <vector>

#include <stdio.h>


namespace jucpp { namespace http {

	using StringList = std::vector<String>;

	static size_t SplitString(const String &strInput, const String delimiter, StringList &strList, int nMax = -1)
	{
		strList.clear();

		if (delimiter.empty())
		{
			strList.push_back(strInput);
			return 1;
		}

		String::size_type i = 0;
		String::size_type j = 0;

		for (;;)
		{
			j = strInput.find(delimiter, i);
			if (j == String::npos || (nMax > 0 && ((int)strList.size() >= (nMax - 1))))
			{
				strList.push_back(strInput.substr(i));
				break;
			}

			strList.push_back(strInput.substr(i, j - i));
			i = j + delimiter.size();

			if (i == strInput.size())
			{
				strList.push_back(String());
				break;
			}
		}

		return strList.size();
	}

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
				s = _this->EventHandler(req, res);
			}
			catch (std::exception& e)
			{
				mg_send_status(conn, 501);
				mg_printf_data(conn, "exception: %s", e.what());
				return MG_TRUE;
			}
			
			if (s == Server::ServeStaticFile)
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
	
	Server::ResponseStatus Server::EventHandler(Request &req, Response &res)
	{
		const auto& l = m_functions.find(req.Method());
		String url = req.Url();
		if (url.length() > 0 && url.at(url.length() - 1) == '/')
			url = url.substr(0, url.length() - 1);

		if (l != m_functions.end())
		{
			StringList urlParts;
			SplitString(url, "/", urlParts);

			// find in param
			for (const auto &f : (*l).second)
			{
				const String& pattern = f.first;
				
				if (pattern != "*" && pattern != url) // direct and * (all) match
				{
					StringList patternList;
					SplitString(pattern, "/", patternList);
					
					if (urlParts.size() != patternList.size()) continue;
					
					bool matched = true;
					StringStringMap getValues;
					
					for (size_t i = 0; i != urlParts.size(); ++i)
					{
						const String& a = urlParts[i];
						const String& b = patternList[i];
						
						if (b.length() && b.at(0) == ':')
							getValues[b.substr(1)] = a;
						else if (b != a)
						{
							matched = false;
							break;
						}
					}
					
					if (!matched) continue;

					for (StringStringMap::const_iterator it = getValues.begin(); it != getValues.end(); ++it)
						req.setGet(it->first, it->second);
				}

				ResponseStatus s = f.second(req, res);
				if (s != Skipped)
					return s;
			}
		}
		return Skipped;
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
