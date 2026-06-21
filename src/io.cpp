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
                          "========\nName: %s\nPoints count: %zu\n========",
                          wh.name.c_str(), wh.points_count);
  char *result = new char[len + 1];

  std::snprintf(result, len + 1,
                "========\nName: %s\nPoints count: %zu\n========", wh.name.c_str(),
                wh.points_count);
  return result;
}

Shelf add_shelf(const std::string& name, Point<float> *points, size_t points_count,
                size_t height, Warehouse &wh,
                HashMap<Shelf, Warehouse> &wh_sh_table, HashMap<std::string, Shelf>& sh_map) {
  (void)points_count;
  float sum_x = 0, sum_y = 0;
  for (size_t i = 0; i < 4; i++) {
    sum_x += points[i].x;
    sum_y += points[i].y;
  }
  Point<float> center{sum_x / 4, sum_y / 4};
  sort_around_center(points, 4, center);
  Shelf sh(name, height, points[0], points[1], points[2], points[3]);
  if (!wh.contain_shelf(sh)) {
    delete[] sh.points;
    throw std::logic_error("Shelf outside the warehouse");
  }
  wh_sh_table.insert(sh, &wh);
  sh_map.insert(name, &sh);
  return sh;
}

void delete_shelf(Shelf &sh, HashMap<Shelf, Warehouse> &wh_sh_table) {
  wh_sh_table.remove(sh);
  delete[] sh.points;
  sh.points = nullptr;
}

char *inspect_shelf(Shelf &sh,Warehouse &wh) {
  int len = std::snprintf(nullptr, 0, "========\nName: %s\nHeight: %zu\nWarehouse: %s\n========\n",
                          sh.name.c_str(), sh.height, wh.name.c_str());
  char *result = new char[len + 1];

  std::snprintf(result, len + 1, "========\nName: %s\nHeight: %zu\nWarehouse: %s\n========\n",
                sh.name.c_str(), sh.height, wh.name.c_str());
  return result;
}

Shelf update_shelf();
