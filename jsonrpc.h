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

#ifndef JSON_RPC_H
#define JSON_RPC_H

#include <string>
#include <cstring>
#include <vector>
#include <exception>
#include "json.hpp"


using Json = nlohmann::json;

namespace jsonrpc
{


class Entity
{
public:
	enum class entity_t : uint8_t
	{
		unknown,
		id,
		error,
		response,
		request,
		notification,
		batch
	};

	Entity(entity_t type);
	Entity(entity_t type, std::nullptr_t);
	virtual ~Entity();
	
	bool is_id();
	bool is_error();
	bool is_response();
	bool is_request();
	bool is_notification();
	bool is_batch();

	virtual Json to_json() const = 0;
	virtual void parse(const Json& json) = 0;

	virtual void parse(const std::string& json_str);
	
	virtual explicit operator bool() const
	{
		 return !isNull;
	}

protected:
	bool isNull;
	entity_t entity;
};



struct Id : Entity
{
	enum class value_t : uint8_t
	{
		null,
		string,
		integer
	};

	Id() : Entity(entity_t::id, nullptr), type(value_t::null), int_id(0), string_id("")
	{
	}

	Id(int id) : Entity(entity_t::id), type(value_t::integer), int_id(id), string_id("")
	{
	}

	Id(const std::string& id) : Entity(entity_t::id), type(value_t::string), int_id(0), string_id(id)
	{
	}

	Id(const Json& json_id) : Entity(entity_t::id), type(value_t::null)
	{
		parse(json_id);
	}

	virtual void parse(const Json& json);

	virtual Json to_json() const;

	friend std::ostream& operator<< (std::ostream &out, const Id &id)
	{
		out << id.to_json();
		return out;
	}

	value_t type;
	int int_id;
	std::string string_id;
};



class Error : public Entity
{
public:
	Error(const Json& json = nullptr);

	Error(std::nullptr_t) : Entity(entity_t::error, nullptr), code(0), message(""), data(nullptr)
	{
	}

	Error(int code, const std::string& message, const Json& data = nullptr) : Entity(entity_t::error), code(code), message(message), data(data)
	{
	}

	Error(const std::string& message, const Json& data = nullptr) : Error(-32603, message, data)
	{
	}

	virtual void parse(const Json& json);

	virtual Json to_json() const;

	int code;
	std::string message;
	Json data;
};



class RpcException : public std::exception {
  char* text_;
public:
	RpcException(const char* text)
	{
		text_ = new char[std::strlen(text) + 1];
		std::strcpy(text_, text);
	}

	RpcException(const std::string& text) : RpcException(text.c_str())
	{
	}

	RpcException(const RpcException& e) : RpcException(e.what())
	{
	}

	virtual ~RpcException() throw()
	{
		delete[] text_;
	}

	virtual const char* what() const noexcept
	{
		return text_;
	}
};




class RequestException : public RpcException
{
  Error error_;
  Id id_;
public:
	RequestException(const char* text, int errorCode = 0, const Id& requestId = Id()) : RpcException(text), error_(text, errorCode), id_(requestId)
	{
	}

	RequestException(const std::string& text, int errorCode = 0, const Id& requestId = Id()) : RpcException(text), error_(text, errorCode), id_(requestId)
	{
	}

	RequestException(const RequestException& e) :  RpcException(e.what()), error_(error()), id_(e.id_)
	{
	}

	virtual Error error() const noexcept
	{
		return error_;
	}

	Json getResponse() const noexcept
	{
		Json response = {
			{"jsonrpc", "2.0"},
			{"error", error_.to_json()},
			{"id", id_.to_json()}
		};

		return response;
	}
};


//	-32600	Invalid Request	The JSON sent is not a valid Request object.
//	-32601	Method not found	The method does not exist / is not available.
//	-32602	Invalid params	Invalid method parameter(s).
//	-32603	Internal error	Internal JSON-RPC error.

class InvalidRequestException : public RequestException
{
public:
	InvalidRequestException(const Id& requestId = Id()) : RequestException("invalid request", -32600, requestId)
	{
	}

	InvalidRequestException(const std::string& message, const Id& requestId = Id()) : RequestException(message, -32600, requestId)
	{
	}
};



class MethodNotFoundException : public RequestException
{
public:
	MethodNotFoundException(const Id& requestId = Id()) : RequestException("method not found", -32601, requestId)
	{
	}

	MethodNotFoundException(const std::string& message, const Id& requestId = Id()) : RequestException(message, -32601, requestId)
	{
	}
};



class InvalidParamsException : public RequestException
{
public:
	InvalidParamsException(const Id& requestId = Id()) : RequestException("invalid params", -32602, requestId)
	{
	}

	InvalidParamsException(const std::string& message, const Id& requestId = Id()) : RequestException(message, -32602, requestId)
	{
	}
};


class InternalErrorException : public RequestException
{
public:
	InternalErrorException(const Id& requestId = Id()) : RequestException("internal error", -32603, requestId)
	{
	}

	InternalErrorException(const std::string& message, const Id& requestId = Id()) : RequestException(message, -32603, requestId)
	{
	}
};




/// JSON-RPC 2.0 request
/**
 * Simple jsonrpc 2.0 parser with getters
 * Currently no named parameters are supported, but only array parameters
 */
class Request : public Entity
{
public:
	std::string method;
	Json params;
	Id id;

	Request(const Json& json = nullptr);

	virtual void parse(const Json& json);
	virtual void parse(const std::string& json_str);

	Json getParam(const std::string& key);
	bool hasParam(const std::string& key);

	template<typename T>
	T getParam(const std::string& key, const T& lowerRange, const T& upperRange)
	{
		T value = getParam(key).get<T>();
		if (value < lowerRange)
			throw InvalidParamsException(key + " out of range", id);
		else if (value > upperRange)
			throw InvalidParamsException(key + " out of range", id);

		return value;
	}

	virtual Json to_json() const;

};



class Response : public Entity
{
public:
	Id id;
	Json result;
	Error error;

	Response(const Json& json = nullptr);
	Response(const Id& id, const Json& result);
	Response(const Id& id, const Error& error);
	Response(const Request& request, const Json& result);
	Response(const Request& request, const Error& error);

	virtual void parse(const Json& json);

	virtual Json to_json() const;
};



class Notification : public Entity
{
public:
	std::string method;
	Json params;
	Notification(const Json& json = nullptr);

	static Json getJson(const std::string& method, const Json& data);
	virtual void parse(const Json& json);
	virtual Json to_json() const;
};


typedef std::shared_ptr<Entity> entity_ptr;

class Batch : public Entity
{
public:
	std::vector<entity_ptr> requests;

	Batch(const Json& json = nullptr);

	virtual void parse(const Json& json);
	virtual Json to_json() const;
};


typedef std::shared_ptr<Request> request_ptr;
typedef std::shared_ptr<Notification> notification_ptr;
typedef std::shared_ptr<Batch> batch_ptr;
typedef std::shared_ptr<Response> response_ptr;



class Parser
{
public:
	static entity_ptr parse(const std::string& json_str);
};



}



#endif
