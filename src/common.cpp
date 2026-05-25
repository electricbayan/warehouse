#include "common.hpp"
#include <algorithm>
#include <cmath>

float EPS = 1e-9f;

float rotation(const Point<float> &o, const Point<float> &a, const Point<float> &b) noexcept {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

bool on_segment(const Point<float> &p, const Point<float> &a,
                const Point<float> &b) {
  return std::fabs(rotation(a, b, p)) <= EPS &&
         p.x >= std::min(a.x, b.x) - EPS && p.x <= std::max(a.x, b.x) + EPS &&
         p.y >= std::min(a.y, b.y) - EPS && p.y <= std::max(a.y, b.y) + EPS;
}

bool point_on_polygon_edge(Point<float> *polygon, size_t polygon_size,
                           const Point<float> &point) {
  for (size_t i = 0; i < polygon_size; i++) {
    const Point<float> &a = polygon[i];
    const Point<float> &b = polygon[(i + 1) % polygon_size];
    if (on_segment(point, a, b)) {
      return true;
    }
  }
  return false;
}

bool same_line(const Point<float> &a, const Point<float> &b,
               const Point<float> &c, const Point<float> &d) {
  return std::fabs(rotation(c, d, a)) <= EPS && std::fabs(rotation(c, d, b)) <= EPS;
}

bool segments_intersect(const Point<float> &a, const Point<float> &b,
                        const Point<float> &c, const Point<float> &d) {
  const float d1 = rotation(c, d, a);
  const float d2 = rotation(c, d, b);
  const float d3 = rotation(a, b, c);
  const float d4 = rotation(a, b, d);

  if (((d1 > EPS && d2 < -EPS) || (d1 < -EPS && d2 > EPS)) &&
      ((d3 > EPS && d4 < -EPS) || (d3 < -EPS && d4 > EPS))) {
    return true;
  }

  if (std::fabs(d1) <= EPS && on_segment(a, c, d)) {
    return true;
  }
  if (std::fabs(d2) <= EPS && on_segment(b, c, d)) {
    return true;
  }
  if (std::fabs(d3) <= EPS && on_segment(c, a, b)) {
    return true;
  }
  if (std::fabs(d4) <= EPS && on_segment(d, a, b)) {
    return true;
  }
  return false;
}

bool contain_point(Point<float> *polygon, size_t polygon_size,
                   const Point<float> &point) {
  if (point_on_polygon_edge(polygon, polygon_size, point)) {
    return true;
  }

  bool inside = false;
  for (size_t i = 0, j = polygon_size - 1; i < polygon_size; j = i++) {
    const Point<float> &pi = polygon[i];
    const Point<float> &pj = polygon[j];

    if ((pi.y > point.y) != (pj.y > point.y)) {
      const float x_intersect =
          (pj.x - pi.x) * (point.y - pi.y) / (pj.y - pi.y) + pi.x;
      if (point.x < x_intersect) {
        inside = !inside;
      }
    }
  }
  return inside;
}

bool contain_segment(Point<float> *polygon, size_t polygon_size,
                     const Point<float> &a, const Point<float> &b) {

  for (size_t i = 0; i < polygon_size; i++) {
    const Point<float> &p = polygon[i];
    const Point<float> &q = polygon[(i + 1) % polygon_size];
    if (!segments_intersect(a, b, p, q)) {
      continue;
    }
    if (same_line(a, b, p, q) && on_segment(a, p, q) && on_segment(b, p, q)) {
      continue;
    }
    return false;
  }
  return true;
}

void sort_around_center(Point<float> *polygon, size_t polygon_size,
                        Point<float> center) {
  std::sort(polygon, polygon + polygon_size,
            [&](const Point<float> &a, const Point<float> &b) {
              float angleA = std::atan2(a.y - center.y, a.x - center.x);
              float angleB = std::atan2(b.y - center.y, b.x - center.x);
              return angleA < angleB;
            });
}
