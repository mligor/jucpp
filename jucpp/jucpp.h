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

#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG DEBUG
#endif

namespace jucpp
{
	using StringList = std::vector<class String>;
	class String : public std::string
	{
	public:
		String() {}
		String(const char* chrValue, size_type len) : std::string(chrValue, len) {}
		String(const char* chrVal) : std::string(chrVal) {}
		String(const std::string& str) : std::string(str) {}

		static const String EmptyString;
	
		StringList split(const String delimiter, int nMax = -1) const;
		String lowercase() const;
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
	
	/*
	 * Base Job class
	 */
	class Job
	{
	public:
		// Virtual methods to implemented
		virtual void Execute() = 0;
		virtual void OnFinish() {};

	protected:
		friend class JobPtr;
		virtual void run() = 0;
		virtual void wait() = 0;
		virtual void stop() = 0;
		virtual void terminate() = 0;
	};
	
	class ThreadJob : public Job
	{
	public:
		~ThreadJob();
		
	protected:
		virtual void run() override;
		virtual void wait() override;
		virtual void stop() override;
		virtual void terminate() override;
		
		bool shouldStop() const { return m_bStop; }
	public:
		virtual bool isMyThread();

	private:
		void* m_thread = nullptr;
		volatile bool m_bStop = false;
	};
	
	class JobPtr
	{
	public:
		JobPtr(Job* pJob) : m_pJob(pJob) {};
		
		void run()	{ m_pJob->run(); }
		void wait() { m_pJob->wait(); }
		void stop() { m_pJob->stop(); }
		void terminate() { m_pJob->terminate(); }
		
		Job* operator->() { return m_pJob; }
		Job* getJob() { return m_pJob; }
		
	private:
		Job* m_pJob;
	};
	
	using JobPtrList = std::vector<JobPtr>;
}

#endif // _JUCPP_H_
