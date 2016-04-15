#include "sql.h"
#include <libs/sqlite/sqlite3.h>


namespace jucpp { namespace sql {

	// SQLDB
	
	SQLDB::SQLDB(DBType t, const char* dbName, bool readonly)
	{
		set(t, dbName, readonly);
	}
	
	SQLDB::~SQLDB()
	{
		if (m_p)
		{
			m_p->close();
			delete m_p, m_p = nullptr;
		}
	}

	void SQLDB::set(DBType t, const char* dbName, bool readonly)
	{
		if (m_p)
			throw SQLException("DB already initialized");
		
		switch (t)
		{
			case Unknown:
				throw SQLException("Unknown database type");
				break;
			case SQLite:
				m_p = new class SQLite(dbName, readonly);
				break;
			case MySQL:
				//m_p = new class SQLite(dbName, readonly);
				throw SQLException("MySQL not implemented");
				break;
		}
	}

	
	void SQLDB::open(const char* dbName, bool readonly) { m_p->open(dbName, readonly); }
	void SQLDB::close() { m_p->close(); }
	SQLDB::Result SQLDB::query(const char* q) { return m_p->query(q); }
	String SQLDB::lastInsertRowId() { return m_p->lastInsertRowId(); };

	
	// SQLite
	
	void SQLite::open(const char* dbName, bool readonly)
	{
		if (m_db != nullptr)
			return; // already opened
		
		int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
		if (readonly)
			flags |= SQLITE_OPEN_READONLY;
		
		int res = sqlite3_open_v2(dbName, (sqlite3**)&m_db, flags, NULL);
		
		if (res != SQLITE_OK)
			throw SQLException(sqlite3_errmsg((sqlite3*)m_db));
	}
	
	void SQLite::close()
	{
		if (!																																								m_db)
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
	
	
	// namespace end
}}