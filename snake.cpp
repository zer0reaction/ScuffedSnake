// TODO
// Animations

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>

// Showing what is in a grid vector to the window
void renderField(sf::RenderWindow &window, std::vector<std::vector<int>> grid,
                 int gridSize, int squareSize, int borderThickness, int head_y,
                 int head_x, std::string direction, sf::Text scoreText) {
  window.clear();

  for (int y = 0; y < gridSize; y++)
    for (int x = 0; x < gridSize; x++) {
      sf::RectangleShape r(sf::Vector2f(squareSize, squareSize));
      r.setOutlineThickness(borderThickness);
      r.setPosition(x * squareSize, y * squareSize);
      r.setOutlineColor(sf::Color::Black);

      // If it is snake's body
      if (grid[y][x] >= 1) {
        r.setFillColor(sf::Color::White);
        r.setOutlineColor(sf::Color::Black);
      }
      // If it is an apple
      else if (grid[y][x] == -1) {
        r.setFillColor(sf::Color::Green);
        r.setOutlineColor(sf::Color::Black);
      }
      // If it is a wall
      else if (grid[y][x] == -2) {
        r.setFillColor(sf::Color::Red);
        r.setOutlineColor(sf::Color::Black);
      } else {
        r.setFillColor(sf::Color::Black);
        r.setOutlineColor(sf::Color(80, 80, 80));
        r.setOutlineThickness(-1);
      }

      window.draw(r);
    }

  sf::RectangleShape eye1(sf::Vector2f(squareSize / 5, squareSize / 5));
  sf::RectangleShape eye2(sf::Vector2f(squareSize / 5, squareSize / 5));
  eye1.setFillColor(sf::Color::Red);
  eye2.setFillColor(sf::Color::Red);

  int xOffset1, yOffset1, xOffset2, yOffset2;

  if (direction == "up") {
    xOffset1 = 10;
    xOffset2 = squareSize - 10 - 10;
    yOffset1 = 0;
    yOffset2 = 0;
  } else if (direction == "down") {
    xOffset1 = 10;
    xOffset2 = squareSize - 10 - 10;
    yOffset1 = squareSize - 10;
    yOffset2 = squareSize - 10;
  } else if (direction == "left") {
    xOffset1 = 0;
    xOffset2 = 0;
    yOffset1 = 10;
    yOffset2 = squareSize - 10 - 10;
  } else if (direction == "right") {
    xOffset1 = squareSize - 10;
    xOffset2 = squareSize - 10;
    yOffset1 = 10;
    yOffset2 = squareSize - 10 - 10;
  }

  eye1.setPosition(head_x * squareSize + xOffset1,
                   head_y * squareSize + yOffset1);
  eye2.setPosition(head_x * squareSize + xOffset2,
                   head_y * squareSize + yOffset2);
  window.draw(eye1);
  window.draw(eye2);
  window.draw(scoreText);

  window.display();
}

// Tries to generate an apple, returns false if it can't be generated
bool generateApple(std::vector<std::vector<int>> &grid, int gridSize) {
  bool canGenerate = false;
  std::vector<std::vector<int>> freeSpaces(0, std::vector<int>(2));

  for (int y = 0; y < gridSize; y++)
    for (int x = 0; x < gridSize; x++) {
      if (grid[y][x] == 0) {
        canGenerate = true;
        std::vector<int> v = {y, x};
        freeSpaces.push_back(v);
      }
    }

  if (!canGenerate)
    return false;

  std::vector<int> applePos = freeSpaces[rand() % freeSpaces.size()];

  grid[applePos[0]][applePos[1]] = -1;

  return true;
}

// Checking what the snake hits next
std::string checkCollision(std::vector<std::vector<int>> &grid, int gridSize,
                           std::string direction, int head_y, int head_x) {
  // Next position of the snake's head
  int next_y = head_y, next_x = head_x;

  // Setting next head position and checking if it hit a wall
  if (direction == "up" && head_y != 0)
    next_y--;
  else if (direction == "down" && head_y != gridSize - 1)
    next_y++;
  else if (direction == "left" && head_x != 0)
    next_x--;
  else if (direction == "right" && head_x != gridSize - 1)
    next_x++;
  else
    return "wall";

  // Checking other collisions
  if (grid[next_y][next_x] == -2)
    return "wall";
  else if (grid[next_y][next_x] > 0)
    return "snake";
  else if (grid[next_y][next_x] == -1)
    return "apple";
  else if (grid[next_y][next_x] == 0)
    return "air";

  return "error";
}

