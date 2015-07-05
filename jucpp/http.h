#ifndef _JUCPP_HTTP_H_
#define _JUCPP_HTTP_H_

#include <jucpp/jucpp.h>

#include <string>
#include <functional>


namespace jucpp { namespace http {

	class Request
	{
	public:
		Request(void* connection);
		
		const char* HttpVersion() const { return m_httpVersion.c_str(); }
		const char* Headers(const char* name) const;
		const char* RawHeaders() const { return m_rawHeaders.c_str(); }
		const char* Method() const { return m_method.c_str(); }
		const char* Url() const { return m_url.c_str(); }
		
	private:
		StringStringMap m_headers;
		std::string m_rawHeaders;
		std::string m_httpVersion;
		std::string m_method; // GET, POST, PUT, etc..
		std::string m_url;
	};
	
	class Response
	{
	public:
		void writeHead(int code, const StringStringMap& headers) {}
		
		void write(const char* text) { m_output += text; }

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
