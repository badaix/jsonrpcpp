/***
    This file is part of snapcast
    Copyright (C) 2014-2016  Johannes Pohl

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/

#include "jsonrpc.h"


using namespace std;

namespace jsonrpc
{


/////////////////////////// Entity implementation /////////////////////////////

Entity::Entity(entity_t type) : isNull(false), entity(type)
{
}


Entity::Entity(entity_t type, std::nullptr_t) : isNull(true), entity(type)
{
}


Entity::~Entity()
{
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





/////////////////////////// Id implementation /////////////////////////////////

void Id::parse(const Json& json)
{
	if (json.is_null())
	{
		isNull = true;
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
	if (isNull || (type == value_t::null))
		return nullptr;
	else if (type == value_t::string)
		return string_id;
	else if (type == value_t::integer)
		return int_id;

	return nullptr;
}





//////////////////////// Error implementation /////////////////////////////////

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

	if (data)
		j["data"] = data;
	return j;
}





///////////////////// Response implementation /////////////////////////////////

void Response::parse(const Json& json)
{
	try
	{
		if (json.count("jsonrpc") == 0)
			throw RpcException("jsonrpc is missing");
		string jsonrpc = json_["jsonrpc"].get<string>();
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
	bool isError(error);
	std::clog << "Response::to_json error: " << isError << "\n";
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
		json_ = json;
		if (json_.count("id") == 0)
			throw InvalidRequestException("id is missing");

		try
		{
			id = Id(json_["id"]);
		}
		catch(const std::exception& e)
		{
			throw InvalidRequestException(e.what());
		}

		if (json_.count("jsonrpc") == 0)
			throw InvalidRequestException("jsonrpc is missing", id);
		string jsonrpc = json_["jsonrpc"].get<string>();
		if (jsonrpc != "2.0")
			throw InvalidRequestException("invalid jsonrpc value: " + jsonrpc, id);

		if (json_.count("method") == 0)
			throw InvalidRequestException("method is missing", id);
		method = json_["method"].get<string>();
		if (method.empty())
			throw InvalidRequestException("method must not be empty", id);

		params.clear();
		try
		{
			if (json_["params"] != nullptr)
			{
				Json p = json_["params"];
				for (Json::iterator it = p.begin(); it != p.end(); ++it)
					params[it.key()] = it.value();
			}
		}
		catch (const exception& e)
		{
			throw InvalidParamsException(e.what(), id);
		}
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
		throw RequestException(e.what(), -32700);
	}
}



jsonrpc::Response Request::getResponse(const Json& result) const
{
	return Response(id, result);
}


jsonrpc::Response Request::getError(const jsonrpc::Error& error) const
{
	return Response(id, error);
}


bool Request::hasParam(const std::string& key)
{
	return (params.find(key) != params.end());
}


Json Request::getParam(const std::string& key)
{
	if (!hasParam(key))
		throw InvalidParamsException(id);
	return params[key];
}


Json Request::to_json() const
{
	return json_;
}


/*
bool JsonRequest::isParam(size_t idx, const std::string& param)
{
	if (idx >= params.size())
		throw InvalidParamsException(*this);
	return (params[idx] == param);
}
*/





///////////////// Notification implementation /////////////////////////////////

Notification::Notification() : Entity(entity_t::notification)
{
}


Json Notification::getJson(const std::string& method, const Json& data)
{
	Json notification = {
		{"jsonrpc", "2.0"},
		{"method", method},
		{"params", data}
	};

	return notification;
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
		method = json["method"];
		if (method.empty())
			throw RpcException("method must not be empty");
		//TODO: params
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





//////////////////////// Batch implementation /////////////////////////////////

Batch::Batch() : Entity(entity_t::batch)
{
}



entity_ptr Parser::parse(const std::string& json_str)
{
	try
	{
		Json json = Json::parse(json_str);
		if (json.count("method") && json.count("id"))
		{
			//Request: contains "method" and "id"
			cout << "Request\n";
		}
		else if (json.count("method"))
		{
			//Notification: Request w/o "id" => contains "method"
			cout << "Notification\n";
		}
		else if (json.count("result") && json.count("id"))
		{
			//Response: contains "result" and "id"
			cout << "Response\n";
		}
		else if (json.count("code") && json.count("message"))
		{
			//Error: contains "code" and "message"
			cout << "Error\n";
		}
		else if (json.is_array())
		{
			//Batch: contains an array
			cout << "Batch\n";
		}
	}
	catch (const exception& e)
	{
		throw RpcException(e.what());
	}

	return nullptr;
}


}


