# jucpp - web development in C++


__jucpp__ is Web Development Framework written in C++, to enable writing web services or even complete web sites in C++. 
__jucpp__ intentionally similar to node.js. It shuld be compilable and should work on Windows, Mac OS and Linux (Visual Studio, XCode and gcc).

__jucpp__ is using C++11 features and it requires C++11 compiler. It is still in early development and if you are interested to join development, please contact me.



## Example
```c++
#include <jucpp/jucpp.h>
#include <jucpp/http.h>
#include <jucpp/sqlite.h>

using namespace jucpp;
using namespace jucpp::http;
using namespace jucpp::sqlite;

int main()
{
	// Server decalaration
	Server server = Http::createServer([](const Request &req, Response &res)
		 {
			 res.addHeader("Content-Type", "application/json");
			 res.setStatus(200);

			 Object data;
			 data["request_url"] = req.Url();
			 data["request_language"] = req.Headers("Accept-Language");
			 data["request_content"] = req.ContentAsJson();
			 
			 // access DB
			 SQLite db;
			 try
			 {
				 db.open("myfile.db");
				 SQLite::Result dbRes = db.query("SELECT * FROM test");
				 data["data_from_db"] = dbRes;
				 db.close();
			 }
			 catch (SQLiteException& ex)
			 {
				 data["error"] = "SQLite Exception : " + ex.description();
			 }
			 res.write(data);
		 });

	// listen on port 8000
	Job serverJob = server.listen(8000);
	
	// output on server (can be redirected to log file)
	printf("Server is running at http://127.0.0.1:8000/\n");
	
	// wait forever	
	serverJob.wait();
	return 0;
}
```
