/*  ----------------------------------------------------------------
 YDK - YANG Development Kit
 Copyright 2016-2019 Cisco Systems. All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 -------------------------------------------------------------------
 This file has been modified by Yan Gorelik, YDK Solutions.
 All modifications in original under CiscoDevNet domain
 introduced since October 2019 are copyrighted.
 All rights reserved under Apache License, Version 2.0.
 ------------------------------------------------------------------*/

#pragma once

#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "filters.hpp"

namespace ydk {

typedef unsigned short uint8;
typedef unsigned int uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef signed short int8;
typedef signed int int16;
typedef signed int int32;
typedef signed long long int64;

typedef struct Empty {
  bool set;
} Empty;

class Entity;
class YLeaf;
class YLeafList;

class LeafData {
 public:
  LeafData(const std::string& value, YFilter yfilter, bool is_set,
           const std::string& name_space, const std::string& name_space_prefix);
  ~LeafData();

  bool operator==(LeafData& other) const;
  bool operator==(const LeafData& other) const;
  friend std::ostream& operator<<(std::ostream& stream, const LeafData& value);

 public:
  std::string value;
  std::string name_space;
  std::string name_space_prefix;
  YFilter yfilter;
  bool is_set;
};

struct EntityPath {
  std::string path;
  std::vector<std::pair<std::string, LeafData>> value_paths;

  EntityPath(const std::string& path,
             std::vector<std::pair<std::string, LeafData>>& value_paths);

  ~EntityPath();

  bool operator==(EntityPath& other) const;
  bool operator==(const EntityPath& other) const;

  bool operator!=(EntityPath& other) const;
  bool operator!=(const EntityPath& other) const;
};

class NonTypedYList {
 public:
  explicit NonTypedYList(std::vector<std::string>&& ylist_key_names);
  virtual void review(std::shared_ptr<Entity> ep) = 0;

 protected:
  std::optional<std::string> build_key(std::shared_ptr<Entity> ep);
  std::string build_temp_key() { return std::to_string(counter++); }

 private:
  std::vector<std::string> ylist_key_names;
  int counter = 1000000;
};

class Entity {
 public:
  Entity();
  virtual ~Entity();

 public:
  virtual std::string get_segment_path() const = 0;

  virtual bool has_data() const = 0;
  virtual bool has_operation() const = 0;
  virtual std::string get_absolute_path() const;

  virtual void set_value(const std::string& path, const std::string& value,
                         const std::string& name_space = "",
                         const std::string& name_space_prefix = "") = 0;
  virtual void set_filter(const std::string& path, YFilter filter) = 0;
  virtual std::shared_ptr<Entity> get_child_by_name(
      const std::string& yang_name, const std::string& segment_path = "") = 0;

  virtual bool has_leaf_or_child_of_name(const std::string& name) const = 0;

  virtual std::vector<std::pair<std::string, LeafData>> get_name_leaf_data()
      const = 0;

  virtual std::map<std::string, std::shared_ptr<Entity>> get_children()
      const = 0;
  virtual std::vector<std::string> get_order_of_children() const;

  virtual std::shared_ptr<Entity> clone_ptr() const;
  std::shared_ptr<Entity> clone() const;

  virtual void set_parent(Entity* p);
  virtual Entity* get_parent() const;

  virtual std::string get_bundle_yang_models_location() const;
  virtual std::string get_bundle_name() const;

  virtual void set_child_by_name(const std::string& yang_name,
                                 std::shared_ptr<Entity> _ent) = 0;

  virtual std::string get_namespace() const;

  bool operator==(Entity& other) const;
  bool operator==(const Entity& other) const;
  bool operator!=(Entity& other) const;
  bool operator!=(const Entity& other) const;

 public:
  Entity* parent = nullptr;
  std::string yang_name;
  std::string yang_parent_name;
  YFilter yfilter;
  bool is_presence_container;
  bool is_top_level_class;
  bool has_list_ancestor;
  bool ignore_validation;
  std::vector<std::string> ylist_key_names;
  std::string ylist_key;
  NonTypedYList* ylist = nullptr;

