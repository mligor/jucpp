//
//  sqlite.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 29/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include "sqlite.h"
#include <libs/sqlite/sqlite3.h>


namespace jucpp { namespace sqlite {

	void SQLite::open(const String& dbName, bool readonly)
	{
		if (m_db != nullptr)
			return; // already opened
		
		int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
		if (readonly)
			flags |= SQLITE_OPEN_READONLY;
		
		int res = sqlite3_open_v2(dbName.c_str(), (sqlite3**)&m_db, flags, NULL);
		
		if (res != SQLITE_OK)
			throw SQLiteException(sqlite3_errmsg((sqlite3*)m_db));
	}
	
	void SQLite::close()
	{
		if (!m_db)
			return;
		
		int res = sqlite3_close((sqlite3*)m_db);
		m_db = nullptr;
			
		if (res != SQLITE_OK)
			throw SQLiteException(sqlite3_errmsg((sqlite3*)m_db));
	}
	
	SQLite::Result SQLite::query(const char* q)
	{
		int res = SQLITE_OK;
		Result ret;
		
		sqlite3_stmt* stmt = nullptr;
		res = sqlite3_prepare_v2((sqlite3*)m_db, q, -1, &stmt, nullptr);
		
		if (res != SQLITE_OK)
			throw SQLiteException(sqlite3_errmsg((sqlite3*)m_db));
		
		int sr;
		do
		{
			sr = sqlite3_step(stmt);
			if (sr == SQLITE_ROW)
			{
				Row newRow;
				int nrOfRows = sqlite3_data_count(stmt);
				for (int i = 0; i < nrOfRows; ++i)
				{
					const char* name = sqlite3_column_name(stmt, i);
					const char* value = (const char*)sqlite3_column_text(stmt, i);
					newRow[name] = value;
				}
				
				ret.append(newRow);
			}
		}
		while (sr == SQLITE_ROW);
		
		sqlite3_finalize(stmt);
		return ret;
	}
	
	String SQLite::getLastInsertRowId()
	{
		return std::to_string(sqlite3_last_insert_rowid((sqlite3*)m_db));
	}
	
	
	// namespace end
}}