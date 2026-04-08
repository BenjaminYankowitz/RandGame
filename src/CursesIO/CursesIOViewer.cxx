module CursesIO;

std::streamsize PrintToViewer::xsputn(const char_type *s, std::streamsize count) {
  std::string_view input(s, count);
  std::size_t currentStart = 0;
  while (true) {
    auto nextNewLine = input.find('\n', currentStart);
    if (nextNewLine == std::string_view::npos) {
      buffer_ += input.substr(currentStart);
      break;
    }
    buffer_ += input.substr(currentStart, nextNewLine - currentStart);
    parent_->addEvent(std::move(buffer_));
    buffer_.clear();
    currentStart = nextNewLine + 1;
  }
  return count;
}

void CursesEventViewer::debug(std::string_view message) {
  printWith_ << message << '\n';
}

void CursesEventViewer::exception(const std::exception &exception) noexcept {
  const auto time = viewer_.parent_->getTime().impl;
  std::fstream logfile("log.txt", std::ios_base::out | std::ios_base::app);
  if (!logfile.is_open()) {
    std::cerr << "Unhandeled exception, and log file does not open\n"
              << time << ": " << exception.what() << '\n';
    std::exit(1);
  }
  logfile << time << ": " << exception.what() << '\n';
  if (logfile.bad()) {
    std::cerr << "I/O error while reading - badbit is true\n"
              << exception.what() << '\n';
    std::exit(1);
  } else if (logfile.fail()) {
    std::cerr << "Logical error on i/o operation - failbit is true\n"
              << exception.what() << '\n';
    std::exit(1);
  }
  logfile.sync();
}

void CursesEventViewer::itemPickup(MonsterInterface grabber, ObjectInterface grabed) {
  printWith_ << grabber << " picked up " << grabed << '\n';
}

void CursesEventViewer::monsterHitMonster(HitInfo info, MonsterInterface attacker, MonsterInterface attacked) {
  if (attacked.isPlayer()) {
    viewer_.parent_->alertBeenHit();
  }
  printWith_ << attacker << ' ' << (info.killed ? "killed" : "hit") << ' ' << attacked;
  if (info.damageDone) {
    printWith_ << ' ' << (info.killed ? "by dealing" : "for") << ' ' << *info.damageDone << " damage";
  }
  printWith_ << '\n';
}

void CursesEventViewer::monsterHitWall(MonsterInterface attacker, TerrainType attacked) {
  printWith_ << attacker << " hit " << attacked << '\n';
}

void CursesEventViewer::monsterAte(MonsterInterface eater, ObjectInterface eaten) {
  printWith_ << eater << " ate " << eaten << '\n';
}
