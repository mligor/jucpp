//
//  sqlite.h
//  jucpp
//
//  Created by Igor Mladenovic on 29/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#ifndef JUCPP_SQLITE_H
#define JUCPP_SQLITE_H

#include "jucpp.h"

// STL
#include <vector>

namespace jucpp { namespace sqlite {

	class SQLite
	{
	public:
		using Row = Object;
		using Result = Array;
		
	public:
		SQLite() {}
		SQLite(const String& dbName, bool readonly = false) { open(dbName, readonly); }
		~SQLite() { close();}
		
		void open(const String& dbName, bool readonly = false);
		void close();
		Result query(const char* q);
		Result query(const String& q) { return query(q.c_str()); };
		String getLastInsertRowId();
		
		//bool execute(const String& q);

	protected:
		void* m_db = nullptr;
		
	};
	
	class SQLiteException : public std::exception
	{
	public:
		SQLiteException(const char* description)
		{
			m_description = "SQLite Error : ";
			if (description)
				m_description +=  description;
			
		}
		virtual const char* what() const NOEXCEPT { return m_description.c_str(); }
		
	private:
		String m_description;
		
	};
	
// namespace end
}}


#endif // JUCPP_SQLITE_H
