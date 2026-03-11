module;
#include <cstdint>
#include <utility>
#include <iostream>
#include <algorithm>
#include <array>
#include <charconv>
#include <iterator>
#include <memory>
#include <string_view>
#include <exception>
#include <cursesw.h>
export module CursesLowLevel;
namespace CursesLowLevel {
alignas(32) static thread_local std::array<std::byte, 1024> buffer; //NOLINT
export namespace IOExceptions {
struct IOModuleException : std::exception {
};
struct WindowFailure : public IOModuleException {};
struct EraseFailure : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Erase Failure"; }
};
struct PrintFailure : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Print Failure"; }
};
struct ScreenUpdateFailure : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Screen Update Failure"; }
};
struct MoveCursorFailure : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Move Cursor Failure"; }
};
struct ReadDataFailure : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Failed to read line of data"; }
};
struct BufferOverflow : public WindowFailure {
  explicit BufferOverflow(std::size_t needed) noexcept : needed_(needed) {}
  [[nodiscard]] const char *what() const noexcept override {
    char *const bufferBegin = reinterpret_cast<char *>(buffer.data());
    char *bufferLoc = bufferBegin;
    char *bufferEnd = bufferBegin + buffer.size();
    auto addStr = [&bufferLoc, bufferBegin](std::string_view str) {
      const std::size_t chrPrint = std::min<std::size_t>(str.size(), buffer.size() - std::distance(bufferBegin, bufferLoc));
      if (chrPrint == 0) {
        return;
      }
      std::memcpy(bufferLoc, str.data(), chrPrint);
      bufferLoc += chrPrint;
    };
    auto addNum = [&bufferLoc, bufferEnd](std::size_t n) {
      if (bufferLoc == bufferEnd) {
        return;
      }
      char *nBufferLoc = std::to_chars(bufferLoc, bufferEnd, n).ptr;
      bufferLoc = nBufferLoc;
    };
    addStr("Buffer too small only has ");
    addNum(buffer.size());
    addStr(" Bytes but ");
    addNum(needed_);
    addStr("Bytes needed\n");
    if (bufferLoc != bufferEnd) {
      *bufferLoc = '\0';
    } else {
      *(bufferEnd - 1) = '\0';
    }
    return bufferBegin;
  }

private:
  std::size_t needed_;
};
struct BoxResizeFailure : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Tried to resize Box and failed"; }
};
struct BoxUnderBound : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Tried to resize Box to smaller than max size"; }
};
struct BoxMoveFailure : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Failed to move a box"; }
};
struct MultipleCursesRAII : public IOModuleException {
  [[nodiscard]] const char *what() const noexcept override { return "Multiple CursesRAII Objects created at the same time"; }
};
struct BightBackground : public IOModuleException {
  [[nodiscard]] const char *what() const noexcept override { return "Tried to make a background color bright"; }
};
} // namespace IOExceptions
using namespace IOExceptions;
export using chtype = wchar_t;
export using attr_t = attr_t;
export enum class BaseColor : std::uint8_t { Black,
                                             Red,
                                             Green,
                                             Brown,
                                             Blue,
                                             Magenta,
                                             Cyan,
                                             White };

constexpr BaseColor DefgetColor(NCURSES_COLOR_T color) noexcept {
  switch (color) {
  case COLOR_BLACK:
    return BaseColor::Black;
  case COLOR_RED:
    return BaseColor::Red;
  case COLOR_GREEN:
    return BaseColor::Green;
  case COLOR_YELLOW:
    return BaseColor::Brown;
  case COLOR_BLUE:
    return BaseColor::Blue;
  case COLOR_MAGENTA:
    return BaseColor::Magenta;
  case COLOR_CYAN:
    return BaseColor::Cyan;
  case COLOR_WHITE:
    return BaseColor::White;
  default:
    assert(false);
    return BaseColor::White;
  }
};

