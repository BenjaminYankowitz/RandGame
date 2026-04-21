module;
#include <algorithm>
#include <array>
#include <cstdint>
#include <cursesw.h>
#include <exception>
#include <iostream>
#include <memory>
#include <ranges>
#include <string_view>
#include <utility>
export module CursesLowLevel;
namespace CursesLowLevel {
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
struct BoxResizeFailure : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Tried to resize Box and failed"; }
};
struct BoxUnderBound : public WindowFailure {
  [[nodiscard]] const char *what() const noexcept override { return "Tried to resize Box to smaller than min size"; }
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
export using attr_t = attr_t;
export using chtype = wchar_t;
export enum class BaseColor : std::uint8_t { Black,
                                             Red,
                                             Green,
                                             Brown,
                                             Blue,
                                             Magenta,
                                             Cyan,
                                             White };
namespace {

} // namespace
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
constexpr auto ColorMap = std::to_array<std::pair<BaseColor, NCURSES_COLOR_T>>({
    {BaseColor::Black, COLOR_BLACK},
    {BaseColor::Red, COLOR_RED},
    {BaseColor::Green, COLOR_GREEN},
    {BaseColor::Brown, COLOR_YELLOW},
    {BaseColor::Blue, COLOR_BLUE},
    {BaseColor::Magenta, COLOR_MAGENTA},
    {BaseColor::Cyan, COLOR_CYAN},
    {BaseColor::White, COLOR_WHITE},
});
export class Color {
public:
  BaseColor baseColor;
  bool isBright;
  constexpr Color() = default;
  constexpr Color(BaseColor baseColor_, bool isBright_) noexcept : baseColor(baseColor_), isBright(isBright_) {}
  [[nodiscard]] static constexpr BaseColor getEnum(NCURSES_COLOR_T color) noexcept {
    const auto *it = std::ranges::find(ColorMap, color, &std::pair<BaseColor, NCURSES_COLOR_T>::second);
    assert(it != ColorMap.end());
    return it != ColorMap.end() ? it->first : BaseColor::White;
  }
  [[nodiscard]] static constexpr NCURSES_COLOR_T getDef(BaseColor color) noexcept {
    const auto *it = std::ranges::find(ColorMap, color, &std::pair<BaseColor, NCURSES_COLOR_T>::first);
    return it->second;
  }
  [[nodiscard]] static constexpr NCURSES_PAIRS_T getId(NCURSES_COLOR_T front, NCURSES_COLOR_T back) noexcept { return static_cast<NCURSES_PAIRS_T>(1 + (2 * (front + (back * NumBaseColor)))); }
  [[nodiscard]] static constexpr NCURSES_PAIRS_T getId(Color front, Color back) noexcept {
    return getId(getDef(front.baseColor), getDef(back.baseColor));
  }
  [[nodiscard]] static constexpr attr_t getColorModifer(Color front, Color back) noexcept {
    return (front.isBright ? WA_BOLD : WA_NORMAL) | (back.isBright ? WA_BLINK : WA_NORMAL);
  }
};

static_assert(std::ranges::all_of(std::views::iota(0, NumBaseColor), [](int i) { return Color::getDef(Color::getEnum(i)) == i; }));
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

export namespace Modifier {
constexpr attr_t Standout = WA_STANDOUT;
constexpr attr_t Normal = WA_NORMAL;
} // namespace Modifier

export class Symbol {
public:
  constexpr Symbol() noexcept = default;
  constexpr Symbol(chtype c) noexcept : character_(c), attributes_(Modifier::Normal), frontColor_(White), backColor_(Black) {}; // NOLINT(google-explicit-constructor)
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
  [[nodiscard]] constexpr attr_t modifers() const noexcept { return attributes_ | Color::getColorModifer(frontColor_, backColor_); }
  [[nodiscard]] constexpr int color() const noexcept { return Color::getId(frontColor_, backColor_); }

private:
  chtype character_;
  attr_t attributes_;
  Color frontColor_;
  Color backColor_;
};
#undef NCURSES_ACS
namespace {
[[nodiscard]] constexpr chtype NCURSES_ACS(char c) noexcept { return A_ALTCHARSET + static_cast<chtype>(c); }
} // namespace
[[nodiscard]] consteval chtype operator""_toch(const char *str, unsigned long len) {
  std::array<char, 4> data = {};
  data.fill(0);
  std::ranges::copy_n(str, len, data.rbegin());
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
constexpr chtype ShiftLeft = KEY_SLEFT;
constexpr chtype ShiftRight = KEY_SRIGHT;
constexpr chtype ShiftUp = KEY_SR;
constexpr chtype ShiftDown = KEY_SF;
} // namespace SpecialChar

namespace {
void initColors() noexcept {
  for (NCURSES_COLOR_T back = 0; back < NumBaseColor; back++) {
    for (NCURSES_COLOR_T front = 0; front < NumBaseColor; front++) {
      init_pair(Color::getId(front, back), front, back);
    }
  }
}
} // namespace

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
  CursesRAII(const CursesRAII&) = delete;
  CursesRAII(const CursesRAII&&) = delete;
  CursesRAII& operator=(const CursesRAII&) = delete;
  CursesRAII& operator=(const CursesRAII&&) = delete;
  ~CursesRAII() noexcept {
    exists = false;
    endwin();
  }
  static auto getChar() noexcept { return getch(); }
  static std::optional<chtype> tryGetChar() noexcept {
    timeout(0);
    auto ch = getch();
    timeout(-1);
    return ch == ERR ? std::nullopt : std::optional<chtype>(ch);
  }
  static int setCursorState(int n) {
    return curs_set(n);
  }

private:
  static bool exists;
};
bool CursesRAII::exists = false;
export std::pair<int, int> getMaxDims() { return {getmaxy(stdscr), getmaxx(stdscr)}; }

