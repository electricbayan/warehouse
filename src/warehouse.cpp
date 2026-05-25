#include "warehouse.hpp"

Shelf::Shelf(std::string name, size_t height, Point<float> left_bottom,
             Point<float> right_top, Point<float> right_bottom,
             Point<float> left_top)
    : points(nullptr), angle(0), name(name), height(height), center() {
  points = new Point<float>[4]{left_bottom, left_top, right_top, right_bottom};
}

Warehouse::Warehouse(const std::string &name) : name(name) {
  points_count = 0;
  points = nullptr;
}

Warehouse::~Warehouse() { delete[] points; }

void Warehouse::add_points(Point<float> *p, size_t size) {

  if (size < 3) {
    throw std::logic_error("Warehouse must contain at least 3 points");
  }
  bool all_in_line = true;
  for (size_t i = 0; i < size - 2; i++) {
    if (rotation(p[i], p[i + 1], p[i + 2]) != 0) {
      all_in_line = false;
      break;
    }
  }

  if (all_in_line) {
    throw std::logic_error("All points allocated in line");
  }

  Point<float> *tmp;
  float center_x = 0, center_y = 0;
  tmp = new Point<float>[points_count + size];
  for (size_t i = 0; i < points_count; i++) {
    tmp[i] = points[i];
    center_x += points[i].x;
    center_y += points[i].y;
  }
  delete[] points;
  for (size_t i = points_count; i < points_count + size; i++) {
    tmp[i] = p[i - points_count];
    center_x += p[i - points_count].x;
    center_y += p[i - points_count].y;
  }
  points = tmp;
  points_count += size;
  center = Point<float>{center_x / points_count, center_y / points_count};
  sort_around_center(points, points_count, center);
}

bool Warehouse::contain_shelf(Shelf shelf) {
  for (size_t i = 0; i < 4; i++) {
    if (!contain_point(points, points_count, shelf.points[i])) {
      return false;
    }
  }
  return contain_lines(shelf);
}

bool Warehouse::contain_shelf(Point<float> *shelf_points) {
  float sum_x = 0, sum_y = 0;
  for (size_t i = 0; i < 4; i++) {
    sum_x += shelf_points[i].x;
    sum_y += shelf_points[i].y;
  }
  sort_around_center(shelf_points, 4, Point<float>{sum_x / 4, sum_y / 4});
  for (size_t i = 0; i < 4; i++) {
    if (!contain_point(this->points, points_count, shelf_points[i])) {
      return false;
    }
  }
  return contain_lines(shelf_points);
}

bool Warehouse::contain_lines(Shelf shelf) {

  for (size_t i = 0; i < 4; i++) {
    size_t j = i + 1;
    if (i == 3) {
      j = 0;
    }
    if (!contain_segment(points, points_count, shelf.points[i],
                         shelf.points[j])) {
      return false;
    }
  }
  return true;
}

bool Warehouse::contain_lines(Point<float> *shelf_points) {

  for (size_t i = 0; i < 4; i++) {
    size_t j = i + 1;
    if (i == 3) {
      j = 0;
    }
    if (!contain_segment(this->points, points_count, shelf_points[i],
                         shelf_points[j])) {
      return false;
    }
  }
  return true;
}