constexpr NCURSES_COLOR_T getColorDef(BaseColor color) noexcept {
  switch (color) {
  case BaseColor::Black:
    return COLOR_BLACK;
  case BaseColor::Red:
    return COLOR_RED;
  case BaseColor::Green:
    return COLOR_GREEN;
  case BaseColor::Brown:
    return COLOR_YELLOW;
  case BaseColor::Blue:
    return COLOR_BLUE;
  case BaseColor::Magenta:
    return COLOR_MAGENTA;
  case BaseColor::Cyan:
    return COLOR_CYAN;
  case BaseColor::White:
    return COLOR_WHITE;
  }
}

consteval int lg(std::size_t n) {
  assert(std::popcount(n) <= 1);
  int r = -1;
  while (n > 0) {
    n >>= 1;
    r++;
  }
  return r;
}

constexpr NCURSES_COLOR_T NumBaseColor = 1 + COLOR_WHITE;
constexpr int NumBaseColorBits = lg(NumBaseColor);
static_assert(std::popcount(static_cast<std::size_t>(NumBaseColor)) == 1);

export class Color {
public:
  BaseColor baseColor;
  bool isBright;
  constexpr Color() = default;
  constexpr Color(BaseColor baseColor_, bool isBright_) noexcept : baseColor(baseColor_), isBright(isBright_) {}
};
export constexpr Color Black(BaseColor::Black, false);
export constexpr Color Grey(BaseColor::Black, true);
export constexpr Color Red(BaseColor::Red, false);
export constexpr Color Orange(BaseColor::Red, true);
export constexpr Color DarkGreen(BaseColor::Green, false);
export constexpr Color BrightGreen(BaseColor::Green, true);
export constexpr Color Brown(BaseColor::Brown, false);
export constexpr Color Yellow(BaseColor::Brown, true);
export constexpr Color DarkBlue(BaseColor::Blue, false);
export constexpr Color Blue(BaseColor::Blue, true);
export constexpr Color DarkMagenta(BaseColor::Magenta, false);
export constexpr Color Magenta(BaseColor::Magenta, true);
export constexpr Color Green(BaseColor::Cyan, false);
export constexpr Color Cyan(BaseColor::Cyan, true);
export constexpr Color White(BaseColor::White, false);
export constexpr Color BrightWhite(BaseColor::White, true);

constexpr NCURSES_PAIRS_T getColorId(NCURSES_COLOR_T front, NCURSES_COLOR_T back) noexcept { return static_cast<NCURSES_PAIRS_T>(1 + 2 * (front + back * NumBaseColor)); }

constexpr int getColorId(Color front, Color back) noexcept {
  return getColorId(getColorDef(front.baseColor), getColorDef(back.baseColor));
}

constexpr attr_t getColorModifer(Color front, Color back) noexcept {
  return (front.isBright ? WA_BOLD : WA_NORMAL) | (back.isBright ? WA_BLINK : WA_NORMAL);
}

export namespace Modifier {
  constexpr attr_t Standout = WA_STANDOUT;
  constexpr attr_t Normal = WA_NORMAL;
}

