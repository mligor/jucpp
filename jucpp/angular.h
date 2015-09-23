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
using namespace jucpp::http;

namespace jucpp { namespace angular {
    
    class AngularRestServer : public http::Server
    {
        
    public:
        AngularRestServer& AngularBinding(String name, String apiUrl, String jsUrl, String tableName);
        AngularRestServer& setDatabaseName(String databaseName)
        {
            m_databaseName = databaseName;
            return *this;
        }
        
    protected:
        
        ResponseStatus getItems(const Request &req, Response &res);
        ResponseStatus getItem(const Request &req, Response &res);
        ResponseStatus addItem(const Request &req, Response &res);
        ResponseStatus editItem(const Request &req, Response &res);
        ResponseStatus deleteItem(const Request &req, Response &res);
        ResponseStatus getAngularFactory(const Request &req, Response &res);
        
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
        void generateJsFactory(AngularBindingData& abd)
        {
            abd.jsContent += "//automaticly generated file using jucpp (http://www.jucpp.com)\n";
            abd.jsContent = "(function(angular){'use strict';\n";
            abd.jsContent += "angular.module('jucpp', ['zimco.rest']).factory('" + abd.name + "', ['REST', '$q', function(REST, $q) {\n";
            
            abd.jsContent += "  var actions = {\n";
            abd.jsContent += "    get : { url:'" + abd.apiUrl + "', method:'GET', isArray:true},\n";
            abd.jsContent += "    getOne : { url:'" + abd.apiUrl + "/:id', method:'GET'},\n";
            abd.jsContent += "    write : { url:'" + abd.apiUrl + "', method:'PUT'},\n";
            abd.jsContent += "    delete : { url:'" + abd.apiUrl + "/:id' , method:'DELETE'},\n";
            abd.jsContent += "    add : { url:'" + abd.apiUrl + "', method:'POST'}\n";
            abd.jsContent += "  };\n";
            abd.jsContent += "  var rest = REST('', {}, actions);\n";
            abd.jsContent += "  var restRequest = function(rr) {\n";
            abd.jsContent += "    return function() {\n";
            abd.jsContent += "      var deferred = $q.defer();\n";
            abd.jsContent += "      var gp = rr.apply(this, arguments).$promise;\n";
            abd.jsContent += "      gp.then(function(d){ deferred.resolve(d); }, function(e){ deferred.reject(e); });\n";
            abd.jsContent += "      return deferred.promise;\n";
            abd.jsContent += "    };\n";
            abd.jsContent += "  };\n";
            
            abd.jsContent += "  var obj = {};\n";
            abd.jsContent += "  obj.get = restRequest(rest.get);\n";
            abd.jsContent += "  obj.getOne = restRequest(rest.getOne);\n";
            abd.jsContent += "  obj.write = restRequest(rest.write);\n";
            abd.jsContent += "  obj.delete = restRequest(rest.delete);\n";
            abd.jsContent += "  obj.add = restRequest(rest.add);\n";
            abd.jsContent += "  return obj;\n";
            abd.jsContent += "}]);\n";
            abd.jsContent += "})(angular);\n";
            
        }
        
    private:
        AngularBindingList m_angularBinding;
        StringStringMap m_jsUrlMapping;
        String m_databaseName;
    };
    

    
}}

#endif /* angular_h */
