#pragma once

#include "hashmap.hpp"
#include "warehouse.hpp"
#include <string>

Warehouse *make_warehouse(const std::string &name, Point<float> *points,
                          size_t points_count,
                          HashMap<std::string, Warehouse> *wh_map);

void delete_warehouse(Warehouse *wh, HashMap<std::string, Warehouse> *wh_map);

char *inspect_warehouse(Warehouse &wh);

Shelf add_shelf(const std::string &name, Point<float> *points, size_t points_count,
                size_t height, Warehouse &wh,
                HashMap<Shelf, Warehouse> &wh_sh_table, HashMap<std::string, Shelf>& sh_map);
char *inspect_shelf(Shelf &sh, Warehouse &wh);
