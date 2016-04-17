//
//  angular.c
//  jucpp
//
//  Created by Igor Mladenovic on 23/09/15.
//  Copyright © 2015 BeanOX UG. All rights reserved.
//

#include "angular.h"
#include "sql.h"

using namespace jucpp::sql;

namespace jucpp { namespace angular {

    AngularRestServer& AngularRestServer::AngularBinding(String name, String apiUrl, String jsUrl, String tableName)
    {
        AngularBindingData abd;
        abd.name = name;
        abd.apiUrl = apiUrl;
        abd.jsUrl = jsUrl;
        abd.tableName = tableName;
        generateJsFactory(abd);
        
        BIND_GET(apiUrl, &AngularRestServer::getItems);
        BIND_GET(apiUrl + "/:id", &AngularRestServer::getItem);
        BIND_POST(apiUrl, &AngularRestServer::addItem);
        BIND_PUT(apiUrl, &AngularRestServer::editItem);
        BIND_DELETE(apiUrl + "/:id", &AngularRestServer::deleteItem);
		if (jsUrl != "")
			BIND_GET(jsUrl, &AngularRestServer::getAngularFactory);
        
        m_angularBinding[apiUrl] = abd;
        m_jsUrlMapping[jsUrl] = apiUrl;
        
		SQLDB db(m_storageSettings);
        db.query("CREATE TABLE IF NOT EXISTS `%s` (id INTEGER UNIQUE PRIMARY KEY %s, data TEXT)", abd.tableName.c_str(), db.AUTOINCREMENT());
        return *this;
    }
    
    Server::ResponseStatus AngularRestServer::getItems(const Request &req, Response &res)
    {
        auto it = m_angularBinding.find(req.Url());
        if (it == m_angularBinding.end())
            return Proceeded;
		
        try
        {
			String filter = "";
			String listOfRows = getListOfRows((*it).second.name, RequestTypeGet, "", req, res, filter);
			if (listOfRows == "")
			{
				res.setStatus(403);
				res.write("Request not allowed");
				return Proceeded;
			}
			
			SQLDB db(m_storageSettings);
			String query = "SELECT " + listOfRows + " FROM `" + (*it).second.tableName + "`";
			if (filter != "")
				query += " WHERE " + filter;
            sql::SQLDB::Result r = db.query(query.c_str());
            res.write(r);
        }
        catch (std::exception& e)
        {
            res.setStatus(500);
            res.write(e.what());
        }
        return Proceeded;
    }
    
    Server::ResponseStatus AngularRestServer::getItem(const Request &req, Response &res)
    {
		String url = req.Url();
		std::size_t p = url.find_last_of("/");
		if (p == url.npos)
			return Proceeded;
		url = url.substr(0, p);

        auto it = m_angularBinding.find(url);
        if (it == m_angularBinding.end())
            return Proceeded;
        try
        {
			String id = req.PathParam("id");
			String filter = "";
			String listOfRows = getListOfRows((*it).second.name, RequestTypeGetOne, id, req, res, filter);
			if (listOfRows == "")
			{
				res.setStatus(403);
				res.write("Request not allowed");
				return Proceeded;
			}

			SQLDB db(m_storageSettings);
            sql::SQLDB::Result r = db.query("SELECT %s FROM `%s` WHERE id='%s'", listOfRows.c_str(), (*it).second.tableName.c_str(), id.c_str());
            if (r.size() > 0)
                res.write(r[(unsigned int)0]);
            
        }
        catch (std::exception& e)
        {
            res.setStatus(500);
            res.write(e.what());
        }
        return Proceeded;
    }
    
    Server::ResponseStatus AngularRestServer::addItem(const Request &req, Response &res)
    {
        auto it = m_angularBinding.find(req.Url());
        if (it == m_angularBinding.end())
        {
            res.setStatus(500);
            return Proceeded;
        }
        
        try
        {
			SQLDB db(m_storageSettings);
            String data = Server::jsonEncode(req.Data());
            db.query("INSERT INTO `%s` (data) VALUES ('%s')", (*it).second.tableName.c_str(), data.c_str());
            String id = db.lastInsertRowId();
            SQLDB::Result r = db.query("SELECT * FROM `%s` WHERE id='%s'", (*it).second.tableName.c_str(), id.c_str());
            if (r.size() > 0)
            {
                auto row = r[(unsigned int)0];
                Variant o = Server::jsonDecode(row["data"].asString());
                o["id"] = row["id"];
                res.write(o);
            }
        }
        catch (std::exception& e)
        {
            res.setStatus(500);
            res.write(e.what());
        }
        return Proceeded;
    }
    
    Server::ResponseStatus AngularRestServer::editItem(const Request &req, Response &res)
    {
        Variant id = req.Data("id");
        if (id.isNull())
            return addItem(req, res);
        
        auto it = m_angularBinding.find(req.Url());
        if (it == m_angularBinding.end())
        {
            res.setStatus(500);
            return Proceeded;
        }
        
        try
        {
            String data = Server::jsonEncode(req.Data());
			SQLDB db(m_storageSettings);
            db.query("INSERT OR REPLACE INTO `%s` (id, data) VALUES ('%s', '%s')", (*it).second.tableName.c_str(), id.asString().c_str(), data.c_str());
        }
        catch (std::exception& e)
        {
            res.setStatus(500);
            res.write(e.what());
        }
        return Proceeded;
    }
    
    Server::ResponseStatus AngularRestServer::deleteItem(const Request &req, Response &res)
    {
        String url = req.Url();
        std::size_t p = url.find_last_of("/");
        if (p == url.npos)
            return Proceeded;
        url = url.substr(0, p);
        
        
        auto it = m_angularBinding.find(url);
        if (it == m_angularBinding.end())
            return Proceeded;
        
        try
        {
			SQLDB db(m_storageSettings);
            SQLDB::Result r = db.query("DELETE FROM `%s` WHERE id='%s'", (*it).second.tableName.c_str(), req.PathParam("id").c_str());
        }
        catch (std::exception& e)
        {
            res.setStatus(500);
            res.write(e.what());
        }
        return Proceeded;
        
    }
    
    Server::ResponseStatus AngularRestServer::getAngularFactory(const Request &req, Response &res)
    {
        auto it = m_jsUrlMapping.find(req.Url());
        if (it == m_jsUrlMapping.end())
            return Proceeded;
        
        String apiUrl = (*it).second;
        
        auto it2 = m_angularBinding.find(apiUrl);
        if (it2 == m_angularBinding.end())
            return Proceeded;
        res.addHeader("Content-Type", "application/javascript");
        res.write(it2->second.jsContent.c_str());
        return Proceeded;
    }
    
    void AngularRestServer::generateJsFactory(AngularBindingData& abd)
    {
        abd.jsContent += "//automaticly generated file using jucpp (http://www.jucpp.com)\n";
        abd.jsContent = "(function(angular){'use strict';\n";
        abd.jsContent += "angular.module('jucpp." + abd.name + "', ['zimco.rest']).factory('" + abd.name + "', ['REST', '$q', function(REST, $q) {\n";
        
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



}} // namespace end
