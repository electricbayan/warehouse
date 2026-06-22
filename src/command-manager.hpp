#ifndef COMMAND_MANAGER
#define COMMAND_MANAGER
#include <iostream>
#include <sstream>
#include <string>

#include "astar.hpp"
#include "graph.hpp"
#include "io.hpp"

namespace volkovich {
  class CommandManager {
    using Handler = void (CommandManager::*)(std::istream&, std::ostream&);
    HashTable< std::string, Handler, SipHash, std::equal_to< std::string > > commands_;

    HashMap< Shelf, Warehouse > shelf_warehouse_map_;
    HashMap< std::string, Warehouse > warehouse_map_;
    HashMap< std::string, Shelf > shelf_map_;
    HashMap< std::string, Item > item_map_;
    HashMap< Item, Shelf > item_shelf_map_;
    HashMap< std::string, RouteEdge > edge_map_;

    struct RouteItemState {
      Item* item = nullptr;
      Point< float > coords;
      size_t quantity = 0;
    };

    struct Args {
      std::string* data = nullptr;
      size_t size = 0;
      size_t capacity = 0;
      ~Args();
      Args() = default;
      Args(const Args&) = delete;
      Args& operator=(const Args&) = delete;
      Args(Args&& other) noexcept;
      Args& operator=(Args&& other) noexcept;
      void push(const std::string& value);
      std::string& operator[](size_t index);
      const std::string& operator[](size_t index) const;
    };

    struct RouteItemStates {
      RouteItemState* data = nullptr;
      size_t size = 0;
      size_t capacity = 0;
      ~RouteItemStates();
      RouteItemStates() = default;
      RouteItemStates(const RouteItemStates&) = delete;
      RouteItemStates& operator=(const RouteItemStates&) = delete;
      RouteItemState* find(Item* item);
      RouteItemState* add(Item* item);
    };

    static Args readArgs(std::istream& input);
    static bool parsePointsCount(const Args& args, size_t& points_count, size_t& coord_start);
    static void appendRoute(Route& destination, Route& source, bool skip_first);

    void handleCommandList(std::istream&, std::ostream&);
    void handleMakeWarehouse(std::istream&, std::ostream&);
    void handleInspectWarehouse(std::istream&, std::ostream&);
    void handleDeleteWarehouse(std::istream&, std::ostream&);


    void handleAddShelf(std::istream&, std::ostream&);
    void handleInspectShelf(std::istream&, std::ostream&);
    void handleDeleteShelf(std::istream&, std::ostream&);
    void handleUpdateShelf(std::istream&, std::ostream&);


    void handleAddItem(std::istream&, std::ostream&);
    void handleInspectItem(std::istream&, std::ostream&);
    void handleDeleteItem(std::istream&, std::ostream&);
    void handleUpdateItem(std::istream&, std::ostream&);


    void handleAddEdge(std::istream&, std::ostream&);
    void handleDeleteEdge(std::istream&, std::ostream&);

    void handleMakeRoute(std::istream&, std::ostream&);


   public:
    CommandManager();
    ~CommandManager();
    CommandManager(const CommandManager&) = delete;
    CommandManager& operator=(const CommandManager&) = delete;
    void readCommand(const std::string& command, std::ostream& output);
  };
}
#endif
