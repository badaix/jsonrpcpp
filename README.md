# jsonrpc++

C++ [JSON-RPC 2.0](http://www.jsonrpc.org/specification) library

When grown up, this library will be a leightweight JSON-RPC 2.0 C++ library. 

It parses and constructs JSON RPC 2.0 objects, like 
* [Request](http://www.jsonrpc.org/specification#request_object)
  * [Notification](http://www.jsonrpc.org/specification#notification)
  * [Parameter](http://www.jsonrpc.org/specification#parameter_structures)
* [Response](http://www.jsonrpc.org/specification#response_object)
  * [Error](http://www.jsonrpc.org/specification#error_object)
* [Batch](http://www.jsonrpc.org/specification#batch)

jsonrpc++ is completely transport agnostic, i.e. it doesn't care about transportation of the messages and there are no TCP client or server components shipped with this lib. 

As JSON backbone [JSON for Modern C++](https://nlohmann.github.io/json/) is used.


Stay tuned, there's more to come.


##some code
````c++
jsonrpc::entity_ptr entity = jsonrpc::Parser::parse(R"({"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3})");
if (entity && entity->is_request())
{
	jsonrpc::request_ptr request = dynamic_pointer_cast<jsonrpc::Request>(entity);
	cout << " Request: " << request->method << ", id: " << request->id << ", has params: " << !request->params.is_null() << "\n";
	if (request->method == "subtract")
	{
		int result;
		if (request->params.is_array())
			result = request->params.get<int>(0) - request->params.get<int>(1);
		else
			result = request->params.get<int>("minuend") - request->params.get<int>("subtrahend");

		jsonrpc::Response response(*request, result);
		cout << " Response: " << response.to_json().dump() << "\n";
	}
	else if (request->method == "sum")
	{
		int result = 0;
		for (const auto& summand: request->params.param_array)
			result += summand.get<int>();
		jsonrpc::Response response(*request, result);
		cout << " Response: " << response.to_json().dump() << "\n";
	}
	else 
	{
		throw jsonrpc::MethodNotFoundException(*request);
	}
}
  ````
