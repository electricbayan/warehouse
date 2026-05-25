#include <iostream>
#include "io.hpp"
#include <string>
#include <sstream>
#include <algorithm>

using std::string;
void print_command_list() {
  std::cout<<"Command List:\n0. command-list 1. make-warehouse\n2. delete-warehouse\n3. inspect-warehouse";
}

string* get_args(string input) {
  std::stringstream ss(input);
  string* args = new string[std::count(input.begin(), input.end(), ' ') + 1];
  for (size_t i =0; i<std::count(input.begin(), input.end(), ' ') + 1;i++) {
    ss>>args[i];
  }
  return args;
}

int main() {
  HashMap<Shelf, Warehouse>* shelf_warehouse_map;
  print_command_list();
  while (true)
  {
    string input;
    if (std::cin>>input) {
      if (input == "0") {
        print_command_list();
        continue;
      }
      string* args = get_args();
      if (input.compare(0, 14, "make-warehouse")) {

        make_warehouse();
      } else if (input == "2") {

      } else if (input == "3") {

      } else {
        std::cout<<"To view command list press 0.\nTo escape press Ctrl+C";
      }
      delete[] args;
    }
  }

}
