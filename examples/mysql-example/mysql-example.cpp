#include <jucpp/jucpp.h>
#include <jucpp/http.h>
#include <jucpp/sql.h>
#include <jucpp/session.h>

#include <thread>
#include <chrono>

using namespace jucpp;
using namespace jucpp::http;
using namespace jucpp::sql;

int main()
{
	int port = 8000;
	
	SQLDBSettings dbSettings(SQLDBSettings::MySQL, "devtest"); // Connect to local mysql DB devtest
	Session::SetSessionStorageSettings(dbSettings);	// Use mysql db for session storage

	Server()
	.OnStart([&port, &dbSettings]()
	{
		SQLDB db(dbSettings);
		if (db)
			db.query("CREATE TABLE IF NOT EXISTS `mytable` (id INTEGER UNIQUE PRIMARY KEY %s, data TEXT)", db.AUTOINCREMENT());
		else
			return false;

		printf("Server available on http://localhost:%d\n", port);
		return true;
	})
	.GET("/dbtest", [&dbSettings](const Request &req, Response &res)
	{
		SQLDB db(dbSettings);
		if (db)
		{
			// read values
			SQLDB::Result r = db.query("SELECT * FROM mytable");

			for (Json::Value const& row : r)
			{
				if (row.isObject())
					res.write(row);
			}
		}
		else
			res.write("Error");
		return Server::Proceeded;
	})
	.GET("/counter", [](const Request &req, Response &res)
	{
		Session s(req, res);
		Variant sessionData = s.get(); // Read data from session
		if (sessionData.isNull())
			sessionData = Object();
		if (!sessionData["counter"].isNumeric())
			sessionData["counter"] = 1;
		else
			sessionData["counter"] = sessionData["counter"].asInt() + 1;
		s.set(sessionData); // Write data into session

		res.write(sessionData["counter"]);
		return Server::Proceeded;
	})
    .listen(port);   // wait for connections
	
	return 0;
}