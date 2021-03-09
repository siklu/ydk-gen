/*  ----------------------------------------------------------------
 Copyright 2016 Cisco Systems

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ------------------------------------------------------------------*/

#include <string.h>

#include <iostream>

#include "catch.hpp"
#include "config.hpp"
#include "ydk/crud_service.hpp"
#include "ydk/netconf_provider.hpp"
#include "ydk_ydktest/oc_pattern.hpp"
#include "ydk_ydktest/ydktest_sanity.hpp"

using namespace ydk;
using namespace std;
using namespace ydktest;
using namespace oc_pattern;

void print_entity(shared_ptr<ydk::Entity> entity,
                  ydk::path::RootSchemaNode& root);

TEST_CASE("one_level_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // READ
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  auto one = make_unique<ydktest_sanity::Runner::One>();
  one->number = 1;
  one->name = "1.2.3.4";
  reply = crud.create(provider, *one);
  REQUIRE(reply);

  auto filter = make_unique<ydktest_sanity::Runner>();
  auto sanity_one_filter = filter->one;
  auto r_2 = crud.read(provider, *sanity_one_filter);
  REQUIRE(r_2 != nullptr);
  auto* r_2_ptr = dynamic_cast<ydktest_sanity::Runner::One*>(r_2.get());
  REQUIRE(r_2_ptr != nullptr);

  REQUIRE(one->number == r_2_ptr->number);
  REQUIRE(one->name == r_2_ptr->name);

  // UPDATE
  one->number = 10;
  one->name = "runner/one/name";
  reply = crud.update(provider, *one);
  REQUIRE(reply);

  r_2 = crud.read(provider, *sanity_one_filter);
  REQUIRE(r_2 != nullptr);
  r_2_ptr = dynamic_cast<ydktest_sanity::Runner::One*>(r_2.get());
  REQUIRE(r_2_ptr != nullptr);

  REQUIRE(one->number == r_2_ptr->number);
  REQUIRE(one->name == r_2_ptr->name);

  // DELETE
  reply = crud.delete_(provider, *one);
  REQUIRE(reply);
}

TEST_CASE("one_level_aug_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // READ
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  r_1->ydktest_sanity_augm_one->twin_number = 1;
  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  auto filter = make_unique<ydktest_sanity::Runner>();
  auto r_2 = crud.read(provider, *filter);
  REQUIRE(r_2 != nullptr);
  ydktest_sanity::Runner* r_2_ptr =
      dynamic_cast<ydktest_sanity::Runner*>(r_2.get());
  REQUIRE(r_2_ptr != nullptr);

  REQUIRE(r_1->ydktest_sanity_augm_one->twin_number ==
          r_2_ptr->ydktest_sanity_augm_one->twin_number);

  // UPDATE
  r_1 = make_unique<ydktest_sanity::Runner>();
  r_1->ydktest_sanity_augm_one->twin_number = 10;

  filter = make_unique<ydktest_sanity::Runner>();
  reply = crud.update(provider, *r_1);
  REQUIRE(reply);

  r_2 = crud.read(provider, *filter);
  REQUIRE(r_2 != nullptr);
  r_2_ptr = dynamic_cast<ydktest_sanity::Runner*>(r_2.get());
  REQUIRE(r_2_ptr != nullptr);

  REQUIRE(r_1->ydktest_sanity_augm_one->twin_number ==
          r_2_ptr->ydktest_sanity_augm_one->twin_number);

  // DELETE
  filter = make_unique<ydktest_sanity::Runner>();
  reply = crud.delete_(provider, *filter);
  REQUIRE(reply);
}

