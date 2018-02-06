/***
    This file is part of jsonrpc++
    Copyright (C) 2017 Johannes Pohl
    
    This software may be modified and distributed under the terms
    of the MIT license.  See the LICENSE file for details.
***/


#include <iostream>
#include "jsonrp.hpp"

using namespace std;


jsonrpcpp::Parser parser;


jsonrpcpp::Response getRespone(jsonrpcpp::request_ptr request)
{
	//cout << " Request: " << request->method << ", id: " << request->id << ", has params: " << !request->params.is_null() << "\n";
	if (request->method == "subtract")
	{
		if (request->params)
		{
			int result;
			if (request->params.is_array())
				result = request->params.get<int>(0) - request->params.get<int>(1);
			else
				result = request->params.get<int>("minuend") - request->params.get<int>("subtrahend");

			return jsonrpcpp::Response(*request, result);
		}
		else
			throw jsonrpcpp::InvalidParamsException(*request);
	}
	else if (request->method == "sum")
	{
		int result = 0;
		for (const auto& summand: request->params.param_array)
			result += summand.get<int>();
		return jsonrpcpp::Response(*request, result);
	}
	else if (request->method == "get_data")
	{
		return jsonrpcpp::Response(*request, Json({"hello", 5}));
	}
	else 
	{
		throw jsonrpcpp::MethodNotFoundException(*request);
	}
}



void test(const std::string& json_str)
{
	try
	{
		cout << "--> " << json_str << "\n";
		jsonrpcpp::entity_ptr entity = parser.parse(json_str);
		if (entity)
		{
			//cout << " Json: " << entity->to_json().dump() << "\n";
			if (entity->is_response())
			{
				cout << "<-- " << entity->to_json().dump() << "\n";
			}
			if (entity->is_request())
			{
				jsonrpcpp::Response response = getRespone(dynamic_pointer_cast<jsonrpcpp::Request>(entity));
				cout << "<-- " << response.to_json().dump() << "\n";
			}
			else if (entity->is_notification())
			{
				jsonrpcpp::notification_ptr notification = dynamic_pointer_cast<jsonrpcpp::Notification>(entity);
				cout << "Notification: " << notification->method << ", has params: " << !notification->params.is_null() << "\n";
			}
			else if (entity->is_batch())
			{
				jsonrpcpp::batch_ptr batch = dynamic_pointer_cast<jsonrpcpp::Batch>(entity);
				jsonrpcpp::Batch responseBatch;
				//cout << " Batch\n";
				for (const auto& batch_entity: batch->entities)
				{
					//cout << batch_entity->type_str() << ": \t" << batch_entity->to_json() << "\n";
					if (batch_entity->is_request())
					{
						try
						{
							jsonrpcpp::Response response = getRespone(dynamic_pointer_cast<jsonrpcpp::Request>(batch_entity));
							responseBatch.add(response); //<jsonrpcpp::Response>
						}
						catch(const jsonrpcpp::RequestException& e)
						{
							responseBatch.add(e); //<jsonrpcpp::RequestException>
						}
					}
					else if (batch_entity->is_exception())
					{
						responseBatch.add_ptr(batch_entity);
					}
					else if (batch_entity->is_error())
					{
						jsonrpcpp::error_ptr error = dynamic_pointer_cast<jsonrpcpp::Error>(batch_entity);
						responseBatch.add(jsonrpcpp::RequestException(*error));
					}
				}
				if (!responseBatch.entities.empty())
					cout << "<-- " << responseBatch.to_json().dump() << "\n";
			}
		}
	}
	catch(const jsonrpcpp::RequestException& e)
	{
		cout << "<-- " << e.to_json().dump() << "\n";
		//cout << " Response: " << jsonrpcpp::Response(e).to_json().dump() << "\n";
		//cerr << "RequestException: " << e.what() << "\n";
	}
	catch(const jsonrpcpp::ParseErrorException& e)
	{
		cout << "<-- " << e.to_json().dump() << "\n";
	}
	catch(const jsonrpcpp::RpcException& e)
	{
		cerr << "RpcException: " << e.what() << "\n";
		cout << "<-- " << jsonrpcpp::ParseErrorException(e.what()).to_json().dump() << "\n";
	}
	catch(const std::exception& e)
	{
		cerr << "Exception: " << e.what() << "\n";
	}
	cout << "\n";
}


void test(const jsonrpcpp::Entity& entity)
{
	test(entity.to_json().dump());
}


