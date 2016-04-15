//
//  sqlite.h
//  jucpp
//
//  Created by Igor Mladenovic on 29/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#ifndef JUCPP_SQL_H
#define JUCPP_SQL_H

#include "jucpp.h"

// STL
#include <vector>

namespace jucpp { namespace sql {
	
	class SQLDBBase;
	
	class SQLDB
	{
	public:
		using Row = Object;
		using Result = Array;
		
		enum DBType { Unknown = 0, SQLite = 1, MySQL = 2 };

	public:
		SQLDB(SQLDBBase* p) : m_p(p) {}
		SQLDB(){}

		//TODO: check move operator
		SQLDB(SQLDB&& p) { m_p = p.m_p; p.m_p = nullptr; }
		
		SQLDB(DBType t, const char* dbName, bool readonly = false);
		SQLDB(DBType t, const String& dbName, bool readonly = false) : SQLDB(t, dbName.c_str(), readonly) {}
		
		void set(DBType t, const char* dbName, bool readonly = false);
		void set(DBType t, const String& dbName, bool readonly = false) { set(t, dbName.c_str(), readonly); }
		
		~SQLDB();
		
		void open(const char* dbName, bool readonly = false);
		void open(const String& dbName, bool readonly = false) { open(dbName.c_str(), readonly); }
		void close();
		
		Result query(const char* q);
		Result query(const String& q) { return query(q.c_str()); }
		String lastInsertRowId();
		
		operator bool() { return m_p != nullptr; }

	private:
		SQLDBBase* m_p = nullptr;
	};
	
	class SQLDBBase
	{
	public:
		virtual ~SQLDBBase() {}
		virtual SQLDB::Result query(const char* q) = 0;
		virtual void open(const char* dbName, bool readonly) = 0;
		virtual void close() = 0;
		virtual String lastInsertRowId() = 0;
	};
	

	class SQLite : public SQLDBBase
	{
	public:
		SQLite() {}
		virtual ~SQLite() {}
		SQLite(const char* dbName, bool readonly = false) { open(dbName, readonly); }
		SQLite(const String& dbName, bool readonly = false) { open(dbName.c_str(), readonly); }
		
		virtual void open(const char* dbName, bool readonly) override;
		virtual void close() override;
		virtual SQLDB::Result query(const char* q) override;
		virtual String lastInsertRowId() override;

	protected:
		void* m_db = nullptr;
	};
	
	class SQLException : public std::exception
	{
	public:
		SQLException(const char* description)
		{
			m_description = "SQL Error : ";
			if (description)
				m_description +=  description;
			
		}
		virtual const char* what() const NOEXCEPT { return m_description.c_str(); }
		
	private:
		String m_description;
		
	};
	
// namespace end
}}


#endif // JUCPP_SQL_H
