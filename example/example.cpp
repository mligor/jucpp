#include <jucpp/jucpp.h>
#include <jucpp/http.h>

#include <thread>
#include <chrono>

using namespace jucpp;
using namespace jucpp::http;

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
	.GET("/wait", [](const Request &req, Response &res)
	 {
		 std::this_thread::sleep_for(std::chrono::seconds(15));
		 res.write(".. after 15 sec");
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
    .listen(port);   // wait for connections
	
	printf("Server done\n"); // should never come here
	
	return 0;
}