export class Symbol {
public:
  constexpr Symbol() noexcept = default;
  constexpr Symbol(chtype c) noexcept : character_(c), attributes_(Modifier::Normal), frontColor_(White), backColor_(Black){}; // NOLINT
  [[nodiscard]] constexpr chtype print() const noexcept { return character_; }
  constexpr void setFrontColor(Color c) noexcept {
    frontColor_ = c;
  }
  constexpr void setBackColor(Color c) noexcept {
    backColor_ = c;
  }
  [[nodiscard]] constexpr Color getFrontColor() const noexcept { return frontColor_; }
  [[nodiscard]] constexpr Color getBackColor() const noexcept { return backColor_; }
  constexpr void addModifier(chtype mod) noexcept { attributes_ |= mod; }
  constexpr void removeModifer(chtype mod) noexcept { attributes_ &= ~mod; }
  [[nodiscard]] constexpr attr_t modifers() const noexcept { return attributes_ | getColorModifer(frontColor_, backColor_); }
  [[nodiscard]] constexpr int color() const noexcept {return getColorId(frontColor_,backColor_);}

private:
  chtype character_;
  attr_t attributes_;
  Color frontColor_;
  Color backColor_;
};
#undef NCURSES_ACS
[[nodiscard]] constexpr chtype NCURSES_ACS(char c) noexcept { return A_ALTCHARSET + static_cast<chtype>(c); }
[[nodiscard]] consteval chtype operator""_toch(const char* str, unsigned long len) {
  std::array<char,4> data = {};
  data.fill(0);
  std::ranges::copy_n(str,len,data.rbegin());
  return std::bit_cast<chtype>(data);
}
export namespace SpecialChar {
constexpr chtype Escape = 27;
constexpr chtype Diamond = ACS_DIAMOND;
constexpr chtype Up = KEY_UP;
constexpr chtype Down = KEY_DOWN;
constexpr chtype Left = KEY_LEFT;
constexpr chtype Right = KEY_RIGHT;
constexpr chtype Backspace = KEY_BACKSPACE;
enum class Directions {
  None = 0,
  Up = 1,
  Down = Up<<1,
  Left = Down<<1,
  Right = Left<<1 
};
[[nodiscard]] constexpr Directions operator|(Directions d1, Directions d2) noexcept{
  return static_cast<Directions>(std::to_underlying(d1)|std::to_underlying(d2));
}
constexpr Directions& operator|=(Directions& d1, Directions d2) noexcept{
  d1 = d1 | d2;
  return d1;
}
constexpr auto Walls = [](){ //NOLINT says Walls is unused even though it is exported
  class RetType {
    public:
    [[nodiscard]] constexpr chtype& operator[](Directions dir) noexcept {
      return impl_[std::to_underlying(dir)];
    }
    [[nodiscard]] constexpr chtype operator[](Directions dir) const noexcept {
      return impl_[std::to_underlying(dir)];
    }
    private:
    std::array<chtype,(1<<4)> impl_;
  };
  using enum Directions;
  RetType ret;
  // auto q = 0x00002610;
  ret[None] = L'▯';
  ret[Up] = L'│';
  ret[Down] = L'│';
  ret[Left] = L'─';
  ret[Right] = L'─';
  ret[Up | Down] = L'│';
  ret[Up | Left] = L'┘';
  ret[Up | Right] = L'└';
  ret[Down | Left] = L'┐';
  ret[Down | Right] = L'┌';
  ret[Left | Right] = L'─';
  ret[Up | Down | Left] = L'┤';
  ret[Up | Down | Right] = L'├';
  ret[Up | Left | Right] = L'┴';
  ret[Down | Left | Right] = L'┬';
  ret[Up | Down | Left | Right] = L'┼';
  return ret;
}();
} // namespace SpecialChar



void initColors() noexcept {
  for (NCURSES_COLOR_T back = 0; back < NumBaseColor; back++) {
    for (NCURSES_COLOR_T front = 0; front < NumBaseColor; front++) {
      init_pair(getColorId(front, back), front, back);
    }
  }
}

export auto getChar() noexcept { return getch(); }

export class CursesRAII {
public:
  CursesRAII() {
    if (exists) {
      throw MultipleCursesRAII{};
    }
    exists = true;
    setlocale(LC_ALL, "");
    ESCDELAY = 4;
    // meta(stdscr, true);
    initscr();
    start_color();
    keypad(stdscr, true);
    initColors();
    cbreak();
    noecho();
    curs_set(0);
    refresh();
  }
  ~CursesRAII() noexcept {
    exists = false;
    endwin();
  }

private:
  static bool exists;
};
bool CursesRAII::exists = false;
export std::pair<int, int> getMaxDims() { return {getmaxy(stdscr), getmaxx(stdscr)}; }

template <class T>
concept NumberC = (std::integral<T> || std::floating_point<T>) && !std::same_as<T, char>;

constexpr std::string_view numInBuffer(NumberC auto num) noexcept {
  char *const bufferData = reinterpret_cast<char *>(buffer.data());
  return std::string_view(bufferData, std::to_chars(bufferData, bufferData + buffer.size(), num).ptr);
}

template<class T>
struct FormatSpecifer{
  static_assert(false);
};

template<>
struct FormatSpecifer<signed char>{
  static constexpr const char *Specifer = "%hhd";
};

template<>
struct FormatSpecifer<unsigned char>{
  static constexpr const char *Specifer = "%hhu";
};