TEST_CASE("two_level_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto filter = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *filter);
  REQUIRE(reply);

  // CREATE
  auto two = make_unique<ydktest_sanity::Runner::Two>();
  two->number = 2;
  two->name = "runner:two:name";
  two->sub1->number = 21;

  reply = crud.create(provider, *two);
  REQUIRE(reply);

  // READ
  filter = make_unique<ydktest_sanity::Runner>();
  auto two_filter = filter->two;
  auto r_2 = crud.read(provider, *two_filter);
  REQUIRE(r_2 != nullptr);
  auto r_2_ptr = dynamic_cast<ydktest_sanity::Runner::Two*>(r_2.get());
  REQUIRE(r_2_ptr != nullptr);

  REQUIRE(two->number == r_2_ptr->number);
  REQUIRE(two->name == r_2_ptr->name);
  REQUIRE(two->sub1->number == r_2_ptr->sub1->number);

  // UPDATE
  two->number = 20;
  two->name = "runner/two/name";
  two->sub1->number = 210;

  reply = crud.update(provider, *two);
  REQUIRE(reply);

  // READ UPDATE
  r_2 = crud.read(provider, *two_filter);
  REQUIRE(r_2 != nullptr);
  r_2_ptr = dynamic_cast<ydktest_sanity::Runner::Two*>(r_2.get());
  REQUIRE(r_2_ptr != nullptr);

  REQUIRE(two->number == r_2_ptr->number);
  REQUIRE(two->name == r_2_ptr->name);
  REQUIRE(two->sub1->number == r_2_ptr->sub1->number);

  // DELETE
  reply = crud.delete_(provider, *two);
  REQUIRE(reply);
}

TEST_CASE("three_level_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // CREATE
  auto three = make_unique<ydktest_sanity::Runner::Three>();
  three->number = 3;
  three->name = "runner:three:name";
  three->sub1->number = 31;
  three->sub1->sub2->number = 311;

  bool reply = crud.create(provider, *three);
  REQUIRE(reply);

  // READ
  auto filter = make_unique<ydktest_sanity::Runner>();
  auto three_filter = filter->three;
  auto r_2 = crud.read(provider, *three_filter);
  REQUIRE(r_2 != nullptr);
  auto r_2_ptr = dynamic_cast<ydktest_sanity::Runner::Three*>(r_2.get());
  REQUIRE(r_2_ptr != nullptr);

  REQUIRE(three->number == r_2_ptr->number);
  REQUIRE(three->name == r_2_ptr->name);
  REQUIRE(three->sub1->number == r_2_ptr->sub1->number);
  REQUIRE(three->sub1->sub2->number == r_2_ptr->sub1->sub2->number);

  // UPDATE
  three->number = 30;
  three->name = "runner/three/name";
  three->sub1->number = 310;
  three->sub1->sub2->number = 3110;

  reply = crud.update(provider, *three);
  REQUIRE(reply);

  // READ UPDATE
  r_2 = crud.read(provider, *three_filter);
  REQUIRE(r_2 != nullptr);
  r_2_ptr = dynamic_cast<ydktest_sanity::Runner::Three*>(r_2.get());
  REQUIRE(r_2_ptr != nullptr);

  REQUIRE(three->number == r_2_ptr->number);
  REQUIRE(three->name == r_2_ptr->name);
  REQUIRE(three->sub1->number == r_2_ptr->sub1->number);
  REQUIRE(three->sub1->sub2->number == r_2_ptr->sub1->sub2->number);

  // DELETE
  reply = crud.delete_(provider, *three);
  REQUIRE(reply);
}

TEST_CASE("onelist") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto e_1 = make_unique<ydktest_sanity::Runner::OneList::Ldata>();
  auto e_2 = make_unique<ydktest_sanity::Runner::OneList::Ldata>();
  e_1->number = 1;
  e_1->name = "foo";
  e_2->number = 2;
  e_2->name = "bar";

  reply = crud.create(provider, *e_1);
  REQUIRE(reply);

  reply = crud.create(provider, *e_2);
  REQUIRE(reply);

  // READ and VALIDATE
  auto filter = make_unique<ydktest_sanity::Runner>();
  auto one_list = filter->one_list;
  auto r_read = crud.read(provider, *one_list);
  REQUIRE(r_read != nullptr);
  auto list_ptr = dynamic_cast<ydktest_sanity::Runner::OneList*>(r_read.get());
  REQUIRE(list_ptr != nullptr);

  auto l_1 = dynamic_cast<ydktest_sanity::Runner::OneList::Ldata*>(
      list_ptr->ldata[0].get());
  auto l_2 = dynamic_cast<ydktest_sanity::Runner::OneList::Ldata*>(
      list_ptr->ldata[1].get());
  REQUIRE(l_1->name.value == "foo");
  REQUIRE(l_2->name.value == "bar");

  // DELETE and VALIDATE
  reply = crud.delete_(provider, *e_1);
  REQUIRE(reply);

  reply = crud.delete_(provider, *e_2);
  REQUIRE(reply);

  auto r = crud.read(provider, *r_1);
  REQUIRE(r == nullptr);
}

