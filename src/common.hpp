#pragma once

#include <cstddef>

template <class T> class Point {
public:
  T x, y;
  Point(T x, T y) : x(x), y(y) {};
  Point() : x(0), y(0) {}
};

void sort_around_center(Point<float>* polygon, size_t polygon_size, Point<float> center);
bool contain_point(Point<float> *polygon, size_t polygon_size,
                   const Point<float> &point);
bool contain_segment(Point<float> *polygon, size_t polygon_size,
                     const Point<float> &a, const Point<float> &b);

float rotation(const Point<float> &o, const Point<float> &a, const Point<float> &b) noexcept;
