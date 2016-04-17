//
//  session.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 18/09/15.
//  Copyright © 2015 BeanOX UG. All rights reserved.
//

#include "session.h"

#include <stdlib.h>
#include <algorithm>


namespace jucpp { namespace http {
	
	// ServerSessions
	
	class SessionManager
	{
	public:
		SessionManager(sql::SQLDBSettings const& storageSettings);
		Variant getSession(String s);
		String setSession(Variant o, String s = "");
		void deleteSession(String s);
	private:
		static String random_string(size_t length);
		sql::SQLDB m_db;
	};

	SessionManager::SessionManager(sql::SQLDBSettings const& storageSettings)
	{
		m_db.set(storageSettings);

		m_db.query("CREATE TABLE IF NOT EXISTS jucpp_session_info "
			  "(id INTEGER UNIQUE PRIMARY KEY %s, "
			  "c TEXT, t DATETIME, obj TEXT"
			  ")", m_db.AUTOINCREMENT());
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
			sql::SQLDB::Result r = m_db.query("SELECT c FROM jucpp_session_info WHERE id='%s'", id.c_str());
			if (r.size())
			{
				if (r[(unsigned int)0]["c"] == check)
				{
					m_db.query("REPLACE INTO jucpp_session_info (t, id, c, obj) VALUES (%s, '%s', '%s', '%s')", 
						m_db.NOW(), id.c_str(), check.c_str(), value.c_str());
					String sessionId = check + "-" + id;
					return sessionId;
				}
			}
		}
		check = random_string(64);
		m_db.query("INSERT INTO jucpp_session_info (t, c, obj) VALUES (%s, '%s', '%s')", m_db.NOW(), check.c_str(), value.c_str());
		id = m_db.lastInsertRowId();
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
			sql::SQLDB::Result r = m_db.query("SELECT obj FROM jucpp_session_info WHERE id='%s' AND c='%s'", id.c_str(), check.c_str());
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
			m_db.query("DELETE FROM jucpp_session_info WHERE id='%s' AND c='%s'", id.c_str(), check.c_str());
	}
	
	// Session
	
	String Session::s_sessionCookieName = "jucpp-session";
	sql::SQLDBSettings Session::s_storageSettings{ sql::SQLDBSettings::SQLite, "jucpp.sessions.db" };
	
	Session::Session(const Request &req, Response &res)
	: m_request(req)
	, m_response(res)
	{
		m_sessionId = m_request.getCookie(s_sessionCookieName).Value();
	}
	
	void Session::set(Variant o)
	{
		SessionManager sm(s_storageSettings);
		m_sessionId = sm.setSession(o, m_sessionId);
		m_response.addCookie(Cookie(s_sessionCookieName, m_sessionId));
	}

	Variant Session::get()
	{
		SessionManager sm(s_storageSettings);
		return sm.getSession(m_sessionId);
	}
	
	void Session::remove()
	{
		if (m_sessionId.length())
		{
			SessionManager sm(s_storageSettings);
			sm.deleteSession(m_sessionId);
			//TODO: ugly hack, should be changed when Cookie support expiration time
			m_response.addCookie(Cookie(s_sessionCookieName, " ;Expires=Thu, 01-Jan-1970 00:00:01 GMT;"));
		}
	}

}} // namespace jucpp::http
