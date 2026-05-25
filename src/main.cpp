#include "io.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using std::string;

void print_command_list() {
  std::cout << "Command List:\n0. command-list\n1. make-warehouse\n2. "
               "delete-warehouse\n3. inspect-warehouse\n\n";
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
  HashMap<Item, Shelf> item_map;
  (void)shelf_warehouse_map;
  (void)item_map;

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
        std::cout << "Requires: make-warehouse <name> [<count>] <x1> <y1> ... "
                     "(at least 3 points)\n";
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
        delete[] output;
      } else {
        std::cout<<"Warehouse doesn't exist\n";
      }
    } else if (len && args[0].compare(0, 16, "delete-warehouse") == 0) {
      Warehouse *wh = warehouse_map.get(args[1]);
      if (wh) {
        warehouse_map.remove(wh->name);
        wh->~Warehouse();
      } else {
        std::cout<<"Warehouse doesn't exist\n";
      }
    } else {
      std::cout << "To view command list press 0.\nTo escape press Ctrl+C\n";
    }

    delete[] args;
  }
}
