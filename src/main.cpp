import std;
import CursesIO;
import Game;

void runGame(IOModule::Interface &interface) {
  interface.updateGameScreen();
  while (interface.doAction()) {
    interface.updateGameScreen();
  }
}

void doMain() {
  IOModule::Interface IORII;
  IORII.createTiedGameInterface();
  runGame(IORII);
}

int main() {
  try {
    doMain();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}