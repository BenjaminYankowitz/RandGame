export module DungeonMaker;
import Common;
import PerlinNoise;


template <class T>
class RoomSplitHelper {
public:
  RoomSplitHelper(Static2DArr<T> &floor, std::size_t rowB, std::size_t colB, std::size_t rowE, std::size_t colE, std::normal_distribution<> areaDist, bool dir) noexcept : floor_(floor), rowB_(rowB), colB_(colB), rowE_(rowE), colE_(colE), areaDist_(areaDist), dir_(dir) {}
  [[nodiscard]] bool doSmallStop() const noexcept {
    const std::size_t width = colE_ - colB_;
    const std::size_t height = rowE_ - rowB_;
    const std::size_t area = width * height;
    return Rnd::get(areaDist_) >= area;
  }
  [[nodiscard]] constexpr bool doThinSkip() const noexcept {
    return xE() - xB() <= 2;
  }
  [[nodiscard]] constexpr std::size_t &xB() noexcept {
    return dir_ ? rowB_ : colB_;
  }
  [[nodiscard]] constexpr std::size_t &xE() noexcept {
    return dir_ ? rowE_ : colE_;
  }
  [[nodiscard]] constexpr std::size_t &yB() noexcept {
    return dir_ ? colB_ : rowB_;
  }
  [[nodiscard]] constexpr std::size_t &yE() noexcept {
    return dir_ ? colE_ : rowE_;
  }
  [[nodiscard]] constexpr std::size_t xB() const noexcept {
    return dir_ ? rowB_ : colB_;
  }
  [[nodiscard]] constexpr std::size_t xE() const noexcept {
    return dir_ ? rowE_ : colE_;
  }
  [[nodiscard]] constexpr std::size_t yB() const noexcept {
    return dir_ ? colB_ : rowB_;
  }
  [[nodiscard]] constexpr std::size_t yE() const noexcept {
    return dir_ ? colE_ : rowE_;
  }
  [[nodiscard]] constexpr T &operator[](std::size_t x, std::size_t y) noexcept {
    if (dir_) {
      return floor_[x, y];
    }
    return floor_[y, x];
  }
  [[nodiscard]] constexpr RoomSplitHelper child(std::size_t wallX, bool right) noexcept {
    RoomSplitHelper ret = *this;
    if (right) {
      ret.xB() = wallX + 1;
    } else {
      ret.xE() = wallX;
    }
    ret.dir_ = !ret.dir_;
    return ret;
  }
  [[nodiscard]] constexpr RoomSplitHelper bigChild() noexcept {
    RoomSplitHelper ret = *this;
    ret.dir_ = !ret.dir_;
    return ret;
  }

private:
  Static2DArr<T> &floor_;
  std::size_t rowB_;
  std::size_t colB_;
  std::size_t rowE_;
  std::size_t colE_;
  mutable std::normal_distribution<> areaDist_;
  bool dir_;
};

template <auto Wall, decltype(Wall) Empty>
void fillArea(RoomSplitHelper<decltype(Wall)> rsh) noexcept {
  if (rsh.doSmallStop()) {
    return;
  }
  if (rsh.doThinSkip()) {
    fillArea<Wall, Empty>(rsh.bigChild());
    return;
  }
  std::uniform_int_distribution<std::size_t> dist(rsh.xB() + 1, rsh.xE() - 2);
  const std::size_t wallX = Rnd::get(dist);
  for (std::size_t y = rsh.yB(); y < rsh.yE(); y++) {
    rsh[wallX, y] = Wall;
  }
  fillArea<Wall, Empty>(rsh.child(wallX, false));
  fillArea<Wall, Empty>(rsh.child(wallX, true));
}

constexpr double DefaultRoomArea = 100;
constexpr double DefaultRoomStandardDeviation = 0.1;

export namespace DungeonMaker {
template <auto Wall, decltype(Wall) Empty>
void rooms(Static2DArr<decltype(Wall)> &floor, double roomArea = DefaultRoomArea, double areaStddev = DefaultRoomStandardDeviation) noexcept {
  floor.fill(Empty);
  std::bernoulli_distribution dist(0.5);
  fillArea<Wall, Empty>(RoomSplitHelper(floor, 0, 0, floor.rows(), floor.cols(), std::normal_distribution<>(roomArea, areaStddev), Rnd::get(dist)));
}

template <auto Wall, decltype(Wall) Empty>
void perlin(Static2DArr<decltype(Wall)> &floor,double xscale, double yscale, double threshold = 0.0)noexcept{
  if(xscale<1.0){
    xscale = 1.0;
  }
  if(yscale<1.0){
    yscale = 1.0;
  }
  PerlinNoise::Generator gen(std::ceil(floor.cols()/xscale)+1,std::ceil(floor.rows()/yscale)+1);
  for(std::size_t x = 0; x < floor.cols(); x++){
    for(std::size_t y = 0; y < floor.rows(); y++){
      if(gen.getHeight(x/xscale, y/yscale)>=threshold){
        floor[y,x] = Wall;
      } else {
        floor[y,x] = Empty;
      }
    }
  }
}

template <auto Wall, decltype(Wall) Empty>
void maze(Static2DArr<decltype(Wall)> &floor, std::size_t extraConnections = 0) {
  if (floor.size() == 0) {
    return;
  }
  floor.fill(Wall);
  for(std::size_t row = 0; row < floor.rows(); row+=2){
    for(std::size_t col = 0; col < floor.cols(); col+=2){
      floor[row,col] = Empty;
    }
  }
  const std::size_t hcols = (floor.cols() - 1) / 2;
  const std::size_t hrows = (floor.rows() + 1) / 2;
  const std::size_t hsize = hcols * hrows;
  const std::size_t vcols = (floor.cols() + 1) / 2;
  const std::size_t vrows = (floor.rows() - 1) / 2;
  const std::size_t vsize = vcols * vrows;
  const std::size_t orows = hrows;
  const std::size_t ocols = vcols;
  const std::size_t osize = orows * ocols;
  std::vector<std::size_t> ordering(hsize + vsize);
  std::iota(ordering.begin(), ordering.end(), 0);
  Rnd::shuffle(ordering);
  DisjointSet<std::size_t> connected(osize);
  std::size_t extraLeft = extraConnections;
  while(!ordering.empty()) {
    const std::size_t toOpen = ordering.back();
    ordering.pop_back();
    auto [arow, acol, horizontal] = [&]() {
      if (toOpen >= hsize) {
        const std::size_t row = (toOpen - hsize) % vrows;
        const std::size_t col = (toOpen - hsize) / vrows;
        return std::tuple{row,col,false};
      }
      const std::size_t row = toOpen % hrows;
      const std::size_t col = toOpen / hrows;
      return std::tuple{row,col,true};
    }();
    auto &tile = floor[arow*2+!horizontal,acol*2+horizontal];
    if (connected.union_set(acol*orows+arow, acol*orows+arow+(horizontal?orows:1))) {
      tile = Empty;
    } else if(extraLeft>0){
      extraLeft--;
      tile=Empty;
    }
  }
}
} // namespace DungeonMaker

// //NOLINTBEGIN
// static void f(){ //give func which uses createDungeon so clangd will be able to report errors.
//     Static2DArr<int> q(1,1);
//     DungeonMaker::createDungeon<0,1>(q);
// }
// //NOLINTEND