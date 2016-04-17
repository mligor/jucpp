#ifndef _JUCPP_HTTP_H_
#define _JUCPP_HTTP_H_

#include "jucpp.h"

#include "base64.h"

// STL
#include <string>
#include <functional>
#include <memory>

#ifdef _DEBUG
#define DEFAULT_LOG_LEVEL LogDebug
#else
#define DEFAULT_LOG_LEVEL LogError
#endif 


namespace jucpp { namespace http {

	/**
	 HTTP Cookie object
	 */
	//TODO: currently only session cookies are supported
	// Other cookie properties should be added : path, domain, expiration, secure, ...
	class Cookie
	{
	public:
		Cookie() {}
		Cookie(String text) { parse(text); }
		Cookie(String name, String value) : m_name(name), m_value(value) { }
		const String& Name() const { return m_name; }
		const String& Value() const { return m_value; }
		static const Cookie InvalidCookie;
	private:
		void parse(String text);
		String m_name;
		String m_value;
	};
	
	using StringCookieMap = std::map<String, Cookie>;

	/**
	 HTTP Request object
	 */
	class Request
	{
	public:
		Request(void* msg);
		
		//const String& HttpVersion() const { return m_httpVersion; }
		const String& Header(const String& name) const;
		const StringStringMap& Header() const { return m_headers; };
		const String& RawHeaders() const { return m_rawHeaders; }
		const String& Content() const { return m_content; }
		const String& Method() const { return m_method; }
		const String& Url() const { return m_url; }
		const String& QueryString() const { return m_queryString; }

        String Get(const String& name) const;
        const String& PathParam(const String& name) const;
        const Cookie& getCookie(const String& name) const;
		
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
		
		void setPathParam(const String& name, const String& value) { m_pathParams[name] = value; }
	private:
		StringStringMap m_headers;
        StringStringMap m_pathParams; // parameter from the named path (e.g. /feed/:name -> m_pathParams["name"])
		StringCookieMap m_cookies;
		String m_rawHeaders;
		//String m_httpVersion;
		String m_method; // GET, POST, PUT, etc..
		String m_url;
		String m_queryString;
		String m_content;
		
	private:
		mutable Variant m_jsonContent; // dont't use it directly, always use Data()
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
		void write_styled(const Json::Value& v);

		const String& getContent() { return m_output; }
		const int getStatus() const { return m_status; }
		const StringStringMap& getHeaders() const { return m_headers; }
        const StringCookieMap& getCookies() const { return m_cookies; }
		
		/// Set response status code
		void setStatus(int status, const String& statusText = String{}) { m_status = status; m_statusText = statusText; }
		
		/// Add custom header to the response
		void addHeader(const String& name, const String& value) { m_headers[name] = value; }
        void addCookie(const Cookie& cookie) { m_cookies[cookie.Name()] = cookie; }
        
        void redirect(String url, int status = 302);
		
	protected:
		void writeHead(int status, const StringStringMap& headers) { m_headers = headers; m_status = status; }

	private:
		String m_output;
		StringStringMap m_headers;
        StringCookieMap m_cookies;
		int m_status = 200;
		String m_statusText;
	};
	
	
	/**
	 HTTP Server object
	 */
    class Server : public ThreadJob
	{
	public:
		enum ResponseStatus { Skipped = 0, Proceeded = 1, ServeStaticFile = 2 };
		
		using Fn = std::function<ResponseStatus (const Request &req, Response &res)>;
		using OnStartFn = std::function<bool()>;
		
		using FnList = std::vector<std::pair<String, Fn>>;
		using FnListMap = std::map<String, FnList>;
		
	public:
        Server();
        ~Server();
		Server& listen(int port);
        
        // ThreadJob
        virtual void Execute() override;
        
        Server& setDocumentRoot(String documentRoot) { m_documentRoot = documentRoot; return *this; }

		static void addCORSHeaders(const Request& req, Response& res);
        static void addCommonHeaders(const Request& req, Response& res);
		
		/***
		 * Valid patterns:
		   "/myurl"     - exackt match
		   "/user/:id"  - match url with parameter (e.g. /user/333, /user/tom)
		   "/api/ *"     - match all urls that starts with /api/ (wildcard must be at the end)
		   "/api/!*"    - negative start match - match all that do not start with /api/
	    */
		Server& GET(String cp, Fn fn) { m_functions["GET"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& POST(String cp, Fn fn) { m_functions["POST"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& PUT(String cp, Fn fn) { m_functions["PUT"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& DELETE(String cp, Fn fn) { m_functions["DELETE"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& OPTIONS(String cp, Fn fn) { m_functions["OPTIONS"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }
		Server& HEAD(String cp, Fn fn) { m_functions["HEAD"].push_back(std::pair<String, Fn>(cp,fn)); return *this; }

		Server& OnStart(OnStartFn fn) { m_onStart = fn; return *this; };
        
    public:
        static String jsonEncode(const Variant &v);
        static Variant jsonDecode(const String &s, bool *error = nullptr);
        
	protected:
		virtual ResponseStatus EventHandler(Request &req, Response &res);

	public:
		/*
			Fatal	Highest level: important stuff down
			Error	For example application crashes / exceptions.
			Warn	Incorrect behavior but the application can continue
			Info	Normal behavior like mail sent, user updated profile etc.
			Debug	Executed queries, user authenticated, session expired
			Trace	Begin method X, end method X etc
		*/
		enum LogLevel { LogTrace, LogDebug, LogInfo, LogWarn, LogError, LogFatal };
		void Log(LogLevel l, const char* fmt, ...);
		Server& setLogLevel(LogLevel l) { m_logLevel = l; return *this; }
        
        void* GetMongoosePtr() {return m_mongoose;}

	private:
		static void _MongooseEventHandler(void*, int, void*);
        void MongooseEventHandler(void*, int, void*);
        
    private:
        void send_status_result(void *conn, int status, const char *msg, const char* text = nullptr);
        
        void* m_mongoose = nullptr;
		String m_documentRoot;
		FnListMap m_functions;
		OnStartFn m_onStart;
		LogLevel m_logLevel = DEFAULT_LOG_LEVEL;
	};
	
	#define BIND(method, uri, function) method(uri, std::bind(function, this, std::placeholders::_1, std::placeholders::_2))
	
	#define BIND_GET(uri, function) BIND(GET, uri, function)
	#define BIND_POST(uri, function) BIND(POST, uri, function)
	#define BIND_PUT(uri, function) BIND(PUT, uri, function)
	#define BIND_DELETE(uri, function) BIND(DELETE, uri, function)
	#define BIND_OPTIONS(uri, function) BIND(OPTIONS, uri, function)
	#define BIND_HEAD(uri, function) BIND(HEAD, uri, function)
	
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
