#pragma once

#include "common.hpp"
#include "graph.hpp"
#include "hashmap.hpp"
#include "warehouse.hpp"
#include <functional>
#include <string>

using RouteGraph = volkovich::Graph<std::string, volkovich::SipHash, std::equal_to<std::string>>;

struct RouteEdge {
  std::string name;
  std::string warehouse_name;
  std::string item_name;
  size_t quantity;
  Point<float> destination;
};

struct Route {
  Point<float>* points = nullptr;
  size_t count = 0;

  Route() = default;
  ~Route();
  Route(const Route&) = delete;
  Route& operator=(const Route&) = delete;
  Route(Route&& other) noexcept;
  Route& operator=(Route&& other) noexcept;
};

bool point_in_shelves(Point<float> point, Warehouse *wh,
                      HashMap<std::string, Shelf> &shelf_map,
                      HashMap<Shelf, Warehouse> &shelf_warehouse_map);

Route make_route(Warehouse &wh, Point<float> start_point, Item &item,
                 HashMap<std::string, Shelf> &shelf_map,
                 HashMap<Shelf, Warehouse> &shelf_warehouse_map,
                 std::string &error);

Route make_route_between(Warehouse &wh, Point<float> start_point,
                         Point<float> finish_point,
                         HashMap<std::string, Shelf> &shelf_map,
                         HashMap<Shelf, Warehouse> &shelf_warehouse_map,
                         std::string &error);
