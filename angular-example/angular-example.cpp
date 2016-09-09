//
//  main.cpp
//  angularExample
//
//  Created by Igor Mladenovic on 23/09/15.
//  Copyright © 2015 BeanOX UG. All rights reserved.
//

#include <jucpp/jucpp.h>
#include <jucpp/http.h>
#include <jucpp/sql.h>
#include <jucpp/angular.h>

using namespace jucpp;
using namespace jucpp::angular;

int main()
{
	sql::SQLDBSettings dbSettings(sql::SQLDBSettings::SQLite, "angularObject.db");

    AngularRestServer(dbSettings)
    .AngularBinding("User", "/api/user", "/js/user.js", "user")
    .AngularBinding("Feed", "/api/feed", "/js/feed.js", "feed")
    .setDocumentRoot("./html")
    .GET("*", [](const Request &req, Response &res)
    {
        String url = req.Url();
        if (url.find("/api/") != 0)
            return Server::ServeStaticFile;
        return Server::Skipped;
    })
    .OPTIONS("*", [](const Request &req, Response &res)
    {
        Server::addCORSHeaders(req, res);
        return Server::Proceeded;
    })
    .listen(8000);
    
    return 0;
}