TEST_CASE("test_onelist_neg_update_key_nonexist") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto r_2 = make_unique<ydktest_sanity::Runner>();
  auto e_1 = make_shared<ydktest_sanity::Runner::OneList::Ldata>();
  e_1->number = 1;
  e_1->name = "foo";
  r_1->one_list->ldata.append(e_1);
  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  // READ
  auto r_read = crud.read(provider, *r_2);
  REQUIRE(r_read != nullptr);

  // UPDATE
  auto ldata = dynamic_cast<ydktest_sanity::Runner::OneList::Ldata*>(
      r_1->one_list->ldata[0].get());
  ldata->name = "2";
  ldata->number = 2;
  reply = crud.update(provider, *r_1);
  REQUIRE(reply);
  r_read = crud.read(provider, *r_2);
  REQUIRE(r_read != nullptr);
}

TEST_CASE("test_twolist_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto two_list = make_unique<ydktest_sanity::Runner::TwoList>();
  auto l_1 = make_shared<ydktest_sanity::Runner::TwoList::Ldata>();
  auto l_2 = make_shared<ydktest_sanity::Runner::TwoList::Ldata>();
  auto s_1 = make_shared<ydktest_sanity::Runner::TwoList::Ldata::Subl1>();
  auto s_2 = make_shared<ydktest_sanity::Runner::TwoList::Ldata::Subl1>();

  l_1->number = 21;
  l_1->name = "21name";
  l_2->number = 22;
  l_2->name = "22name";
  s_1->number = 221;
  s_1->name = "221name";
  s_2->number = 222;
  s_2->name = "222name";

  l_1->subl1.extend({s_1, s_2});
  two_list->ldata.extend({l_1, l_2});

  reply = crud.create(provider, *two_list);
  REQUIRE(reply);

  // READ
  auto r_read = crud.read(provider, *r_1);
  REQUIRE(r_read != nullptr);

  // UPDATE
  l_1->number = 23;
  l_1->name = "23name";
  s_1->number = 223;
  s_1->name = "223name";
  l_2->number = 24;
  l_2->name = "24name";
  s_2->number = 224;
  s_2->name = "224name";
  reply = crud.update(provider, *two_list);
  REQUIRE(reply);
  r_read = crud.read(provider, *r_1);
  REQUIRE(r_read != nullptr);
}

TEST_CASE("test_three_list_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_shared<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto l_1 = make_shared<ydktest_sanity::Runner::ThreeList::Ldata>();
  auto l_2 = make_shared<ydktest_sanity::Runner::ThreeList::Ldata>();
  auto s_1 = make_shared<ydktest_sanity::Runner::ThreeList::Ldata::Subl1>();
  auto s_2 = make_shared<ydktest_sanity::Runner::ThreeList::Ldata::Subl1>();
  auto ss_1 =
      make_shared<ydktest_sanity::Runner::ThreeList::Ldata::Subl1::SubSubl1>();
  auto ss_2 =
      make_shared<ydktest_sanity::Runner::ThreeList::Ldata::Subl1::SubSubl1>();

  l_1->number = 21;
  l_1->name = "21name";
  l_2->number = 22;
  l_2->name = "22name";
  s_1->number = 221;
  s_1->name = "221name";
  s_2->number = 222;
  s_2->name = "222name";
  ss_1->number = 2221;
  ss_1->name = "2221name";
  ss_2->number = 2222;
  ss_2->name = "2222name";

  s_1->sub_subl1.append(ss_1);
  s_2->sub_subl1.append(ss_2);
  l_1->subl1.append(s_1);
  l_2->subl1.append(s_2);
  r_1->three_list->ldata.extend({l_1, l_2});

  reply = crud.create(provider, *r_1->three_list);
  REQUIRE(reply);

  // READ
  auto r_2 = ydktest_sanity::Runner();
  auto r_read = crud.read(provider, r_2);
  REQUIRE(r_read != nullptr);

  // UPDATE
  l_1->number = 23;
  l_1->name = "23name";
  s_1->number = 223;
  s_1->name = "223name";
  ss_1->number = 2223;
  ss_1->name = "2223name";
  l_2->number = 24;
  l_2->name = "24name";
  s_2->number = 224;
  s_2->name = "224name";
  ss_2->number = 2224;
  ss_2->name = "2224name";
  reply = crud.update(provider, *r_1);
  REQUIRE(reply);

  r_2 = ydktest_sanity::Runner();
  r_read = crud.read(provider, r_2);
  REQUIRE(r_read != nullptr);
}

