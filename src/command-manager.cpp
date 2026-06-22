#include "command-manager.hpp"

#include <fstream>
#include <stdexcept>

namespace volkovich {

  CommandManager::CommandManager() {
    commands_.add("0", &CommandManager::handleCommandList);
    commands_.add("command-list", &CommandManager::handleCommandList);
    commands_.add("make-warehouse", &CommandManager::handleMakeWarehouse);
    commands_.add("inspect-warehouse", &CommandManager::handleInspectWarehouse);
    commands_.add("delete-warehouse", &CommandManager::handleDeleteWarehouse);
    commands_.add("add-shelf", &CommandManager::handleAddShelf);
    commands_.add("inspect-shelf", &CommandManager::handleInspectShelf);
    commands_.add("delete-shelf", &CommandManager::handleDeleteShelf);
    commands_.add("update-shelf", &CommandManager::handleUpdateShelf);
    commands_.add("add-item", &CommandManager::handleAddItem);
    commands_.add("inspect-item", &CommandManager::handleInspectItem);
    commands_.add("delete-item", &CommandManager::handleDeleteItem);
    commands_.add("update-item", &CommandManager::handleUpdateItem);
    commands_.add("add-edge", &CommandManager::handleAddEdge);
    commands_.add("delete-edge", &CommandManager::handleDeleteEdge);
    commands_.add("make-route", &CommandManager::handleMakeRoute);
  }

  CommandManager::~CommandManager() {
    for (auto it = edge_map_.begin(); it != edge_map_.end(); ++it) {
      delete it->value;
    }
    for (auto it = item_map_.begin(); it != item_map_.end(); ++it) {
      delete it->value;
    }
    for (auto it = shelf_map_.begin(); it != shelf_map_.end(); ++it) {
      Shelf* shelf = it->value;
      if (shelf) {
        delete[] shelf->points;
        delete shelf;
      }
    }
    for (auto it = warehouse_map_.begin(); it != warehouse_map_.end(); ++it) {
      delete it->value;
    }
  }

  CommandManager::Args::~Args() {
    delete[] data;
  }

  CommandManager::Args::Args(Args&& other) noexcept
      : data(other.data), size(other.size), capacity(other.capacity) {
    other.data = nullptr;
    other.size = 0;
    other.capacity = 0;
  }

  CommandManager::Args& CommandManager::Args::operator=(Args&& other) noexcept {
    if (this != &other) {
      delete[] data;
      data = other.data;
      size = other.size;
      capacity = other.capacity;
      other.data = nullptr;
      other.size = 0;
      other.capacity = 0;
    }
    return *this;
  }

  void CommandManager::Args::push(const std::string& value) {
    if (size == capacity) {
      capacity = capacity ? capacity * 2 : 4;
      std::string* tmp = new std::string[capacity];
      for (size_t i = 0; i < size; ++i) {
        tmp[i] = data[i];
      }
      delete[] data;
      data = tmp;
    }
    data[size++] = value;
  }

  std::string& CommandManager::Args::operator[](size_t index) {
    return data[index];
  }

  const std::string& CommandManager::Args::operator[](size_t index) const {
    return data[index];
  }

  CommandManager::RouteItemStates::~RouteItemStates() {
    delete[] data;
  }

  CommandManager::RouteItemState* CommandManager::RouteItemStates::find(Item* item) {
    for (size_t i = 0; i < size; ++i) {
      if (data[i].item == item) {
        return &data[i];
      }
    }
    return nullptr;
  }

  CommandManager::RouteItemState* CommandManager::RouteItemStates::add(Item* item) {
    if (size == capacity) {
      capacity = capacity ? capacity * 2 : 4;
      RouteItemState* tmp = new RouteItemState[capacity];
      for (size_t i = 0; i < size; ++i) {
        tmp[i] = data[i];
      }
      delete[] data;
      data = tmp;
    }
    data[size] = RouteItemState{item, item->coords, 0};
    return &data[size++];
  }

  CommandManager::Args CommandManager::readArgs(std::istream& input) {
    Args args;
    std::string arg;
    while (input >> arg) {
      args.push(arg);
    }
    return args;
  }