template<>
struct FormatSpecifer<signed short>{ //NOLINT
  static constexpr const char *Specifer = "%hd";
};

template<>
struct FormatSpecifer<unsigned short>{ //NOLINT
  static constexpr const char *Specifer = "%hu";
};

template<>
struct FormatSpecifer<int>{
  static constexpr const char *Specifer = "%d";
};

template<>
struct FormatSpecifer<unsigned>{
  static constexpr const char *Specifer = "%u";
};

template<>
struct FormatSpecifer<long>{ //NOLINT
  static constexpr const char *Specifer = "%ld";
};

template<>
struct FormatSpecifer<unsigned long>{ //NOLINT
  static constexpr const char *Specifer = "%lu";
};

template<>
struct FormatSpecifer<long long>{ //NOLINT
  static constexpr const char *Specifer = "%lld";
};

template<>
struct FormatSpecifer<unsigned long long>{ //NOLINT
  static constexpr const char *Specifer = "%llu";
};

template<>
struct FormatSpecifer<float>{
  static constexpr const char *Specifer = "%g";
};

template<>
struct FormatSpecifer<double>{
  static constexpr const char *Specifer = "%g";
};

template<>
struct FormatSpecifer<long double>{
  static constexpr const char *Specifer = "%Lg";
};




export class WindowWrapper {
public:
  constexpr WindowWrapper() noexcept : impl_(nullptr), width_(0), height_(0), xoffset_(0), yoffset_(0){};
  WindowWrapper(int width, int height, int xoffset, int yoffset) noexcept : impl_(newwin(height, width, yoffset, xoffset)), width_(width), height_(height), xoffset_(xoffset), yoffset_(yoffset) {}
  void clear() const {
    if (werase(impl_.get()) == ERR) {
      throw EraseFailure{};
    }
  }
  void UpdateScreen() {
    if (wrefresh(impl_.get()) == ERR) {
      throw ScreenUpdateFailure{};
    }
  }
  bool Move(int x, int y) {
    if (x == xoffset_ && y == yoffset_) {
      return false;
    }
    xoffset_ = x;
    yoffset_ = y;
    if (mvwin(impl_.get(), y, x) == ERR) {
      throw BoxMoveFailure{};
    }
    return true;
  }
  bool SetDims(int x, int y) {
    if (x < 0 || y < 0) {
      throw BoxUnderBound{};
    }
    if (x == width_ && y == height_) {
      return false;
    }
    width_ = x;
    height_ = y;
    if (wresize(impl_.get(), y, x) == ERR) {
      throw BoxResizeFailure{};
    }
    return true;
  }
  [[nodiscard]] constexpr int GetWidth() const noexcept { return width_; }
  [[nodiscard]] constexpr int GetHeight() const noexcept { return height_; }
  [[nodiscard]] constexpr int GetXoffset() const noexcept { return xoffset_; }
  [[nodiscard]] constexpr int GetYoffset() const noexcept { return yoffset_; }
  void moveCursor(int x, int y) {
    if (wmove(impl_.get(), y, x) == ERR) {
      if(y>=GetHeight()||x>=GetWidth()){
        std::cout << y << ',' << x << ',' << GetHeight() << ',' << GetWidth() << '\n';
        std::exit(1);
      }
      throw MoveCursorFailure{};
    }
  }
  void place(chtype c) {
    place(Symbol(c));
  }
  void place(Symbol c) { 
      cchar_t ct;
      std::array<wchar_t,2> chars = {c.print(),0};
      setcchar(&ct, chars.data(), c.modifers(), c.color(), nullptr);
      wadd_wch(impl_.get(), &ct);
  }
  template <class CharT>
  void place(std::basic_string_view<CharT> str) {
    if (str.size() == 0) {
      return;
    }
    place_impl(str);
  }
  void place(NumberC auto num) {
    if(wprintw(impl_.get(),FormatSpecifer<decltype(num)>::Specifer,num)==ERR){
      throw PrintFailure{};
    }
  }
  [[nodiscard]] int cursorX() const noexcept { return getcurx(impl_.get()); }
  [[nodiscard]] int cursorY() const noexcept { return getcury(impl_.get()); }

private:
  void place_impl(std::basic_string_view<chtype> str) {
    if (waddnwstr(impl_.get(), str.data(), static_cast<int>(str.size())) == ERR) {
      throw PrintFailure{};
    }
  }
  void place_impl(std::string_view str) {
    if (waddnstr(impl_.get(), str.data(), static_cast<int>(str.size())) == ERR) {
      throw PrintFailure{};
    }
  }
  void place_impl(std::basic_string_view<Symbol> str) {
    for(auto i : str){
      place(i);
    }
  }
  using ptrTyp = std::unique_ptr<WINDOW, decltype([](WINDOW* w){delwin(w);})>;
  ptrTyp impl_;
  int width_;
  int height_;
  int xoffset_;
  int yoffset_;
  // derwin must be used
};
class WindowWrapper;
template <class T>
concept Printable = requires(WindowWrapper &w, T a) {
  { w.place(a) };
};

