#ifndef _JUCPP_H_
#define _JUCPP_H_

// json
#include "../libs/json/value.h" //TODO: remove this dependency from header file (implement own Value/Variant type)

// STL
#include <stdio.h>
#include <map>
#include <string>

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

namespace jucpp
{
	
	class String : public std::string
	{
	public:
		String() {}
		String(const char* chrValue, size_type len) : std::string(chrValue, len) {}
		String(const char* chrVal) : std::string(chrVal) {}
		String(const std::string& str) : std::string(str) {}
		
		static const String EmptyString;
	};

	using Variant = Json::Value;
	
	extern const Variant EmptyVariant;

	class Object : public Json::Value
	{
	public:
		Object() : Json::Value(Json::ValueType::objectValue) {}
	};
	
	class Exception : public std::exception
	{
	private:
		String m_error;
	public:
		Exception(const char* error) : m_error(error) { };
		virtual const char* what() const NOEXCEPT override { return m_error.c_str(); }
	};
	
	class Array : public Json::Value
	{
	public:
		Array() : Json::Value(Json::ValueType::arrayValue) {}
		Array(std::initializer_list<Json::Value> data)
		{
			for (auto it = data.begin(); it != data.end(); it++)
				append((*it));
		}
		
		Json::Value& operator[](unsigned int idx)
		{
			return Json::Value::operator[](idx);
		}
		Json::Value& operator[]( const char *key ) = delete;
	};
	
	class StringStringMap : public std::map<String, String>
	{
		
	};
	
	class JobBase
	{
	public:
		// Virtual methods to implemented
		virtual void Execute() = 0;
		virtual void OnFinish() {};
		
	protected:
		friend class Job;
		virtual void run() = 0;
		virtual void wait() = 0;
		virtual void stop() = 0;
	};
	
	class ThreadJob : public JobBase
	{
	public:
		~ThreadJob();
	protected:
		friend void s_Job_ThreadFn(void* p);
		
	protected:
		virtual void run() override;
		virtual void wait() override;
		virtual void stop() override;
		
		virtual bool stopThread() { return false; }

	private:
		void* m_thread = nullptr;
	};
	
	class Job
	{
	public:
		Job(JobBase* pJob) : m_pJob(pJob) {};
		
		void run()	{ m_pJob->run(); }
		void wait() { m_pJob->wait(); }
		void stop() { m_pJob->stop(); }

	private:
		JobBase* m_pJob;
	};
}

#endif // _JUCPP_H_
