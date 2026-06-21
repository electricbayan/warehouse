#ifndef GRAPH
#define GRAPH
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "htable.hpp"

namespace volkovich {
  template < class Vertex, class Hash, class Equal >
  class Graph {
    struct Edge {
      Vertex to;
      int* weight = nullptr;
      size_t weight_size = 0;
    };

    struct EdgeList {
      Edge* edges = nullptr;
      size_t count = 0;
    };

    static EdgeList copyEdgeList(const EdgeList& src) {
      EdgeList result;
      if (src.count > 0) {
        result.edges = new Edge[src.count];
        for (size_t i = 0; i < src.count; ++i) {
          result.edges[i].to = src.edges[i].to;
          result.edges[i].weight_size = src.edges[i].weight_size;
          if (src.edges[i].weight_size > 0) {
            result.edges[i].weight = new int[src.edges[i].weight_size];
            for (size_t j = 0; j < src.edges[i].weight_size; ++j) {
              result.edges[i].weight[j] = src.edges[i].weight[j];
            }
          }
        }
        result.count = src.count;
      }
      return result;
    }

    void appendEdge(EdgeList& list, const Vertex& to, int weight) {
      for (size_t i = 0; i < list.count; ++i) {
        if (list.edges[i].to == to) {
          int* new_weights = new int[list.edges[i].weight_size + 1];
          for (size_t j = 0; j < list.edges[i].weight_size; ++j) {
            new_weights[j] = list.edges[i].weight[j];
          }
          new_weights[list.edges[i].weight_size] = weight;
          delete[] list.edges[i].weight;
          list.edges[i].weight = new_weights;
          ++list.edges[i].weight_size;
          return;
        }
      }
      Edge* new_edges = new Edge[list.count + 1];
      for (size_t i = 0; i < list.count; ++i) {
        new_edges[i] = list.edges[i];
      }
      int* weights = new int[1];
      weights[0] = weight;
      new_edges[list.count] = Edge{to, weights, 1};
      delete[] list.edges;
      list.edges = new_edges;
      ++list.count;
    }

    void removeEdgeAt(EdgeList& list, size_t index) {
      if (index >= list.count) {
        return;
      }
      delete[] list.edges[index].weight;
      list.edges[index].weight = nullptr;
      list.edges[index].weight_size = 0;
      if (list.count == 1) {
        delete[] list.edges;
        list.edges = nullptr;
        list.count = 0;
        return;
      }
      Edge* new_edges = new Edge[list.count - 1];
      size_t pos = 0;
      for (size_t i = 0; i < list.count; ++i) {
        if (i != index) {
          new_edges[pos++] = list.edges[i];
        }
      }
      delete[] list.edges;
      list.edges = new_edges;
      --list.count;
    }

    bool removeEdgeWeight(EdgeList& list, const Vertex& to, int weight) {
      for (size_t i = 0; i < list.count; ++i) {
        if (list.edges[i].to != to) {
          continue;
        }
        size_t remove_index = list.edges[i].weight_size;
        for (size_t j = 0; j < list.edges[i].weight_size; ++j) {
          if (list.edges[i].weight[j] == weight) {
            remove_index = j;
            break;
          }
        }
        if (remove_index == list.edges[i].weight_size) {
          return false;
        }
        if (list.edges[i].weight_size == 1) {
          removeEdgeAt(list, i);
          return true;
        }
        int* new_weights = new int[list.edges[i].weight_size - 1];
        size_t pos = 0;
        for (size_t j = 0; j < list.edges[i].weight_size; ++j) {
          if (j != remove_index) {
            new_weights[pos++] = list.edges[i].weight[j];
          }
        }
        delete[] list.edges[i].weight;
        list.edges[i].weight = new_weights;
        --list.edges[i].weight_size;
        return true;
      }
      return false;
    }

    void freeAllEdges() {
      if (graph_.table_size() == 0) {
        return;
      }
      for (auto it = graph_.begin(); it != graph_.end(); ++it) {
        for (size_t i = 0; i < it->value.count; ++i) {
          delete[] it->value.edges[i].weight;
          it->value.edges[i].weight = nullptr;
          it->value.edges[i].weight_size = 0;
        }
        delete[] it->value.edges;
        it->value.edges = nullptr;
        it->value.count = 0;
      }
    }

   public:
    volkovich::HashTable< Vertex, EdgeList, Hash, Equal > graph_;

    Graph() = default;

    ~Graph() {
      freeAllEdges();
    }

    Graph(const Graph& other) {
      for (auto it = other.graph_.begin(); it != other.graph_.end(); ++it) {
        graph_.add(it->key, copyEdgeList(it->value));
      }
    }