TEST_CASE("test_InbtwList_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_shared<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto r_2 = make_shared<ydktest_sanity::Runner>();
  auto l_1 = make_shared<ydktest_sanity::Runner::InbtwList::Ldata>();
  auto l_2 = make_shared<ydktest_sanity::Runner::InbtwList::Ldata>();
  auto s_1 =
      make_shared<ydktest_sanity::Runner::InbtwList::Ldata::Subc::SubcSubl1>();
  auto s_2 =
      make_shared<ydktest_sanity::Runner::InbtwList::Ldata::Subc::SubcSubl1>();

  l_1->number = 21;
  l_1->name = "21name";
  l_2->number = 22;
  l_2->name = "22name";
  s_1->number = 221;
  s_1->name = "221name";
  s_2->number = 222;
  s_2->name = "222name";

  l_1->subc->subc_subl1.append(s_1);
  l_2->subc->subc_subl1.append(s_2);
  r_1->inbtw_list->ldata.extend({l_1, l_2});

  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  // READ
  auto r_read = crud.read(provider, *r_2);
  REQUIRE(r_read != nullptr);

  // UPDATE
  l_1->number = 221;
  l_1->name = "221name";
  s_1->number = 2221;
  s_1->name = "2221name";
  l_2->number = 222;
  l_2->name = "222name";
  s_2->number = 2222;
  s_2->name = "2222name";
  reply = crud.update(provider, *r_1);
  REQUIRE(reply);
  r_read = crud.read(provider, *r_2);
  REQUIRE(r_read != nullptr);
}

TEST_CASE("test_leafref_simple_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto r_2 = make_unique<ydktest_sanity::Runner>();

  r_1->ytypes->built_in_t->number8 = 100;
  r_1->ytypes->built_in_t->leaf_ref = r_1->ytypes->built_in_t->number8.get();
  crud.create(provider, *r_1);

  // READ
  auto r_read = crud.read(provider, *r_2);
  REQUIRE(r_read != nullptr);

  // UPDATE
  r_1->ytypes->built_in_t->number8 = 110;
  r_1->ytypes->built_in_t->leaf_ref = r_1->ytypes->built_in_t->number8.get();
  crud.update(provider, *r_1);
  r_read = crud.read(provider, *r_2);
  REQUIRE(r_read != nullptr);
}

TEST_CASE("test_leafref_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_shared<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  r_1->one->name = "1.2.3.4";
  r_1->two->sub1->number = 21;
  r_1->three->sub1->sub2->number = 311;
  auto e_1 = make_shared<ydktest_sanity::Runner::InbtwList::Ldata>();
  auto e_2 = make_shared<ydktest_sanity::Runner::InbtwList::Ldata>();
  e_1->number = 11;
  e_2->number = 21;
  e_1->name = "11name";
  e_2->name = "21name";
  e_1->subc->number = 111;
  e_2->subc->number = 121;
  e_1->subc->name = "111name";
  e_2->subc->name = "121name";
  auto e_11 =
      make_shared<ydktest_sanity::Runner::InbtwList::Ldata::Subc::SubcSubl1>();
  auto e_12 =
      make_shared<ydktest_sanity::Runner::InbtwList::Ldata::Subc::SubcSubl1>();
  e_11->number = 111;
  e_12->number = 112;
  e_11->name = "111name";
  e_12->name = "112name";
  e_1->subc->subc_subl1.extend({e_11, e_12});
  auto e_21 =
      make_shared<ydktest_sanity::Runner::InbtwList::Ldata::Subc::SubcSubl1>();
  auto e_22 =
      make_shared<ydktest_sanity::Runner::InbtwList::Ldata::Subc::SubcSubl1>();
  e_21->number = 121;
  e_22->number = 122;
  e_21->name = "121name";
  e_22->name = "122name";
  e_2->subc->subc_subl1.extend({e_21, e_22});
  r_1->inbtw_list->ldata.extend({e_1, e_2});

  r_1->leaf_ref->ref_one_name = "1.2.3.4";
  r_1->leaf_ref->ref_two_sub1_number = 21;
  r_1->leaf_ref->ref_three_sub1_sub2_number =
      r_1->three->sub1->sub2->number.get();
  r_1->leaf_ref->one->name_of_one = "1.2.3.4";
  r_1->leaf_ref->one->two->self_ref_one_name = "1.2.3.4";
  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  auto r_filter = make_unique<ydktest_sanity::Runner>();
  auto r_read = crud.read(provider, *r_filter);
  ydktest_sanity::Runner* r_2 =
      dynamic_cast<ydktest_sanity::Runner*>(r_read.get());
  REQUIRE(r_1->one->name == r_2->one->name);
  REQUIRE(r_1->one->number == r_2->one->number);

  auto ldata0 = dynamic_cast<ydktest_sanity::Runner::InbtwList::Ldata*>(
      r_2->inbtw_list->ldata[0].get());
  auto ldata1 = dynamic_cast<ydktest_sanity::Runner::InbtwList::Ldata*>(
      r_2->inbtw_list->ldata[1].get());
  auto subl00 =
      dynamic_cast<ydktest_sanity::Runner::InbtwList::Ldata::Subc::SubcSubl1*>(
          ldata0->subc->subc_subl1[0].get());
  REQUIRE(e_1->number == ldata0->number);
  REQUIRE(e_1->name == ldata0->name);
  REQUIRE(e_2->number == ldata1->number);
  REQUIRE(e_2->name == ldata1->name);
  REQUIRE(e_11->number == subl00->number);
  REQUIRE(e_11->name == subl00->name);
}

