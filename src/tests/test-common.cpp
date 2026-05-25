#define BOOST_TEST_MODULE COMMON
#include <boost/test/included/unit_test.hpp>
#include "common.hpp"

BOOST_AUTO_TEST_CASE(is_line_detects_collinear_points) {
  Point<float> a{0, 0};
  Point<float> b{1, 1};
  Point<float> c{2, 2};
  BOOST_TEST(!rotation(a, b, c));
}

BOOST_AUTO_TEST_CASE(is_line_rejects_non_collinear_points) {
  Point<float> a{0, 0};
  Point<float> b{1, 0};
  Point<float> c{0, 1};
  BOOST_TEST(rotation(a, b, c));
}

BOOST_AUTO_TEST_CASE(contain_point_inside_square) {
  Point<float> square[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  BOOST_TEST(contain_point(square, 4, {5, 5}));
}

BOOST_AUTO_TEST_CASE(contain_point_outside_square) {
  Point<float> square[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  BOOST_TEST(!contain_point(square, 4, {15, 5}));
}

BOOST_AUTO_TEST_CASE(contain_point_on_boundary_counts_as_inside) {
  Point<float> square[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  BOOST_TEST(contain_point(square, 4, {0, 5}));
}

BOOST_AUTO_TEST_CASE(contain_segment_inside_square) {
  Point<float> square[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  BOOST_TEST(contain_segment(square, 4, {2, 2}, {4, 4}));
}

BOOST_AUTO_TEST_CASE(contain_segment_rejects_outside_endpoints) {
  Point<float> square[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  BOOST_TEST(!contain_segment(square, 4, {2, 2}, {15, 15}));
}

BOOST_AUTO_TEST_CASE(contain_segment_rejects_segment_crossing_boundary) {
  Point<float> square[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  BOOST_TEST(!contain_segment(square, 4, {5, 5}, {15, 5}));
}

BOOST_AUTO_TEST_CASE(sort_around_center_orders_vertices) {
  Point<float> polygon[] = {{0, 10}, {10, 0}, {0, 0}, {10, 10}};
  Point<float> center{5, 5};
  sort_around_center(polygon, 4, center);

  bool has_increasing_angles = true;
  for (size_t i = 0; i < 4; i++) {
    const Point<float> &a = polygon[i];
    const Point<float> &b = polygon[(i + 1) % 4];
    const float cross =
        (a.x - center.x) * (b.y - center.y) - (a.y - center.y) * (b.x - center.x);
    if (cross < -1e-6f) {
      has_increasing_angles = false;
      break;
    }
  }
  BOOST_TEST(has_increasing_angles);
}
