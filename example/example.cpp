#include <jucpp/jucpp.h>
#include <jucpp/http.h>

using namespace jucpp;
using namespace jucpp::http;

int main()
{
	// Global GET handler - will catch all GET requests
	Server()
	.setDocumentRoot(".") // Allow to ServerStaticFile		
	.GET("*", [](const Request &req, Response &res)
	{
		String url = req.Url();
		
		if (url == "/favicon.ico")
			return Server::Skipped; // allow GET /favicon.ico handler to response
		else if (url == "/example.cpp")
			return Server::ServerStaticFile; // jucpp will try to server main.cpp as static file
		
		Object data;
		data["url"] = url;
		data["language"] = req.Header("Accept-Language");
		res.write(data);
		
		return Server::Proceeded; // inform jucpp framework that request is processed
	})
	
	// special GET handler, will catch only specific URL
	.GET("/favicon.ico", [](const Request &req, Response &res)
	 {
		 res.write("no icon today");
		 return Server::Proceeded;
	 })
	
	.listen(8000)   // listen on port 8000
	.wait();        // wait for connections
	
	return 0;
}