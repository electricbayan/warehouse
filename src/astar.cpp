#include "astar.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

constexpr float GRID_STEP = 0.5f;

Route::~Route() {
  delete[] points;
}

Route::Route(Route&& other) noexcept : points(other.points), count(other.count) {
  other.points = nullptr;
  other.count = 0;
}

Route& Route::operator=(Route&& other) noexcept {
  if (this != &other) {
    delete[] points;
    points = other.points;
    count = other.count;
    other.points = nullptr;
    other.count = 0;
  }
  return *this;
}

std::string grid_vertex(size_t id) {
  return std::to_string(id);
}

struct GridRouteMap {
  float min_x = 0;
  float min_y = 0;
  size_t width = 0;
  size_t height = 0;
  size_t points_count = 0;
  Point<float>* points = nullptr;
  char* free = nullptr;
  RouteGraph graph;

  ~GridRouteMap() {
    delete[] points;
    delete[] free;
  }

  GridRouteMap() = default;
  GridRouteMap(const GridRouteMap&) = delete;
  GridRouteMap& operator=(const GridRouteMap&) = delete;
};

struct IdRoute {
  size_t* ids = nullptr;
  size_t count = 0;

  ~IdRoute() {
    delete[] ids;
  }

  IdRoute() = default;
  IdRoute(const IdRoute&) = delete;
  IdRoute& operator=(const IdRoute&) = delete;
  IdRoute(IdRoute&& other) noexcept : ids(other.ids), count(other.count) {
    other.ids = nullptr;
    other.count = 0;
  }
};

struct QueueNode {
  int cost;
  size_t id;
};

struct MinHeap {
  QueueNode* data = nullptr;
  size_t size = 0;
  size_t capacity = 0;

  explicit MinHeap(size_t initial_capacity) : capacity(initial_capacity ? initial_capacity : 1) {
    data = new QueueNode[capacity];
  }

  ~MinHeap() {
    delete[] data;
  }

  bool empty() const {
    return size == 0;
  }

  void push(QueueNode node) {
    if (size == capacity) {
      capacity *= 2;
      QueueNode* tmp = new QueueNode[capacity];
      for (size_t i = 0; i < size; ++i) {
        tmp[i] = data[i];
      }
      delete[] data;
      data = tmp;
    }
    size_t pos = size++;
    while (pos > 0) {
      size_t parent = (pos - 1) / 2;
      if (data[parent].cost <= node.cost) {
        break;
      }
      data[pos] = data[parent];
      pos = parent;
    }
    data[pos] = node;
  }

  QueueNode pop() {
    QueueNode result = data[0];
    QueueNode last = data[--size];
    size_t pos = 0;
    while (true) {
      size_t left = pos * 2 + 1;
      size_t right = left + 1;
      if (left >= size) {
        break;
      }
      size_t child = left;
      if (right < size && data[right].cost < data[left].cost) {
        child = right;
      }
      if (data[child].cost >= last.cost) {
        break;
      }
      data[pos] = data[child];
      pos = child;
    }
    if (size > 0) {
      data[pos] = last;
    }
    return result;
  }
};

size_t grid_id(const GridRouteMap& map, size_t x, size_t y) {
  return y * map.width + x;
}

bool point_in_shelves(Point<float> point, Warehouse* wh,
                      HashMap<std::string, Shelf>& shelf_map,
                      HashMap<Shelf, Warehouse>& shelf_warehouse_map) {
  for (auto it = shelf_map.begin(); it != shelf_map.end(); ++it) {
    Shelf* shelf = it->value;
    if (shelf && shelf_warehouse_map.get(*shelf) == wh &&
        contain_point(shelf->points, 4, point)) {
      return true;
    }
  }
  return false;
}

void build_route_map(GridRouteMap& map, Warehouse& wh,
                     HashMap<std::string, Shelf>& shelf_map,
                     HashMap<Shelf, Warehouse>& shelf_warehouse_map) {
  float max_x = wh.points[0].x;
  float max_y = wh.points[0].y;
  map.min_x = wh.points[0].x;
  map.min_y = wh.points[0].y;
  for (size_t i = 1; i < wh.points_count; ++i) {
    map.min_x = std::min(map.min_x, wh.points[i].x);
    map.min_y = std::min(map.min_y, wh.points[i].y);
    max_x = std::max(max_x, wh.points[i].x);
    max_y = std::max(max_y, wh.points[i].y);
  }

  map.width = static_cast<size_t>(std::floor((max_x - map.min_x) / GRID_STEP)) + 1;
  map.height = static_cast<size_t>(std::floor((max_y - map.min_y) / GRID_STEP)) + 1;
  map.points_count = map.width * map.height;
  map.points = new Point<float>[map.points_count];
  map.free = new char[map.points_count]{};

  for (size_t y = 0; y < map.height; ++y) {
    for (size_t x = 0; x < map.width; ++x) {
      const size_t id = grid_id(map, x, y);
      Point<float> point{map.min_x + x * GRID_STEP, map.min_y + y * GRID_STEP};
      map.points[id] = point;
      map.free[id] = contain_point(wh.points, wh.points_count, point) &&
                     !point_in_shelves(point, &wh, shelf_map, shelf_warehouse_map);
      if (map.free[id]) {
        map.graph.addVertex(grid_vertex(id));
      }
    }
  }

  const int dx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
  const int dy[8] = {0, 0, 1, -1, 1, -1, 1, -1};
  const int weight[8] = {5, 5, 5, 5, 7, 7, 7, 7};
  for (size_t y = 0; y < map.height; ++y) {
    for (size_t x = 0; x < map.width; ++x) {
      const size_t from = grid_id(map, x, y);
      if (!map.free[from]) {
        continue;
      }
      for (size_t dir = 0; dir < 8; ++dir) {
        const int nx = static_cast<int>(x) + dx[dir];
        const int ny = static_cast<int>(y) + dy[dir];
        if (nx < 0 || ny < 0 || nx >= static_cast<int>(map.width) ||
            ny >= static_cast<int>(map.height)) {
          continue;
        }
        const size_t to = grid_id(map, static_cast<size_t>(nx), static_cast<size_t>(ny));
        if (!map.free[to]) {
          continue;
        }
        if (dx[dir] != 0 && dy[dir] != 0) {
          const size_t side_a = grid_id(map, static_cast<size_t>(nx), y);
          const size_t side_b = grid_id(map, x, static_cast<size_t>(ny));
          if (!map.free[side_a] || !map.free[side_b]) {
            continue;
          }
        }
        map.graph.addEdge(grid_vertex(from), grid_vertex(to), weight[dir]);
      }
    }
  }
}

