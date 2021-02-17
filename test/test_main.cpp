/***
    This file is part of jsonrpc++
    Copyright (C) 2017-2021 Johannes Pohl

    This software may be modified and distributed under the terms
    of the MIT license.  See the LICENSE file for details.
***/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "jsonrpcpp.hpp"

using namespace std;

TEST_CASE("Main test")
{
    jsonrpcpp::entity_ptr entity =
        jsonrpcpp::Parser::do_parse(R"({"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3})");
    REQUIRE(entity->is_request());
    jsonrpcpp::request_ptr request = dynamic_pointer_cast<jsonrpcpp::Request>(entity);
    REQUIRE(request->method() == "subtract");
    int result = request->params().get<int>("minuend") - request->params().get<int>("subtrahend");
    REQUIRE(result == 19);
    jsonrpcpp::Response response(*request, result);
    REQUIRE(response.id().type() == jsonrpcpp::Id::value_t::integer);
    REQUIRE(response.id().int_id() == 3);
    REQUIRE(response.result() == 19);
    REQUIRE(response.to_json() == nlohmann::json::parse(R"({"jsonrpc": "2.0", "result": 19, "id": 3})"));
}

TEST_CASE("Null parameter")
{
    jsonrpcpp::entity_ptr entity = jsonrpcpp::Parser::do_parse(R"({"jsonrpc": "2.0", "method": "nullrequest", "params": null, "id": 4})");
    REQUIRE(entity->is_request());
    jsonrpcpp::request_ptr request = dynamic_pointer_cast<jsonrpcpp::Request>(entity);
    REQUIRE(request->method() == "nullrequest");
    REQUIRE(request->params().to_json() == nullptr);
    REQUIRE(request->params().is_null() == true);
    int result = 12;
    jsonrpcpp::Response response(*request, result);
    REQUIRE(response.id().type() == jsonrpcpp::Id::value_t::integer);
    REQUIRE(response.id().int_id() == 4);
    REQUIRE(response.result() == 12);
    REQUIRE(response.to_json() == nlohmann::json::parse(R"({"jsonrpc": "2.0", "result": 12, "id": 4})"));
}