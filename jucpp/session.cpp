//
//  session.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 18/09/15.
//  Copyright © 2015 BeanOX UG. All rights reserved.
//

#include "session.h"
#include "sqlite.h"

#include <stdlib.h>
#include <algorithm>


namespace jucpp { namespace http {
	
	// ServerSessions
	
	class SessionManager : public sqlite::SQLite
	{
	public:
		SessionManager(String databasePath);
		Variant getSession(String s);
		String setSession(Variant o, String s = "");
		void deleteSession(String s);
	private:
		static String random_string(size_t length);
	};

	SessionManager::SessionManager(String databasePath)
	{
		open(databasePath);
		query("CREATE TABLE IF NOT EXISTS jucpp_session_info "
			  "(id INTEGER UNIQUE PRIMARY KEY AUTOINCREMENT, "
			  "c TEXT, t DATETIME, obj TEXT"
			  ")");
	}
	
	String SessionManager::random_string(size_t length)
	{
		auto randchar = []() -> char
		{
			const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
			const size_t max_index = (sizeof(charset) - 1);
			return charset[ rand() % max_index ];
		};
		std::string str(length, 0);
		std::generate_n( str.begin(), length, randchar );
		return str;
	}
	
	String SessionManager::setSession(Variant o, String s)
	{
		String id;
		String check;
		
		if (s.length())
		{
			size_t pos = s.find("-");
			if (pos != s.npos)
			{
				id = s.substr(pos+1);
				check = s.substr(0, pos);
			}
		}
		
		String value = Server::jsonEncode(o);
		
		if (id.length() && check.length())
		{
			Result r = query("SELECT c FROM jucpp_session_info WHERE id='" + id + "'");
			if (r.size())
			{
				if (r[(unsigned int)0]["c"] == check)
				{
					query("REPLACE INTO jucpp_session_info (t, id, c, obj) VALUES (datetime('now'), '" + id + "', '" + check + "', '" + value + "')");
					String sessionId = check + "-" + id;
					return sessionId;
				}
			}
		}
		check = random_string(64);
		query("INSERT INTO jucpp_session_info (t, c, obj) VALUES (datetime('now'), '" + check + "', '" + value + "')");
		id = getLastInsertRowId();
		String sessionId = check + "-" + id;
		return sessionId;
	}
	
	Variant SessionManager::getSession(String s)
	{
		if (!s.length())
			return Variant();
		
		String id;
		String check;
		
		size_t pos = s.find("-");
		if (pos != s.npos)
		{
			id = s.substr(pos+1);
			check = s.substr(0, pos);
		}
		
		if (id.length() && check.length())
		{
			Result r = query("SELECT obj FROM jucpp_session_info WHERE id='" + id + "' AND c='" + check + "'");
			if (r.size())
			{
				return Server::jsonDecode(r[(unsigned int)0]["obj"].asString());
			}
		}
		return Variant();
	}
	
	void SessionManager::deleteSession(String s)
	{
		if (!s.length())
			return;
		
		String id;
		String check;
		
		size_t pos = s.find("-");
		if (pos != s.npos)
		{
			id = s.substr(pos+1);
			check = s.substr(0, pos);
		}
		
		if (id.length() && check.length())
			query("DELETE FROM jucpp_session_info WHERE id='" + id + "' AND c='" + check + "'");
	}
	
	// Session
	
	String Session::s_sessionDatabasePath = "jucpp.sessions.db";
	String Session::s_sessionCookieName = "jucpp-session";
	
	Session::Session(const Request &req, Response &res)
	: m_request(req)
	, m_response(res)
	{
		m_sessionId = m_request.getCookie("jucpp-session").Value();
	}
	
	void Session::set(Variant o)
	{
		SessionManager sm(s_sessionDatabasePath);
		m_sessionId = sm.setSession(o, m_sessionId);
		m_response.addCookie(Cookie(s_sessionCookieName, m_sessionId));
	}

	Variant Session::get()
	{
		SessionManager sm(s_sessionDatabasePath);
		return sm.getSession(m_sessionId);
	}
	
	void Session::remove()
	{
		if (m_sessionId.length())
		{
			SessionManager sm(s_sessionDatabasePath);
			sm.deleteSession(m_sessionId);
			//TODO: ugly hack, should be changed when Cookie support expiration time
			m_response.addCookie(Cookie(s_sessionCookieName, " ;Expires=Thu, 01-Jan-1970 00:00:01 GMT;"));
		}
	}

}} // namespace jucpp::http
