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
	
	printf("Server available on http://localhost:%d\n", port);
	
	Server()
	.setDocumentRoot(".") // Allow to ServeStaticFile
	.GET("/favicon.ico", [](const Request &req, Response &res)
	 {
		 res.addHeader("Content-Type", "image/x-icon");
		 res.write("");
		 return Server::Proceeded;
	 })
	.GET("/dbtest", [](const Request &req, Response &res)
	{
		SQLDB db(SQLDB::SQLite, "test.sqlite");
		if (db)
			res.write("OK");
		else
			res.write("Error");
		return Server::Proceeded;
	 })
	.GET("/test", [](const Request &req, Response &res)
	 {
		 res.write("Test OK");
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
	.GET("/wait", [](const Request &req, Response &res)
	 {
		 std::this_thread::sleep_for(std::chrono::seconds(30));
		 res.write(".. after 30 sec");
		 return Server::Proceeded;
	 })
	.GET("*", [](const Request &req, Response &res)
	 {
		 String url = req.Url();
		 if (url == "/example.cpp")
			 return Server::ServeStaticFile; // jucpp will try to serve example.cpp as a static file
		 
		 Object data;
		 data["url"] = url;
		 data["language"] = req.Header("Accept-Language");
		 res.write(data);
		 
		 return Server::Proceeded; // inform jucpp framework that request is processed
	 })
	.listen(port)   // set port
	.wait();        // wait for connections
	
	printf("Server done\n"); // should never come here
	
	return 0;
}