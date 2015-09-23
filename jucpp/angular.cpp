//
//  angular.c
//  jucpp
//
//  Created by Igor Mladenovic on 23/09/15.
//  Copyright © 2015 BeanOX UG. All rights reserved.
//

#include "angular.h"
#include "sqlite.h"

using namespace jucpp::sqlite;

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
        BIND_GET(jsUrl, &AngularRestServer::getAngularFactory);
        
        m_angularBinding[apiUrl] = abd;
        m_jsUrlMapping[jsUrl] = apiUrl;
        
        SQLite db(m_databaseName);
        db.query("CREATE TABLE IF NOT EXISTS `" + abd.tableName + "` (id INTEGER UNIQUE PRIMARY KEY AUTOINCREMENT, data TEXT)");
        return *this;
    }
    
    Server::ResponseStatus AngularRestServer::getItems(const Request &req, Response &res)
    {
        auto it = m_angularBinding.find(req.Url());
        if (it == m_angularBinding.end())
            return Proceeded;
        
        try
        {
            SQLite db(m_databaseName);
            SQLite::Result r = db.query("SELECT * FROM `" + (*it).second.tableName + "`");
            
            Array ret;
            for (auto row: r)
            {
                Variant o = Server::jsonDecode(row["data"].asString());
                o["id"] = row["id"];
                ret.append(o);
            }
            res.write(ret);
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
        auto it = m_angularBinding.find(req.Url());
        if (it == m_angularBinding.end())
            return Proceeded;
        try
        {
            SQLite db(m_databaseName);
            SQLite::Result r = db.query("SELECT * FROM `" + (*it).second.tableName + "` WHERE id='" + req.PathParam("id") + "'");
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
            SQLite db(m_databaseName);
            String data = Server::jsonEncode(req.Data());
            db.query("INSERT INTO `" + (*it).second.tableName + "` (data) VALUES ('" + data + "')");
            String id = db.getLastInsertRowId();
            SQLite::Result r = db.query("SELECT * FROM `" + (*it).second.tableName + "` WHERE id='" + id + "'");
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
            SQLite db(m_databaseName);
            db.query("INSERT OR REPLACE INTO `" + (*it).second.tableName + "` (id, data) VALUES ('" + id.asString() + "', '" + data + "')");
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
            SQLite db(m_databaseName);
            SQLite::Result r = db.query("DELETE FROM `" + (*it).second.tableName + "` WHERE id='" + req.PathParam("id") + "'");
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
