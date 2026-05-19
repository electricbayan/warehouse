#include <algorithm>
#include <cmath>

template <class T> class Point {
public:
  T x, y;
  Point(T x, T y) : x(x), y(y) {};
  Point() : x(0), y(0) {}
};

bool is_line(Point<float> a, Point<float> b, Point<float> c) noexcept {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) == 0;
}

bool contain_point(Point<float> *polygon, const Point<float> &point) {

};

void sort_around_center(Point<float> *polygon, size_t polygon_size,
                        Point<float> center) {
  std::sort(polygon, polygon + polygon_size,
            [&](const Point<float> &a, const Point<float> &b) {
              float angleA = std::atan2(a.y - center.y, a.x - center.x);

              float angleB = std::atan2(b.y - center.y, b.x - center.x);

              return angleA < angleB;
            });
};