template <class T>
struct IsViewS : public std::false_type {};

template <class T>
struct IsViewS<std::basic_string_view<T>> : public std::true_type {};

template <class T>
concept PrintableView = Printable<T> && static_cast<bool>(IsViewS<T>{});

template <class T>
concept PrintableChar = Printable<T> && static_cast<bool>(!IsViewS<T>{});

export using Symbol_string_view = std::basic_string_view<Symbol>;
static_assert(Printable<Symbol_string_view>);
export class BoxedWindow {
public:
  constexpr BoxedWindow() noexcept = default;
  BoxedWindow(int width, int height, int xoffset, int yoffset) : impl_(width + 2, height + 2, xoffset, yoffset) {
    makeBox();
    updateScreen();
  }
  void setDims(int width, int height) {
    if (impl_.SetDims(width + 2, height + 2)) {
      makeBox();
    }
  }
  void place(PrintableChar auto sym) { impl_.place(sym); }
  void place(PrintableView auto view) { impl_.place(view.substr(0, leftOnLine())); }
  void place(const char* str){place(std::string_view(str));}
  void place(int x, int y, Printable auto sym) {
    moveCursor(x, y);
    place(sym);
  }
  [[nodiscard]] int cursorX() const noexcept { return impl_.cursorX() - 1; }
  [[nodiscard]] int cursorY() const noexcept { return impl_.cursorY() - 1; }
  void moveCursor(int x, int y) { return impl_.moveCursor(x + 1, y + 1); }
  BoxedWindow &operator<<(Printable auto sym) {
    place(sym);
    return *this;
  }

  void updateScreen() { impl_.UpdateScreen(); }
  void move(int x, int y) { impl_.Move(x, y); }
  [[nodiscard]] constexpr int prntWidth() const noexcept { return impl_.GetWidth() - 2; }
  [[nodiscard]] constexpr int prntHeight() const noexcept { return impl_.GetHeight() - 2; }
  void clear() {
    impl_.clear();
    makeBox();
  }
  [[nodiscard]] std::size_t leftOnLine() const noexcept { return std::max(prntWidth() - impl_.cursorX(),0); }

private:
  void makeBox() {
    impl_.clear();
    impl_.moveCursor(0, 0);
    impl_.place(L'┌');
    impl_.moveCursor(prntWidth() + 1, 0);
    impl_.place(L'┐');
    impl_.moveCursor(0, prntHeight() + 1);
    impl_.place(L'└');
    impl_.moveCursor(prntWidth() + 1, prntHeight() + 1);
    impl_.place(L'┘');
    impl_.moveCursor(1, 0);
    for (int i0 = 0; i0 < 2; i0++) {
      for (int i = 1; i < prntWidth() + 1; i++) {
        impl_.place(L'─');
      }
      impl_.moveCursor(1, prntHeight() + 1);
    }
    for (int i = 1; i < prntHeight() + 1; i++) {
      impl_.moveCursor(0, i);
      impl_.place(L'│');
      impl_.moveCursor(prntWidth() + 1, i);
      impl_.place(L'│');
    }
  }
  WindowWrapper impl_;
};
}  // namespace CursesLowLevel