#define BOOST_TEST_MODULE ROUTE
#include <boost/test/included/unit_test.hpp>

#include "astar.hpp"
#include "io.hpp"
#include <string>

namespace {
  void fill_square_warehouse(Warehouse &warehouse, float size = 6.0f) {
    Point<float> pts[] = {{0, 0}, {size, 0}, {size, size}, {0, size}};
    warehouse.add_points(pts, 4);
  }

  Shelf *make_stored_shelf(Warehouse &warehouse,
                           HashMap<Shelf, Warehouse> &shelf_warehouse_map,
                           HashMap<std::string, Shelf> &shelf_map) {
    Point<float> shelf_points[] = {{2, 2}, {3, 2}, {3, 3}, {2, 3}};
    return add_shelf("shelf", shelf_points, 4, 2, warehouse, shelf_warehouse_map,
                     shelf_map);
  }

  void delete_stored_shelf(Shelf *shelf, HashMap<Shelf, Warehouse> &shelf_warehouse_map,
                           HashMap<std::string, Shelf> &shelf_map) {
    if (!shelf) {
      return;
    }
    shelf_warehouse_map.remove(*shelf);
    shelf_map.remove(shelf->name);
    delete[] shelf->points;
    delete shelf;
  }
}

BOOST_AUTO_TEST_CASE(add_shelf_stores_shelf_and_warehouse_relation) {
  Warehouse warehouse("wh");
  fill_square_warehouse(warehouse);
  HashMap<Shelf, Warehouse> shelf_warehouse_map;
  HashMap<std::string, Shelf> shelf_map;

  Shelf *shelf = make_stored_shelf(warehouse, shelf_warehouse_map, shelf_map);

  BOOST_REQUIRE(shelf != nullptr);
  BOOST_TEST(shelf_map.get("shelf") == shelf);
  BOOST_TEST(shelf_warehouse_map.get(*shelf) == &warehouse);
  BOOST_TEST(shelf->height == 2);
  BOOST_TEST(shelf->center.x == 2.5f);
  BOOST_TEST(shelf->center.y == 2.5f);

  delete_stored_shelf(shelf, shelf_warehouse_map, shelf_map);
}

BOOST_AUTO_TEST_CASE(add_item_update_and_delete_item) {
  Warehouse warehouse("wh");
  fill_square_warehouse(warehouse);
  HashMap<Shelf, Warehouse> shelf_warehouse_map;
  HashMap<std::string, Shelf> shelf_map;
  HashMap<std::string, Item> item_map;
  HashMap<Item, Shelf> item_shelf_map;
  Shelf *shelf = make_stored_shelf(warehouse, shelf_warehouse_map, shelf_map);

  Item *item = add_item("box", 1, 5, *shelf, item_map, item_shelf_map);

  BOOST_REQUIRE(item != nullptr);
  BOOST_TEST(item_map.get("box") == item);
  BOOST_TEST(item_shelf_map.get(*item) == shelf);
  BOOST_TEST(item->quantity == 5);
  BOOST_TEST(item->floor == 1);

  update_item(*item, 9);
  BOOST_TEST(item->quantity == 9);

  delete_item(item, item_map, item_shelf_map);
  BOOST_TEST(item_map.get("box") == nullptr);

  delete_stored_shelf(shelf, shelf_warehouse_map, shelf_map);
}

BOOST_AUTO_TEST_CASE(route_edge_keeps_name_destination_and_quantity) {
  RouteEdge edge{"edge-1", "wh", "box", 3, {4, 4}};

  BOOST_TEST(edge.name == "edge-1");
  BOOST_TEST(edge.warehouse_name == "wh");
  BOOST_TEST(edge.item_name == "box");
  BOOST_TEST(edge.quantity == 3);
  BOOST_TEST(edge.destination.x == 4.0f);
  BOOST_TEST(edge.destination.y == 4.0f);
}

BOOST_AUTO_TEST_CASE(make_route_builds_path_around_shelf_obstacle) {
  Warehouse warehouse("wh");
  fill_square_warehouse(warehouse);
  HashMap<Shelf, Warehouse> shelf_warehouse_map;
  HashMap<std::string, Shelf> shelf_map;
  HashMap<std::string, Item> item_map;
  HashMap<Item, Shelf> item_shelf_map;
  Shelf *shelf = make_stored_shelf(warehouse, shelf_warehouse_map, shelf_map);
  Item *item = add_item("box", 1, 5, *shelf, item_map, item_shelf_map);

  std::string error;
  Route route =
      make_route_between(warehouse, {0.5f, 0.5f}, {4.0f, 4.0f}, shelf_map,
                         shelf_warehouse_map, error);

  BOOST_TEST(error.empty());
  BOOST_REQUIRE(route.count > 0);
  BOOST_TEST(route.points[0].x == 0.5f);
  BOOST_TEST(route.points[0].y == 0.5f);
  BOOST_TEST(route.points[route.count - 1].x == 4.0f);
  BOOST_TEST(route.points[route.count - 1].y == 4.0f);
  for (size_t i = 0; i < route.count; ++i) {
    BOOST_TEST(!point_in_shelves(route.points[i], &warehouse, shelf_map, shelf_warehouse_map));
  }

  delete_item(item, item_map, item_shelf_map);
  delete_stored_shelf(shelf, shelf_warehouse_map, shelf_map);
}

BOOST_AUTO_TEST_CASE(route_quantity_decreases_only_after_successful_route) {
  Warehouse warehouse("wh");
  fill_square_warehouse(warehouse);
  HashMap<Shelf, Warehouse> shelf_warehouse_map;
  HashMap<std::string, Shelf> shelf_map;
  HashMap<std::string, Item> item_map;
  HashMap<Item, Shelf> item_shelf_map;
  Shelf *shelf = make_stored_shelf(warehouse, shelf_warehouse_map, shelf_map);
  Item *item = add_item("box", 1, 5, *shelf, item_map, item_shelf_map);
  RouteEdge edge{"edge-1", "wh", "box", 3, {4, 4}};

  std::string error;
  Route route =
      make_route_between(warehouse, {0.5f, 0.5f}, item->coords, shelf_map,
                         shelf_warehouse_map, error);
  BOOST_REQUIRE(route.count > 0);
  BOOST_REQUIRE(edge.quantity <= item->quantity);
  item->quantity -= edge.quantity;

  BOOST_TEST(item->quantity == 2);

  RouteEdge too_many{"edge-2", "wh", "box", 3, {1, 4}};
  BOOST_TEST(too_many.quantity > item->quantity);
  BOOST_TEST(item->quantity == 2);

  delete_item(item, item_map, item_shelf_map);
  delete_stored_shelf(shelf, shelf_warehouse_map, shelf_map);
}