size_t nearest_free_cell(const GridRouteMap& map, Point<float> point) {
  size_t best = map.points_count;
  float best_dist = std::numeric_limits<float>::max();
  for (size_t id = 0; id < map.points_count; ++id) {
    if (!map.free[id]) {
      continue;
    }
    const float dx = map.points[id].x - point.x;
    const float dy = map.points[id].y - point.y;
    const float dist = dx * dx + dy * dy;
    if (dist < best_dist) {
      best_dist = dist;
      best = id;
    }
  }
  return best;
}

int route_heuristic(const GridRouteMap& map, size_t from, size_t to) {
  const float dx = map.points[from].x - map.points[to].x;
  const float dy = map.points[from].y - map.points[to].y;
  return static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy) * 10));
}

IdRoute a_star_route(const GridRouteMap& map, size_t start, size_t goal) {
  const int inf = std::numeric_limits<int>::max() / 4;
  int* dist = new int[map.points_count];
  size_t* parent = new size_t[map.points_count];
  char* closed = new char[map.points_count]{};
  for (size_t i = 0; i < map.points_count; ++i) {
    dist[i] = inf;
    parent[i] = map.points_count;
  }

  MinHeap open(map.points_count);
  dist[start] = 0;
  open.push(QueueNode{route_heuristic(map, start, goal), start});
  while (!open.empty()) {
    const QueueNode current = open.pop();
    if (closed[current.id]) {
      continue;
    }
    if (current.id == goal) {
      break;
    }
    closed[current.id] = 1;
    map.graph.forEachAdjacent(grid_vertex(current.id),
        [&](const std::string& next_key, int weight) {
          const size_t next = static_cast<size_t>(std::stoull(next_key));
          if (closed[next]) {
            return;
          }
          const int next_dist = dist[current.id] + weight;
          if (next_dist < dist[next]) {
            dist[next] = next_dist;
            parent[next] = current.id;
            open.push(QueueNode{next_dist + route_heuristic(map, next, goal), next});
          }
        });
  }

  IdRoute route;
  if (dist[goal] != inf) {
    for (size_t id = goal; id != map.points_count; id = parent[id]) {
      ++route.count;
      if (id == start) {
        break;
      }
    }
    route.ids = new size_t[route.count];
    size_t pos = route.count;
    for (size_t id = goal; id != map.points_count; id = parent[id]) {
      route.ids[--pos] = id;
      if (id == start) {
        break;
      }
    }
  }

  delete[] dist;
  delete[] parent;
  delete[] closed;
  return route;
}

Route make_route(Warehouse& wh, Point<float> start_point, Item& item,
                 HashMap<std::string, Shelf>& shelf_map,
                 HashMap<Shelf, Warehouse>& shelf_warehouse_map,
                 std::string& error) {
  return make_route_between(wh, start_point, item.coords, shelf_map, shelf_warehouse_map, error);
}

Route make_route_between(Warehouse& wh, Point<float> start_point,
                         Point<float> finish_point,
                         HashMap<std::string, Shelf>& shelf_map,
                         HashMap<Shelf, Warehouse>& shelf_warehouse_map,
                         std::string& error) {
  error.clear();
  GridRouteMap map;
  build_route_map(map, wh, shelf_map, shelf_warehouse_map);
  const size_t start = nearest_free_cell(map, start_point);
  const size_t goal = nearest_free_cell(map, finish_point);
  if (start == map.points_count || goal == map.points_count) {
    error = "Route doesn't exist";
    return {};
  }

  IdRoute route_ids = a_star_route(map, start, goal);
  if (route_ids.count == 0) {
    error = "Route doesn't exist";
    return {};
  }

  Route route;
  route.count = route_ids.count;
  route.points = new Point<float>[route.count];
  for (size_t i = 0; i < route.count; ++i) {
    route.points[i] = map.points[route_ids.ids[i]];
  }
  return route;
}
