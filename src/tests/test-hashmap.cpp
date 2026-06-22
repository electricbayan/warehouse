#define BOOST_TEST_MODULE HASHMAP
#include <boost/test/included/unit_test.hpp>
#include "hashmap.hpp"
#include <cstdio>
#include <stdexcept>

Shelf make_named_shelf(const char *name) {
  return Shelf(name, {1, 1}, {2, 2}, {2, 1}, {1, 2}, 1);
}


BOOST_AUTO_TEST_CASE(insert_and_get_by_shelf_name) {
  HashMap<Shelf, Warehouse> map(8);
  Warehouse warehouse("ozon");
  Shelf shelf = make_named_shelf("products");

  map.insert(shelf, &warehouse);

  BOOST_TEST(map.get("products") == &warehouse);
  BOOST_TEST(map.get(shelf) == &warehouse);
  BOOST_TEST(map.count() == 1);

  delete[] shelf.points;
}

BOOST_AUTO_TEST_CASE(insert_updates_existing_key) {
  HashMap<Shelf, Warehouse> map(8);
  Warehouse first("first");
  Warehouse second("second");
  Shelf shelf = make_named_shelf("shared");

  map.insert(shelf, &first);
  map.insert(shelf, &second);

  BOOST_TEST(map.get("shared") == &second);
  BOOST_TEST(map.count() == 1);

  delete[] shelf.points;
}

BOOST_AUTO_TEST_CASE(remove_existing_key) {
  HashMap<Shelf, Warehouse> map(8);
  Warehouse warehouse("ozon");
  Shelf shelf = make_named_shelf("to-remove");

  map.insert(shelf, &warehouse);
  BOOST_TEST(map.remove(shelf));
  BOOST_TEST(map.get("to-remove") == nullptr);
  BOOST_TEST(map.count() == 0);

  delete[] shelf.points;
}

BOOST_AUTO_TEST_CASE(insert_null_warehouse_throws) {
  HashMap<Shelf, Warehouse> map(8);
  Shelf shelf = make_named_shelf("broken");

  BOOST_REQUIRE_THROW(map.insert(shelf, nullptr), std::invalid_argument);

  delete[] shelf.points;
}

BOOST_AUTO_TEST_CASE(get_missing_key_returns_null) {
  HashMap<Shelf, Warehouse> map(8);
  BOOST_TEST(map.get("missing") == nullptr);
}

BOOST_AUTO_TEST_CASE(rehash_keeps_all_entries) {
  HashMap<Shelf, Warehouse> map(8);
  Warehouse *warehouses[20];
  Shelf *shelves[20];
  char names[20][16];

  for (size_t i = 0; i < 20; i++) {
    char warehouse_name[16];
    std::snprintf(names[i], sizeof(names[i]), "shelf-%zu", i);
    std::snprintf(warehouse_name, sizeof(warehouse_name), "wh-%zu", i);
    warehouses[i] = new Warehouse(warehouse_name);
    shelves[i] = new Shelf(make_named_shelf(names[i]));
    map.insert(*shelves[i], warehouses[i]);
  }

  BOOST_TEST(map.count() == 20);
  for (size_t i = 0; i < 20; i++) {
    BOOST_TEST(map.get(names[i]) == warehouses[i]);
  }

  for (size_t i = 0; i < 20; i++) {
    delete[] shelves[i]->points;
    delete shelves[i];
    delete warehouses[i];
  }
}
