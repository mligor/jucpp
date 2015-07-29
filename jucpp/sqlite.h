//
//  sqlite.h
//  jucpp
//
//  Created by Igor Mladenovic on 29/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#ifndef jucpp_sqlite_h
#define jucpp_sqlite_h

#include <vector>
#include "jucpp.h"

namespace jucpp { namespace sqlite {

	class SQLite
	{
	public:
		using Row = Object;
		using Result = Array;
		
	public:
		SQLite() : m_db(nullptr){}
		
		void open(const String& dbName, bool readonly = false);
		void close();
		Result query(const char* q);
		Result query(const String& q) { return query(q.c_str()); };
		
		//bool execute(const String& q);

	protected:
		void* m_db;
		
	};
	
	class SQLiteException
	{
	public:
		SQLiteException(const char* description)
		{
			if (description)
				m_description = description;
			
		}
		
		const String& description() const { return m_description; }
		
	private:
		String m_description;
		
	};
	
// namespace end
}}


#endif
