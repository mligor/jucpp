//
//  session.hpp
//  jucpp
//
//  Created by Igor Mladenovic on 18/09/15.
//  Copyright © 2015 BeanOX UG. All rights reserved.
//

#ifndef session_h
#define session_h

#include "http.h"
#include "sql.h"


namespace jucpp { namespace http {
	
	/**
	 Session is a class to enable storing server session. Session will create cookie 
	 with session ID and store the data in database on server side.
	 Database path can be set using Session::SetSessionDatabasePath() - 
	 default database is 'jucpp.sessions.db' in current directory.
	 
	 Example:
	 
	 @code
	 // writing (create) session
	 Variant sessionData;
	 sessionData["username"] = "myname";
	 Session s(req, res);
	 s.set(sessionData);
	 
	 // read a session
	 Session s(req, res);
	 Variant sessionData = s.get();
	 
	 // delete a session
	 Session s(req, res);
	 s.remove();
	 @endcode
	 */
	class Session
	{
	public:
		Session(const Request &req, Response &res);
		void set(Variant o);
		/// Get data stored into session. Returns empty Variant when no session is found.
		Variant get();
		/// Remove current session
		void remove();
		
		const String& Id() { return m_sessionId; }
		
		/// @brief Set session database path
		/// Default database is jucpp.sessions.db in the current directory.
		static void SetSessionStorageSettings(sql::SQLDBSettings const& storageSettings) { s_storageSettings = storageSettings; }
		static sql::SQLDBSettings const& GetSessionStorageSettings() { return s_storageSettings; }
		static void SetSessionCookieName(String name) { s_sessionCookieName = name; }
		static String GetSessionCookieName() { return s_sessionCookieName; }
		
	private:
		String m_sessionId;
		const Request& m_request;
		Response& m_response;
		
		static sql::SQLDBSettings s_storageSettings;
		static String s_sessionCookieName;
	};
	
}} // namespace jucpp::http


#endif /* session_h */
