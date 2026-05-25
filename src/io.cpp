#include "io.hpp"
#include <cstdio>
#include <iostream>

Warehouse *make_warehouse(const std::string &name, Point<float> *points,
                          size_t points_count,
                          HashMap<std::string, Warehouse> *wh_map) {
  Warehouse *wh = new Warehouse(name);
  wh->add_points(points, points_count);
  wh_map->insert(name, wh);
  return wh;
}

void delete_warehouse(Warehouse *wh, HashMap<std::string, Warehouse> *wh_map) {
  if (!wh) {
    return;
  }
  wh_map->remove(wh->name);
  delete wh;
}

char *inspect_warehouse(Warehouse &wh) {
  int len = std::snprintf(nullptr, 0,
                          "========Name: %s\nPoints count: %zu\n========",
                          wh.name.c_str(), wh.points_count);
  char *result = new char[len + 1];

  std::snprintf(result, len + 1,
                "========Name: %s\nPoints count: %zu\n========", wh.name.c_str(),
                wh.points_count);
  return result;
}

Shelf add_shelf(const char *name, Point<float> *points, size_t points_count,
                size_t height, Warehouse &wh,
                HashMap<Shelf, Warehouse> &wh_sh_table) {
  if (points_count != 4) {
    throw std::logic_error("Must be 4 points");
  }
  float sum_x = 0, sum_y = 0;
  for (size_t i = 0; i < 4; i++) {
    sum_x += points[i].x;
    sum_y += points[i].y;
  }
  Point<float> center{sum_x / 4, sum_y / 4};
  sort_around_center(points, 4, center);
  Shelf sh(name, points[0], points[1], points[2], points[3], height);
  if (!wh.contain_shelf(sh)) {
    delete[] sh.points;
    throw std::logic_error("Shelf outside the warehouse");
  }
  wh_sh_table.insert(sh, &wh);
  return sh;
}

void delete_shelf(Shelf &sh, HashMap<Shelf, Warehouse> &wh_sh_table) {
  wh_sh_table.remove(sh);
  delete[] sh.points;
  sh.points = nullptr;
}

char *inspect_shelf(Shelf &sh) {
  int len = std::snprintf(nullptr, 0, "========Name: %s\nHeight: %zu\n========",
                          sh.name, sh.height);
  char *result = new char[len + 1];

  std::snprintf(result, len + 1, "========Name: %s\nHeight: %zu\n========",
                sh.name, sh.height);
  return result;
}

Shelf update_shelf();
