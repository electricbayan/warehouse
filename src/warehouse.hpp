#pragma once

#include "common.hpp"
#include <iostream>

class Shelf {
public:
  Point<float> *points;
  float angle;
  const char *name;
  size_t height;
  Point<float> center;
  Shelf(const char *name, Point<float> left_bottom, Point<float> right_top,
        Point<float> right_bottom, Point<float> left_top, size_t height);

  Shelf(const char *name, Point<float> left_bottom, Point<float> right_top,
        float angle);
  void update(const char *name);
  void update(size_t height);
};

class Warehouse {
  bool contain_lines(Shelf shelf);
  Point<float> center;

public:
  bool contain_shelf(Shelf shelf);
  size_t points_count;
  Point<float> *points;
  const char *name;

  Warehouse(const char *name);
  ~Warehouse();
  void add_points(Point<float> *p, size_t size);
};

class Item {
public:
  const char *name;
  size_t height;
  Point<float> *coords;
  Item(const char *name, size_t quantity, Point<float> coords);
};
