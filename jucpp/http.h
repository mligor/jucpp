#ifndef _JUCPP_HTTP_H_
#define _JUCPP_HTTP_H_

#include <jucpp/jucpp.h>

#include <string>

namespace jucpp { namespace http {

	class Request
	{
	};
	
	class Response
	{
	public:
		void writeHead(int code, const char* header) {}
		void end(std::string text) { m_output += text; }

	public: //TODO: make it private
		std::string m_output;
	};
	
	class Server
	{
	public:
		typedef void (*ServerFn)(const Request &req, Response &res);

		Server(ServerFn fn) : m_fn(fn) {}
		
		Job listen(int port);

	private:
		friend int s_Server_EventHandler(void*, int);
		int EventHandler(const Request &req, Response &res);

		ServerFn m_fn;
	};

	class Http
	{
	public:
		Http() 
		{
			
		}
		
		Server createServer(Server::ServerFn fn)
		{
			Server serv(fn);
			return serv;
		}
		
		std::wstring getInfo()
		{
			return L"this is info";
		}
	};
	
// namespace end
}}

#endif // _JUCPP_HTTP_H_
