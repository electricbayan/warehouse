#include "io.hpp"
#include "graph.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using std::string;

using RouteGraph = volkovich::Graph<std::string, volkovich::SipHash, std::equal_to<std::string>>;

void print_command_list() {
  std::cout
      << "Command List:\n0. command-list\n1. make-warehouse <name> "
         "<points-count> <points>\n2. "
         "delete-warehouse <name>\n3. inspect-warehouse <name>\n\n4. "
         "add-shelf <warehouse-name> <shelf-name> <floors> <points>\n5. "
         "delete-shelf <name>\n6. inspect-shelf <name>\n7. update-shelf\n8. "
         "add-item <warehouse-name> <shelf-name> <floor> <item-name> <quantity>\n9. "
         "delete-item <warehouse-name> <shelf-name> <item-name>\n10. "
         "update-item <warehouse-name> <shelf-name> <item-name> <quantity>\n11. "
         "inspect-item <warehouse-name> <shelf-name> <item-name>\n12. "
         "add-edge <warehouse-name> <shelf-name> <item-name> <x> <y>\n13. "
         "delete-edge <warehouse-name> <shelf-name> <item-name> <x> <y>\n\n";
}

std::string point_vertex(Point<float> point) {
  std::ostringstream out;
  out << point.x << ',' << point.y;
  return out.str();
}

int edge_weight(Point<float> from, Point<float> to) {
  const float dx = from.x - to.x;
  const float dy = from.y - to.y;
  return static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy)));
}

string *get_args(const string &input, size_t &len) {
  std::stringstream ss(input);
  len = 0;
  string token;
  while (ss >> token) {
    len++;
  }

  string *args = new string[len];
  ss.clear();
  ss.str(input);
  for (size_t i = 0; i < len; i++) {
    ss >> args[i];
  }
  return args;
}

bool parse_points_count(size_t len, string *args, size_t &points_count,
                        size_t &coord_start) {
  if (len < 9) {
    return false;
  }

  try {
    const size_t declared = std::stoul(args[2]);
    const size_t from_coords = (len - 3) / 2;
    if ((len - 3) % 2 == 0 && declared >= 3 && declared == from_coords) {
      points_count = declared;
      coord_start = 3;
      return true;
    }
  } catch (const std::exception &) {
  }

  if ((len - 2) % 2 != 0) {
    return false;
  }

  points_count = (len - 2) / 2;
  coord_start = 2;
  return points_count >= 3;
}

