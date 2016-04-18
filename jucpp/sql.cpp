#include "sql.h"
#include <libs/sqlite/sqlite3.h>
#include "jucpp_mysql.h"

#include <stdlib.h>


namespace jucpp { namespace sql {

	// SQLDB
	
	SQLDB::SQLDB(SQLDBSettings const& settings)
	{
		set(settings);
	}
	
	SQLDB::~SQLDB()
	{
		if (m_p)
		{
			m_p->close();
			delete m_p, m_p = nullptr;
		}
	}

	void SQLDB::set(SQLDBSettings const& settings)
	{
		if (m_p)
			throw SQLException("DB already initialized");
		
		switch (settings.t)
		{
		case SQLDBSettings::DBType::Unknown:
			throw SQLException("Unknown database type");
			break;
		case SQLDBSettings::DBType::SQLite:
			m_p = new class SQLite(settings.dbName, settings.readonly);
			break;
		case SQLDBSettings::DBType::MySQL:
#ifdef JUCPP_MYSQL
			m_p = new class MySQL(settings.dbName, settings.host, settings.username, settings.password);
#else
			throw SQLException("jucpplib is not compiled with Mysql support");
#endif
			break;
		}
		
		if (settings.autoopen) m_p->open();
	}

	void SQLDB::set(SQLDBBase * p)
	{
		if (m_p)
			throw SQLException("DB already initialized");

		if (!p)
			throw SQLException("Invalid DB pointer");

		m_p = p;
	}

	
	void SQLDB::open() { m_p->open(); }
	void SQLDB::close() { m_p->close(); }
	
	SQLDB::Result SQLDB::query(const char* q, ...) 
	{ 
		String query;
		va_list argptr;
		va_start(argptr, q);
		size_t n = vsnprintf(nullptr, 0, q, argptr);
		va_end(argptr);
		if (n == (size_t)-1)
			throw SQLException("Invalid Query formating");
		
		va_start(argptr, q);
		if (n > 0)
		{
			char* buff = (char*)malloc(n + 1);
			n = vsnprintf(buff, n + 1, q, argptr);
			query = String(buff, n);
			free(buff);
		}
		va_end(argptr);

		return m_p->query(query.c_str()); 
	}

	String SQLDB::lastInsertRowId() { return m_p->lastInsertRowId(); }
	const char * SQLDB::AUTOINCREMENT() { if (!m_p) return nullptr; return m_p->AUTOINCREMENT(); }
	const char * SQLDB::NOW() { if (!m_p) return nullptr; return m_p->NOW(); }

	// SQLite
	
	void SQLite::open()
	{
		if (m_db != nullptr)
			return; // already opened
		
		int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
		if (m_readonly)
			flags |= SQLITE_OPEN_READONLY;
		
		int res = sqlite3_open_v2(m_dbName.c_str(), (sqlite3**)&m_db, flags, NULL);
		
		if (res != SQLITE_OK)
			throw SQLException(sqlite3_errmsg((sqlite3*)m_db));
	}
	
	void SQLite::close()
	{
		if (!m_db)
			return;
		
		int res = sqlite3_close((sqlite3*)m_db);
		m_db = nullptr;
			
		if (res != SQLITE_OK)
			throw SQLException(sqlite3_errmsg((sqlite3*)m_db));
	}
	
	SQLDB::Result SQLite::query(const char* q)
	{
		int res = SQLITE_OK;
		SQLDB::Result ret;
		
		sqlite3_stmt* stmt = nullptr;
		res = sqlite3_prepare_v2((sqlite3*)m_db, q, -1, &stmt, nullptr);
		
		if (res != SQLITE_OK)
			throw SQLException(sqlite3_errmsg((sqlite3*)m_db));
		
		int sr;
		do
		{
			sr = sqlite3_step(stmt);
			if (sr == SQLITE_ROW)
			{
				SQLDB::Row newRow;
				int nrOfRows = sqlite3_data_count(stmt);
				for (int i = 0; i < nrOfRows; ++i)
				{
					const char* name = sqlite3_column_name(stmt, i);
					const char* value = (const char*)sqlite3_column_text(stmt, i);
					if (value == NULL)
						newRow[name] = Variant::null;
					else
						newRow[name] = value;
				}
				ret.append(newRow);
			}
		}
		while (sr == SQLITE_ROW);
		
		sqlite3_finalize(stmt);
		return ret;
	}
	
	String SQLite::lastInsertRowId()
	{
		return std::to_string(sqlite3_last_insert_rowid((sqlite3*)m_db));
	}
	

	// MySQL

#ifdef JUCPP_MYSQL
	sql::MySQL::MySQL(String const& dbName, String const& host, String const& username, String const& password)
	{
		m_dbName = dbName;
		m_host = host;
		m_username = username;
		m_password = password;
	}

	SQLDB::Result sql::MySQL::query(const char * q)
	{
		return mysql::jucpp_mysql_query(m_db, q);
	}

	void sql::MySQL::open()
	{
		if (m_db != nullptr)
			return; // already opened

		m_db = mysql::jucpp_mysql_open(m_host.c_str(), m_username.c_str(), m_password.c_str(), m_dbName.c_str());
	}

	void sql::MySQL::close()
	{
		mysql::jucpp_mysql_close(m_db);
	}

	String sql::MySQL::lastInsertRowId()
	{
		return mysql::jucpp_mysql_last_inserted_row_id(m_db);
	}

#endif // JUCPP_MYSQL
	// namespace end
} }