TEST_CASE("aug_one_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  r_1->one->one_aug->number = 1;
  r_1->one->one_aug->name = "one";
  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  // READ
  auto r_filter = make_unique<ydktest_sanity::Runner>();
  auto r_read = crud.read(provider, *r_filter);
  REQUIRE(r_read != nullptr);
  ydktest_sanity::Runner* r_2 =
      dynamic_cast<ydktest_sanity::Runner*>(r_read.get());
  REQUIRE(r_1->one->one_aug->number == r_2->one->one_aug->number);
  REQUIRE(r_1->one->one_aug->name == r_2->one->one_aug->name);

  // UPDATE
  r_1->one->one_aug->number = 10;
  r_1->one->one_aug->name = "onenone";
  reply = crud.update(provider, *r_1);
  REQUIRE(reply);
}

TEST_CASE("aug_onelist_pos") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_shared<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto one_aug_list =
      make_unique<ydktest_sanity::Runner::OneList::OneAugList>();
  auto e_1 = make_shared<ydktest_sanity::Runner::OneList::OneAugList::Ldata>();
  auto e_2 = make_shared<ydktest_sanity::Runner::OneList::OneAugList::Ldata>();
  e_1->number = 1;
  e_1->name = "1name";
  e_2->number = 2;
  e_2->name = "2name";
  one_aug_list->ldata.extend({e_1, e_2});
  one_aug_list->enabled = true;
  reply = crud.create(provider, *one_aug_list);
  REQUIRE(reply);

  // READ
  auto r_filter = make_unique<ydktest_sanity::Runner>();
  auto one_aug_list_filter = r_filter->one_list->one_aug_list;
  auto r_read = crud.read(provider, *one_aug_list_filter);
  REQUIRE(r_read != nullptr);

  auto r_2 =
      dynamic_cast<ydktest_sanity::Runner::OneList::OneAugList*>(r_read.get());
  auto ldata =
      dynamic_cast<ydktest_sanity::Runner::OneList::OneAugList::Ldata*>(
          r_2->ldata[0].get());
  REQUIRE(e_1->name == ldata->name);
  REQUIRE(e_1->number == ldata->number);
}

TEST_CASE("parent_empty") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  r_1->ytypes->built_in_t->emptee = Empty();
  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  // READ
  auto r_filter = make_unique<ydktest_sanity::Runner>();
  auto r_read = crud.read(provider, *r_filter);
  REQUIRE(r_read != nullptr);
}

TEST_CASE("aug_leaf") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  r_1->one->augmented_leaf = "test";
  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  // READ
  auto r_filter = make_unique<ydktest_sanity::Runner>();
  auto r_read = crud.read(provider, *r_filter);
  REQUIRE(r_read != nullptr);
  ydktest_sanity::Runner* r_2 =
      dynamic_cast<ydktest_sanity::Runner*>(r_read.get());
  REQUIRE(r_2->one->augmented_leaf == r_1->one->augmented_leaf);
}

