#include <clocale>
#include <cursesw.h>
import Common;
import std;

constexpr int MapW = 40;
constexpr int MapH = 20;

struct InstrumentedMap {
  const StaticPositionArr<bool> &real;
  [[nodiscard]] constexpr int extent(int n) const noexcept {
    switch (n) {
    case 0:
      return real.rows();
    case 1:
      return real.cols();
    default:
      std::unreachable();
    }
  }
  mutable std::vector<Position> checked;
  [[nodiscard]] bool operator[](Position p) const {
    checked.push_back(p);
    return real[p];
  }
};

StaticPositionArr<bool> buildMap() {
  StaticPositionArr<bool> map(MapW, MapH);
  // Fill with open
  for (int y = 0; y < MapH; y++)
    for (int x = 0; x < MapW; x++)
      map[Position{x, y}] = true;

  // Border walls
  for (int x = 0; x < MapW; x++) {
    map[Position{x, 0}] = false;
    map[Position{x, MapH - 1}] = false;
  }
  for (int y = 0; y < MapH; y++) {
    map[Position{0, y}] = false;
    map[Position{MapW - 1, y}] = false;
  }

  // Some interior walls for testing
  for (int y = 3; y < 12; y++)
    map[Position{10, y}] = false;
  for (int x = 10; x < 20; x++)
    map[Position{x, 8}] = false;
  for (int y = 5; y < 15; y++)
    map[Position{25, y}] = false;
  for (int x = 15; x < 25; x++)
    map[Position{x, 14}] = false;
  // A small room
  for (int x = 30; x < 37; x++) {
    map[Position{x, 3}] = false;
    map[Position{x, 9}] = false;
  }
  for (int y = 3; y < 10; y++) {
    map[Position{30, y}] = false;
    map[Position{36, y}] = false;
  }
  // Door in the room
  map[Position{33, 3}] = true;

  return map;
}

Dir keyToDir(int key) {
  switch (key) {
  case KEY_LEFT:
  case 'h':
    return {-1, 0};
  case KEY_DOWN:
  case 'j':
    return {0, 1};
  case KEY_UP:
  case 'k':
    return {0, -1};
  case KEY_RIGHT:
  case 'l':
    return {1, 0};
  case 'y':
    return {-1, -1};
  case 'u':
    return {1, -1};
  case 'b':
    return {-1, 1};
  case 'n':
    return {1, 1};
  default:
    return {0, 0};
  }
}

struct BoolMap {
  const StaticPositionArr<bool> &real;
  [[nodiscard]] constexpr int extent(int n) const noexcept {
    switch (n) {
    case 0:
      return real.rows();
    case 1:
      return real.cols();
    default:
      std::unreachable();
    }
  }
  [[nodiscard]] bool operator[](Position p) const { return real[p]; }
};

StaticPositionArr<int> computeVisibility(const StaticPositionArr<bool> &map, Position from) {
  StaticPositionArr<int> visible(MapW, MapH);
  visible.fill(0);
  BoolMap bmap{map};
  for (int y = 0; y < MapH; y++)
    for (int x = 0; x < MapW; x++)
      visible[Position{x, y}] = LineOfSight::inLineOfSight(bmap, from, Position{x, y}) ? 1 : 0;
  return visible;
}

StaticPositionArr<int> computeVisibilityAll(const StaticPositionArr<bool> &map, Position from) {
  StaticPositionArr<int> visible(MapW, MapH);
  visible.fill(0);
  BoolMap bmap{map};
  for (auto p : LineOfSight::allInLineOfSight(bmap, from))
    visible[p]++;
  return visible;
}

// mode=0: normal (with visibility overlay from cursor), mode=1: show checked tiles result
void drawMap(const StaticPositionArr<bool> &map, Position cursor,
             std::optional<Position> start,
             const std::vector<Position> *checked, Position end,
             bool losResult,
             const StaticPositionArr<int> *visibility) {
  // Build a set of checked positions for fast lookup
  auto posHash = [](Position p) { return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 16); };
  auto posEq = [](Position a, Position b) { return a.x == b.x && a.y == b.y; };
  std::unordered_set<Position, decltype(posHash), decltype(posEq)> checkedSet(0, posHash, posEq);
  if (checked) {
    for (auto p : *checked)
      checkedSet.insert(p);
  }

  for (int y = 0; y < MapH; y++) {
    for (int x = 0; x < MapW; x++) {
      Position p{x, y};
      if (checked && p == *start) {
        attron(COLOR_PAIR(3));
        mvaddch(y, x, 'S');
        attroff(COLOR_PAIR(3));
      } else if (checked && p == end) {
        attron(COLOR_PAIR(losResult ? 3 : 2));
        mvaddch(y, x, 'E');
        attroff(COLOR_PAIR(losResult ? 3 : 2));
      } else if (checked && checkedSet.contains(p)) {
        attron(COLOR_PAIR(4));
        mvaddch(y, x, '*');
        attroff(COLOR_PAIR(4));
      } else if (!map[p]) {
        // Wall: bright if visible from cursor, dim otherwise
        int vis = visibility ? (*visibility)[p] : 0;
        int pair = vis > 1 ? 7 : vis > 0 ? 5
                                         : 6;
        attron(COLOR_PAIR(pair));
        mvaddch(y, x, '#');
        attroff(COLOR_PAIR(pair));
      } else {
        // Open tile: green if visible, blue if visible multiple times, dim otherwise
        int vis = visibility ? (*visibility)[p] : 0;
        if (vis > 1) {
          attron(COLOR_PAIR(7));
          mvaddch(y, x, '.');
          attroff(COLOR_PAIR(7));
        } else if (vis > 0) {
          attron(COLOR_PAIR(3));
          mvaddch(y, x, '.');
          attroff(COLOR_PAIR(3));
        } else {
          attron(COLOR_PAIR(6));
          mvaddch(y, x, '.');
          attroff(COLOR_PAIR(6));
        }
      }
    }
  }

  // Draw cursor and start marker on top
  if (!checked) {
    if (start) {
      attron(COLOR_PAIR(3) | A_BOLD);
      mvaddch(start->y, start->x, 'S');
      attroff(COLOR_PAIR(3) | A_BOLD);
    }
    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddch(cursor.y, cursor.x, '+');
    attroff(COLOR_PAIR(1) | A_BOLD);
  }
}

