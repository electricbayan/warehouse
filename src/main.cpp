#include "command-manager.hpp"

#include <iostream>
#include <string>

int main() {
  volkovich::CommandManager manager;
  manager.readCommand("command-list", std::cout);

  std::string line;
  while (std::getline(std::cin, line)) {
    if (!line.empty()) {
      manager.readCommand(line, std::cout);
    }
  }
}
