# jucpp - web development in C++


__jucpp__ is Web Development Framework written in C++, to enable writing web services or even complete web sites in C++. 
__jucpp__ intentionally similar to node.js. It shuld be compilable and should work on Windows, Mac OS and Linux (Visual Studio, XCode and gcc).

__jucpp__ is using C++11 features and it requires C++11 compiler. It is still in early development and if you are interested to join development, please contact me.

## Why C++? 

Today there are many different programing languages used for Web Development: PHP, NodeJS (JavaScript), Ruby, Perl, Java, Python, but none of them generate native code (none of them use real compiler). On another side C++ does. 
Having native code increase performance and memory usage – therefore your Web Services/Web Page will optimally use your server and will able to handle much more clients with lower resource usage.

It is clear that if you have powerful web server (e.g. XEON with 16GB RAM) and you need REST server to handle few thousand requests per day, you don’t need C++ at all – You can use Java, PHP or whatever you want other language. But if you have Raspberry Pi or other embedded system I’m pretty sure you don’t want to install Java or run Tomcat on it.

We have our build server where we are running 4 Tomcat services (Jira, Bamboo, Stash and Confluence) and each of them needs around 1GB of RAM and it handles requests very slow. The same tasks, when server is writing in C++, would work much better with much lower hardware.

For example: Current version of jucpp that supports Sqlite, Json serialisation and HTTP server (based on mongoose) has binary size of 800KB-2MB (depends of platform, without any external dependency - everything is IN the binary itself). Required RAM is around 2-3MB when using SQLite DB. I'm sure RAM and size will increase when using MySQL or Mongo, but not as much as when you use Java or PHP (at the end Java and PHP are just wrappers for DB acces that will use C/C++ Libraries).

##Libraries and Portability

It is not everything about resources – there is something about available software libraries and portability. C and C++ are most portable languages – and number of libraries that you can find for C/C++ is extremely high. C++ code must be compiled but C++ compilers exists for almost every platform that exists on the planet (PC, Mac, Linux, Android, iOS, …). Every of those platforms are written itself in C or C++ and there are mass of libraries available – database access, Json, XML, Sockets, TCP/IP, HTTP Server, controlling IO Ports, working with serial/parallel ports, regular expressions, encryption, test frameworks, controling various devices, etc…

With this project I will try to make C++11 wrappers for avarage web service use - various Database Supports, CURL, XML, RegExp, HTML parsing etc.. And using external libraries will be much easier then with other languages - just #include <mylib> and go.

##Security

C++ is one of the most secure languages on the world. Like I wrote before all OS are written in C/C++ and as much your OS is secure, your web service will be. Anyway, if you write unsecure code, there is no programming language that will help you to make your services secure.

##Sell your services or Open Source

I like Open Source projects - I think you can still sell your work even with open source, but there are many companies that do not prefere that way - using __jucpp__ you can distribute your work in binary format - without giving source to your clients (needs to be carefull with GPL license anyway). This is not my intension with this project, but I'm sure this will be much better possible then with PHP or JavaScript.

## Simple Web Service example
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