template <class T>
concept NumberC = (std::integral<T> || std::floating_point<T>) && !std::same_as<T, char>;
namespace {
struct SymbolTraits { // should be specialzation of std::char_traits but that's not working due to compiler bug
  using char_type = Symbol;
};
} // namespace

export using string_view_Symbol = std::basic_string_view<Symbol, SymbolTraits>;
export class WindowWrapper {
public:
  WindowWrapper() noexcept : WindowWrapper(0,0,0,0){}
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
  bool move(int x, int y) {
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
  [[nodiscard]] int GetWidth() const noexcept { return width_; }
  [[nodiscard]] int GetHeight() const noexcept { return height_; }
  [[nodiscard]] int GetXoffset() const noexcept { return xoffset_; }
  [[nodiscard]] int GetYoffset() const noexcept { return yoffset_; }
  void moveCursor(int x, int y) {
    if (wmove(impl_.get(), y, x) == ERR) {
      throw MoveCursorFailure{};
    }
  }
  void place(std::same_as<chtype> auto c) {
    place(Symbol(c));
  }
  void place(std::same_as<Symbol> auto c) {
    cchar_t ct;
    std::array<wchar_t, 2> chars = {c.print(), 0};
    setcchar(&ct, chars.data(), c.modifers(), c.color(), nullptr);
    wadd_wch(impl_.get(), &ct);
  }
  template <class CharT, class TypeTraits>
  void place(std::basic_string_view<CharT, TypeTraits> str) {
    if (str.size() == 0) {
      return;
    }
    place_impl(str);
  }
  void placeAt(auto toPlace, int x, int y){
    moveCursor(x,y);
    place(toPlace);
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
  void place_impl(string_view_Symbol str) {
    for (auto i : str) {
      place(i);
    }
  }
  using ptrTyp = std::unique_ptr<WINDOW, decltype([](WINDOW *w) { delwin(w); })>;
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
concept NPrintable = !Printable<T>;
namespace {
template <class T>
struct IsViewS : public std::false_type {};
template <class T>
struct IsViewS<std::basic_string_view<T>> : public std::true_type {};
} // namespace

template <class T>
concept PrintableView = Printable<T> && static_cast<bool>(IsViewS<T>{});

template <class T>
concept PrintableChar = Printable<T> && static_cast<bool>(!IsViewS<T>{});

static_assert(Printable<string_view_Symbol>);
static_assert(NPrintable<int>);

export class BoxedWindow;
class WindowStreamBuf : public std::streambuf {
public:
  explicit WindowStreamBuf(BoxedWindow *parent) noexcept : parent_(parent) {}

protected:
  std::streamsize xsputn(const char_type *s, std::streamsize count) override;
  int_type overflow(int_type ch) override;

private:
  BoxedWindow *parent_;
};

export class BoxedWindow {
public:
  BoxedWindow() : BoxedWindow(0,0,0,0){}
  BoxedWindow(int width, int height, int xoffset, int yoffset) : impl_(width + 2, height + 2, xoffset, yoffset) {
    makeBox();
    updateScreen();
  }
  void setDims(int width, int height) {
    if (width < 0 || height < 0) {
      throw BoxUnderBound{};
    }
    if (impl_.SetDims(width + 2, height + 2)) {
      makeBox();
    }
  }
  void place(PrintableChar auto sym) { impl_.place(sym); }
  void place(PrintableView auto view) { impl_.place(view.substr(0, leftOnLine())); }
  void place(const char *str) { place(std::string_view(str)); }
  void place(NPrintable auto const &obj) {
    ostream_ << obj;
  }
  void place(int x, int y, Printable auto sym) {
    moveCursor(x, y);
    place(sym);
  }
  [[nodiscard]] int cursorX() const noexcept { return impl_.cursorX() - 1; }
  [[nodiscard]] int cursorY() const noexcept { return impl_.cursorY() - 1; }
  [[nodiscard]] bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x <= prntWidth() && y <= prntHeight(); }
  void moveCursor(int x, int y) { impl_.moveCursor(x + 1, y + 1); }
  BoxedWindow &operator<<(auto sym) {
    place(sym);
    return *this;
  }

  void updateScreen() { impl_.UpdateScreen(); }
  void move(int x, int y) { impl_.move(x, y); }
  [[nodiscard]] constexpr int prntWidth() const noexcept { return impl_.GetWidth() - 2; }
  [[nodiscard]] constexpr int prntHeight() const noexcept { return impl_.GetHeight() - 2; }
  void clear() {
    makeBox();
  }
  [[nodiscard]] std::size_t leftOnLine() const noexcept { return std::max(prntWidth() - impl_.cursorX(), 0); }

private:
  void makeBox() {
    const int rSide = prntWidth() + 1;
    const int dSide = prntHeight() + 1;
    impl_.clear();
    impl_.placeAt(L'┌',0,0);
    impl_.placeAt(L'┐',rSide, 0);
    impl_.placeAt(L'└',0, dSide);
    impl_.placeAt(L'┘',rSide, dSide);
    impl_.moveCursor(1, 0);
    for (int i : std::views::iota(1,rSide)) {
      impl_.placeAt(L'─', i, 0);
      impl_.placeAt(L'─', i, dSide);
    }
    for (int i  : std::views::iota(1,dSide)) {
      impl_.placeAt(L'│',0,i);
      impl_.placeAt(L'│',rSide, i);
    }
  }
  WindowWrapper impl_;
  WindowStreamBuf streamBuf_{this};
  std::ostream ostream_{&streamBuf_};
};

std::streamsize WindowStreamBuf::xsputn(const char_type *s, std::streamsize count) {
  parent_->place(std::string_view(s, static_cast<std::size_t>(count)));
  return count;
}

auto WindowStreamBuf::overflow(int_type ch) -> int_type {
  if (ch != traits_type::eof()) {
    char c = traits_type::to_char_type(ch);
    parent_->place(std::string_view(&c, 1));
  }
  return ch;
}

} // namespace CursesLowLevel
