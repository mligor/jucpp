//
//  angular.h
//  jucpp
//
//  Created by Igor Mladenovic on 23/09/15.
//  Copyright © 2015 BeanOX UG. All rights reserved.
//

#ifndef angular_h
#define angular_h

#include "http.h"
#include "sql.h"

using namespace jucpp::http;

namespace jucpp { namespace angular {
    
    class AngularRestServer : public http::Server
    {
        
    public:
		AngularRestServer(jucpp::sql::SQLDBSettings const& storage) : m_storageSettings(storage) {};

        AngularRestServer& AngularBinding(String name, String apiUrl, String jsUrl, String tableName);
		enum RequestType
		{
			RequestTypeUnknown,
			RequestTypeGet,
			RequestTypeGetOne,
			RequestTypeAdd,
			RequestTypeEdit,
			RequestTypeDelete
		};
		
		// return list of rows that are available (e.g. "firstName, lastName")
		virtual String getListOfRows(String name, RequestType rt, String id, const Request &req, Response &res, String& filter) { return "*"; };
    private:
        struct AngularBindingData
        {
            String name;
            String apiUrl;
            String jsUrl;
            String tableName;
            String jsContent;
        };
        using AngularBindingList = std::map<String, AngularBindingData>;

    private:
        ResponseStatus getItems(const Request &req, Response &res);
        ResponseStatus getItem(const Request &req, Response &res);
        ResponseStatus addItem(const Request &req, Response &res);
        ResponseStatus editItem(const Request &req, Response &res);
        ResponseStatus deleteItem(const Request &req, Response &res);
        ResponseStatus getAngularFactory(const Request &req, Response &res);
        void generateJsFactory(AngularBindingData& abd);

    private:
        AngularBindingList m_angularBinding;
        StringStringMap m_jsUrlMapping;
		jucpp::sql::SQLDBSettings m_storageSettings;
    };
    
}}

#endif /* angular_h */
