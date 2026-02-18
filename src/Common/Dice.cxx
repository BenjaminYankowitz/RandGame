export module Common:Dice;
import :Random;
import :Misc;
import std;

struct MonoState{};

constexpr void stringViewToNumber(std::string_view str, std::uint16_t &out) {
  std::from_chars_result result = std::from_chars(str.begin(), str.end(), out);
  if (result.ec != std::errc()) {
    throw ERRCException(result.ec);
  }
}


export namespace Dice {
class SingleTypeGroup {
  public:
  consteval explicit SingleTypeGroup(std::string_view writtenExplanation) {
    const std::size_t dLoc = writtenExplanation.find_first_of("dD");
    const std::size_t start = writtenExplanation.find_first_not_of(' ');
    if (dLoc == start) {
      number_ = 1;
    } else {
      stringViewToNumber(writtenExplanation.substr(start, dLoc-start), number_);
    }
    if(dLoc==std::string_view::npos){
      faces_ = 1;
    } else {
      stringViewToNumber(writtenExplanation.substr(dLoc + 1), faces_);
    }
  }
  constexpr SingleTypeGroup(std::uint16_t faces, std::uint16_t number) noexcept : faces_(faces), number_(number){}
  constexpr SingleTypeGroup() noexcept :faces_(0), number_(0){}
  [[nodiscard]] std::size_t operator()() const noexcept {
    std::uniform_int_distribution<std::size_t> dist(1,faces_);
    auto view = std::views::repeat(MonoState{},number_);
    return std::transform_reduce(view.begin(),view.end(),0ul,std::plus<>(),[&dist](MonoState){
        return Rnd::get(dist);
    });
  }
  [[nodiscard]] constexpr std::uint16_t getFaces() const noexcept {return faces_;}
  [[nodiscard]] constexpr std::uint16_t getNumber() const noexcept {return number_;}
private:
  std::uint16_t faces_;
  std::uint16_t number_;
};

class Group {
  public:
  constexpr static std::size_t MaxTypes = 2;
  consteval explicit Group(std::string_view writtenExplanation) {
    constant_ = 0;
    const std::size_t strLen = writtenExplanation.size();
    std::size_t beginSection = 0;
    std::size_t endSection = std::min(writtenExplanation.find_first_of('+'),strLen);
    std::size_t typeNum = 0;
    while(beginSection<strLen){
      const SingleTypeGroup g(writtenExplanation.substr(beginSection,endSection-beginSection));
      if(g.getFaces()==1){
        constant_+=g.getNumber();
      } else {
        dice_[typeNum++] = g;
      }
      if(endSection==std::string_view::npos){
        break;
      }
      beginSection = endSection+1;
      endSection = std::min(writtenExplanation.find_first_of('+',endSection+1),strLen);
    }
  }
  constexpr explicit Group(std::uint16_t constant, SingleTypeGroup s1 = {}, SingleTypeGroup s2 = {}) noexcept : constant_(constant), dice_({s1,s2}){}
  [[nodiscard]] std::size_t operator()() const noexcept {
    return std::transform_reduce(dice_.begin(),dice_.end(),constant_,std::plus<>(),[](SingleTypeGroup die){
        return die();
    });
  }
  private:
  std::uint16_t constant_;
  std::array<SingleTypeGroup,MaxTypes> dice_;
};


namespace Literals {
consteval Group operator ""_dice( const char* str, std::size_t len ) noexcept{
  return Group(std::string_view(str,len));      
}
consteval SingleTypeGroup operator ""_diceST( const char* str, std::size_t len ) noexcept{
  return SingleTypeGroup(std::string_view(str,len));      
}
}

}; // namespace Dice