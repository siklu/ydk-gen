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

#include "xml_subtree_codec.hpp"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <unordered_map>

#include "entity_lookup.hpp"
#include "entity_util.hpp"
#include "logger.hpp"
#include "xml_util.hpp"

using namespace std;

namespace ydk {
struct XmlDocDeleter {
  void operator()(xmlDoc *ptr) { xmlFreeDoc(ptr); }
};

static void decode_xml(xmlDocPtr doc, xmlNodePtr root, Entity &entity,
                       Entity *parent, const string &leaf_name);

static void walk_children(Entity &entity, xmlNodePtr root_node);
static void populate_xml_node(Entity &entity, xmlNodePtr xml_node);
static void populate_xml_node_contents(Entity &entity, EntityPath &path,
                                       xmlNodePtr xml_node);

XmlSubtreeCodec::XmlSubtreeCodec() {}

XmlSubtreeCodec::~XmlSubtreeCodec() {}

//////////////////////////////////////////////////////////////////
// XmlSubtreeCodec::encode
//////////////////////////////////////////////////////////////////
std::string XmlSubtreeCodec::encode(Entity &entity,
                                    path::RootSchemaNode &root_schema) {
  (void)root_schema;
  EntityPath root_path = get_entity_path(entity, nullptr);
  std::unique_ptr<xmlDoc, XmlDocDeleter> doc(xmlNewDoc(to_xmlchar("1.0")));
  xmlNodePtr root_node = xmlNewNode(NULL, to_xmlchar(entity.yang_name));
  xmlDocSetRootElement(doc.get(), root_node);
  if (!entity.get_namespace().empty()) {
    set_xml_namespace(entity.get_namespace(), root_node);
  }

  populate_xml_node_contents(entity, root_path, root_node);
  walk_children(entity, root_node);

  string xml_str = to_string(doc.get(), root_node);
  return xml_str;
}

static void walk_children(Entity &entity, xmlNodePtr xml_node) {
  std::map<string, shared_ptr<Entity>> children = entity.get_children();
  YLOG_DEBUG("XMLCodec: Children count for: {} : {}",
             get_entity_path(entity, entity.parent).path, children.size());
  for (auto const &child : children) {
    if (child.second == nullptr) continue;
    YLOG_DEBUG("==================");
    YLOG_DEBUG("XMLCodec: Looking at child '{}'", child.first);
    if (child.second->has_operation() || child.second->has_data() ||
        child.second->is_presence_container)
      populate_xml_node(*(child.second), xml_node);
    else
      YLOG_DEBUG("XMLCodec: Child has no data and no operations");
  }
}

static void set_operation_from_yfilter(YFilter yfilter, xmlNodePtr xml_node) {
  if (yfilter != YFilter::read) {
    xmlNewProp(xml_node, to_xmlchar("operation"),
               to_xmlchar(to_string(yfilter)));
  }
}

static xmlNodePtr create_and_populate_xml_node(Entity &entity, YFilter yfilter,
                                               xmlNodePtr parent_xml_node,
                                               const xmlChar *content,
                                               const std::string &yang_name) {
  xmlNodePtr child =
      xmlNewChild(parent_xml_node, NULL, to_xmlchar(yang_name), content);

  // We only set top level entities' namespace. namespace is not required for
  // other entities.
  if (entity.is_top_level_class) {
    set_xml_namespace(entity.get_namespace(), child);
  }

  if (is_set(yfilter)) {
    set_operation_from_yfilter(yfilter, child);
  }
  return child;
}

static void populate_xml_node(Entity &entity, xmlNodePtr xml_node) {
  EntityPath path = get_entity_path(entity, entity.parent);

  xmlNodePtr child = create_and_populate_xml_node(
      entity, entity.yfilter, xml_node, NULL, entity.yang_name);
  populate_xml_node_contents(entity, path, child);
  walk_children(entity, child);
}

static const xmlChar *get_content_from_leafdata(LeafData &leaf_data) {
  const xmlChar *content = NULL;
  if (leaf_data.is_set) {
    content = to_xmlchar(leaf_data.value);
  } else if (is_set(leaf_data.yfilter)) {
    content = NULL;
  }
  return content;
}

static bool leaf_to_be_created(LeafData &leaf_data) {
  return leaf_data.is_set || is_set(leaf_data.yfilter);
}

static void set_prefixed_namespace_from_leafdata(LeafData &leaf_data,
                                                 xmlNodePtr xml_node) {
  if (leaf_data.name_space.size() > 0 &&
      leaf_data.name_space_prefix.size() > 0) {
    ostringstream os;
    os << "xmlns:" << leaf_data.name_space_prefix;
    xmlNewProp(xml_node, to_xmlchar(os.str()),
               to_xmlchar(leaf_data.name_space));
  }
}

static void populate_xml_node_contents(Entity &entity, EntityPath &path,
                                       xmlNodePtr xml_node) {
  YLOG_DEBUG("XMLCodec: Leaf count: {}", path.value_paths.size());
  for (const std::pair<std::string, LeafData> &name_value : path.value_paths) {
    LeafData leaf_data = name_value.second;
    YLOG_DEBUG("XMLCodec: Creating child {} of {} with value: '{}', is_set: {}",
               name_value.first, entity.yang_name, leaf_data.value,
               leaf_data.is_set);

    const xmlChar *content = get_content_from_leafdata(leaf_data);
    if (leaf_to_be_created(leaf_data)) {
      xmlNodePtr child = create_and_populate_xml_node(
          entity, leaf_data.yfilter, xml_node, content, name_value.first);
      set_prefixed_namespace_from_leafdata(leaf_data, child);
      if (is_set(leaf_data.yfilter)) {
        YLOG_DEBUG("XMLCodec: Storing operation '{}' for leaf {}",
                   to_string(leaf_data.yfilter), name_value.first);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////
// XmlSubtreeCodec::decode
//////////////////////////////////////////////////////////////////
std::shared_ptr<Entity> XmlSubtreeCodec::decode(
    const std::string &payload, std::shared_ptr<Entity> entity) {
  if (entity->get_augment_capabilities_function()) {
    entity->get_augment_capabilities_function()();
  }

  std::unique_ptr<xmlDoc, XmlDocDeleter> doc(
      xmlParseDoc(reinterpret_cast<const xmlChar *>(payload.c_str())));

  xmlNodePtr root = xmlDocGetRootElement(doc.get());
  if (entity->yang_name != to_string(root->name)) {
    YLOG_ERROR("XMLCodec: Top entity '{}' does not match the payload",
               entity->yang_name);
    throw YServiceProviderError{"Top entity does not match the payload"};
  }
  decode_xml(doc.get(), root->children, *entity, nullptr, "");
  return entity;
}

static void check_and_set_leaf(Entity &entity, Entity *parent,
                               xmlNodePtr xml_node, xmlDocPtr doc) {
  string current_node_name{to_string(xml_node->name)};
  if (xml_node->children == NULL) {
    YLOG_DEBUG("XMLCodec: Creating leaf '{}' with no value", current_node_name);
    entity.set_filter(current_node_name, YFilter::read);
  } else {
    decode_xml(doc, xml_node->children, entity, parent, current_node_name);
  }
}

static string resolve_leaf_value_namespace(const string &content,
                                           const string &name_space,
                                           const string &name_space_prefix,
                                           Entity *entity) {
  string c{content};
  if (name_space.size() > 0 && name_space_prefix.size() > 0) {
    if (content.find(name_space_prefix) != string::npos &&
        content.find(":") != string::npos) {
      auto s = content.find(":");
      c = content.substr(s + 1);
    }
    Entity *p = entity;
    while (p->parent != nullptr) {
      p = p->parent;
    }
    auto m = p->get_namespace_identity_lookup();
    YLOG_DEBUG("XMLCodec: Got namespace identity lookup with '{}' elements",
               m.size());
    if (m.find({c, name_space}) != m.end()) {
      string module_name = m[{c, name_space}];
      c = module_name + ":" + c;
    }
  }
  return c;
}

static void check_and_set_content(Entity &entity, const string &leaf_name,
                                  xmlNodePtr parent_xml_node, xmlChar *content,
                                  xmlDocPtr doc) {
  if (leaf_name.size() > 0 && !isonlywhitespace(content)) {
    xmlNsPtr *nsList = xmlGetNsList(doc, parent_xml_node);
    string name_space;
    string name_space_prefix;
    if (nsList) {
      for (xmlNsPtr ns = *nsList; ns; ns = ns->next) {
        name_space = to_string(ns->href);
        name_space_prefix = to_string(ns->prefix);
        break;
      }
      xmlFree(nsList);
    }
    string c = resolve_leaf_value_namespace(to_string(content), name_space,
                                            name_space_prefix, &entity);

    YLOG_DEBUG("XMLCodec: Creating leaf '{}' with value '{}'", leaf_name, c);
    entity.set_value(leaf_name, c, name_space, name_space_prefix);
  }
}

static void check_payload_to_raise_exception(Entity &entity,
                                             const xmlChar *name) {
  string current_node_name{to_string(name)};
  if (!entity.has_leaf_or_child_of_name(current_node_name)) {
    ostringstream os;
    os << "XMLCodec: Wrong payload! No element '" << current_node_name
       << "' found in '" << entity.yang_name << "'";
    YLOG_ERROR(os.str().c_str());
    throw YServiceProviderError{os.str()};
  }
}

static void check_and_set_node(Entity &entity, Entity *parent,
                               xmlNodePtr xml_node, xmlDocPtr doc) {
  YLOG_DEBUG("XMLCodec: Looking for child '{}' in '{}'",
             to_string(xml_node->name), entity.yang_name);
  check_payload_to_raise_exception(entity, xml_node->name);
  auto child_name = to_string(xml_node->name);
  if (xml_node->ns->href && xml_node->parent && xml_node->parent->ns->href) {
    auto child_ns = to_string(xml_node->ns->href);
    if (child_ns != to_string(xml_node->parent->ns->href)) {
      string module_name;
      auto capabilities_lookup_table = get_global_capabilities_lookup_tables();
      auto it = capabilities_lookup_table.find(child_ns);
      if (it != capabilities_lookup_table.end()) {
        module_name = it->second.module;
      } else {
        module_name = child_ns.substr(child_ns.rfind("/") + 1);
      }
      child_name = module_name + ":" + child_name;
    }
  }
  auto child = entity.get_child_by_name(child_name);
  if (child) {
    YLOG_DEBUG("XMLCodec: Creating child entity '{}' in '{}'", child_name,
               entity.yang_name);
    if (!child->parent) {
      child->parent = parent;
    }
    decode_xml(doc, xml_node->children, *child, &entity, "");
    if (child->ylist) child->ylist->review(child);
  } else {
    check_and_set_leaf(entity, parent, xml_node, doc);
  }
}

static void decode_xml(xmlDocPtr doc, xmlNodePtr root, Entity &entity,
                       Entity *parent, const string &leaf_name) {
  xmlNodePtr xml_node = NULL;

  for (xml_node = root; xml_node; xml_node = xml_node->next) {
    if (xml_node->type == XML_TEXT_NODE) {
      check_and_set_content(entity, leaf_name, xml_node->parent,
                            xml_node->content, doc);
    } else {
      // We skip the decode of "input"
      auto input = entity.get_child_by_name("input");
      if (!parent && input) {
        decode_xml(doc, xml_node, *input, &entity, leaf_name);
      } else {
        check_and_set_node(entity, parent, xml_node, doc);
      }
    }
  }
}

}  // namespace ydk
