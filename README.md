# jsonrpc++

Leightweight C++ [JSON-RPC 2.0](http://www.jsonrpc.org/specification) library

[![Github Releases](https://img.shields.io/github/release/badaix/jsonrpcpp.svg)](https://github.com/badaix/jsonrpcpp/releases)
[![Build Status](https://travis-ci.org/badaix/jsonrpcpp.svg?branch=master)](https://travis-ci.org/badaix/jsonrpcpp)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/badaix/jsonrpcpp.svg)](https://lgtm.com/projects/g/badaix/jsonrpcpp/context:cpp)  

## What it is

jsonrpc++ parses and constructs [JSON-RPC 2.0](https://www.jsonrpc.org/specification) objects, like

* [Request](http://www.jsonrpc.org/specification#request_object)
  * [Notification](http://www.jsonrpc.org/specification#notification)
  * [Parameter](http://www.jsonrpc.org/specification#parameter_structures)
* [Response](http://www.jsonrpc.org/specification#response_object)
  * [Error](http://www.jsonrpc.org/specification#error_object)
* [Batch](http://www.jsonrpc.org/specification#batch)

### Example: Parsing a request

```c++
jsonrpcpp::entity_ptr entity = jsonrpcpp::Parser::do_parse(R"({"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3})");
if (entity->is_request())
{
    jsonrpcpp::request_ptr request = dynamic_pointer_cast<jsonrpcpp::Request>(entity);
    if (request->method() == "subtract")
    {
        int result = request->params().get<int>("minuend") - request->params().get<int>("subtrahend");
        jsonrpcpp::Response response(*request, result);
        cout << " Response: " << response.to_json().dump() << "\n";
        //will print: {"jsonrpc": "2.0", "result": 19, "id": 3}
    }
    else
        throw jsonrpcpp::MethodNotFoundException(*request);
}
```

## What it not is

jsonrpc++ is completely transport agnostic, i.e. it doesn't care about transportation of the messages and there is no TCP client or server component shipped with this library.

As JSON backbone [JSON for Modern C++](https://nlohmann.github.io/json/) is used.

## Some code example

```c++
jsonrpcpp::entity_ptr entity =
    jsonrpcpp::Parser::do_parse(R"({"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3})");
if (entity && entity->is_request())
{
    jsonrpcpp::request_ptr request = dynamic_pointer_cast<jsonrpcpp::Request>(entity);
    cout << " Request: " << request->method() << ", id: " << request->id() << ", has params: " << !request->params().is_null() << "\n";
    if (request->method() == "subtract")
    {
        int result;
        if (request->params().is_array())
            result = request->params().get<int>(0) - request->params().get<int>(1);
        else
            result = request->params().get<int>("minuend") - request->params().get<int>("subtrahend");

        jsonrpcpp::Response response(*request, result);
        cout << " Response: " << response.to_json().dump() << "\n";
    }
    else if (request->method() == "sum")
    {
        int result = 0;
        for (const auto& summand : request->params().param_array)
            result += summand.get<int>();
        jsonrpcpp::Response response(*request, result);
        cout << " Response: " << response.to_json().dump() << "\n";
    }
    else
    {
        throw jsonrpcpp::MethodNotFoundException(*request);
    }
}
```