TEST_CASE("oc_pattern") {
  ydk::path::Repository repo{TEST_HOME};
  NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  ydk::path::RootSchemaNode& schema = provider.get_session().get_root_schema();
  ydk::path::Codec s{};

  auto& oc = schema.create_datanode("oc-pattern:oc-A[a='xyz']", "");
  oc.create_datanode("B/b", "xyz");
  auto x = s.encode(oc, ydk::EncodingFormat::XML, false);
  REQUIRE(x ==
          "<oc-A "
          "xmlns=\"http://cisco.com/ns/yang/oc-pattern\"><a>xyz</a><B><b>xyz</"
          "b></B></oc-A>");

  oc_pattern::OcA o{};
  o.a = "xyz";
  o.b->b = "xyz";

  bool reply = crud.create(provider, o);
  REQUIRE(reply);

  oc_pattern::OcA o_f{};
  o_f.a = "xyz";
  auto d = crud.read(provider, o_f);
  REQUIRE((*d) == o);

  reply = crud.delete_(provider, o_f);
  REQUIRE(reply);
}

TEST_CASE("mtus") {
  NetconfServiceProvider provider{"127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto mt = make_unique<ydktest_sanity::Runner::Mtus::Mtu>();
  mt->owner = "test";
  mt->mtu = 1200;
  reply = crud.create(provider, *mt);
  REQUIRE(reply);

  // READ
  auto r_filter = make_unique<ydktest_sanity::Runner>();
  auto mt_filter = make_shared<ydktest_sanity::Runner::Mtus::Mtu>();
  mt_filter->owner = "test";
  r_filter->mtus->mtu.append(mt_filter);
  auto mt_read = crud.read(provider, *mt_filter);
  REQUIRE(mt_read != nullptr);
  auto mtr = dynamic_cast<ydktest_sanity::Runner::Mtus::Mtu*>(mt_read.get());
  REQUIRE(mtr->mtu.value == "1200");

  // DELETE
  reply = crud.delete_(provider, *mt);
  REQUIRE(reply);

  // READ
  auto r_read = crud.read(provider, *mt_filter);
  REQUIRE(r_read == nullptr);
}

TEST_CASE("passive") {
  NetconfServiceProvider provider{"127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  // DELETE
  auto r_1 = make_unique<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  auto passive = make_shared<ydktest_sanity::Runner::Passive>();
  passive->name = "abc";
  auto i = make_shared<ydktest_sanity::Runner::Passive::Interfac>();
  i->test = "xyz";
  passive->interfac.append(i);
  passive->testc->xyz =
      make_shared<ydktest_sanity::Runner::Passive::Testc::Xyz>();
  passive->testc->xyz->parent = passive.get();
  passive->testc->xyz->xyz = 25;
  r_1->passive.append(passive);

  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  // READ
  auto r_filter = make_unique<ydktest_sanity::Runner>();
  auto r_read = crud.read(provider, *r_filter);
  REQUIRE(r_read != nullptr);

  // DELETE
  r_1 = make_unique<ydktest_sanity::Runner>();
  reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);
}

TEST_CASE("inner_pres") {
  NetconfServiceProvider provider{"127.0.0.1", "admin", "admin", 12022};
  CrudService crud{};

  ydk::path::RootSchemaNode& schema = provider.get_session().get_root_schema();
  ydk::path::Codec s{};

  auto& r = schema.create_datanode("ydktest-sanity:runner");
  r.create_datanode("outer/inner");
  auto x = s.encode(r, ydk::EncodingFormat::XML, false);
  REQUIRE(x ==
          "<runner "
          "xmlns=\"http://cisco.com/ns/yang/ydktest-sanity\"><outer><inner/></"
          "outer></runner>");

  auto r_1 = make_shared<ydktest_sanity::Runner>();
  bool reply = crud.delete_(provider, *r_1);
  REQUIRE(reply);

  // CREATE
  r_1->outer->inner = make_shared<ydktest_sanity::Runner::Outer::Inner>();
  r_1->outer->inner->parent = r_1->outer.get();
  reply = crud.create(provider, *r_1);
  REQUIRE(reply);

  // READ
  ydktest_sanity::Runner f{};
  auto r_read = crud.read(provider, f);
  REQUIRE(r_read != nullptr);
  REQUIRE(*r_read == *r_1);
}