  std::string get_ylist_key() const;
};

class Bits {
 public:
  Bits();
  virtual ~Bits();
  bool& operator[](std::string key);
  const std::map<std::string, bool>& get_bitmap() const;

 private:
  std::map<std::string, bool> bitmap;
};

class Decimal64 {
 public:
  explicit Decimal64(const std::string& value) : value(value) {}
  virtual ~Decimal64() {}

  std::string value;
};

class Identity {
 public:
  Identity(const std::string& name_space, const std::string& namespace_prefix,
           const std::string& tag)
      : name_space(name_space), namespace_prefix(namespace_prefix), tag(tag) {}

  virtual ~Identity() {}

  std::string to_string() { return tag; }

 public:
  std::string name_space;
  std::string namespace_prefix;

 private:
  std::string tag;
};

class Enum {
 public:
  class YLeaf {
   public:
    YLeaf(int value, const std::string& name) : value(value), name(name) {}
    ~YLeaf() {}

    int value;
    std::string name;
  };

  Enum() {}
  ~Enum() {}
};

enum class YType {
  uint8,
  uint16,
  uint32,
  uint64,
  int8,
  int16,
  int32,
  int64,
  empty,
  identityref,
  str,
  boolean,
  enumeration,
  bits,
  decimal64
};

class YLeaf {
 public:
  YLeaf(YType type, std::string name);
  ~YLeaf();

  YLeaf(const YLeaf& val);
  YLeaf(YLeaf&& val);

  YLeaf& operator=(const YLeaf& val) = delete;
  YLeaf& operator=(YLeaf&& val) = delete;

  const std::string get() const;
  std::pair<std::string, LeafData> get_name_leafdata() const;

  void operator=(uint8 val);
  void operator=(uint32 val);
  void operator=(uint64 val);
  void operator=(long val);
  void operator=(int8 val);
  void operator=(int32 val);
  void operator=(int64 val);
  void operator=(double val);
  void operator=(Empty val);
  void operator=(Identity val);
  void operator=(Bits val);
  void operator=(std::string val);
  void operator=(Enum::YLeaf val);
  void operator=(Decimal64 val);

  void set(uint8 val);
  void set(uint32 val);
  void set(uint64 val);
  void set(long val);
  void set(int8 val);
  void set(int32 val);
  void set(int64 val);
  void set(double val);
  void set(Empty val);
  void set(Identity val);
  void set(Bits val);
  void set(std::string val);
  void set(Enum::YLeaf val);
  void set(Decimal64 val);

  operator std::string() const;
  bool operator==(YLeaf& other) const;
  bool operator==(const YLeaf& other) const;

  bool& operator[](std::string key);

  bool is_set;

  std::string value_namespace;
  std::string value_namespace_prefix;

  YFilter yfilter;
  void operator=(YFilter filter) { yfilter = filter; };
  void store_value(std::string&& val);
  std::string get_bits_string() const;

  std::string name;
  std::string value;
  int enum_value;
  YType type;
  Bits bits_value;
};

class YLeafList {
 public:
  YLeafList(YType type, const std::string& name);
  virtual ~YLeafList();

  YLeafList(const YLeafList& val);
  YLeafList(YLeafList&& val);

  YLeafList& operator=(const YLeafList& val);
  YLeafList& operator=(YLeafList&& val);

  virtual void append(uint8 val);
  virtual void append(uint32 val);
  virtual void append(uint64 val);
  virtual void append(long val);
  virtual void append(int8 val);
  virtual void append(int32 val);
  virtual void append(int64 val);
  virtual void append(double val);
  virtual void append(Empty val);
  virtual void append(Identity val);
  virtual void append(Bits val);
  virtual void append(std::string val);
  virtual void append(Enum::YLeaf val);
  virtual void append(Decimal64 val);

  virtual YLeaf& operator[](size_t index);

