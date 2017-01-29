/***
    This file is part of jsonrpcpp
    Copyright (C) 2017 Johannes Pohl
    
    This software may be modified and distributed under the terms
    of the MIT license.  See the LICENSE file for details.
***/

#include "jsonrp.hpp"


using namespace std;

namespace jsonrpcpp
{


/////////////////////////// Entity implementation /////////////////////////////

Entity::Entity(entity_t type) : entity(type)
{
}


Entity::~Entity()
{
}


bool Entity::is_exception()
{
	return (entity == entity_t::exception);
}


bool Entity::is_id()
{
	return (entity == entity_t::id);
}


bool Entity::is_error()
{
	return (entity == entity_t::error);
}


bool Entity::is_response()
{
	return (entity == entity_t::response);
}


bool Entity::is_request()
{
	return (entity == entity_t::request);
}


bool Entity::is_notification()
{
	return (entity == entity_t::notification);
}


bool Entity::is_batch()
{
	return (entity == entity_t::batch);
}


void Entity::parse(const std::string& json_str)
{
	parse(Json::parse(json_str));
}


std::string Entity::type_str() const
{
	switch (entity)
	{
		case entity_t::unknown: 
			return "unknown";
		case entity_t::id:
			return "id";
		case entity_t::exception:
			return "exception";
		case entity_t::error:
			return "error";
		case entity_t::response:
			return "response";
		case entity_t::request:
			return "request";
		case entity_t::notification:
			return "notification";
		case entity_t::batch:
			return "batch";
		default:
			return "unknown";
	}
}





/////////////////////////// NullableEntity implementation /////////////////////

NullableEntity::NullableEntity(entity_t type) : Entity(type), isNull(false)
{
}


NullableEntity::NullableEntity(entity_t type, std::nullptr_t) : Entity(type), isNull(true)
{
}


NullableEntity::~NullableEntity()
{
};





/////////////////////////// Id implementation /////////////////////////////////

Id::Id() : Entity(entity_t::id), type(value_t::null), int_id(0), string_id("")
{
}


Id::Id(int id) : Entity(entity_t::id), type(value_t::integer), int_id(id), string_id("")
{
}


Id::Id(const std::string& id) : Entity(entity_t::id), type(value_t::string), int_id(0), string_id(id)
{
}


Id::Id(const Json& json_id) : Entity(entity_t::id), type(value_t::null)
{
	parse(json_id);
}


void Id::parse(const Json& json)
{
	if (json.is_null())
	{
		type = value_t::null;
	}
	else if (json.is_number_integer())
	{
		int_id = json.get<int>();
		type = value_t::integer;
	}
	else if (json.is_string())
	{
		string_id = json.get<std::string>();
		type = value_t::string;
	}
	else
		throw std::invalid_argument("id must be integer, string or null");
}


Json Id::to_json() const
{
	if (type == value_t::null)
		return nullptr;
	else if (type == value_t::string)
		return string_id;
	else if (type == value_t::integer)
		return int_id;

	return nullptr;
}





//////////////////////// Error implementation /////////////////////////////////

Parameter::Parameter(std::nullptr_t) : NullableEntity(entity_t::id, nullptr), type(value_t::null)
{
}


Parameter::Parameter(const Json& json) : NullableEntity(entity_t::id), type(value_t::null)
{	
}


void Parameter::parse(const Json& json)
{
	if (json.is_array())
	{
		param_array = json.get<std::vector<Json>>();
		param_map.clear();
		type = value_t::array;
	}
	else
	{
		param_map = json.get<std::map<std::string, Json>>();
		param_array.clear();
		type = value_t::map;
	}
}


Json Parameter::to_json() const
{
	if (type == value_t::array)
		return param_array;
	else if (type == value_t::map)
		return param_map;
	else
		return nullptr;
}


bool Parameter::is_array() const
{
	return type == value_t::array;
}


bool Parameter::is_map() const
{
	return type == value_t::map;
}


bool Parameter::is_null() const
{
	return isNull;
}


bool Parameter::has(const std::string& key) const
{
	if (type != value_t::map)
		return false;
	return (param_map.find(key) != param_map.end());
}


Json Parameter::get(const std::string& key) const
{
	return param_map.at(key);
}


bool Parameter::has(size_t idx) const
{
	if (type != value_t::array)
		return false;
	return (param_array.size() > idx);
}


Json Parameter::get(size_t idx) const
{
	return param_array.at(idx);
}





//////////////////////// Error implementation /////////////////////////////////

Error::Error(const Json& json) : Error("Internal error", -32603, nullptr)
{
	if (json != nullptr)
		parse(json);
}


Error::Error(std::nullptr_t) : NullableEntity(entity_t::error, nullptr), code(0), message(""), data(nullptr)
{
}


Error::Error(const std::string& message, int code, const Json& data) : NullableEntity(entity_t::error), code(code), message(message), data(data)
{
}


void Error::parse(const Json& json)
{
	try
	{
		if (json.count("code") == 0)
			throw RpcException("code is missing");
		code = json["code"];
		if (json.count("message") == 0)
			throw RpcException("message is missing");
		message = json["message"];
		if (json.count("data"))
			data = json["data"];
		else
			data = nullptr;
	}
	catch (const RpcException& e)
	{
		throw;
	}
	catch (const exception& e)
	{
		throw RpcException(e.what());
	}
}


Json Error::to_json() const
{
	Json j = {
			{"code", code},
			{"message", message},
		};

	if (!data.is_null())
		j["data"] = data;
	return j;
}





////////////////////// Request implementation /////////////////////////////////

Request::Request(const Json& json) : Entity(entity_t::request), method(""), id()
{
	if (json != nullptr)
		parse(json);
}


void Request::parse(const Json& json)
{
	try
	{
		if (json.count("id") == 0)
			throw InvalidRequestException("id is missing");

		try
		{
			id = Id(json["id"]);
		}
		catch(const std::exception& e)
		{
			throw InvalidRequestException(e.what());
		}

		if (json.count("jsonrpc") == 0)
			throw InvalidRequestException("jsonrpc is missing", id);
		string jsonrpc = json["jsonrpc"].get<string>();
		if (jsonrpc != "2.0")
			throw InvalidRequestException("invalid jsonrpc value: " + jsonrpc, id);

		if (json.count("method") == 0)
			throw InvalidRequestException("method is missing", id);
		if (!json["method"].is_string())
			throw InvalidRequestException("method must be a string value", id);
		method = json["method"];
		if (method.empty())
			throw InvalidRequestException("method must not be empty", id);

		if (json.count("params"))
			params.parse(json["params"]);
		else
			params = nullptr;
	}
	catch (const RequestException& e)
	{
		throw;
	}
	catch (const exception& e)
	{
		throw InternalErrorException(e.what(), id);
	}
}


void Request::parse(const std::string& json_str)
{
	// http://www.jsonrpc.org/specification
	//	code	message	meaning
	//	-32700	Parse error	Invalid JSON was received by the server. An error occurred on the server while parsing the JSON text.
	//	-32600	Invalid Request	The JSON sent is not a valid Request object.
	//	-32601	Method not found	The method does not exist / is not available.
	//	-32602	Invalid params	Invalid method parameter(s).
	//	-32603	Internal error	Internal JSON-RPC error.
	//	-32000 to -32099	Server error	Reserved for implementation-defined server-errors.
	try
	{
		parse(Json::parse(json_str));
	}
	catch (const exception& e)
	{
		throw RequestException(Error(e.what(), -32700));
	}
}


Json Request::to_json() const
{
	Json json = {
		{"jsonrpc", "2.0"},
		{"method", method},
		{"id", id.to_json()}
	};

	if (params)
		json["params"] = params.to_json();
	
	return json;
}





///////////////////// Response implementation /////////////////////////////////

Response::Response(const Json& json) : Entity(entity_t::response)
{
	if (json != nullptr)
		parse(json);
}


Response::Response(const Id& id, const Json& result) : Entity(entity_t::response), id(id), result(result), error(nullptr)
{
}


Response::Response(const Id& id, const Error& error) : Entity(entity_t::response), id(id), result(), error(error)
{
}


Response::Response(const Request& request, const Json& result) : Response(request.id, result)
{
}


Response::Response(const Request& request, const Error& error) : Response(request.id, error)
{
}


Response::Response(const RequestException& exception) : Response(exception.id, exception.error)
{
}


void Response::parse(const Json& json)
{
	try
	{
		error = nullptr;
		result = nullptr;
		if (json.count("jsonrpc") == 0)
			throw RpcException("jsonrpc is missing");
		string jsonrpc = json["jsonrpc"].get<string>();
		if (jsonrpc != "2.0")
			throw RpcException("invalid jsonrpc value: " + jsonrpc);
		if (json.count("id") == 0)
			throw RpcException("id is missing");
		id = Id(json["id"]);
		if (json.count("result"))
			result = json["result"];
		else if (json.count("error"))
			error.parse(json["error"]);
		else
			throw RpcException("response must contain result or error");			
	}
	catch (const RpcException& e)
	{
		throw;
	}
	catch (const exception& e)
	{
		throw RpcException(e.what());
	}
}


Json Response::to_json() const
{
	Json j = {
		{"jsonrpc", "2.0"},
		{"id", id.to_json()},
	};

	if (error)
		j["error"] = error.to_json();
	else
		j["result"] = result;
	
	return j;
}





///////////////// Notification implementation /////////////////////////////////

Notification::Notification(const Json& json) : Entity(entity_t::notification)
{
	if (json != nullptr)
		parse(json);
}


void Notification::parse(const Json& json)
{
	try
	{
		if (json.count("jsonrpc") == 0)
			throw RpcException("jsonrpc is missing");
		string jsonrpc = json["jsonrpc"].get<string>();
		if (jsonrpc != "2.0")
			throw RpcException("invalid jsonrpc value: " + jsonrpc);

		if (json.count("method") == 0)
			throw RpcException("method is missing");
		if (!json["method"].is_string())
			throw RpcException("method must be a string value");
		method = json["method"];
		if (method.empty())
			throw RpcException("method must not be empty");

		if (json.count("params"))
			params.parse(json["params"]);
		else
			params = nullptr;
	}
	catch (const RpcException& e)
	{
		throw;
	}
	catch (const exception& e)
	{
		throw RpcException(e.what());
	}
}


Json Notification::to_json() const
{
	Json json = {
		{"jsonrpc", "2.0"},
		{"method", method},
	};

	if (params)
		json["params"] = params.to_json();

	return json;
}





//////////////////////// Batch implementation /////////////////////////////////

Batch::Batch(const Json& json) : Entity(entity_t::batch)
{
	if (json != nullptr)
		parse(json);
}


void Batch::parse(const Json& json)
{
//	cout << "Batch::parse: " << json.dump() << "\n";
	entities.clear();
	for (auto it = json.begin(); it != json.end(); ++it) 
	{
//		cout << "x: " << it->dump() << "\n";
		entity_ptr entity(nullptr);
		try
		{
			entity = Parser::parse(*it);
			if (!entity)
				entity = make_shared<Error>("Invalid Request", -32600);
		}
		catch(const RequestException& e)
		{
			entity = make_shared<RequestException>(e);
		}
		catch(const std::exception& e)
		{
			entity = make_shared<Error>(e.what(), -32600);
		}
		entities.push_back(entity);
	}
	if (entities.empty())
		throw InvalidRequestException();
}


Json Batch::to_json() const
{
	Json result;
	for (const auto& j: entities)
		result.push_back(j->to_json());
	return result;
}





//////////////////////// Parser implementation ////////////////////////////////

entity_ptr Parser::parse(const std::string& json_str)
{
	try
	{
		return parse(Json::parse(json_str));
	}
	catch (const exception& e)
	{
		throw;
	}

	return nullptr;
}


entity_ptr Parser::parse(const Json& json)
{
	try
	{
		if (is_request(json))
			return make_shared<Request>(json);
		else if (is_notification(json))
			return make_shared<Notification>(json);
		else if (is_response(json))
			return make_shared<Response>(json);
		else if (is_batch(json))
			return make_shared<Batch>(json);
	}
	catch (const RpcException& e)
	{
		throw;
	}
	catch (const exception& e)
	{
		throw RpcException(e.what());
	}

	return nullptr;
}


bool Parser::is_request(const std::string& json_str)
{
	try
	{
		return is_request(Json::parse(json_str));
	}
	catch (const exception& e)
	{
		return false;
	}
}


bool Parser::is_request(const Json& json)
{
	return (json.count("method") && json.count("id"));
}


bool Parser::is_notification(const std::string& json_str)
{
	try
	{
		return is_notification(Json::parse(json_str));
	}
	catch (const exception& e)
	{
		return false;
	}
}


bool Parser:: is_notification(const Json& json)
{
	return (json.count("method") && (json.count("id") == 0));
}


bool Parser::is_response(const std::string& json_str)
{
	try
	{
		return is_response(Json::parse(json_str));
	}
	catch (const exception& e)
	{
		return false;
	}
}


bool Parser::is_response(const Json& json)
{
	return (json.count("result") && json.count("id"));
}


bool Parser::is_batch(const std::string& json_str)
{
	try
	{
		return is_batch(Json::parse(json_str));
	}
	catch (const exception& e)
	{
		return false;
	}
}


bool Parser::is_batch(const Json& json)
{
	return (json.is_array());
}



}

