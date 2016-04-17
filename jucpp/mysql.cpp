#include "mysql.h"

#ifndef JUCPP_NO_MYSQL

#include "mysql\include\mysql.h"

namespace jucpp { namespace sql { namespace mysql {

	void * mysql::jucpp_mysql_open(const char * host, const char * username, const char * password, const char * dbName)
	{
		MYSQL* con = mysql_init(NULL);

		if (!con)
			throw jucpp::sql::SQLException(mysql_error(con));

		MYSQL* p = mysql_real_connect(con, host, username, password, dbName, 3306, nullptr, 0);

		if (!p)
		{
			mysql_close(p);
			throw jucpp::sql::SQLException(mysql_error(con));
		}
		return con;
	}

	void jucpp_mysql_close(void * db)
	{
		mysql_close((MYSQL*)db);
	}

	SQLDB::Result jucpp_mysql_query(void * db, const char * query)
	{
		if (mysql_query((MYSQL*)db, query))
			throw jucpp::sql::SQLException(mysql_error((MYSQL*)db));

		MYSQL_RES *result = mysql_store_result((MYSQL*)db);

		SQLDB::Result ret;
		if (!result)
			return ret;

		int num_fields = mysql_num_fields(result);

		jucpp::Array fields;

		if (num_fields)
		{
			MYSQL_FIELD *field;
			while (field = mysql_fetch_field(result))
				fields.append(String(field->name, field->name_length));
		}

		MYSQL_ROW row;
		while ((row = mysql_fetch_row(result)))
		{
			jucpp::Object obj;
			for (int i = 0; i < num_fields; i++)
				obj[fields[(unsigned int)i].asString().c_str()] = row[i];
			ret.append(obj);
		}

		mysql_free_result(result);
		return ret;
	}

	String jucpp_mysql_last_inserted_row_id(void * db)
	{
		return std::to_string(mysql_insert_id((MYSQL*)db));
	}

// namespace end
} } }

#endif // JUCPP_NO_MYSQL