    Graph(Graph&& other) noexcept : graph_(std::move(other.graph_)) {
    }

    Graph& operator=(const Graph& other) {
      if (this != &other) {
        Graph tmp(other);
        swap(tmp);
      }
      return *this;
    }

    Graph& operator=(Graph&& other) noexcept {
      if (this != &other) {
        freeAllEdges();
        graph_ = std::move(other.graph_);
      }
      return *this;
    }

    void swap(Graph& other) noexcept {
      graph_.swap(other.graph_);
    }

    bool addVertex(const Vertex& v) {
      return graph_.add(v, EdgeList{});
    }

    void addEdge(const Vertex& from, const Vertex& to, int weight) {
      if (!hasVertex(from)) {
        addVertex(from);
      }
      if (!hasVertex(to)) {
        addVertex(to);
      }
      EdgeList* edges_from = graph_.find(from);
      if (!edges_from) {
        return;
      }
      appendEdge(*edges_from, to, weight);
    }

    bool removeEdge(const Vertex& from, const Vertex& to, int weight) {
      EdgeList* edges_from = graph_.find(from);
      if (!edges_from) {
        return false;
      }
      return removeEdgeWeight(*edges_from, to, weight);
    }

    bool hasVertex(const Vertex& v) const {
      return graph_.has(v);
    }

    const Edge* getEdge(const Vertex& from, const Vertex& to) const {
      const EdgeList* edges_from = graph_.find(from);
      if (!edges_from) {
        return nullptr;
      }
      for (size_t i = 0; i < edges_from->count; ++i) {
        if (edges_from->edges[i].to == to) {
          return &edges_from->edges[i];
        }
      }
      return nullptr;
    }

    // bool hasEdge(const Vertex& from, const Vertex& to, int weight) const {
    //   const Edge* edge = getEdge(from, to);
    //   return edge && edge->weight == weight;
    // }

    bool hasEdge(const Vertex& from, const Vertex& to) const {
      return getEdge(from, to) != nullptr;
    }

    bool hasEdge(const Vertex& from, const Vertex& to, int weight) const {
      const Edge* edge = getEdge(from, to);
      if (!edge) {
        return false;
      }
      for (size_t i = 0; i < edge->weight_size; ++i) {
        if (edge->weight[i] == weight) {
          return true;
        }
      }
      return false;
    }

    void mergeFrom(const Graph& other) {
      for (auto it = other.graph_.begin(); it != other.graph_.end(); ++it) {
        if (!hasVertex(it->key)) {
          addVertex(it->key);
        }
      }
      for (auto it = other.graph_.begin(); it != other.graph_.end(); ++it) {
        for (size_t i = 0; i < it->value.count; ++i) {
          for (size_t j = 0; j < it->value.edges[i].weight_size; ++j) {
            addEdge(it->key, it->value.edges[i].to, it->value.edges[i].weight[j]);
          }
        }
      }
    }
  };

  template < class Vertex, class Hash, class Equal >
  class Graphs {
    using GraphMap = HashTable< std::string, Graph< Vertex, Hash, Equal >, Hash, Equal >;

   public:
    using iterator = typename GraphMap::iterator;
    using const_iterator = typename GraphMap::constIterator;
    using SubGraph = Graph< Vertex, Hash, Equal >;
    HashTable< std::string, SubGraph, Hash, Equal > graphs_;

    iterator begin() {
      return graphs_.begin();
    }
    iterator end() {
      return graphs_.end();
    }
    const_iterator begin() const {
      return graphs_.begin();
    }
    const_iterator end() const {
      return graphs_.end();
    }
    void readGraphsFromFile(std::ifstream& f) {
      std::string line;
      while (std::getline(f, line)) {
        if (line.empty()) {
          continue;
        }
        std::istringstream ss(line);
        std::string name;
        size_t edges;
        if (!(ss >> name >> edges)) {
          continue;
        }
        Graph< Vertex, Hash, Equal > gr;
        for (size_t i = 0; i < edges; i++) {
          if (!std::getline(f, line)) {
            throw std::logic_error("Wrong graph description");
          }
          if (line.empty()) {
            edges++;
            continue;
          }
          std::istringstream estream(line);
          Vertex from, to;
          int weight;
          if (!(estream >> from >> to >> weight)) {
            throw std::logic_error("Wrong graph description");
          }
          if (weight == 0) {
            continue;
          }
          gr.addEdge(from, to, weight);
        }
        graphs_.add(name, gr);
      }
    }

    SubGraph& addGraph(const std::string& name) {
      SubGraph gr;
      if (!graphs_.add(name, gr)) {
        throw std::logic_error("Graph exists");
      }
      SubGraph* stored = graphs_.find(name);
      return *stored;
    }
  };
}
#endif
