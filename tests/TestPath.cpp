import Common;
using TestResult = int;
// constexpr TestResult FailedTest = 1;
constexpr TestResult PassedTest = 0;
struct Accessor {
public:
  using element_type = bool;
  using data_handle_type = const char *;
  using reference = element_type;
  constexpr static reference access(data_handle_type arr, std::size_t n) {
    return arr[n] == ' ' || arr[n] == 'e' || arr[n] == 's';
  }
};
using spanT = std::mdspan<bool, std::extents<int, std::dynamic_extent, std::dynamic_extent>, std::layout_right, Accessor>;

constexpr bool testMap(std::string_view map, int height, int width, int expectedLen) {
    if (static_cast<int>(map.size()) != height * width) {
      // std::cout << "Map size " << map.size() << " does not match dims " << height << " x " << width << '\n';
      return false;
    }
    Position start{-1, -1};
    Position end{-1, -1};
    for (int row = 0; row < static_cast<int>(height); row++) {
      for (int col = 0; col < static_cast<int>(width); col++) {
        const char c = map[row * width + col];
        if (c == 's' || c == 'b') {
          start = {col, row};
        } else if (c == 'e' || c == 'f') {
          end = {col, row};
        }
      }
    }
    if (start == Position{-1, -1}) {
      // std::cout << "No start in map\n";
      return false;
    }
    if (end == Position{-1, -1}) {
      // std::cout << "No end in map\n";
      return false;
    }
    int len = 0;
    spanT mapView(map.data(),height,width);
    Position cSpot = start;
    while(len < expectedLen && cSpot!=end){
      auto dir = FindPath::findPath(mapView, cSpot, end);
        if(dir!=capDir(dir)){
            // std::cout << "Tried jumping larger distance\n";
            return false;
        }
        cSpot+=dir;
        if(!cSpot.within({width-1,height-1})){
            // std::cout << "Tried exiting map\n";
            return false;
        }
        if(cSpot != end && !mapView[cSpot.y,cSpot.x]){
            // std::cout << "Tried walking into wall\n";
            return false;
        }
        len++;
    }
    if(cSpot!=end){
        // std::cout << "Did not reach end in time\n";
        return false;
    }
    if(len!=expectedLen){
        // std::cout << "Got to end faster than should be possible (likily an error with test)\n";
        return false;
    }
    return true;
}

static_assert(testMap("\
s  \
xx \
e  \
", 3, 3, 4));

TestResult main(int /*unused*/, char ** /*unused*/) {
  return PassedTest;
}