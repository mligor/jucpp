#ifndef _JUCPP_HTTP_H_
#define _JUCPP_HTTP_H_

#include "jucpp.h"

#include "base64.h"

// STL
#include <string>
#include <functional>
#include <memory>


namespace jucpp { namespace http {

	/**
	 HTTP Request object
	 */
	class Request
	{
	public:
		Request(void* connection);
		
		const String& HttpVersion() const { return m_httpVersion; }
		const String& Header(const String& name) const;
		const StringStringMap& Header() const { return m_headers; };
		const String& Get(const String& name) const;
		const StringStringMap& Get() const { return m_get; };
		const String& RawHeaders() const { return m_rawHeaders; }
		const String& Content() const { return m_content; }
		const String& Method() const { return m_method; }
		const String& Url() const { return m_url; }
		const String& QueryString() const { return m_queryString; }
		
		const Variant& Data() const { return m_jsonContent; }
		const Variant& Data(const char* key) const
		{
			if (m_jsonContent.isObject())
				return m_jsonContent[key];
			return EmptyVariant;
		}
		const Variant& Data(const String& key) const { return Data(key.c_str()); }
		const Variant& DataArray(unsigned int idx) const
		{
			if (m_jsonContent.isArray())
				return m_jsonContent[idx];
			return EmptyVariant;
		}
		
		void setGet(const String& name, const String& value) { m_get[name] = value; }
	private:
		//const Variant& ContentAsJson() const;

		StringStringMap m_headers;
		StringStringMap m_get; // GET parameters
		
		String m_rawHeaders;
		String m_httpVersion;
		String m_method; // GET, POST, PUT, etc..
		String m_url;
		String m_queryString;
		
		String m_content;
		
	private:
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
        
        void redirect(String url, int status = 302);
		
	protected:
		void writeHead(int status, const StringStringMap& headers) { m_headers = headers; m_status = status; }

	private:
		String m_output;
		StringStringMap m_headers;
		int m_status = 200;
		String m_statusText;
	};
	
	
	/**
	 HTTP Server object
	 */
	class Server
	{
	public:
		enum ResponseStatus { Skipped = 0, Proceeded = 1, ServeStaticFile = 2 };
		
		using Fn = std::function<ResponseStatus (const Request &req, Response &res)>;
		
		using FnList = std::vector<std::pair<String, Fn>>;
		using FnListMap = std::map<String, FnList>;
		
	public:
		Server() {}
		Job listen(int port);
		Server& setDocumentRoot(String documentRoot) { m_documentRoot = documentRoot; return *this; }
		
		void addCORSHeaders(const Request& req, Response& res);
		
		Server& GET(String cp, Fn fn) { m_functions["GET"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& POST(String cp, Fn fn) { m_functions["POST"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& PUT(String cp, Fn fn) { m_functions["PUT"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& DELETE(String cp, Fn fn) { m_functions["DELETE"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& OPTIONS(String cp, Fn fn) { m_functions["OPTIONS"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& HEAD(String cp, Fn fn) { m_functions["HEAD"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
        
    public:
        static String jsonEncode(const Variant &v);
        static Variant jsonDecode(const String &s, bool *error = nullptr);
        
	protected:
		virtual ResponseStatus EventHandler(Request &req, Response &res);

	private:
		friend int s_Server_EventHandler(void*, int);

		String m_documentRoot;
		FnListMap m_functions;
	};
	
	#define BIND(method, uri, function) method(uri, std::bind(function, this, std::placeholders::_1, std::placeholders::_2))
	
	#define BIND_GET(uri, function) BIND(GET, uri, function)
	#define BIND_POST(uri, function) BIND(POST, uri, function)
	#define BIND_PUT(uri, function) BIND(PUT, uri, function)
	#define BIND_DELETE(uri, function) BIND(DELETE, uri, function)
	#define BIND_OPTIONS(uri, function) BIND(OPTIONS, uri, function)
	#define BIND_HEAD(uri, function) BIND(HEAD, uri, function)
	
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


	class Auth
	{
	public:
		virtual bool isAuthenticated() = 0;
		virtual String getUsername() = 0;
		virtual String getPassword() = 0;
		virtual void requestAuthentication(Response& res, String realm) = 0;
		void deny(Response& res)
		{
			res.setStatus(401, "Not Authorized");
			res.write("401 Not Authorized");
		}
	};
	
	using AuthPtr = std::unique_ptr<Auth>;
	
	/**
	 * Basic authentication
	 */
	class BasicAuth : public Auth
	{
	public:
		BasicAuth(const Request& req) : m_req(req)
		{
			String authHeader = m_req.Header("Authorization");
			
			if (authHeader.substr(0, 6) == "Basic ")
			{
				authHeader = authHeader.substr(6);
				
				int len = Base64decode_len(authHeader.c_str());
				String data;
				data.resize(len);
				
				int bytesDecoded = Base64decode(&data[0], authHeader.c_str());
				
				if (bytesDecoded)
				{
					data.resize(bytesDecoded);
					m_decodedData = data;
				}
			}
		}
		
		virtual bool isAuthenticated() override { return !m_decodedData.empty(); }
		
		virtual String getUsername() override
		{
			if (m_decodedData.empty())
				return "";
			
			size_t pos = m_decodedData.find(':');
			if (pos == m_decodedData.npos)
				return "";
			
			return m_decodedData.substr(0, pos);
		}
		
		virtual String getPassword() override
		{
			if (m_decodedData.empty())
				return "";
			
			size_t pos = m_decodedData.find(':');
			if (pos == m_decodedData.npos)
				return "";
			
			return m_decodedData.substr(pos+1);
		}
		
		virtual void requestAuthentication(Response& res, String realm) override
		{
			res.addHeader("WWW-Authenticate", "Basic realm=\"" + realm + "\"");
			deny(res);
		}
		
	private:
		const Request& m_req;
		String m_decodedData;
	};
	
// namespace end
}}

#endif // _JUCPP_HTTP_H_
