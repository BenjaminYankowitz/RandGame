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

constexpr auto SaveFileName = "RandGameSave";

void doMain() {
  GameState game;
  auto gi = std::make_unique<GameInterface>(reinterpret_cast<IGameState &>(game), game.getPlayer().getId());
  std::string loadError;
  std::ifstream file(SaveFileName, std::ios::binary);
  if (file) {
    auto result = gi->load(file);
    if (!result.ok()) {
      if (result.error == GameInterface::LoadResult::Error::BadMagic)
        loadError = "Failed to load save: not a valid save file.";
      else
        loadError = "Failed to load save: version mismatch (file version " + std::to_string(result.fileVersion) + ", expected " + std::to_string(result.expectedVersion) + ").";
    }
  }
  IOModule::Interface IORII(std::move(gi));
  if (!loadError.empty()) {
    IORII.addEvent(std::move(loadError));
  }
  runGame(IORII);
}
} // namespace

int main() {
  try {
    doMain();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  std::cout << "so long and thanks for all the fish\n";
  return 0;
}