// Decreases every grid cell by 1 (except walls and aplle)
void decreaseLength(std::vector<std::vector<int>> &grid, int gridSize) {
  for (int y = 0; y < gridSize; y++)
    for (int x = 0; x < gridSize; x++)
      if (grid[y][x] > 0)
        grid[y][x]--;
}

// Reading user keyboard input
void userInput(std::string &direction, bool &usedMove) {
  while (true) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && direction != "down" &&
        !usedMove) {
      direction = "up";
      usedMove = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && direction != "right" &&
        !usedMove) {
      direction = "left";
      usedMove = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && direction != "up" &&
        !usedMove) {
      direction = "down";
      usedMove = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && direction != "left" &&
        !usedMove) {
      direction = "right";
      usedMove = true;
    }
    sf::sleep(sf::milliseconds(50));
  }
}

void stopGame(sf::RenderWindow &window, sf::Thread &thread) {
  window.close();
  thread.terminate();
}

// Moves the snake
void moveSnake(std::vector<std::vector<int>> &grid, int gridSize,
               std::string direction, int &head_y, int &head_x, int &length,
               sf::RenderWindow &window, sf::Thread &thread, bool &usedMove) {
  std::vector newGrid = grid; // Not used when the snake eats an apple
  decreaseLength(newGrid, gridSize);

  // Setting next head position
  int next_y = head_y, next_x = head_x;
  if (direction == "up")
    next_y--;
  else if (direction == "down")
    next_y++;
  else if (direction == "left")
    next_x--;
  else if (direction == "right")
    next_x++;

  std::string collision =
      checkCollision(newGrid, gridSize, direction, head_y, head_x);

  if (collision == "wall" || collision == "snake") {
    stopGame(window, thread);
  } else if (collision == "air") {
    newGrid[next_y][next_x] = length;
    head_x = next_x;
    head_y = next_y;
    grid = newGrid;
  } else if (collision == "apple") {
    length++;
    grid[next_y][next_x] = length;
    head_x = next_x;
    head_y = next_y;
    generateApple(grid, gridSize);
  }
}

int main() {
  unsigned seed = (unsigned)time(NULL);
  srand(seed); // Setting a seed for random number generator

  const int gridSize = 20;        // Size of the square field
  const int squareSize = 50;      // Size of the rectangles displayed
  const int borderThickness = -1; // Thickness of the rectangle's border
  const int framerateLimit = 5;   // FPS, calculations are done every frame
  int length = 3;                 // Snake's length
  bool w_pressed = false, a_pressed = false, s_pressed = false,
       d_pressed = false;
  std::string direction = "right";

  int head_y = (gridSize - 1) / 2;
  int head_x = (gridSize - 1) / 2;
  bool usedMove = false;

  // Grid in which the current state of the game is stored (0 - empty, >=1 -
  // snake's body, -1 - apple, -2 - wall)
  std::vector<std::vector<int>> grid(gridSize, std::vector<int>(gridSize, 0));

  // Window int which the game is displayed
  sf::RenderWindow window(
      sf::VideoMode(gridSize * squareSize, gridSize * squareSize),
      "Snake in C++ with SFML", sf::Style::Close);
  // Game speed (difficulty)
  window.setFramerateLimit(framerateLimit);

  // For displaying current score
  sf::Font font;
  if (!font.loadFromFile("Hack-Regular.ttf"))
    std::cout << "Error loading font!\n";
  sf::Text scoreText;
  scoreText.setFont(font);
  scoreText.setString("Score: " + std::to_string(length - 3));
  scoreText.setCharacterSize(24);
  scoreText.setFillColor(sf::Color::Red);
  scoreText.setPosition(10, 10);

  grid[head_y][head_x] = length;
  generateApple(grid, gridSize);

  // Thread to see user input independently from framerate
  sf::Thread thread(
      std::bind(&userInput, std::ref(direction), std::ref(usedMove)));
  thread.launch();

  // Main loop of the game
  while (window.isOpen()) {
    scoreText.setString("Score: " + std::to_string(length - 3));
    renderField(window, grid, gridSize, squareSize, borderThickness, head_y,
                head_x, direction, scoreText);

    usedMove = false;

    // userInput(direction, w_pressed, a_pressed, s_pressed, d_pressed);
    moveSnake(grid, gridSize, direction, head_y, head_x, length, window, thread,
              usedMove);

    sf::Event event;
    // Handling events in the window
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
        thread.terminate();
      }
    }
  }
}
