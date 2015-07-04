#include <webcpp.h>
#include <webcpp/http.h>

using namespace webcpp;
using namespace webcpp::http;

int main()
{
	Http http_server;
	
	Server server = http_server.createServer([](Request req, Response res)
	{
		//http_server.getInfo();
		
		res.writeHead(200, L"Content-Type: text/plain");
		res.end(L"Hello from WebCpp\n");
		printf("Request arrived\n");
	});
	
	Job serverJob = server.listen(8000);
	
	printf("Server is running at http://127.0.0.1:8000/\n");
	
	serverJob.wait();
	return 0;
}

