#include "io.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

using std::string;
void print_command_list() {
  std::cout << "Command List:\n0. command-list 1. make-warehouse\n2. "
               "delete-warehouse\n3. inspect-warehouse";
}

string *get_args(string input, size_t len) {
  std::stringstream ss(input);
  len = std::count(input.begin(), input.end(), ' ') + 1;
  string *args = new string[len];
  for (size_t i = 0; i < std::count(input.begin(), input.end(), ' ') + 1; i++) {
    ss >> args[i];
  }
  return args;
}

int main() {
  HashMap<Shelf, Warehouse> *shelf_warehouse_map;
  HashMap<const char*, Warehouse>* warehouse_map;
  HashMap<Item, Shelf>* item_map;
  print_command_list();
  while (true) {
    string input;
    if (std::cin >> input) {
      if (input == "0") {
        print_command_list();
        continue;
      }
      size_t len;
      string *args = get_args(input, len);
      if (input.compare(0, 14, "make-warehouse")) {
        if (len < 6) {
          std::cout << "Requires at least 6 args";
          continue;
        }
        if (len % 2 == 1) {
          std::cout << "Incorrect args number";
        }
        Point<float> *points = new Point<float>[len / 2];
        size_t p_iter = 0;
        for (size_t i = 2; i < len; i += 2) {
          Point<float> a{std::stof(args[i]), std::stof(args[i + 1])};
          points[p_iter] = a;
          p_iter++;
        }
        Warehouse wh =
            make_warehouse(args[1].c_str(), points, std::stoul(args[2]));
      } else if (input == "2") {

      } else if (input == "3") {

      } else {
        std::cout << "To view command list press 0.\nTo escape press Ctrl+C";
      }
      delete[] args;
    }
  }
}
