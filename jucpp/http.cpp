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
            
            const StringStringMap& cookies = res.getCookies();
            String setCookie;
            for (StringStringMap::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
            {
                if (setCookie.length()) setCookie.append(" ;");
                setCookie.append((*it).first);
                setCookie.append("=");
                setCookie.append((*it).second);
            }
            if (setCookie.length())
                mg_send_header(conn, "Set-Cookie", setCookie.c_str());
			
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
					StringStringMap pathParams;
					
					for (size_t i = 0; i != urlParts.size(); ++i)
					{
						const String& a = urlParts[i];
						const String& b = patternList[i];
						
						if (b.length() && b.at(0) == ':')
							pathParams[b.substr(1)] = a;
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
    
    const String Request::Cookie(const String &name) const
    {
        String ret;
        String str = Header("Cookie");
        
        if (!str.length())
            return ret;
        
        char* buff = new char[str.length()];
        
        int r = get_var(str.c_str(), str.length(), name.c_str(), buff, str.length());
        
        if (r > 0)
            ret = String(buff, r);
        
        delete [] buff;

        return ret;
    }
	
	// Response

	void Response::write(const Json::Value& v)
	{
		write(Json::FastWriter().write(v).c_str());
	}
    
    void Response::redirect(String url, int status)
    {
        setStatus(status);
        addHeader("Location", url);
    }
	

}} // namespaces