int main() {
  HashMap<Shelf, Warehouse> shelf_warehouse_map;
  HashMap<std::string, Warehouse> warehouse_map;
  HashMap<std::string, Shelf> shelf_map;
  HashMap<std::string, Item> item_map;
  HashMap<Item, Shelf> item_shelf_map;
  RouteGraph route_graph;
  (void)shelf_warehouse_map;

  print_command_list();
  while (true) {
    string line;
    if (!std::getline(std::cin, line)) {
      break;
    }
    if (line.empty()) {
      continue;
    }

    if (line == "0") {
      print_command_list();
      continue;
    }

    size_t len = 0;
    string *args = get_args(line, len);

    if (len && args[0].compare(0, 14, "make-warehouse") == 0) {
      size_t points_count = 0;
      size_t coord_start = 0;
      if (!parse_points_count(len, args, points_count, coord_start)) {
        std::cout << "Wrong args\n";
        delete[] args;
        continue;
      }

      Point<float> *points = new Point<float>[points_count];
      for (size_t i = coord_start; i < coord_start + points_count * 2; i += 2) {
        points[(i - coord_start) / 2] = {std::stof(args[i]),
                                         std::stof(args[i + 1])};
      }

      make_warehouse(args[1], points, points_count, &warehouse_map);
      delete[] points;
    } else if (len && args[0].compare(0, 17, "inspect-warehouse") == 0) {
      Warehouse *wh = warehouse_map.get(args[1]);
      if (wh) {
        char *output = inspect_warehouse(*wh);
        std::cout << output << '\n';
      } else {
        std::cout << "Warehouse doesn't exist\n";
      }
    } else if (len && args[0].compare(0, 16, "delete-warehouse") == 0) {
      Warehouse *wh = warehouse_map.get(args[1]);
      if (wh) {
        warehouse_map.remove(wh->name);
        delete wh;
      } else {
        std::cout << "Warehouse doesn't exist\n";
      }
    } else if (len && args[0].compare(0, 9, "add-shelf") == 0) {
      if (len != 12) {
        std::cout << "Wrong args\n";
        delete[] args;
        continue;
      }
      Point<float> *points = new Point<float>[4];
      for (size_t i = 3; i < 11; i += 2) {
        points[(i - 3) / 2] = {std::stof(args[i]), std::stof(args[i + 1])};
      }
      Warehouse *wh = warehouse_map.get(args[1]);
      if (!wh) {
        std::cout << "Wrong warehouse\n";
        delete[] points;
        delete[] args;
        continue;
      }
      if (!wh->contain_shelf(points)) {
        std::cout << "Shelf outside\n";
        delete[] points;
        delete[] args;
        continue;
      }
      add_shelf(args[2], points, 4, std::stoull(args[3]), *wh,
                shelf_warehouse_map, shelf_map);
      delete[] points;
    } else if (len && args[0].compare(0, 12, "delete-shelf") == 0) {
      if (len != 2) {
        std::cout << "Invalid arg number\n";
        delete[] args;
        continue;
      }
      Shelf *sh = shelf_map.get(args[1]);
      if (sh) {
        shelf_warehouse_map.remove(*sh);
        shelf_map.remove(sh->name);
        delete[] sh->points;
        delete sh;
      } else {
        std::cout << "Shelf doesn't exist\n";
        delete[] args;
        continue;
      }

    } else if (len && args[0].compare(0, 13, "inspect-shelf") == 0) {
      if (len != 2) {
        std::cout << "Invalid arg number\n";
        delete[] args;
        continue;
      }
      Shelf *sh = shelf_map.get(args[1]);
      if (!sh) {
        std::cout << "Shelf doesn't exist\n";
        delete[] args;
        continue;
      }
      Warehouse *wh = shelf_warehouse_map.get(*sh);
      char *output = inspect_shelf(*sh, *wh);
      std::cout << output;
      delete[] output;
    } else if (len && args[0].compare(0, 8, "add-item") == 0) {
      if (len != 6) {
        std::cout << "Invalid arg number\n";
        delete[] args;
        continue;
      }
      Warehouse *wh = warehouse_map.get(args[1]);
      Shelf *sh = shelf_map.get(args[2]);
      if (!wh) {
        std::cout << "Warehouse doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!sh || shelf_warehouse_map.get(*sh) != wh) {
        std::cout << "Shelf doesn't exist\n";
        delete[] args;
        continue;
      }
      try {
        add_item(args[4], std::stoull(args[3]), std::stoull(args[5]), *sh,
                 item_map, item_shelf_map);
      } catch (const std::exception &e) {
        std::cout << e.what() << '\n';
      }
    } else if (len && args[0].compare(0, 11, "delete-item") == 0) {
      if (len != 4) {
        std::cout << "Invalid arg number\n";
        delete[] args;
        continue;
      }
      Warehouse *wh = warehouse_map.get(args[1]);
      Shelf *sh = shelf_map.get(args[2]);
      Item *item = item_map.get(args[3]);
      if (!wh) {
        std::cout << "Warehouse doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!sh || shelf_warehouse_map.get(*sh) != wh) {
        std::cout << "Shelf doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!item || item_shelf_map.get(*item) != sh) {
        std::cout << "Item doesn't exist\n";
        delete[] args;
        continue;
      }
      delete_item(item, item_map, item_shelf_map);
    } else if (len && args[0].compare(0, 11, "update-item") == 0) {
      if (len != 5) {
        std::cout << "Invalid arg number\n";
        delete[] args;
        continue;
      }
      Warehouse *wh = warehouse_map.get(args[1]);
      Shelf *sh = shelf_map.get(args[2]);
      Item *item = item_map.get(args[3]);
      if (!wh) {
        std::cout << "Warehouse doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!sh || shelf_warehouse_map.get(*sh) != wh) {
        std::cout << "Shelf doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!item || item_shelf_map.get(*item) != sh) {
        std::cout << "Item doesn't exist\n";
        delete[] args;
        continue;
      }
      try {
        update_item(*item, std::stoull(args[4]));
      } catch (const std::exception &e) {
        std::cout << e.what() << '\n';
      }
    } else if (len && args[0].compare(0, 12, "inspect-item") == 0) {
      if (len != 4) {
        std::cout << "Invalid arg number\n";
        delete[] args;
        continue;
      }
      Warehouse *wh = warehouse_map.get(args[1]);
      Shelf *sh = shelf_map.get(args[2]);
      Item *item = item_map.get(args[3]);
      if (!wh) {
        std::cout << "Warehouse doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!sh || shelf_warehouse_map.get(*sh) != wh) {
        std::cout << "Shelf doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!item || item_shelf_map.get(*item) != sh) {
        std::cout << "Item doesn't exist\n";
        delete[] args;
        continue;
      }
      char *output = inspect_item(*item, *sh, *wh);
      std::cout << output;
      delete[] output;
    } else if (len && args[0].compare(0, 8, "add-edge") == 0) {
      if (len != 6) {
        std::cout << "Invalid arg number\n";
        delete[] args;
        continue;
      }
      Warehouse *wh = warehouse_map.get(args[1]);
      Shelf *sh = shelf_map.get(args[2]);
      Item *item = item_map.get(args[3]);
      if (!wh) {
        std::cout << "Warehouse doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!sh || shelf_warehouse_map.get(*sh) != wh) {
        std::cout << "Shelf doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!item || item_shelf_map.get(*item) != sh) {
        std::cout << "Item doesn't exist\n";
        delete[] args;
        continue;
      }
      Point<float> destination{std::stof(args[4]), std::stof(args[5])};
      if (!contain_point(wh->points, wh->points_count, destination)) {
        std::cout << "Point outside warehouse\n";
        delete[] args;
        continue;
      }
      const std::string from = item->name;
      const std::string to = point_vertex(destination);
      route_graph.addEdge(from, to, edge_weight(item->coords, destination));
    } else if (len && args[0].compare(0, 11, "delete-edge") == 0) {
      if (len != 6) {
        std::cout << "Invalid arg number\n";
        delete[] args;
        continue;
      }
      Warehouse *wh = warehouse_map.get(args[1]);
      Shelf *sh = shelf_map.get(args[2]);
      Item *item = item_map.get(args[3]);
      if (!wh) {
        std::cout << "Warehouse doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!sh || shelf_warehouse_map.get(*sh) != wh) {
        std::cout << "Shelf doesn't exist\n";
        delete[] args;
        continue;
      }
      if (!item || item_shelf_map.get(*item) != sh) {
        std::cout << "Item doesn't exist\n";
        delete[] args;
        continue;
      }
      Point<float> destination{std::stof(args[4]), std::stof(args[5])};
      const std::string from = item->name;
      const std::string to = point_vertex(destination);
      if (!route_graph.removeEdge(from, to, edge_weight(item->coords, destination))) {
        std::cout << "Edge doesn't exist\n";
      }
    } else {
      std::cout << "To view command list press 0.\nTo escape press Ctrl+C\n";
    }

    delete[] args;
  }
}
