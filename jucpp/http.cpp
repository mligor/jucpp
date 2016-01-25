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
    
    void Server::send_status_result(void *conn, int status, const char *msg, const char* text)
    {
        mg_printf((mg_connection*)conn, "HTTP/1.1 %d %s\r\nContent-type: text/html\r\n\r\n<h2>%d %s</h2>", status, msg, status, msg);
        
        if (text)
            mg_printf((mg_connection*)conn, "%s", text);
        
        mg_printf((mg_connection*)conn, "<p>jucpp v1.0 - <a href='http://www.jucpp.com/'>http://www.jucpp.com/</a></p>");
    }

    void Server::_MongooseEventHandler(void *_conn, int ev, void *ev_data)
	{
        mg_connection* conn = (mg_connection*)_conn;
        Server* _this = (Server*)conn->user_data;
        if (_this)
            _this->MongooseEventHandler(_conn, ev, ev_data);
    }
    
    void Server::MongooseEventHandler(void *_conn, int ev, void *ev_data)
    {
        http_message *hm = (http_message *) ev_data;
        mg_connection* conn = (mg_connection*)_conn;
        switch (ev)
        {
            case MG_EV_HTTP_REQUEST:
            {
                Request req(hm);
                Response res;
                Server::ResponseStatus s = Server::Skipped;
                
                try
                {
                    s = EventHandler(req, res);
                }
                catch (std::exception& e)
                {
                    send_status_result(conn, 501, "Internal Error", e.what());
                    conn->flags |= MG_F_SEND_AND_CLOSE;
                    return;
                }
                
                if (s == Server::ServeStaticFile && m_documentRoot.size() == 0)
                    s = Server::Skipped;
                
                if (s == Server::Proceeded)
                {
                    //addCORSHeaders(req, res);
                    
                    mg_send_head(conn, res.getStatus(), res.getContent().size(), nullptr);
                    mg_printf(conn, res.getContent().c_str(), res.getContent().size());
                    conn->flags |= MG_F_SEND_AND_CLOSE;
                    return;
                }
                else if (s == Server::ServeStaticFile)
                {
                    mg_serve_http_opts opts = {};
                    if (m_documentRoot.size())
                        opts.document_root = m_documentRoot.c_str();

                    mg_serve_http(conn, hm, opts);
                }
                else if (s == Server::Skipped)
                {
                    send_status_result(conn, 404, "Not found");
                    conn->flags |= MG_F_SEND_AND_CLOSE;
                    return;
                }
            }
                break;
            default:
                break;
        }

        /*
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
         */
	}
	
    Server::Server()
    {
        m_mongoose = new mg_mgr();
        
        mg_mgr_init((mg_mgr *)m_mongoose, 0);
    }
    
    Server::~Server()
    {
        mg_mgr_free((mg_mgr *)m_mongoose);
    }
    
	Server::ResponseStatus Server::EventHandler(Request &req, Response &res)
	{
		if (m_logLevel <= LogDebug)
		{
            Log(LogDebug, "%s %s%s%s",
                req.Method().c_str(),
                req.Url().c_str(),
                req.QueryString().length() ? "?" : "",
                req.QueryString().c_str());
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
        
        mg_connection* conn = mg_bind((mg_mgr*)m_mongoose, buffer, (mg_event_handler_t)Server::_MongooseEventHandler);
        conn->user_data = this;

        mg_set_protocol_http_websocket(conn);
        mg_enable_multithreading(conn);

        run();
        wait();
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

    void Server::Execute()
	{
		for (;;)
		{
			if (shouldStop()) break;
            mg_mgr_poll((mg_mgr*)m_mongoose, 1000);
		}
	}
	
	// Request
	Request::Request(void* _msg)
	{
        http_message* msg = (http_message*)_msg;
        
		m_method = String(msg->method.p, msg->method.len);
		m_url = String(msg->uri.p, msg->uri.len);
        
		if (msg->uri.p)
        {
            char* buff = new char[msg->uri.len + 1];
            mg_url_decode(msg->uri.p, (int)msg->uri.len, buff, (int)(msg->uri.len + 1), 0);
            m_url = buff;
            delete [] buff;
        }
        
		if (msg->query_string.p)
		{
			char* buff = new char[msg->query_string.len + 1];
			mg_url_decode(msg->query_string.p, (int)msg->query_string.len, buff, (int)(msg->query_string.len + 1), 0);
			m_queryString = buff;
			delete [] buff;
		}
		
		for (int i = 0; i < MG_MAX_HTTP_HEADERS; ++i)
        {
            if (msg->header_names[i].p)
                m_headers[String(msg->header_names[i].p, msg->header_names[i].len)] = String(msg->header_values[i].p, msg->header_values[i].len);
            else
                break;
        }
        
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
    
		m_content = String(msg->body.p, msg->body.len);
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
	
	String Request::Get(const String& name) const
	{
        String ret;
        if (!m_queryString.length())
            return ret;
        
        mg_str qs{m_queryString.c_str(), m_queryString.length()};
        
        char* buff = new char[m_queryString.length()];
        int r = mg_get_http_var(&qs, name.c_str(), buff, m_queryString.length());
        
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
