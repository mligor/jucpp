//
//  Mail.h
//  jucpp
//
//  Created by Igor Mladenovic on 01/09/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#ifndef __jucpp__mail__
#define __jucpp__mail__

#include <jucpp/jucpp.h>

namespace jucpp { namespace mail {

	class MailConfiguration
	{
	public:
		String server;
		String username;
		String password;
		
	};
	
	class Mail
	{
	public:
		
		Mail(MailConfiguration &&c) { m_config = std::move(c); }
		Mail(MailConfiguration c) : m_config(c) {}
		
		Mail& to(String s) { m_to = s; return *this; }
		Mail& from(String s) { m_from = s; return *this; }
		Mail& from_name(String s) { m_from_name = s; return *this; }
		Mail& subject(String s) { m_subject = s; return *this; }
		Mail& body(String s) { m_body = s; return *this; }
		
		bool send();
		
	protected:
		static size_t s_readFunction(void *ptr, size_t size, size_t nmemb, void *userp);
		virtual size_t BuildMessage(void *ptr, size_t size, size_t nmemb);
		
	private:
		String m_to;
		String m_from;
		String m_from_name;
		String m_subject;
		String m_body;
		
		String m_content; // generated
		MailConfiguration m_config;
	};
	
	
}} // namespace end

#endif /* defined(__jucpp__mail__) */
