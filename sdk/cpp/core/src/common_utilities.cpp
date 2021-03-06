// YANG Development Kit
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

#include "common_utilities.hpp"

#include <set>

#include "entity_data_node_walker.hpp"
#include "entity_util.hpp"
#include "json_subtree_codec.hpp"
#include "path/path_private.hpp"
#include "xml_subtree_codec.hpp"

using namespace std;
namespace ydk {
string trim(const string& str) {
  const string whitespace = " \t\n\r";
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == string::npos) return "";  // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

bool replace(string& subject, const string& search, const string& replace) {
  size_t pos = 0;
  int replace_count = 0;
  while ((pos = subject.find(search, pos)) != string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
    replace_count++;
  }
  return replace_count > 0;
}

size_t has_xml_escape_sequences(const string& xml) {
  size_t pos;
  if ((pos = xml.find("&lt;")) != string::npos ||
      (pos = xml.find("&gt;")) != string::npos ||
      (pos = xml.find("&amp;")) != string::npos ||
      (pos = xml.find("&quot;")) != string::npos ||
      (pos = xml.find("&#13;")) != string::npos) {
    ;
  }
  return pos;
}

string replace_xml_escape_sequences(const string& xml) {
  // Initialize table of conversion
  map<string, string> seqs_table;
  seqs_table[string("&lt;")] = string("<");
  seqs_table[string("&gt;")] = string(">");
  seqs_table[string("&amp;")] = string("&");
  seqs_table[string("&quot;")] = string("\"");
  seqs_table[string("&#13;")] = string("");

  string reply = xml;
  for (auto item : seqs_table) {
    size_t pos = 0;
    while ((pos = reply.find(item.first, pos)) != string::npos) {
      reply = reply.replace(pos, item.first.length(), item.second);
    }
  }
  return reply;
}

string entity_vector_to_string(vector<Entity*>& entity_list) {
  string buf = "[";
  bool first = true;
  for (auto item : entity_list) {
    if (first)
      first = false;
    else
      buf += ", ";
    if (item) buf += item->get_segment_path();
  }
  buf += ']';
  return buf;
}

static shared_ptr<Entity> find_child_entity(shared_ptr<Entity> parent_entity,
                                            Entity& filter_entity) {
  auto filter_absolute_path = absolute_path(filter_entity);
  auto parent_absolute_path = absolute_path(*parent_entity);

  if (filter_absolute_path == parent_absolute_path) {
    YLOG_DEBUG(
        "find_child_entity: Filter matches with parent entity, returning");
    return parent_entity;
  }
  YLOG_DEBUG(
      "find_child_entity: Searching for filter entity '{}' under parent entity "
      "'{}'",
      filter_absolute_path, parent_absolute_path);

  // Traverse parent_entity tree for search of matching filter entity
  auto const& children = parent_entity->get_children();
  if (children.empty()) {
    YLOG_DEBUG("Children map is empty");
    return nullptr;
  }
  auto filter_segment_path = filter_entity.get_segment_path();
  auto it = children.find(filter_segment_path);
  if (it != children.end()) {
    auto child_abs_path = absolute_path(*it->second);
    YLOG_DEBUG("Got child with matching segment path; absolute path is '{}'",
               child_abs_path);
    if (child_abs_path == filter_absolute_path) return it->second;
  }
  YLOG_DEBUG("No matching child found");
  for (auto it = children.begin(); it != children.end(); ++it) {
    auto ch = find_child_entity(it->second, filter_entity);
    if (ch != nullptr) return ch;
  }
  return nullptr;
}

shared_ptr<Entity> get_child_entity_from_top(shared_ptr<Entity> top_entity,
                                             Entity& filter_entity) {
  shared_ptr<Entity> child_entity;
  if (filter_entity.is_top_level_class) {
    if (absolute_path(filter_entity) == absolute_path(*top_entity)) {
      return top_entity;
    } else {
      YLOG_ERROR(
          "get_child_entity_from_top: The filter '{}' points to a different "
          "top-entity",
          absolute_path(filter_entity));
    }
  } else {
    YLOG_DEBUG("Searching for child entity matching non-top level filter '{}'",
               absolute_path(filter_entity));
    child_entity = find_child_entity(top_entity, filter_entity);
    if (child_entity != nullptr) {
      YLOG_DEBUG("Found matching child entity '{}'",
                 absolute_path(*child_entity));
      child_entity->parent = nullptr;
    } else {
      YLOG_DEBUG("Matching child entity was not found");
    }
  }
  return child_entity;
}

shared_ptr<Entity> get_top_entity_from_filter(Entity& filter) {
  if (filter.parent == nullptr) {
    if (filter.is_top_level_class)
      return filter.clone_ptr();
    else {
      YLOG_ERROR(
          "get_top_entity_from_filter: Could not traverse from filter '{}' up "
          "to top-entity",
          absolute_path(filter));
      return nullptr;
    }
  }

  return get_top_entity_from_filter(*(filter.parent));
}

shared_ptr<Entity> read_datanode(Entity& filter,
                                 shared_ptr<path::DataNode> read_data_node) {
  if (read_data_node == nullptr) return {};

  shared_ptr<Entity> top_entity = get_top_entity_from_filter(filter);
  if (top_entity == nullptr) return {};

  get_entity_from_data_node(read_data_node.get(), top_entity);

  return get_child_entity_from_top(top_entity, filter);
}

string get_data_payload(Entity& entity, const ServiceProvider& provider) {
  path::DataNode& datanode = get_data_node_from_entity(
      entity, provider.get_session().get_root_schema());
  const path::DataNode* dn = &datanode;
  while (dn && dn->get_parent()) dn = dn->get_parent();

  if (dn == nullptr) return string{};

  YLOG_DEBUG("Encoding the subtree filter request using path API DataNode");
  path::Codec codec{};
  return codec.encode(*dn, provider.get_encoding(), true);
}

string get_xml_subtree_filter_payload(Entity& entity,
                                      const ServiceProvider& provider) {
  XmlSubtreeCodec xml_subtree_codec{};
  YLOG_DEBUG("Encoding the subtree filter request using XML subtree codec");
  return xml_subtree_codec.encode(entity,
                                  provider.get_session().get_root_schema());
}

string get_json_subtree_filter_payload(Entity& entity,
                                       const ServiceProvider& provider,
                                       bool pretty) {
  JsonSubtreeCodec json_subtree_codec{};
  YLOG_DEBUG("Encoding the subtree filter request using JSON subtree codec");
  return json_subtree_codec.encode(
      entity, provider.get_session().get_root_schema(), pretty);
}

Entity* get_top_entity(Entity* entity) {
  Entity* top_entity = entity;
  while (top_entity->parent && !top_entity->is_top_level_class) {
    top_entity = top_entity->parent;
  }
  if (entity->ignore_validation && !top_entity->is_top_level_class) {
    YLOG_WARN(
        "get_top_entity: Validation cannot be disabled on non-top-level entity "
        "'{}'",
        entity->yang_name);
  }
  return top_entity;
}

vector<string> get_union(vector<string>& v1, vector<string>& v2) {
  set<string> all;
  YLOG_DEBUG("Performing union of vectors with {} & {} elements", v1.size(),
             v2.size());
  for (auto& s : v1) {
    all.insert(s);
  }
  for (auto& s : v2) {
    all.insert(s);
  }
  YLOG_DEBUG("Union contains {} elements", all.size());
  vector<string> v;
  for (auto& s : all) {
    YLOG_DEBUG("Adding unioned element: {}", s);
    v.push_back(s);
  }
  return v;
}

shared_ptr<Entity> execute_rpc(ydk::ServiceProvider& provider, Entity& entity,
                               const string& operation, const string& data_tag,
                               bool set_config_flag) {
  vector<Entity*> input_list{};
  input_list.push_back(&entity);

  vector<shared_ptr<Entity>> output_list =
      execute_rpc(provider, input_list, operation, data_tag, set_config_flag);
  if (output_list.size() == 0) return nullptr;

  return output_list[0];
}

vector<shared_ptr<Entity>> execute_rpc(ydk::ServiceProvider& provider,
                                       vector<Entity*>& filter_list,
                                       const string& operation,
                                       const string& data_tag,
                                       bool set_config_flag) {
  const path::Session& session = provider.get_session();
  path::RootSchemaNode& root_schema = session.get_root_schema();
  shared_ptr<ydk::path::Rpc> ydk_rpc{root_schema.create_rpc(operation)};
  vector<shared_ptr<Entity>> result_list{};

  string xml_payload = "";
  for (Entity* entity : filter_list) {
    if (data_tag == "filter" &&
        provider.get_encoding() == EncodingFormat::XML &&
        entity->is_top_level_class) {
      xml_payload += get_xml_subtree_filter_payload(*entity, provider);
    } else {
      xml_payload += get_data_payload(*entity, provider);
    }
  }

  if (set_config_flag) ydk_rpc->get_input_node().create_datanode("only-config");
  ydk_rpc->get_input_node().create_datanode(data_tag, xml_payload);

  // Get root data node
  shared_ptr<path::DataNode> rnd = (*ydk_rpc)(session);
  if (rnd == nullptr) return result_list;

  // Build mapping of DataNode path to DataNode pointer.
  // Use this mapping to retain order of filter list in results.
  map<string, shared_ptr<path::DataNode>> path_to_datanode{};
  for (auto dn : rnd->get_children()) {
    string internal_key = dn->get_path().substr(1);
    path_to_datanode[internal_key] = dn;
  }

  // Build resulting list of entities
  for (Entity* entity : filter_list) {
    string internal_key = absolute_path(*entity);
    shared_ptr<path::DataNode> datanode;
    for (auto dn_entry : path_to_datanode) {
      if (internal_key.find(dn_entry.first) == 0) {
        datanode = dn_entry.second;
        break;
      }
    }
    if (!datanode) {
      YLOG_DEBUG("Searching for datanode using entity yang name '{}'",
                 entity->yang_name);
      path_to_datanode.erase(internal_key);
      for (auto dn_entry : path_to_datanode) {
        if (dn_entry.first.find(entity->yang_name) != string::npos &&
            dn_entry.second) {
          datanode = dn_entry.second;
          break;
        }
      }
    }
    if (datanode) {
      result_list.push_back(read_datanode(*entity, datanode));
    } else {
      YLOG_DEBUG(
          "CRUD read operation did not return data node on entity '{}'; "
          "returning nullptr.",
          internal_key);
      result_list.push_back(nullptr);
    }
  }

  return result_list;
}

path::DataNode* create_root_datanode(path::RootSchemaNode* root_schema) {
  path::RootSchemaNodeImpl& rs_impl =
      dynamic_cast<path::RootSchemaNodeImpl&>(*root_schema);
  path::RootDataImpl* rd = new path::RootDataImpl{rs_impl, rs_impl.m_ctx, "/"};
  path::DataNodeImpl* rdn = dynamic_cast<path::DataNodeImpl*>(rd);
  return rdn;
}

}  // namespace ydk