  operator std::string() const;
  bool operator==(YLeafList& other) const;
  bool operator==(const YLeafList& other) const;

  virtual std::vector<std::pair<std::string, LeafData>> get_name_leafdata()
      const;
  virtual std::vector<YLeaf> getYLeafs() const;
  virtual void clear();

  YFilter yfilter;
  void operator=(YFilter filter) { yfilter = filter; };

 public:
  std::vector<YLeaf> values;
  YType type;
  std::string name;
};

template <class T>
class YList : public NonTypedYList {
 public:
  YList(Entity* parent_entity, std::initializer_list<std::string> key_names)
      : NonTypedYList{std::move(key_names)}, parent(parent_entity) {}

  std::shared_ptr<T> operator[](const std::string& key) const {
    auto it = entity_map.find(key);
    if (it != entity_map.end())
      return it->second;
    else {
      return nullptr;
    }
  }
  std::vector<std::shared_ptr<T>> entities() const {
    std::vector<std::shared_ptr<Entity>> ev{};
    for (auto key : key_vector) {
      ev.push_back(entity_map.at(key));
    }
    return ev;
  }
  std::vector<std::string> keys() const { return key_vector; }
  bool has_key(const std::string& key) const { return entity_map.count(key); }
  std::size_t len() const { return key_vector.size(); }

  void append(std::shared_ptr<T> ep) {
    ep->parent = parent;

    auto maybe_key = build_key(ep);
    if (!maybe_key) {
      maybe_key = build_temp_key();
    }

    auto [iter, was_inserted] = entity_map.insert_or_assign(*maybe_key, ep);

    if (was_inserted) {
      key_vector.push_back(*maybe_key);
    }
    entity_map[*maybe_key] = ep;
    ep->ylist_key = std::move(*maybe_key);
    ep->ylist = this;
  }
  void extend(std::initializer_list<std::shared_ptr<T>> ep_list) {
    for (auto ep : ep_list) {
      append(ep);
    }
  }

  void review(std::shared_ptr<Entity> ep) override {
    auto key = this->build_key(ep);
    if (!key || key == ep->ylist_key) return;

    // Reinsert the entity with the right key now.
    pop(ep->ylist_key);
    append(std::static_pointer_cast<T>(ep));
  }

  void clear() {
    key_vector.clear();
    entity_map.clear();
  }

  std::shared_ptr<T> pop(const std::string& key) {
    for (std::vector<std::string>::iterator it = key_vector.begin();
         it != key_vector.end(); ++it) {
      if (*it == key) {
        std::shared_ptr<Entity> found = entity_map[key];
        entity_map.erase(key);
        key_vector.erase(it);
        return found;
      }
    }
    return nullptr;
  }
  std::optional<std::shared_ptr<T>> pop(const std::size_t item) {
    if (item >= key_vector.size()) {
      return std::nullopt;
    }

    auto maybe_entity = entity_map.find(key_vector[item]);
    if (maybe_entity == entity_map.end()) return std::nullopt;

    auto& [key, entity] = *maybe_entity;

    entity_map.erase(maybe_entity);
    key_vector.erase(key_vector.begin() + item);
    return entity;
  }

 private:
  typedef std::map<std::string, std::shared_ptr<Entity>> MapType;

  MapType entity_map;
  std::vector<std::string> key_vector;
  Entity* parent;
};

template <class T>
using YListWrapper = YList<T>;

std::ostream& operator<<(std::ostream& stream, const YLeaf& value);
std::ostream& operator<<(std::ostream& stream, const EntityPath& value);
std::ostream& operator<<(std::ostream& stream, Entity& value);
std::ostream& operator<<(std::ostream& stream, const LeafData& value);

std::string get_bool_string(const std::string& value);

enum class EncodingFormat { XML, JSON };

std::string to_string(YFilter yfilter);

enum class Protocol { restconf, netconf };

enum class DataStore { candidate, running, startup, url, na };

template class YList<Entity>;

}  // namespace ydk
