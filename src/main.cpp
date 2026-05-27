#include "main.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>

int main() {
  auto window{create_window()};
  auto board{make_canonical_board()};

  while (window.isOpen()) {
    handle_events(window);

    window.clear(lightYellow);

    window.display();
  }

  return 0;
}