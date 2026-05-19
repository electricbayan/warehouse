#include <iostream>
#include "warehouse.hpp"


int main() {
  Warehouse warehouse("OZON");

  Point<float> *warehouse_points = new Point<float>[4]{
      {0, 0},
      {1, 1},
      {1, 0},
      {0, 1},
  };
  warehouse.add_points(warehouse_points, 4);

  for (size_t i = 0; i < warehouse.points_count; i++) {
    std::cout << warehouse.points[i].x << ' ' << warehouse.points[i].y << '\n';
  }

  Shelf shelf("products", {2, 3}, {3, 5}, {1, 5}, {2, 8}, 3);

  delete[] warehouse_points;
}
