//
//  main.cpp
//  angularExample
//
//  Created by Igor Mladenovic on 23/09/15.
//  Copyright © 2015 BeanOX UG. All rights reserved.
//

#include <jucpp/jucpp.h>
#include <jucpp/http.h>
#include <jucpp/sqlite.h>
#include <jucpp/angular.h>

using namespace jucpp;
using namespace jucpp::angular;

int main()
{
    AngularRestServer()
    .setDatabaseName("angularObject.db")
    .AngularBinding("User", "/api/user", "/js/user.js", "user")
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
    .listen(8000)
    .wait();
    return 0;
}