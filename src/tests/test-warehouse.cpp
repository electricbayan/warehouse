#define BOOST_TEST_MODULE WAREHOUSE
#include <boost/test/included/unit_test.hpp>
#include "warehouse.hpp"
#include <stdexcept>


void fill_square_warehouse(Warehouse &warehouse) {
  Point<float> pts[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  warehouse.add_points(pts, 4);
}

Shelf make_shelf(const char *name, float x1, float y1, float x2, float y2) {
  return Shelf(name, {x1, y1}, {x2, y2}, {x2, y1}, {x1, y2}, 2);
}

BOOST_AUTO_TEST_CASE(warehouse_rejects_fewer_than_three_points) {
  Warehouse warehouse("small");
  Point<float> pts[] = {{0, 0}, {1, 1}};
  BOOST_REQUIRE_THROW(warehouse.add_points(pts, 2), std::logic_error);
}

BOOST_AUTO_TEST_CASE(warehouse_line) {
  Warehouse warehouse("line");
  Point<float> pts[] = {{0, 0}, {1, 1}, {2, 2}, {3, 3}};
  BOOST_REQUIRE_THROW(warehouse.add_points(pts, 4), std::logic_error);
}

BOOST_AUTO_TEST_CASE(warehouse_ok) {
  Warehouse warehouse("ok");
  fill_square_warehouse(warehouse);
  BOOST_TEST(warehouse.points_count == 4);
  BOOST_TEST(warehouse.points != nullptr);
}

BOOST_AUTO_TEST_CASE(shelf_inside) {
  Warehouse warehouse("inside");
  fill_square_warehouse(warehouse);
  Shelf shelf = make_shelf("shelf-in", 2, 2, 4, 4);
  BOOST_TEST(warehouse.contain_shelf(shelf));
  delete[] shelf.points;
}

BOOST_AUTO_TEST_CASE(shelf_outside) {
  Warehouse warehouse("outside");
  fill_square_warehouse(warehouse);
  Shelf shelf = make_shelf("shelf-out", 9, 9, 12, 12);
  BOOST_TEST(!warehouse.contain_shelf(shelf));
  delete[] shelf.points;
}

BOOST_AUTO_TEST_CASE(shelf_near_boundary_is_inside) {
  Warehouse warehouse("boundary");
  fill_square_warehouse(warehouse);
  Shelf shelf = make_shelf("shelf-near", 1, 0.1f, 3, 1.9f);
  BOOST_TEST(warehouse.contain_shelf(shelf));
  delete[] shelf.points;
}

BOOST_AUTO_TEST_CASE(shelf_crossing_edge_is_outside) {
  Warehouse warehouse("cross");
  fill_square_warehouse(warehouse);
  Shelf shelf = make_shelf("shelf-cross", 5, 5, 15, 8);
  BOOST_TEST(!warehouse.contain_shelf(shelf));
  delete[] shelf.points;
}