int main() {
  setlocale(LC_ALL, "");
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  init_pair(1, COLOR_YELLOW, COLOR_BLACK); // cursor
  init_pair(2, COLOR_RED, COLOR_BLACK);    // blocked
  init_pair(3, COLOR_GREEN, COLOR_BLACK);  // start / visible end
  init_pair(4, COLOR_CYAN, COLOR_BLACK);   // checked tiles
  init_pair(5, COLOR_WHITE, COLOR_BLACK);  // walls (visible)
  init_pair(6, COLOR_RED, COLOR_BLACK);    // not visible
  init_pair(7, COLOR_BLUE, COLOR_BLACK);   // visible multiple times

  auto map = buildMap();

  while (true) {
    Position cursor{MapW / 2, MapH / 2};

    bool useAllLOS = false;

    // Phase 1: pick start
    while (true) {
      auto vis = useAllLOS ? computeVisibilityAll(map, cursor) : computeVisibility(map, cursor);
      erase();
      drawMap(map, cursor, std::nullopt, nullptr, {}, false, &vis);
      mvprintw(MapH + 1, 0, "Select START tile (hjkl/arrows, '.' confirm, 'w' wall, 'a' toggle allLOS, 'q' quit)");
      mvprintw(MapH + 2, 0, "Cursor: (%d, %d)  %s  [LOS mode: %s]", cursor.x, cursor.y,
               map[cursor] ? "open" : "WALL",
               useAllLOS ? "allInLineOfSight" : "per-tile inLineOfSight");
      refresh();

      int ch = getch();
      if (ch == 'q' || ch == 27)
        goto done;
      if (ch == '.')
        break;
      if (ch == 'a') {
        useAllLOS = !useAllLOS;
        continue;
      }
      if (ch == 'w') {
        map[cursor] = !map[cursor];
        continue;
      }
      auto d = keyToDir(ch);
      Position next = cursor + d;
      if (next.x >= 0 && next.x < MapW && next.y >= 0 && next.y < MapH)
        cursor = next;
    }

    Position start = cursor;

    // Phase 2: pick end
    while (true) {
      auto vis = useAllLOS ? computeVisibilityAll(map, start) : computeVisibility(map, start);
      erase();
      drawMap(map, cursor, start, nullptr, {}, false, &vis);
      mvprintw(MapH + 1, 0, "Select END tile (hjkl/arrows, '.' confirm, 'w' wall, 'a' toggle allLOS, Esc cancel)");
      mvprintw(MapH + 2, 0, "Start: (%d, %d)  Cursor: (%d, %d)  %s  [LOS: %s]",
               start.x, start.y, cursor.x, cursor.y,
               map[cursor] ? "open" : "WALL",
               useAllLOS ? "allInLineOfSight" : "per-tile");
      refresh();

      int ch = getch();
      if (ch == 27)
        break;
      if (ch == 'q')
        goto done;
      if (ch == 'a') {
        useAllLOS = !useAllLOS;
        continue;
      }
      if (ch == '.') {
        Position end = cursor;

        // Run instrumented LOS
        InstrumentedMap imap{map, {}};
        bool result = LineOfSight::inLineOfSight(imap, start, end);

        // Phase 3: show results
        erase();
        drawMap(map, cursor, start, &imap.checked, end, result, nullptr);
        mvprintw(MapH + 1, 0, "Start: (%d, %d) -> End: (%d, %d)  Result: %s",
                 start.x, start.y, end.x, end.y,
                 result ? "VISIBLE" : "BLOCKED");
        mvprintw(MapH + 2, 0, "Tiles checked: %zu  (press any key to reset, 'q' to quit)",
                 imap.checked.size());
        refresh();

        int ch2 = getch();
        if (ch2 == 'q' || ch2 == 27)
          goto done;
        break; // reset to pick new start
      }
      if (ch == 'w') {
        map[cursor] = !map[cursor];
        continue;
      }
      auto d = keyToDir(ch);
      Position next = cursor + d;
      if (next.x >= 0 && next.x < MapW && next.y >= 0 && next.y < MapH)
        cursor = next;
    }
  }

done:
  endwin();
  return 0;
}