  bool CommandManager::parsePointsCount(const Args& args, size_t& points_count,
                                        size_t& coord_start) {
    if (args.size < 8) {
      return false;
    }

    try {
      const size_t declared = std::stoul(args[1]);
      const size_t from_coords = (args.size - 2) / 2;
      if ((args.size - 2) % 2 == 0 && declared >= 3 && declared == from_coords) {
        points_count = declared;
        coord_start = 2;
        return true;
      }
    } catch (const std::exception&) {
    }

    if ((args.size - 1) % 2 != 0) {
      return false;
    }

    points_count = (args.size - 1) / 2;
    coord_start = 1;
    return points_count >= 3;
  }

  void CommandManager::appendRoute(Route& destination, Route& source, bool skip_first) {
    const size_t start = skip_first && source.count > 0 ? 1 : 0;
    if (start >= source.count) {
      return;
    }
    const size_t add_count = source.count - start;
    Point< float >* tmp = new Point< float >[destination.count + add_count];
    for (size_t i = 0; i < destination.count; ++i) {
      tmp[i] = destination.points[i];
    }
    for (size_t i = 0; i < add_count; ++i) {
      tmp[destination.count + i] = source.points[start + i];
    }
    delete[] destination.points;
    destination.points = tmp;
    destination.count += add_count;
  }

  void CommandManager::readCommand(const std::string& command, std::ostream& output) {
    if (command.empty()) {
      return;
    }
    std::stringstream ss(command);
    std::string instruction;
    ss >> instruction;
    Handler* fn = commands_.find(instruction);
    if (!fn) {
      output << "<INVALID COMMAND>\n";
      return;
    }
    (this->**fn)(ss, output);
  }

  void CommandManager::handleCommandList(std::istream&, std::ostream& output) {
    output
        << "Command List:\n0. command-list\n1. make-warehouse <name> "
           "<points-count> <points>\n2. "
           "delete-warehouse <name>\n3. inspect-warehouse <name>\n\n4. "
           "add-shelf <warehouse-name> <shelf-name> <floors> <points>\n5. "
           "delete-shelf <name>\n6. inspect-shelf <name>\n7. update-shelf "
           "<warehouse-name> <shelf-name> <floors> [points]\n8. "
           "add-item <warehouse-name> <shelf-name> <floor> <item-name> <quantity>\n9. "
           "delete-item <warehouse-name> <shelf-name> <item-name>\n10. "
           "update-item <warehouse-name> <shelf-name> <item-name> <quantity>\n11. "
           "inspect-item <warehouse-name> <shelf-name> <item-name>\n12. "
           "add-edge <warehouse-name> <edge-name> <shelf-name> <item-name> <quantity> <x> <y>\n13. "
           "delete-edge <warehouse-name> <edge-name>\n14. "
           "make-route <warehouse-name> <x> <y> <filename> <edge-name1> ... <edge-nameN>\n\n";
  }

  void CommandManager::handleMakeWarehouse(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    size_t points_count = 0;
    size_t coord_start = 0;
    if (args.size == 0 || !parsePointsCount(args, points_count, coord_start)) {
      output << "Wrong args\n";
      return;
    }

    Point< float >* points = new Point< float >[points_count];
    try {
      for (size_t i = coord_start; i < coord_start + points_count * 2; i += 2) {
        points[(i - coord_start) / 2] = {std::stof(args[i]), std::stof(args[i + 1])};
      }
      make_warehouse(args[0], points, points_count, &warehouse_map_);
    } catch (const std::exception& e) {
      output << e.what() << '\n';
    }
    delete[] points;
  }

