#ifndef _JUCPP_HTTP_H_
#define _JUCPP_HTTP_H_

#include <jucpp/jucpp.h>

#include <string>
#include <functional>


namespace jucpp { namespace http {

	class Request
	{
	};
	
	class Response
	{
	public:
		void writeHead(int code, const char* header) {}
		void end(std::string text) { m_output += text; }
		
		const std::string& getContent() { return m_output; }

	private: //TODO: make it private
		std::string m_output;
	};
	
	class Server
	{
	public:
		typedef std::function<void (const Request &req, Response &res)> ServerFn;

		Server(ServerFn fn) : m_fn(fn) {}
		
		Job listen(int port);

	private:
		friend int s_Server_EventHandler(void*, int);
		int EventHandler(const Request &req, Response &res);

		ServerFn m_fn;
	};
	
	class ServerJob : public ThreadJob
	{
	public:
		ServerJob(void* server) : m_server(server), m_bStop(false) {}
		
	protected:
		// Job virtual functions
		virtual void Execute();
		virtual void OnFinish();
		
		virtual bool stopThread() { m_bStop = true; return true; }
	
	private:
		void* m_server;
		volatile bool m_bStop;
	};

	class Http
	{
	public:
		Http() 
		{
			
		}
		
		static Server createServer(Server::ServerFn fn)
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
