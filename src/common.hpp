template <class T> class Point {
public:
  T x, y;
  Point(T x, T y) : x(x), y(y) {};
  Point() : x(0), y(0) {}
};
bool is_line(Point<float> a, Point<float> b, Point<float> c) noexcept;

void sort_around_center(Point<float>* polygon, size_t polygon_size, Point<float> center);
