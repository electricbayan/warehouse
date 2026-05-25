#include "common.hpp"

class Task{
  Point<float> item_location, destination;
  const char* name;
  Task(const char* name, Point<float> item_location, Point<float> destination);
};

