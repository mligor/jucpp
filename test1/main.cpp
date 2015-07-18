//
//  main.cpp
//  test1
//
//  Created by Igor Mladenovic on 04/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include <jucpp/jucpp.h>
#include <jucpp/http.h>
#include <unistd.h>

#include <json/json.h>

using namespace jucpp;
using namespace jucpp::http;

int main()
{
	Server server = Http::createServer([](const Request &req, Response &res)
		 {
			 //usleep(1000000);
			 
			 Json::FastWriter writer;
			 
			 Json::Value result;
			 result["description"] = "Hello from JuCpp";
			 result["error_code"] = 200;
			 result["url"] = req.Url();

			 StringStringMap headers;
			 headers["Content-Type"] = "application/json";
			 res.writeHead(200, headers);
			 
			 res.write(writer.write(result).c_str());
		 });
	
	Job serverJob = server.listen(8000);
	
	printf("Server is running at http://127.0.0.1:8000/\n");
	
	Job* pCtrlJob = nullptr;
	
	Job ctrlJob = Http::createServer([&serverJob, &pCtrlJob](const Request& req, Response& res)
					   {
						   serverJob.stop();
						   pCtrlJob->stop();
						   
					   }).listen(8001);
	
	pCtrlJob = &ctrlJob;
	ctrlJob.wait();

	serverJob.wait();
	return 0;
}

