import std;
import CursesIO;
import Game;
import GameInterface;

namespace {
void runGame(IOModule::Interface &interface) {
  interface.updateGameScreen();
  while (interface.doAction()) {
    interface.updateGameScreen();
  }
}

void doMain() {
  GameState game;
  IOModule::Interface IORII(std::make_unique<GameInterface>(reinterpret_cast<IGameState&>(game),game.getPlayer().getId()));
  runGame(IORII);
}
}  // namespace

int main() {
  try {
    doMain();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  std::cout << "so long and thanks for all the fish\n";
  return 0;
}