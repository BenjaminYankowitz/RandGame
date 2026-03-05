import Common;
using TestResult = int;
constexpr TestResult PassedTest = 0;

constexpr static std::array<Dir, 8> AnswerKey = std::to_array<Dir>({{-1, -1}, {0, -1}, {1, -1}, {-1, 0},{1, 0}, {-1, 1},  {0, 1},  {1,1}});

static_assert(std::ranges::all_of(std::views::iota(0, 8), [](std::int8_t dirN) {
    return AnswerKey[dirN]==Dir::getBoxDir(dirN);
  }));

static_assert(std::equal(AnswerKey.begin(),AnswerKey.end(),Dir::boxDirs().begin(),Dir::boxDirs().end()));

int main(){
  return PassedTest;
}