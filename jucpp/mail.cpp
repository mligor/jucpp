//
//  Mail.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 01/09/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include "mail.h"

#include <stdlib.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <curl/curl.h>

namespace jucpp { namespace mail {

	
	std::string random_string( size_t length )
	{
		auto randchar = []() -> char
		{
			const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
			const size_t max_index = (sizeof(charset) - 1);
			return charset[ rand() % max_index ];
		};
		srand((unsigned int)time(NULL));
		std::string str(length,0);
		std::generate_n( str.begin(), length, randchar );
		return str;
	}
	
	size_t Mail::s_readFunction(void *ptr, size_t size, size_t nmemb, void *userp)
	{
		return ((Mail*)userp)->BuildMessage(ptr, size, nmemb);
	}

	// Mail
	
	size_t Mail::BuildMessage(void *ptr, size_t size, size_t nmemb)
	{
		if((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
			return 0;
		
		const char* data = m_content.c_str();
		
		if(data)
		{
			size_t len = strlen(data);
			memcpy(ptr, data, len);
			m_content.clear();
			return len;
		}
		return 0;
	}
	
	bool Mail::send()
	{
		CURL *curl = curl_easy_init();
		CURLcode res = CURLE_OK;
		struct curl_slist *recipients = NULL;
		//struct upload_status upload_ctx;
		
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_USERNAME, m_config.username.c_str());
			curl_easy_setopt(curl, CURLOPT_PASSWORD, m_config.password.c_str());
			
			curl_easy_setopt(curl, CURLOPT_URL, String("smtps://" + m_config.server).c_str());
			
			curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
			
			// disable host verification
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			
			curl_easy_setopt(curl, CURLOPT_MAIL_FROM, String("<" + m_from + ">").c_str());
			
			recipients = curl_slist_append(recipients, String("<" + m_to + ">").c_str());
			
			curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
			
			String data;
			data.append("Subject: " + m_subject + "\r\n");
			data.append("From: \"" + m_from_name + "\" <" + m_from + ">\r\n");
			data.append("To: <" + m_to + ">\r\n");
			data.append("Message-Id: <" + random_string(20) + "@beanox.com> \r\n");
			
			std::time_t t = std::time(nullptr);
			std::tm tm = *std::localtime(&t);
			std::stringstream dateBuffer;
			dateBuffer << std::put_time(&tm, "%a, %d %b %Y %T %z");
			
			data.append("Date: " + dateBuffer.str() + "\r\n");
			
			data.append("\r\n");
			data.append(m_body);
			data.append("\r\n");
			
			m_content = data;
			
			
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, Mail::s_readFunction);
			curl_easy_setopt(curl, CURLOPT_READDATA, this);
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
			
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 4L);
			
			/* Send the message */
			res = curl_easy_perform(curl);
			
			/* Free the list of recipients */
			curl_slist_free_all(recipients);
			
			/* Always cleanup */
			curl_easy_cleanup(curl);
			
			 if(res == CURLE_OK)
				 return true;
		}
		
		return false;
	}
	
	
}} // namespace end