void update(const jsonrpcpp::Parameter& params)
{
	cout << "Notification callback: update, has params: " << !params.is_null() << "\n";
}

/*
void foobar(const jsonrpcpp::Notification& notification, const jsonrpcpp::Parameter& params)
{
	cout << "Notification callback: " << notification.method << ", has params: " << !notification.params.is_null() << "\n";
}
*/

void foobar(const jsonrpcpp::Parameter& params)
{
	cout << "Notification callback: foobar, has params: " << !params.is_null() << "\n";
}


jsonrpcpp::response_ptr sum(const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params)
{
	int result = 0;
	for (const auto& summand: params.param_array)
		result += summand.get<int>();
	cout << "Request callback: sum, result: " << result << "\n";
	return make_shared<jsonrpcpp::Response>(id, result);
}


//examples taken from: http://www.jsonrpc.org/specification#examples
int main(int argc, char* argv[])
{
	parser.register_notification_callback("update", update);
	parser.register_notification_callback("foobar", foobar);
	parser.register_request_callback("sum", sum);

	cout << "rpc call with positional parameters:\n\n";
	test(R"({"jsonrpc": "2.0", "method": "sum", "params": [1, 2, 3, 4, 5], "id": 1})");
	test(jsonrpcpp::Request(1, "sum", Json({1, 2, 3, 4, 5})));

	test(R"({"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 1})");
	test(jsonrpcpp::Request(1, "subtract", Json({42, 23})));
	test(R"({"jsonrpc": "2.0", "method": "subtract", "params": [23, 42], "id": 2})");
	test(jsonrpcpp::Request(2, "subtract", Json({23, 42})));

	cout << "\n\nrpc call with named parameters:\n\n";
	test(R"({"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3})");
	test(jsonrpcpp::Request(3, "subtract", Json({{"subtrahend", 23}, {"minuend", 42}})));
	test(R"({"jsonrpc": "2.0", "method": "subtract", "params": {"minuend": 42, "subtrahend": 23}, "id": 4})");
	test(jsonrpcpp::Request(4, "subtract", Json({{"minuend", 42}, {"subtrahend", 23}})));

	cout << "\n\na Notification:\n\n";
	test(R"({"jsonrpc": "2.0", "method": "update", "params": [1,2,3,4,5]})");
	test(jsonrpcpp::Notification("update", Json({1, 2, 3, 4, 5})));
	test(R"({"jsonrpc": "2.0", "method": "foobar"})");
	test(jsonrpcpp::Notification("foobar"));

	cout << "\n\nrpc call of non-existent method:\n\n";
	test(R"({"jsonrpc": "2.0", "method": "foobar", "id": "1"})");
	test(jsonrpcpp::Request("1", "foobar"));

	cout << "\n\nrpc call with invalid JSON:\n\n";
	test(R"({"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz])");

	cout << "\n\nrpc call with invalid Request object:\n\n";
	test(R"({"jsonrpc": "2.0", "method": 1, "params": "bar"})");

	cout << "\n\nrpc call Batch, invalid JSON:\n\n";
	test(R"(	[
		{"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
		{"jsonrpc": "2.0", "method"
	])");

	cout << "\n\nrpc call with an empty Array:\n\n";
	test(R"([])");

	cout << "\n\nrpc call with an invalid Batch (but not empty):\n\n";
	test(R"([1])");

	cout << "\n\nrpc call with invalid Batch:\n\n";
	test(R"([1,2,3])");

	cout << "\n\nrpc call Batch:\n\n";
	test(R"(	[
		{"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
		{"jsonrpc": "2.0", "method": "notify_hello", "params": [7]},
		{"jsonrpc": "2.0", "method": "subtract", "params": [42,23], "id": "2"},
		{"foo": "boo"},
		{"jsonrpc": "2.0", "method": 1, "params": "bar"},
		{"jsonrpc": "2.0", "method": 1, "params": "bar", "id": 4},
		{"jsonrpc": "2.0", "method": "foo.get", "params": {"name": "myself"}, "id": "5"},
		{"jsonrpc": "2.0", "method": "get_data", "id": "9"} 
	])");

	cout << "\n\nrpc call Batch (all notifications):\n\n";
	test(R"(	[
		{"jsonrpc": "2.0", "method": "notify_sum", "params": [1,2,4]},
		{"jsonrpc": "2.0", "method": "notify_hello", "params": [7]}
	])");
	
}


