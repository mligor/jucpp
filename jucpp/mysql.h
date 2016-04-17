//
//  mysql.h
//  jucpp
//
//  Created by Igor Mladenovic on 17/04/16.
//  Copyright (c) 2016 BeanOX UG. All rights reserved.
//

#ifndef JUCPP_MYSQL_H
#define JUCPP_MYSQL_H

#include "sql.h"

#ifndef JUCPP_NO_MYSQL

namespace jucpp { namespace sql { namespace mysql {

	void* jucpp_mysql_open(const char* host, const char* username, const char* password, const char* dbName);
	void jucpp_mysql_close(void* db);
	SQLDB::Result jucpp_mysql_query(void* db, const char* query);
	String jucpp_mysql_last_inserted_row_id(void* db);


// namespace end
} } }

#endif // JUCPP_NO_MYSQL

#endif // JUCPP_MYSQL_H
