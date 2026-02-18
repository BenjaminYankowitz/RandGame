import std;
import Common;
using TestResult = int;
constexpr TestResult FailedTest = 1;
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

class MapTest{
    public:
    std::string_view data;
    std::size_t width;
    std::size_t height;
    std::size_t pathLen;
    Position startingSpot{-1,-1};
    Position endingSpot{-1,-1};
    MapTest(std::string_view map, std::size_t widthIn, std::size_t heightIn) : data(map), width(widthIn),height(heightIn){
        if(map.size()!=height*width){
            std::cout << "Map size " << map.size() << " does not match dims " << height << " x " << width << '\n';
            return;
        }
        Position start{-1,-1};
        Position end{-1,-1};
        for(int row = 0; row < static_cast<int>(height); row++){
            for(int col = 0; col < static_cast<int>(width); col++){
                const char c = map[row*width+col];
                if(c=='s'||c=='b'){
                    start = {col,row};
                } else if(c=='e'||c=='f'){
                    end = {col,row};
                }
            }
        }
        if(start==Position{-1,-1}){
            std::cout << "No start in map\n";
            return;
        }
        if(end==Position{-1,-1}){
            std::cout << "No end in map\n";
            return;
        }
        startingSpot = start;
        endingSpot = end;
    }
};

bool runMapTest(const MapTest& mapTest){
    std::vector<Position> path(mapTest.pathLen,{0,0});
}

TestResult main(int /*unused*/, char** /*unused*/){
    int ret = PassedTest;
    std::string_view map1 = "\
s  \
xx \
e  \
";
    auto runTest = [&ret](auto test, std::string_view description){
        if(!test()){
            ret = FailedTest;
            std::cout << "Test failed: " << description << '\n';
        }
    };
    // runTest([&map1](){
    //     auto res = findPath(map1,3,3);
    //     if(res.has_value()) {
    //         std::cout << "dir: " << *res << '\n';
    //     }
    //     return res.value_or(Dir{-2,-2})==Dir{1,0};
    // }, "Short Path");
    return ret;
}