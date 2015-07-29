#ifndef _JUCPP_H_
#define _JUCPP_H_

#include <stdio.h>

#include <map>
#include <string>

#include <json/json.h>


namespace jucpp
{
	
	class String : public std::string
	{
	public:
		String() {}
		String(const char* chrValue, size_type len) : std::string(chrValue, len) {}
		String(const char* chrVal) : std::string(chrVal) {}
		
		static const String EmptyString;
	};
	
	using Variant = Json::Value;
	
	extern const Variant EmptyVariant;

	class Object : public Json::Value
	{
	public:
		Object() : Json::Value(Json::ValueType::objectValue) {}
		
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
		virtual void run();
		virtual void wait();
		virtual void stop();
		
		virtual bool stopThread() { return false; }

	private:
		void* m_thread;
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
