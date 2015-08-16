#ifndef _JUCPP_HTTP_H_
#define _JUCPP_HTTP_H_

#include "jucpp.h"

// STL
#include <string>
#include <functional>


namespace jucpp { namespace http {

	/**
	 HTTP Request object
	 */
	class Request
	{
	public:
		Request(void* connection);
		
		const String& HttpVersion() const { return m_httpVersion; }
		const String& Headers(const String& name) const;
		const String& RawHeaders() const { return m_rawHeaders; }
		const String& Content() const { return m_content; }
		const String& Method() const { return m_method; }
		const String& Url() const { return m_url; }
		const String& QueryString() const { return m_queryString; }
		
		const Variant& Data() const { return m_jsonContent; }
		const Variant& Data(const char* key) const
		{
			if (m_jsonContent.isObject())
				m_jsonContent[key];
			return EmptyVariant;
		}
		const Variant& Data(const String& key) const { return Data(key.c_str()); }
		const Variant& DataArray(unsigned int idx) const
		{
			if (m_jsonContent.isArray())
				return m_jsonContent[idx];
			return EmptyVariant;
		}
		
	private:
		//const Variant& ContentAsJson() const;

		StringStringMap m_headers;
		String m_rawHeaders;
		String m_httpVersion;
		String m_method; // GET, POST, PUT, etc..
		String m_url;
		String m_queryString;
		
		String m_content;
		
	private:
		bool m_jsonContentParsed = false;
		mutable Variant m_jsonContent; // dont't use it directly, always use ContetAsJson() or Data()
	};
	
	/**
	 HTTP Response Object @see Http::createServer
	 */
	class Response
	{
	public:
		Response() {}
		
		void write(const char* text) { if (text) m_output += text; }
		void write(const Json::Value& v);

		const String& getContent() { return m_output; }
		const int getStatus() const { return m_status; }
		const StringStringMap& getHeaders() const { return m_headers; }
		
		/// Set response status code
		void setStatus(int status, const String& statusText = String{}) { m_status = status; m_statusText = statusText; }
		
		/// Add custom header to the response
		void addHeader(const String& name, const String& value) { m_headers[name] = value; }
		
		void ServeStaticFile(bool serverStaticFiles) { m_serveStaticFile = serverStaticFiles; }
		bool ServeStaticFile() const { return m_serveStaticFile; }
		

	protected:
		void writeHead(int status, const StringStringMap& headers) { m_headers = headers; m_status = status; }

	private:
		String m_output;
		StringStringMap m_headers;
		int m_status = 200;
		String m_statusText;
		bool m_serveStaticFile = false;
	};
	
	
	/**
	 HTTP Server object
	 */
	class Server
	{
	public:
		typedef std::function<void (const Request &req, Response &res)> ServerFn;

		Server(ServerFn fn, const String& documentRoot = String::EmptyString) : m_fn{fn}, m_documentRoot{documentRoot} {}
		
		Job listen(int port);

	protected:
		virtual int EventHandler(const Request &req, Response &res);

	private:
		friend int s_Server_EventHandler(void*, int);

		ServerFn m_fn;
		String m_documentRoot;
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
		/**
		 Creates server object with given Server Event funciton @fn
		 
		 Example:
		 @code
Server server = Http::createServer([](const Request &req, Response &res)
{
	res.addHeader("Content-Type", "application/json");
	res.setStatus(200);

	Object data;
	data["text"] = "Hallo world";

	res.write(data);
});
		 @endcode
		 @param fn 
			Server event function that will be called on every request
		 @return @c Server object @see Server
		 
		 */
		static Server createServer(Server::ServerFn fn, const String& documentRoot = String::EmptyString)
		{
			Server serv(fn, documentRoot);
			return serv;
		}
	};
	
// namespace end
}}

#endif // _JUCPP_HTTP_H_
