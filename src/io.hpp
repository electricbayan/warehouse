#include "warehouse.hpp"
#include "hashmap.hpp"

Warehouse &make_warehouse(const char *name, Point<float> *points,
                          size_t points_count);

void delete_warehouse(Warehouse &wh);

char *inspect_warehouse(Warehouse &wh);

Shelf add_shelf(const char *name, Point<float> *points, size_t points_count,
                 size_t height, Warehouse &wh,
                 HashMap<Shelf, Warehouse> wh_sh_table);
