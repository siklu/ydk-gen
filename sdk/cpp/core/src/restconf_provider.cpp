/// YANG Development Kit
// Copyright 2016 Cisco Systems. All rights reserved
//
////////////////////////////////////////////////////////////////
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//
//////////////////////////////////////////////////////////////////

#include "restconf_provider.hpp"

#include <libyang/libyang.h>

#include <iostream>
#include <memory>
#include <sstream>

#include "common_utilities.hpp"
#include "entity_data_node_walker.hpp"
#include "errors.hpp"
#include "ietf_parser.hpp"
#include "logger.hpp"
#include "restconf_client.hpp"
#include "types.hpp"

using namespace std;

namespace ydk {
RestconfServiceProvider::RestconfServiceProvider(
    path::Repository& repo, const string& address, const string& username,
    const string& password, int port, EncodingFormat encoding,
    const string& config_url_root, const string& state_url_root)
    : encoding(encoding),
      session{repo, address,  username,        password,
              port, encoding, config_url_root, state_url_root} {}

RestconfServiceProvider::RestconfServiceProvider(
    std::unique_ptr<RestconfClient> client,
    const std::shared_ptr<ydk::path::RootSchemaNode>& root_schema,
    const std::string& edit_method, const std::string& config_url_root,
    const std::string& state_url_root, EncodingFormat encoding)
    : encoding(encoding), session{move(client),    root_schema,
                                  edit_method,     encoding,
                                  config_url_root, state_url_root} {}

RestconfServiceProvider::~RestconfServiceProvider() {
  YLOG_INFO("Disconnected from device");
}

EncodingFormat RestconfServiceProvider::get_encoding() const {
  return encoding;
}

const path::Session& RestconfServiceProvider::get_session() const {
  return session;
}

static string get_rpc(const string& operation) {
  string rpc;
  if (operation == "create") {
    rpc = "ydk:create";
  } else if (operation == "update") {
    rpc = "ydk:update";
  } else if (operation == "delete") {
    rpc = "ydk:delete";
  } else if (operation == "read") {
    rpc = "ydk:read";
  } else {
    YLOG_ERROR(
        "RestconfServiceProvider::execute_operation: Operation '{}' is not "
        "supported",
        operation);
    throw(
        YServiceProviderError("NetconfServiceProvider::execute_operation: "
                              "Operation is not supported"));
  }
  return rpc;
}

shared_ptr<Entity> RestconfServiceProvider::execute_operation(
    const string& operation, Entity& entity, map<string, string> params) {
  string rpc = get_rpc(operation);
  string data_tag = (operation == "read") ? "filter" : "entity";
  return execute_rpc(*this, entity, rpc, data_tag,
                     (params["mode"] == "config"));
}

vector<shared_ptr<Entity>> RestconfServiceProvider::execute_operation(
    const string& operation, vector<Entity*> entity_list,
    map<string, string> params) {
  string rpc = get_rpc(operation);
  string data_tag = (operation == "read") ? "filter" : "entity";
  return execute_rpc(*this, entity_list, rpc, data_tag,
                     (params["mode"] == "config"));
}

}  // namespace ydk
