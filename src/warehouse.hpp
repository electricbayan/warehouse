#pragma once

#include "common.hpp"
#include <iostream>
#include <string>

class Shelf {
public:
  Point<float> *points;
  float angle;
  std::string name;
  size_t height;
  Point<float> center;
  Shelf( std::string name, size_t height, Point<float> left_bottom, Point<float> right_top,
        Point<float> right_bottom, Point<float> left_top);

  Shelf(const char *name, Point<float> left_bottom, Point<float> right_top,
        float angle);
  void update(const char *name);
  void update(size_t height);
};

class Warehouse {
  bool contain_lines(Shelf shelf);
  bool contain_lines(Point<float>* points);
  Point<float> center;

public:
  bool contain_shelf(Shelf shelf);
  bool contain_shelf(Point<float>* points);
  size_t points_count;
  Point<float> *points;
  std::string name;

  Warehouse(const std::string &name);
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
