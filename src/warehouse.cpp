#include "warehouse.hpp"


class Shelf {
public:
  Point<float> left_bottom, right_top, right_bottom, left_top;
  float angle;
  const char *name;
  size_t height;
  Shelf(const char *name, Point<float> left_bottom, Point<float> right_top, Point<float> right_bottom,
    Point<float> left_top, size_t height)
      : name(name), left_bottom(left_bottom), right_top(right_top), height(height), right_bottom(right_bottom),
      left_top(left_top) {

      }
};

class Warehouse {

public:
  size_t points_count;
  Point<float> *points;
  const char *name;
  Warehouse(const char *name): name(name) {
    points_count = 0;
    points = nullptr;
  }

  ~Warehouse() { delete[] points; }

  void add_points(Point<float> *p, size_t size) {

    if (size < 3) {
      throw std::logic_error("Warehouse must contain at least 3 points");
    }
    bool all_in_line = true;
    for (size_t i = 0; i < size - 2; i++) {
      if (!is_line(p[i], p[i + 1], p[i + 3])) {
        all_in_line = false;
        break;
      }
    }

    if (all_in_line) {
      throw std::logic_error("All points allocated in line");
    }

    Point<float> *tmp = new Point<float>[points_count + size];
    for (size_t i = 0; i < points_count; i++) {
      tmp[i] = points[i];
    }
    delete[] points;
    for (size_t i = points_count; i < points_count + size; i++) {
      tmp[i] = p[i - points_count];
    }
    points = tmp;
    points_count += size;
  }

  bool contain_point(Shelf shelf) {

  }
};

class Item {
public:
  const char *name;
  size_t height;
  Point<float> *coords;
};
