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
#include <cassert>
#include <cstdarg>


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
            
            const StringCookieMap& cookies = res.getCookies();
            for (StringCookieMap::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
            {
				String setCookie;
                setCookie.append((*it).first);
                setCookie.append("=");
                setCookie.append((*it).second.Value());
				mg_send_header(conn, "Set-Cookie", setCookie.c_str());
            }
			
			mg_printf_data((mg_connection*)conn, res.getContent().c_str(), res.getContent().size());
			return MG_TRUE;
		}
		return MG_FALSE;  // Rest of the events are not processed
	}
	
	Server::ResponseStatus Server::EventHandler(Request &req, Response &res)
	{
		if (m_logLevel <= LogDebug)
		{
			for (JobPtrList::iterator it = m_jobList.begin(); it != m_jobList.end(); ++it)
			{
				ServerJob* serverJob = (ServerJob*)(*it).getJob();
				if (serverJob->isMyThread())
				{
					Log(LogDebug, "Job(%d): %s %s%s%s",
						serverJob->m_jobNumber,
						req.Method().c_str(),
						req.Url().c_str(),
						req.QueryString().length() ? "?" : "",
						req.QueryString().c_str());
					break;
				}
			}
		}
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
					// check if pattern has * at the end (e.g. /api/*)
					if (pattern.length() && pattern.at(pattern.length() - 1) == '*')
					{
						// normal or negative match 
						bool normalMatch = (url.length() > 2 && pattern.at(pattern.length() - 2) != '!');

						if (normalMatch)
						{
							if (url.length() >= pattern.length() && 
								pattern.substr(0, pattern.length() - 1) != url.substr(0, pattern.length() - 1))
								continue;
						}
						else
						{
							// negative match
							if (url.length() >= pattern.length() - 1 &&
								pattern.substr(0, pattern.length() - 2) == url.substr(0, pattern.length() - 2))
								continue;
						}
					}
					else
					{
						StringList patternList;
						SplitString(pattern, "/", patternList);

						if (urlParts.size() != patternList.size()) continue;

						bool matched = true;
						StringStringMap pathParams;

						for (size_t i = 0; i != urlParts.size(); ++i)
						{
							const String& a = urlParts[i];
							const String& b = patternList[i];
							Log(LogTrace, "URL PART: url=%s pattern=%s", a.c_str(), b.c_str());

							if (b.length() && b.at(0) == ':')
								pathParams[b.substr(1)] = a;
							else if (b == "*")
								;
							else if (b != a)
							{
								matched = false;
								break;
							}
						}
						if (!matched) continue;

						for (StringStringMap::const_iterator it = pathParams.begin(); it != pathParams.end(); ++it)
							req.setPathParam(it->first, it->second);

					}
				}
				ResponseStatus s = f.second(req, res);
				if (s != Skipped)
					return s;
			}
		}
		return Skipped;
	}

	void Server::Log(LogLevel l, const char * fmt, ...)
	{
		if (l < m_logLevel) return;

		String logText;

		size_t n;
		size_t size = 64;
		char *p, *np;
		va_list ap;

		if ((p = (char*)malloc(size)) == NULL)
			return;

		while (1)
		{
			va_start(ap, fmt);
			n = vsnprintf(p, size, fmt, ap);
			va_end(ap);

			if (n < size)
			{
				logText = String(p, size);
				free(p);
				break;
			}
			size = n * 2;
			if ((np = (char*)realloc(p, size)) == NULL) 
			{
				free(p);
				return;
			}
			else 
				p = np;
		}

		const char* prefix = "UNKNWON";
		switch (l)
		{
		case LogTrace: prefix = "TRACE"; break;
		case LogDebug: prefix = "DEBUG"; break;
		case LogInfo:  prefix = "INFO";  break;
		case LogWarn:  prefix = "WARN";  break;
		case LogError: prefix = "ERROR"; break;
		case LogFatal: prefix = "FATAL"; break;
		}
		printf("%s: %s\n", prefix, logText.c_str());
	}
	
	Server& Server::listen(int port)
	{
		char buffer[33];
		sprintf(buffer, "%d", port);
		mg_server* firstServer = NULL;

		for (int i = 0; i < m_workerPoolCnt; ++i)
		{
			mg_server* server = mg_create_server(this, (mg_handler_t)s_Server_EventHandler);
			if (m_documentRoot.size())
				mg_set_option(server, "document_root", m_documentRoot.c_str());

			if (i == 0)
			{
				firstServer = server;
				const char* err = mg_set_option(server, "listening_port", buffer);

				if (err)
					throw Exception(err);
			}
			else
				mg_copy_listeners(firstServer, server);

			ServerJob* serverJob = new ServerJob(server);
			serverJob->m_jobNumber =  m_workerCnt + i;
			JobPtr job(serverJob);
			job.run();
			m_jobList.push_back(job);
		}
		m_workerCnt += m_workerPoolCnt;
		return *this;
	}
	
	Server& Server::wait()
	{
		for (JobPtrList::iterator it = m_jobList.begin(); it != m_jobList.end(); ++it)
			(*it).wait();

		return *this;
	}
	
	Server& Server::stop()
	{
		if (m_jobList.size())
		{
			for (JobPtrList::iterator it = m_jobList.begin() + 1; it != m_jobList.end(); ++it)
				(*it).stop();
			m_jobList.at(0).stop(); // First job is listen job and it should be stoped at the end;
		}
		return *this;
	}
	
	Server& Server::terminate()
	{
		if (m_jobList.size())
		{
			for (JobPtrList::iterator it = m_jobList.begin() + 1; it != m_jobList.end(); ++it)
				(*it).terminate();
			m_jobList.at(0).terminate();
		}
		return *this;
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
    
    Variant Server::jsonDecode(const String &s, bool *error)
    {
        Variant v;
        Json::Reader reader;
        bool ok = reader.parse(s, v);
        if (error)
            *error = !ok;
        return v;
    }

    String Server::jsonEncode(const Variant &v)
    {
        return Json::FastWriter().write(v);
    }

	
	// ServerJob
	void ServerJob::Execute()
	{
		mg_server* mgserver = (mg_server*)m_server;
		for (;;)
		{
			if (shouldStop()) break;
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
		
		auto it = m_headers.find("Cookie");
		if (it != m_headers.end())
		{
			String cookiesHeader = (*it).second;
			std::size_t pos = 0;
			while (pos != cookiesHeader.npos)
			{
				std::size_t oldPos = pos;
				pos = cookiesHeader.find(";", oldPos);
				String singleCookie;
				if (pos != cookiesHeader.npos)
				{
					singleCookie = cookiesHeader.substr(oldPos, pos - oldPos);
					++pos;
				}
				else
					singleCookie = cookiesHeader.substr(oldPos);
				
				Cookie c(singleCookie);
				m_cookies[c.Name()] = c;
			}
		}
    
		m_httpVersion = pConn->http_version;
		m_content = String(pConn->content, pConn->content_len);
		Json::Reader reader;
		reader.parse(m_content, m_jsonContent);
	}
	
	const String& Request::Header(const String& name) const
	{
		StringStringMap::const_iterator it = m_headers.find(name);
		if (it != m_headers.end())
            return (*it).second;
		return String::EmptyString;
	}
	
    // mongose method
    static int get_var(const char *data, size_t data_len, const char *name,
                       char *dst, size_t dst_len)
    {
        auto mg_strncasecmp = [](const char *s1, const char *s2, size_t len)
        {
            auto lowercase = [](const char *s)
            {
                return tolower(* (const unsigned char *) s);
            };
            
            int diff = 0;
            
            if (len > 0)
                do {
                    diff = lowercase(s1++) - lowercase(s2++);
                } while (diff == 0 && s1[-1] != '\0' && --len > 0);
            
            return diff;
        };
        
        const char *p, *e, *s;
        size_t name_len;
        int len;
        
        if (dst == NULL || dst_len == 0) {
            len = -2;
        } else if (data == NULL || name == NULL || data_len == 0) {
            len = -1;
            dst[0] = '\0';
        } else {
            name_len = strlen(name);
            e = data + data_len;
            len = -1;
            dst[0] = '\0';
            
            // data is "var1=val1&var2=val2...". Find variable first
            for (p = data; p + name_len < e; p++) {
                if ((p == data || p[-1] == '&') && p[name_len] == '=' &&
                    !mg_strncasecmp(name, p, name_len)) {
                    
                    // Point p to variable value
                    p += name_len + 1;
                    
                    // Point s to the end of the value
                    s = (const char *) memchr(p, '&', (size_t)(e - p));
                    if (s == NULL) {
                        s = e;
                    }
                    assert(s >= p);
                    
                    // Decode variable into destination buffer
                    len = mg_url_decode(p, (size_t)(s - p), dst, dst_len, 1);
                    
                    // Redirect error code from -1 to -2 (destination buffer too small).
                    if (len == -1) {
                        len = -2;
                    }
                    break;
                }
            }
        }
        return len;
    }
    
	String Request::Get(const String& name) const
	{
        String ret;
        if (!m_queryString.length())
            return ret;
        
        char* buff = new char[m_queryString.length()];
        int r = get_var(m_queryString.c_str(), m_queryString.length(), name.c_str(), buff, m_queryString.length());
        
        if (r > 0)
            ret = String(buff, r);
        
        delete [] buff;
        return ret;
	}
    
    const String& Request::PathParam(const String& name) const
    {
        StringStringMap::const_iterator it = m_pathParams.find(name);
        if (it != m_pathParams.end())
            return (*it).second;
         return String::EmptyString;
    }
    
    const Cookie& Request::getCookie(const String &name) const
    {
		auto it = m_cookies.find(name);
		if (it != m_cookies.end())
			return it->second;
		return Cookie::InvalidCookie;
    }
	
	// Response
	void Response::write(const Json::Value& v)
	{
		write(Json::FastWriter().write(v).c_str());
	}
	
	void Response::write_styled(const Json::Value& v)
	{
		write(Json::StyledWriter().write(v).c_str());
	}
	
    void Response::redirect(String url, int status)
    {
        setStatus(status);
        addHeader("Location", url);
    }
	
	// Cookie
	const Cookie Cookie::InvalidCookie;
	
	void Cookie::parse(String text)
	{
		auto TrimLeft = [](String s) -> String
		{
			std::size_t pos = s.find_first_not_of(" ");
			if (pos != s.npos)
				return s.substr(pos);
			return s;
		};
		
		auto TrimRight = [](String s) -> String
		{
			std::size_t pos = s.find_last_not_of(" ");
			if (pos != s.npos)
				return s.substr(0, pos+1);
			return s;
		};
		
		m_name.clear();
		m_value.clear();
		
		std::size_t pos = text.find("=");
		if (pos == text.npos)
		{
			m_name = TrimLeft(TrimRight(text));
			return;
		}
		m_name = TrimLeft(text.substr(0, pos));
		m_value = TrimRight(text.substr(pos+1));
	}

}} // namespaces
