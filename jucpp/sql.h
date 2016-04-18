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

	struct SQLDBSettings
	{
		enum DBType { Unknown = 0, SQLite = 1, MySQL = 2 };

		DBType t = DBType::Unknown;
		String dbName;
		String host;
		String username;
		String password;
		bool readonly = false;
		bool autoopen = false;

		SQLDBSettings(DBType _t, String _dbName) : t(_t), dbName(_dbName) {} 
	};
	
	class SQLDB
	{
	public:
		using Row = Object;
		using Result = Array;
		
	public:
		SQLDB(SQLDBBase* p) : m_p(p) {}
		SQLDB(SQLDBSettings const& settings);

		//TODO: check move operator
		SQLDB(SQLDB&& p) { m_p = p.m_p; p.m_p = nullptr; }

		~SQLDB();
		
		void set(SQLDBSettings const& settings);
		void set(SQLDBBase* p);
		
		void open();
		void close();
		
		Result query(const char* q, ...);
		String lastInsertRowId();
		
		operator bool() { return m_p != nullptr; }

		const char* AUTOINCREMENT();
		const char* NOW();

	private:
		SQLDBBase* m_p = nullptr;
	};
	
	class SQLDBBase
	{
	public:
		virtual ~SQLDBBase() {}
		virtual SQLDB::Result query(const char* q) = 0;
		virtual void open() = 0;
		virtual void close() = 0;
		virtual String lastInsertRowId() = 0;

		virtual const char* AUTOINCREMENT() = 0;
		virtual const char* NOW() = 0;
	};
	

	class SQLite : public SQLDBBase
	{
	public:
		SQLite(const String& dbName, bool readonly = false) : m_dbName(dbName), m_readonly(readonly) {}
		
		// Inherited via SQLDBBase
		virtual void open() override;
		virtual void close() override;
		virtual SQLDB::Result query(const char* q) override;
		virtual String lastInsertRowId() override;

		virtual const char* AUTOINCREMENT() override { return "AUTOINCREMENT"; };
		virtual const char* NOW() override { return "datetime('now')"; };

	protected:
		void* m_db = nullptr;
		
	private:
		String m_dbName;
		bool m_readonly;
	};

#ifdef JUCPP_MYSQL
	class MySQL : public SQLDBBase
	{
	public:
		MySQL(String const& dbName, String const& host = "127.0.0.1", String const& username = "", String const& password = "");

		// Inherited via SQLDBBase
		virtual SQLDB::Result query(const char * q) override;
		virtual void open() override;
		virtual void close() override;
		virtual String lastInsertRowId() override;

		virtual const char* AUTOINCREMENT() override { return "AUTO_INCREMENT"; };
		virtual const char* NOW() override { return "NOW()"; };

	protected:
		void* m_db = nullptr;

	private:
		String m_dbName;
		String m_host;
		String m_username;
		String m_password;
	};
#endif // JUCPP_NO_MYSQL
	
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
