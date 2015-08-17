//
//  rest.cpp
//  jucpp
//
//  Created by Igor Mladenovic on 05/08/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include "rest.h"
#include <regex>

namespace jucpp { namespace rest {
	
	int RestServer::EventHandler(jucpp::http::Request &req, jucpp::http::Response &res)
	{
		int ret = jucpp::http::Server::EventHandler(req, res);
		
		if (ret == res.ServeStaticFile())
			return ret;
		
		// write response
		auto it = m_get_functions.find(req.Url());
		
		if (it != m_get_functions.end())
		{
			(*it).second(req, res);
			return true;
		}
		

		// find in param
		for (const auto &f : m_get_functions)
		{
			std::regex regex_part(":[^\\/]*");
			if (std::regex_search(f.first, regex_part, std::regex_constants::match_any))
			{
				// TODO: escape other special characters
				// TODO: build regex_name and catche it when function is inserted
				
				std::string regex_name = std::regex_replace(f.first, std::regex("/"), "\\/");
				
				regex_name = std::regex_replace(regex_name, regex_part, "([^\\/]*?)");
				regex_name.append("$");
				
				std::smatch match_name;
				if (std::regex_match(req.Url(), match_name, std::regex(regex_name)))
				{
					std::sregex_iterator next(f.first.begin(), f.first.end(), regex_part);
					std::sregex_iterator end;
					size_t counter = 0;
					while (next != end)
					{
						if (counter >= match_name.size())
							break;
						std::smatch match = *next;
						String key = match.str().substr(1);
						String value = match_name[counter+1].str();
						req.setGet(key, value);
						next++;
						counter++;
					}
					f.second(req, res);
					return true;
				}
			}
		}
		return false; //TODO: wrong;
	}
	
	// namespace end
}}
