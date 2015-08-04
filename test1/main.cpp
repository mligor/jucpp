//
//  main.cpp
//  test1
//
//  Created by Igor Mladenovic on 04/07/15.
//  Copyright (c) 2015 BeanOX UG. All rights reserved.
//

#include <jucpp/jucpp.h>
#include <jucpp/http.h>
#include <jucpp/sqlite.h>


using namespace jucpp;
using namespace jucpp::http;
using namespace jucpp::sqlite;


int main()
{
	// Server decalaration
	Server server = Http::createServer([](const Request &req, Response &res)
		 {
			 //usleep(1000000);

			 res.addHeader("Content-Type", "application/json");
			 res.setStatus(200, "Everything OK");
			 
			 Array listOfElements = {1,111,222, "MyList"};
			 listOfElements.append(134);
			 listOfElements.append("How are you");
			 listOfElements.append(L"Unicode string"); // NOT working !!
			 
			 Object data;
			 data["mylist"] = listOfElements;
			 data["Status"]["code"] = 200;
			 data["Status"]["text"] = "Everything is OK";
			 data["Requested Url"] = req.Url();
			 
			 data["From_Request"]["Accept-Language"] = req.Headers("Accept-Language");
			 
			 data["secondList"].append(1);
			 data["secondList"].append(2);
			 data["secondList"].append(101);
			 
			 data["_Content"] = req.Content();
			 
			 data["_Content_json"] = req.Data();
			 
			 data["_Content_json_data"] = req.Data("data"); // return Json Content from _data["data"] - ( e.g. { "data": "mydata" } )
			 data["_Content_json_first_array_element"] = req.DataArray(0); // return first element from Json array  - _data[0] - ( e.g. [ "firstEl", "secondEl", "3th el" ] ) 
			 
			 
			 // access DB
			 SQLite db;
			 try
			 {
				 db.open("myfile.sqlite");
				 db.query("CREATE TABLE IF NOT EXISTS test (Name TEXT UNIQUE PRIMARY KEY, Value TEXT)");
				 
				 SQLite::Result dbRes = db.query("SELECT * FROM test");
				 data["data_from_db"] = dbRes;
				 db.close();
			 }
			 catch (SQLiteException& ex)
			 {
				 data["error"] = "SQLite Exception : " + ex.description();
			 }
			 
			 
			 res.write(data);
		 });
	
	Job serverJob = server.listen(8000);
	
	printf("Server is running at http://127.0.0.1:8000/\n");
	
	serverJob.wait();
	
	return 0;
}