  void CommandManager::handleInspectWarehouse(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 1) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    char* text = inspect_warehouse(*wh);
    output << text << '\n';
    delete[] text;
  }

  void CommandManager::handleDeleteWarehouse(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 1) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    warehouse_map_.remove(wh->name);
    delete wh;
  }

  void CommandManager::handleAddShelf(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 11) {
      output << "Wrong args\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    if (!wh) {
      output << "Wrong warehouse\n";
      return;
    }

    Point< float > points[4];
    try {
      for (size_t i = 3; i < 11; i += 2) {
        points[(i - 3) / 2] = {std::stof(args[i]), std::stof(args[i + 1])};
      }
      if (!wh->contain_shelf(points)) {
        output << "Shelf outside\n";
        return;
      }
      add_shelf(args[1], points, 4, std::stoull(args[2]), *wh, shelf_warehouse_map_,
                shelf_map_);
    } catch (const std::exception& e) {
      output << e.what() << '\n';
    }
  }

  void CommandManager::handleInspectShelf(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 1) {
      output << "Invalid arg number\n";
      return;
    }
    Shelf* sh = shelf_map_.get(args[0]);
    if (!sh) {
      output << "Shelf doesn't exist\n";
      return;
    }
    Warehouse* wh = shelf_warehouse_map_.get(*sh);
    char* text = inspect_shelf(*sh, *wh);
    output << text;
    delete[] text;
  }

  void CommandManager::handleDeleteShelf(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 1) {
      output << "Invalid arg number\n";
      return;
    }
    Shelf* sh = shelf_map_.get(args[0]);
    if (!sh) {
      output << "Shelf doesn't exist\n";
      return;
    }
    shelf_warehouse_map_.remove(*sh);
    shelf_map_.remove(sh->name);
    delete[] sh->points;
    delete sh;
  }

  void CommandManager::handleUpdateShelf(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 3 && args.size != 11) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    Shelf* sh = shelf_map_.get(args[1]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    if (!sh || shelf_warehouse_map_.get(*sh) != wh) {
      output << "Shelf doesn't exist\n";
      return;
    }
    try {
      const size_t height = std::stoull(args[2]);
      if (args.size == 3) {
        sh->height = height;
        return;
      }

      Point< float > points[4];
      for (size_t i = 3; i < 11; i += 2) {
        points[(i - 3) / 2] = {std::stof(args[i]), std::stof(args[i + 1])};
      }
      if (!wh->contain_shelf(points)) {
        output << "Shelf outside\n";
        return;
      }
      shelf_warehouse_map_.remove(*sh);
      delete[] sh->points;
      Shelf updated(sh->name, height, points[0], points[2], points[1], points[3]);
      sh->points = updated.points;
      updated.points = nullptr;
      sh->height = updated.height;
      sh->center = updated.center;
      shelf_warehouse_map_.insert(*sh, wh);
    } catch (const std::exception& e) {
      output << e.what() << '\n';
    }
  }

  void CommandManager::handleAddItem(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 5) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    Shelf* sh = shelf_map_.get(args[1]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    if (!sh || shelf_warehouse_map_.get(*sh) != wh) {
      output << "Shelf doesn't exist\n";
      return;
    }
    try {
      add_item(args[3], std::stoull(args[2]), std::stoull(args[4]), *sh, item_map_,
               item_shelf_map_);
      output << "Item added\n";
    } catch (const std::exception& e) {
      output << e.what() << '\n';
    }
  }

  void CommandManager::handleInspectItem(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 3) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    Shelf* sh = shelf_map_.get(args[1]);
    Item* item = item_map_.get(args[2]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    if (!sh || shelf_warehouse_map_.get(*sh) != wh) {
      output << "Shelf doesn't exist\n";
      return;
    }
    if (!item || item_shelf_map_.get(*item) != sh) {
      output << "Item doesn't exist\n";
      return;
    }
    char* text = inspect_item(*item, *sh, *wh);
    output << text;
    delete[] text;
  }

  void CommandManager::handleDeleteItem(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 3) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    Shelf* sh = shelf_map_.get(args[1]);
    Item* item = item_map_.get(args[2]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    if (!sh || shelf_warehouse_map_.get(*sh) != wh) {
      output << "Shelf doesn't exist\n";
      return;
    }
    if (!item || item_shelf_map_.get(*item) != sh) {
      output << "Item doesn't exist\n";
      return;
    }
    delete_item(item, item_map_, item_shelf_map_);
  }

  void CommandManager::handleUpdateItem(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 4) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    Shelf* sh = shelf_map_.get(args[1]);
    Item* item = item_map_.get(args[2]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    if (!sh || shelf_warehouse_map_.get(*sh) != wh) {
      output << "Shelf doesn't exist\n";
      return;
    }
    if (!item || item_shelf_map_.get(*item) != sh) {
      output << "Item doesn't exist\n";
      return;
    }
    try {
      update_item(*item, std::stoull(args[3]));
    } catch (const std::exception& e) {
      output << e.what() << '\n';
    }
  }

  void CommandManager::handleAddEdge(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 7) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    Shelf* sh = shelf_map_.get(args[2]);
    Item* item = item_map_.get(args[3]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    if (!sh || shelf_warehouse_map_.get(*sh) != wh) {
      output << "Shelf doesn't exist\n";
      return;
    }
    if (!item || item_shelf_map_.get(*item) != sh) {
      output << "Item doesn't exist\n";
      return;
    }
    if (edge_map_.get(args[1])) {
      output << "Edge already exists\n";
      return;
    }
    try {
      const size_t quantity = std::stoull(args[4]);
      if (quantity == 0) {
        output << "Wrong item quantity\n";
        return;
      }
      if (quantity > item->quantity) {
        output << "Not enough items\n";
        return;
      }
      Point< float > destination{std::stof(args[5]), std::stof(args[6])};
      if (!contain_point(wh->points, wh->points_count, destination)) {
        output << "Point outside warehouse\n";
        return;
      }
      if (point_in_shelves(destination, wh, shelf_map_, shelf_warehouse_map_)) {
        output << "Point is blocked\n";
        return;
      }
      RouteEdge* edge = new RouteEdge{args[1], wh->name, item->name, quantity, destination};
      edge_map_.insert(edge->name, edge);
    } catch (const std::exception& e) {
      output << e.what() << '\n';
    }
  }

  void CommandManager::handleDeleteEdge(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size != 2) {
      output << "Invalid arg number\n";
      return;
    }
    Warehouse* wh = warehouse_map_.get(args[0]);
    RouteEdge* edge = edge_map_.get(args[1]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }
    if (!edge || edge->warehouse_name != wh->name) {
      output << "Edge doesn't exist\n";
      return;
    }
    edge_map_.remove(edge->name);
    delete edge;
  }

  void CommandManager::handleMakeRoute(std::istream& input, std::ostream& output) {
    CommandManager::Args args = readArgs(input);
    if (args.size < 5) {
      output << "Invalid arg number\n";
      return;
    }

    Warehouse* wh = warehouse_map_.get(args[0]);
    if (!wh) {
      output << "Warehouse doesn't exist\n";
      return;
    }

    Point< float > current_point;
    try {
      current_point = {std::stof(args[1]), std::stof(args[2])};
    } catch (const std::exception& e) {
      output << e.what() << '\n';
      return;
    }
    if (!contain_point(wh->points, wh->points_count, current_point)) {
      output << "Point outside warehouse\n";
      return;
    }
    if (point_in_shelves(current_point, wh, shelf_map_, shelf_warehouse_map_)) {
      output << "Start point is blocked\n";
      return;
    }

    Route route;
    CommandManager::RouteItemStates states;
    for (size_t i = 4; i < args.size; ++i) {
      RouteEdge* edge = edge_map_.get(args[i]);
      if (!edge || edge->warehouse_name != wh->name) {
        output << "Edge doesn't exist\n";
        return;
      }
      Item* item = item_map_.get(edge->item_name);
      Shelf* sh = item ? item_shelf_map_.get(*item) : nullptr;
      if (!item || !sh || shelf_warehouse_map_.get(*sh) != wh) {
        output << "Item doesn't exist\n";
        return;
      }
      RouteItemState* state = states.find(item);
      if (!state) {
        state = states.add(item);
      }
      if (state->quantity + edge->quantity > item->quantity) {
        output << "Not enough items\n";
        return;
      }

      std::string error;
      Route to_item =
          make_route_between(*wh, current_point, state->coords, shelf_map_,
              shelf_warehouse_map_, error);
      if (to_item.count == 0) {
        output << error << '\n';
        return;
      }
      appendRoute(route, to_item, route.count > 0);

      Route to_destination =
          make_route_between(*wh, state->coords, edge->destination, shelf_map_,
              shelf_warehouse_map_, error);
      if (to_destination.count == 0) {
        output << error << '\n';
        return;
      }
      appendRoute(route, to_destination, route.count > 0);
      state->coords = edge->destination;
      state->quantity += edge->quantity;
      current_point = edge->destination;
    }

    std::ofstream file(args[3]);
    if (!file) {
      output << "Cannot open route file\n";
      return;
    }
    file << "Route:\n";
    for (size_t i = 0; i < route.count; ++i) {
      file << route.points[i].x << ' ' << route.points[i].y << '\n';
    }

    for (size_t i = 0; i < states.size; ++i) {
      states.data[i].item->coords = states.data[i].coords;
      states.data[i].item->quantity -= states.data[i].quantity;
    }
    output << "Route written\n";
  }
}
