#define _USE_MATH_DEFINES
#include <math.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <map>
#include <random>
#include <chrono>
#include <cmath>
#include <variant>
#include <string>
#include <functional>
#include <optional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// SFML 3.0 Event handling

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

enum class GameState {
    MainMenu,
    Options,
    ModMenu,
    Keybinds,
    Game,
    GameOver,
    Shop
};

struct Button {
    sf::RectangleShape rect;
    sf::Text text;
    std::function<void()> action;

    Button(const sf::Font& font) : text(font, "") {}
};

struct Slider {
    sf::RectangleShape track;
    sf::RectangleShape handle;
    sf::Text label;
    float* value;
    float minValue;
    float maxValue;
    bool isDragging = false;

    Slider(const sf::Font& font, const std::string& labelText, float* val, float min, float max)
        : label(font, labelText), value(val), minValue(min), maxValue(max) {
        track.setSize(sf::Vector2f(200.f, 10.f));
        track.setFillColor(sf::Color(100, 100, 100));
        handle.setSize(sf::Vector2f(20.f, 20.f));
        handle.setFillColor(sf::Color::White);
        label.setCharacterSize(20);
        label.setFillColor(sf::Color::White);
    }

    void setPosition(float x, float y) {
        track.setPosition(sf::Vector2f(x, y + 25.f));
        label.setPosition(sf::Vector2f(x, y));
        updateHandle();
    }

    void updateHandle() {
        float trackX = track.getPosition().x;
        float trackWidth = track.getSize().x;
        float ratio = (*value - minValue) / (maxValue - minValue);
        float handleX = trackX + ratio * trackWidth - handle.getSize().x / 2.f;
        handle.setPosition(sf::Vector2f(handleX, track.getPosition().y - 5.f));
    }

    bool contains(sf::Vector2f point) {
        return handle.getGlobalBounds().contains(point) || track.getGlobalBounds().contains(point);
    }

    void updateValue(sf::Vector2f mousePos) {
        float trackX = track.getPosition().x;
        float trackWidth = track.getSize().x;
        float ratio = (mousePos.x - trackX) / trackWidth;
        ratio = std::max(0.0f, std::min(1.0f, ratio));
        *value = minValue + ratio * (maxValue - minValue);
        updateHandle();
    }
};

const int CELL_SIZE = 30;
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int WINDOW_WIDTH = CELL_SIZE * BOARD_WIDTH + 300;
const int WINDOW_HEIGHT = CELL_SIZE * BOARD_HEIGHT;

using ShapeMatrix = std::vector<std::vector<int>>;

struct Piece {
    char shape;
    int rotation;
    sf::Color color;
    int x, y;
};

const std::array<std::pair<char, ShapeMatrix>, 7> SHAPES = {{
    {'I', {{1,1,1,1}}},
    {'J', {{1,0,0},{1,1,1}}},
    {'L', {{0,0,1},{1,1,1}}},
    {'O', {{1,1},{1,1}}},
    {'S', {{0,1,1},{1,1,0}}},
    {'T', {{0,1,0},{1,1,1}}},
    {'Z', {{1,1,0},{0,1,1}}}
}};

const std::map<char, sf::Color> COLORS = {
    {'I', sf::Color::Cyan},
    {'J', sf::Color::Blue},
    {'L', sf::Color(255,165,0)}, // Orange
    {'O', sf::Color::Yellow},
    {'S', sf::Color::Green},
    {'T', sf::Color(128,0,128)}, // Purple
    {'Z', sf::Color::Red}
};

class TetrisApp {
public:
    TetrisApp() : window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Tetris Clone C++", sf::Style::Titlebar | sf::Style::Close),
                  board(BOARD_HEIGHT, std::vector<sf::Color>(BOARD_WIDTH, sf::Color::Black)),
                  rng(std::chrono::system_clock::now().time_since_epoch().count()),
                  wobbleEnabled(true), wobbleIntensity(1.0f), dragging(false),
                  font(), gameState(GameState::MainMenu) {
        window.setFramerateLimit(60);
        currentWindowSize = window.getSize();
        
        // Center the window on screen
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        int centerX = (desktop.size.x - WINDOW_WIDTH) / 2;
        int centerY = (desktop.size.y - WINDOW_HEIGHT) / 2;
        window.setPosition(sf::Vector2i(centerX, centerY));

        // Load font first
        if (!font.openFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf")) {
            if (!font.openFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
                if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
                    // If all else fails, try to use system default font
                    std::cerr << "Warning: Could not load any fonts. Using system default." << std::endl;
                }
            }
        }
        
        // Initialize text objects after font is loaded
        titleText = sf::Text(font, "Tetris Clone", 48);
        titleText->setFillColor(sf::Color::White);
        titleText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 150, 50));
        
        subtitleText = sf::Text(font, "Lucys c++ Tetris Clone", 24);
        subtitleText->setFillColor(sf::Color(200, 200, 200));
        subtitleText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 120, 100));

        backText = sf::Text(font, "Back to Menu", 24);
        backText->setFillColor(sf::Color::White);
        backText->setPosition(sf::Vector2f(static_cast<float>(CELL_SIZE * BOARD_WIDTH + 10), 130.f));

        backRect.setSize(sf::Vector2f(150.f, 30.f));
        backRect.setPosition(sf::Vector2f(static_cast<float>(CELL_SIZE * BOARD_WIDTH + 10), 130.f));
        backRect.setFillColor(sf::Color::Transparent);

        scoreText = sf::Text(font, "Score: 0", 24);
        scoreText->setFillColor(sf::Color::White);
        scoreText->setPosition(sf::Vector2f(static_cast<float>(CELL_SIZE * BOARD_WIDTH + 10), 10.f));

        levelText = sf::Text(font, "Level: 1", 24);
        levelText->setFillColor(sf::Color::White);
        levelText->setPosition(sf::Vector2f(static_cast<float>(CELL_SIZE * BOARD_WIDTH + 10), 40.f));

        linesText = sf::Text(font, "Lines: 0", 24);
        linesText->setFillColor(sf::Color::White);
        linesText->setPosition(sf::Vector2f(static_cast<float>(CELL_SIZE * BOARD_WIDTH + 10), 70.f));

        coinsText = sf::Text(font, "$ 0", 24);
        coinsText->setFillColor(sf::Color::Yellow);
        coinsText->setPosition(sf::Vector2f(static_cast<float>(CELL_SIZE * BOARD_WIDTH + 10), 100.f));

        mainMenuCoinsText = sf::Text(font, "$ 0", 24);
        mainMenuCoinsText->setFillColor(sf::Color::Yellow);
        mainMenuCoinsText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2 + 100, 150));

        resetGame();
        initializeMenus();
        generateTetrisTheme();

        // Generate space background stars
        stars.clear();
        std::uniform_real_distribution<float> distX(0, WINDOW_WIDTH);
        std::uniform_real_distribution<float> distY(0, WINDOW_HEIGHT);
        for (int i = 0; i < 100; ++i) {
            stars.emplace_back(distX(rng), distY(rng));
        }


    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            draw();
        }
    }

private:
    sf::RenderWindow window;
    std::vector<std::vector<sf::Color>> board;
    Piece currentPiece;
    Piece nextPiece;
    int score = 0;
    int level = 1;
    int linesCleared = 0;
    int coins = 0;
    int blocksPlaced = 0;
    int fallSpeed = 500; // milliseconds
    sf::Clock fallClock;

    // Wobble variables
    bool wobbleEnabled;
    float wobbleIntensity;
    bool dragging;
    sf::Vector2i dragStartPos;
    sf::Vector2i windowStartPos;
    float wobbleOffset = 0.f;
    int wobbleDirection = 1;
    sf::Clock wobbleClock;
    bool wobbleActive = false;
    float wobbleVelocity = 0.f;
    float wobbleDamping = 0.95f;
    sf::Vector2i baseWindowPos;

    // Menu variables
    GameState gameState;
    sf::Font font;
    sf::Vector2u currentWindowSize;
    std::optional<sf::Text> titleText;
    std::optional<sf::Text> subtitleText; // New subtitle text
    std::vector<Button> mainButtons;
    std::vector<Button> optionsButtons;
    std::vector<Button> modButtons;
    std::vector<Button> gameOverButtons;
    std::vector<Button> shopButtons;
    std::optional<sf::Text> backText;
    std::optional<sf::Text> scoreText;
    std::optional<sf::Text> levelText;
    std::optional<sf::Text> linesText;
    std::optional<sf::Text> coinsText;
    std::optional<sf::Text> mainMenuCoinsText;
    sf::RectangleShape backRect;
    
    // Sliders
    std::vector<Slider> sliders;

    // Settings
    float brightness = 1.0f;
    float soundVolume = 1.0f;
    float rainbowSpeed = 1.0f; // New rainbow speed setting
    bool modRainbow = false;
    sf::Clock rainbowClock; // Clock for rainbow timing

    // Space background and wallpapers
    std::vector<sf::Vector2f> stars;
    sf::Color backgroundColor = sf::Color::Black;
    bool spaceBackgroundEnabled = true;
    bool blueWallpaperBought = false;
    bool greenWallpaperBought = false;
    bool redWallpaperBought = false;
    std::string activeWallpaper = "space"; // "space", "blue", "green", "red"

    // Audio system
    sf::SoundBuffer tetrisBuffer;
    std::optional<sf::Sound> tetrisMusic;
    std::vector<int16_t> tetrisThemeSamples;
    bool musicLoaded = false;

    std::mt19937 rng;

    void generateTetrisTheme() {
        // Generate the classic Tetris theme from Arduino notes
        const int sampleRate = 44100;
        const float duration = 0.25f; // Quarter note duration
        const int noteSamples = static_cast<int>(sampleRate * duration);

        // Tetris theme melody frequencies
        std::vector<float> melody = {
            659, 494, 523, 587, 659, 587, 523, 494,
            440, 440, 523, 659, 587, 523,
            494, 523, 587, 659, 523, 440, 440, 587,
            587, 698, 880, 784, 698, 659, 523, 659,
            587, 523, 494, 494, 523, 587, 659, 523,
            440, 440
        };

        tetrisThemeSamples.clear();
        tetrisThemeSamples.reserve(melody.size() * noteSamples);

        for (float freq : melody) {
            for (int i = 0; i < noteSamples; ++i) {
                float time = static_cast<float>(i) / sampleRate;
                float amplitude = freq > 0 ? 0.3f * std::sin(2.0f * M_PI * freq * time) : 0.0f;

                // Apply envelope for smoother sound
                float envelope = 1.0f;
                if (i < noteSamples * 0.1f) {
                    envelope = static_cast<float>(i) / (noteSamples * 0.1f); // Attack
                } else if (i > noteSamples * 0.8f) {
                    envelope = 1.0f - static_cast<float>(i - noteSamples * 0.8f) / (noteSamples * 0.2f); // Release
                }

                int16_t sample = static_cast<int16_t>(amplitude * envelope * 32767);
                tetrisThemeSamples.push_back(sample);
            }
        }

        std::vector<sf::SoundChannel> channels = {sf::SoundChannel::Mono};
        if (tetrisBuffer.loadFromSamples(tetrisThemeSamples.data(), tetrisThemeSamples.size(), 1, sampleRate, channels)) {
            tetrisMusic = sf::Sound(tetrisBuffer);
            tetrisMusic->setLooping(true);
            tetrisMusic->setVolume(soundVolume * 100.0f); // Adjust volume scale
            musicLoaded = true;
        } else {
            std::cerr << "Failed to load Tetris theme" << std::endl;
        }
    }

    void resetGame() {
        for (auto& row : board) {
            std::fill(row.begin(), row.end(), sf::Color::Black);
        }
        score = 0;
        level = 1;
        linesCleared = 0;
        fallSpeed = 500;
        currentPiece = getNewPiece();
        nextPiece = getNewPiece();
        fallClock.restart();
        blocksPlaced = 0;
    }

    void saveCoins() {
        std::ofstream file("gamedata.dat", std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(&coins), sizeof(coins));
            file.write(reinterpret_cast<const char*>(&blueWallpaperBought), sizeof(blueWallpaperBought));
            file.write(reinterpret_cast<const char*>(&greenWallpaperBought), sizeof(greenWallpaperBought));
            file.write(reinterpret_cast<const char*>(&redWallpaperBought), sizeof(redWallpaperBought));
            file.write(reinterpret_cast<const char*>(&spaceBackgroundEnabled), sizeof(spaceBackgroundEnabled));
            
            // Save active wallpaper string
            size_t len = activeWallpaper.length();
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(activeWallpaper.c_str(), len);
            
            file.close();
        } else {
            std::cerr << "Failed to save game data to file." << std::endl;
        }
    }

    void loadCoins() {
        std::ifstream file("gamedata.dat", std::ios::binary);
        if (file.is_open()) {
            file.read(reinterpret_cast<char*>(&coins), sizeof(coins));
            file.read(reinterpret_cast<char*>(&blueWallpaperBought), sizeof(blueWallpaperBought));
            file.read(reinterpret_cast<char*>(&greenWallpaperBought), sizeof(greenWallpaperBought));
            file.read(reinterpret_cast<char*>(&redWallpaperBought), sizeof(redWallpaperBought));
            file.read(reinterpret_cast<char*>(&spaceBackgroundEnabled), sizeof(spaceBackgroundEnabled));
            
            // Load active wallpaper string
            size_t len;
            if (file.read(reinterpret_cast<char*>(&len), sizeof(len))) {
                activeWallpaper.resize(len);
                file.read(&activeWallpaper[0], len);
                
                // Set background color based on active wallpaper
                if (activeWallpaper == "blue") backgroundColor = sf::Color::Blue;
                else if (activeWallpaper == "green") backgroundColor = sf::Color::Green;
                else if (activeWallpaper == "red") backgroundColor = sf::Color::Red;
                else backgroundColor = sf::Color::Black;
            }
            
            file.close();
        } else {
            // Default values if no file found
            coins = 0;
            blueWallpaperBought = false;
            greenWallpaperBought = false;
            redWallpaperBought = false;
            spaceBackgroundEnabled = true;
            activeWallpaper = "space";
            backgroundColor = sf::Color::Black;
        }
    }

    sf::Color getRainbowColor() {
        float time = rainbowClock.getElapsedTime().asSeconds() * rainbowSpeed;
        int r = static_cast<int>((std::sin(time * 2.0f) + 1.0f) * 127.5f);
        int g = static_cast<int>((std::sin(time * 2.0f + 2.09f) + 1.0f) * 127.5f); // 120 degree offset
        int b = static_cast<int>((std::sin(time * 2.0f + 4.18f) + 1.0f) * 127.5f); // 240 degree offset
        return sf::Color(r, g, b);
    }

    Piece getNewPiece() {
        std::uniform_int_distribution<int> dist(0, SHAPES.size() - 1);
        int idx = dist(rng);
        char shape = SHAPES[idx].first;
        sf::Color color = COLORS.at(shape);
        if (modRainbow) {
            color = getRainbowColor();
        }
        return Piece{shape, 0, color, BOARD_WIDTH / 2 - 2, 0};
    }

    ShapeMatrix rotateShape(const ShapeMatrix& matrix) {
        int n = matrix.size();
        int m = matrix[0].size();
        ShapeMatrix rotated(m, std::vector<int>(n, 0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                rotated[j][n - i - 1] = matrix[i][j];
        return rotated;
    }

    ShapeMatrix getShapeMatrix(const Piece& piece) {
        ShapeMatrix matrix;
        for (const auto& s : SHAPES) {
            if (s.first == piece.shape) {
                matrix = s.second;
                break;
            }
        }
        for (int i = 0; i < piece.rotation; ++i) {
            matrix = rotateShape(matrix);
        }
        return matrix;
    }

    bool validPosition(const Piece& piece, int adjX = 0, int adjY = 0, int rotation = -1) {
        int rot = (rotation == -1) ? piece.rotation : rotation;
        ShapeMatrix matrix = getShapeMatrix(Piece{piece.shape, rot, piece.color, piece.x, piece.y});
        for (int y = 0; y < (int)matrix.size(); ++y) {
            for (int x = 0; x < (int)matrix[y].size(); ++x) {
                if (matrix[y][x]) {
                    int newX = piece.x + x + adjX;
                    int newY = piece.y + y + adjY;
                    if (newX < 0 || newX >= BOARD_WIDTH || newY >= BOARD_HEIGHT)
                        return false;
                    if (newY >= 0 && board[newY][newX] != sf::Color::Black)
                        return false;
                }
            }
        }
        return true;
    }

    void placePiece() {
        ShapeMatrix matrix = getShapeMatrix(currentPiece);
        for (int y = 0; y < (int)matrix.size(); ++y) {
            for (int x = 0; x < (int)matrix[y].size(); ++x) {
                if (matrix[y][x]) {
                    int boardX = currentPiece.x + x;
                    int boardY = currentPiece.y + y;
                    if (boardY >= 0 && boardY < BOARD_HEIGHT && boardX >= 0 && boardX < BOARD_WIDTH) {
                        board[boardY][boardX] = currentPiece.color;
                    }
                }
            }
        }
        clearLines();
        blocksPlaced++;
        if (blocksPlaced % 5 == 0) {
            coins += 4;
        }
        currentPiece = nextPiece;
        nextPiece = getNewPiece();
        if (!validPosition(currentPiece)) {
            gameOver();
        }
    }

    void clearLines() {
        for (int y = BOARD_HEIGHT - 1; y >= 0; --y) {
            bool fullLine = true;
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                if (board[y][x] == sf::Color::Black) {
                    fullLine = false;
                    break;
                }
            }
            if (fullLine) {
                for (int row = y; row > 0; --row) {
                    board[row] = board[row - 1];
                }
                board[0] = std::vector<sf::Color>(BOARD_WIDTH, sf::Color::Black);
                ++linesCleared;
                score += 100 * level;
                level = linesCleared / 10 + 1;
                fallSpeed = std::max(100, 500 - (level - 1) * 50);
                ++y; // recheck this row
            }
        }
    }

    void gameOver() {
        gameState = GameState::GameOver;
    }



    void initializeMenus() {
        // Get actual window size for dynamic positioning
        sf::Vector2u windowSize = window.getSize();
        float centerX = windowSize.x / 2.0f;
        float buttonWidth = 200.f;
        float buttonHeight = 50.f;

        // Main menu buttons
        mainButtons.clear();
        Button playButton(font);
        playButton.rect.setSize(sf::Vector2f(buttonWidth, buttonHeight));
        playButton.rect.setPosition(sf::Vector2f(centerX - buttonWidth / 2.0f, 170.f));
        playButton.rect.setFillColor(sf::Color::Blue);
        playButton.text.setFont(font);
        playButton.text.setString("Play");
        playButton.text.setCharacterSize(24);
        playButton.text.setFillColor(sf::Color::White);
        // Center text within button
        sf::FloatRect textBounds = playButton.text.getGlobalBounds();
        playButton.text.setPosition(sf::Vector2f(centerX - textBounds.size.x / 2.0f, 180.f));
        playButton.action = [this]() { loadCoins(); resetGame(); gameState = GameState::Game; };
        mainButtons.push_back(playButton);

        Button optionsButton(font);
        optionsButton.rect.setSize(sf::Vector2f(buttonWidth, buttonHeight));
        optionsButton.rect.setPosition(sf::Vector2f(centerX - buttonWidth / 2.0f, 240.f));
        optionsButton.rect.setFillColor(sf::Color::Green);
        optionsButton.text.setFont(font);
        optionsButton.text.setString("Options");
        optionsButton.text.setCharacterSize(24);
        optionsButton.text.setFillColor(sf::Color::White);
        sf::FloatRect optionsTextBounds = optionsButton.text.getGlobalBounds();
        optionsButton.text.setPosition(sf::Vector2f(centerX - optionsTextBounds.size.x / 2.0f, 250.f));
        optionsButton.action = [this]() { gameState = GameState::Options; };
        mainButtons.push_back(optionsButton);

        Button modButton(font);
        modButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        modButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 310.f));
        modButton.rect.setFillColor(sf::Color::Yellow);
        modButton.text.setFont(font);
        modButton.text.setString("Mod Menu");
        modButton.text.setCharacterSize(24);
        modButton.text.setFillColor(sf::Color::Black);
        modButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 50.f, 320.f));
        modButton.action = [this]() { gameState = GameState::ModMenu; };
        mainButtons.push_back(modButton);

        Button shopButton(font);
        shopButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        shopButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 380.f));
        shopButton.rect.setFillColor(sf::Color(128,0,128));
        shopButton.text.setFont(font);
        shopButton.text.setString("Shop");
        shopButton.text.setCharacterSize(24);
        shopButton.text.setFillColor(sf::Color::White);
        shopButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 25.f, 390.f));
        shopButton.action = [this]() { gameState = GameState::Shop; };
        mainButtons.push_back(shopButton);

        Button exitButton(font);
        exitButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        exitButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 450.f));
        exitButton.rect.setFillColor(sf::Color::Red);
        exitButton.text.setFont(font);
        exitButton.text.setString("Exit");
        exitButton.text.setCharacterSize(24);
        exitButton.text.setFillColor(sf::Color::White);
        exitButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 20.f, 460.f));
        exitButton.action = [this]() { saveCoins(); window.close(); };
        mainButtons.push_back(exitButton);

        // Back button
        optionsButtons.clear();
        Button backButton(font);
        backButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        backButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 400.f));
        backButton.rect.setFillColor(sf::Color(128, 128, 128));
        backButton.text.setFont(font);
        backButton.text.setString("Back");
        backButton.text.setCharacterSize(24);
        backButton.text.setFillColor(sf::Color::White);
        backButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 25.f, 410.f));
        backButton.action = [this]() { saveCoins(); gameState = GameState::MainMenu; };

        // Initialize sliders
        sliders.clear();
        sliders.emplace_back(font, "Brightness: " + std::to_string(brightness), &brightness, 0.3f, 1.5f);
        sliders.back().setPosition(50.f, 150.f);
        
        sliders.emplace_back(font, "Volume: " + std::to_string(soundVolume), &soundVolume, 0.0f, 1.0f);
        sliders.back().setPosition(50.f, 200.f);
        
        sliders.emplace_back(font, "Rainbow Speed: " + std::to_string(rainbowSpeed), &rainbowSpeed, 0.1f, 3.0f);
        sliders.back().setPosition(50.f, 250.f);

        Button wobbleButton(font);
        wobbleButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        wobbleButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 320.f));
        wobbleButton.rect.setFillColor(sf::Color::Cyan);
        wobbleButton.text.setFont(font);
        wobbleButton.text.setString(wobbleEnabled ? "Window Wobble: On" : "Window Wobble: Off");
        wobbleButton.text.setCharacterSize(24);
        wobbleButton.text.setFillColor(sf::Color::Black);
        wobbleButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 90.f, 330.f));
        wobbleButton.action = [this]() { wobbleEnabled = !wobbleEnabled; };
        optionsButtons.push_back(wobbleButton);

        optionsButtons.push_back(backButton);

        // Mod buttons
        modButtons.clear();
        Button rainbowButton(font);
        rainbowButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        rainbowButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 150.f));
        rainbowButton.rect.setFillColor(sf::Color::Magenta);
        rainbowButton.text.setFont(font);
        rainbowButton.text.setString(modRainbow ? "Rainbow Mode: On" : "Rainbow Mode: Off");
        rainbowButton.text.setCharacterSize(24);
        rainbowButton.text.setFillColor(sf::Color::White);
        rainbowButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 85.f, 160.f));
        rainbowButton.action = [this]() { modRainbow = !modRainbow; };
        modButtons.push_back(rainbowButton);

        modButtons.push_back(backButton);

        // Game over buttons
        gameOverButtons.clear();
        Button tryAgainButton(font);
        tryAgainButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        tryAgainButton.rect.setPosition(sf::Vector2f(centerX - 100.f, 250.f));
        tryAgainButton.rect.setFillColor(sf::Color::Green);
        tryAgainButton.text.setFont(font);
        tryAgainButton.text.setString("Try Again");
        tryAgainButton.text.setCharacterSize(24);
        tryAgainButton.text.setFillColor(sf::Color::White);
        sf::FloatRect tryTextBounds = tryAgainButton.text.getGlobalBounds();
        tryAgainButton.text.setPosition(sf::Vector2f(centerX - tryTextBounds.size.x / 2.0f, 260.f));
        tryAgainButton.action = [this]() { resetGame(); gameState = GameState::Game; };
        gameOverButtons.push_back(tryAgainButton);

        Button mainMenuButton(font);
        mainMenuButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        mainMenuButton.rect.setPosition(sf::Vector2f(centerX - 100.f, 320.f));
        mainMenuButton.rect.setFillColor(sf::Color::Blue);
        mainMenuButton.text.setFont(font);
        mainMenuButton.text.setString("Go back to menu");
        mainMenuButton.text.setCharacterSize(24);
        mainMenuButton.text.setFillColor(sf::Color::White);
        sf::FloatRect menuTextBounds = mainMenuButton.text.getGlobalBounds();
        mainMenuButton.text.setPosition(sf::Vector2f(centerX - menuTextBounds.size.x / 2.0f, 330.f));
        mainMenuButton.action = [this]() { gameState = GameState::MainMenu; };
        gameOverButtons.push_back(mainMenuButton);

        // Shop buttons
        shopButtons.clear();
        Button buyBlueButton(font);
        buyBlueButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        buyBlueButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 150.f));
        buyBlueButton.rect.setFillColor(sf::Color::Blue);
        buyBlueButton.text.setFont(font);
        buyBlueButton.text.setString(blueWallpaperBought ? "Blue Wallpaper (Owned)" : "Buy Blue Wallpaper: 10 coins");
        buyBlueButton.text.setCharacterSize(18);
        buyBlueButton.text.setFillColor(sf::Color::White);
        buyBlueButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 90.f, 160.f));
        buyBlueButton.action = [this]() {
            if (!blueWallpaperBought && coins >= 10) {
                coins -= 10;
                blueWallpaperBought = true;
                activeWallpaper = "blue";
                spaceBackgroundEnabled = false;
                backgroundColor = sf::Color::Blue;
                initializeMenus(); // Refresh menu to update button text
            } else if (blueWallpaperBought) {
                activeWallpaper = "blue";
                spaceBackgroundEnabled = false;
                backgroundColor = sf::Color::Blue;
            }
        };
        shopButtons.push_back(buyBlueButton);


        Button buyGreenButton(font);
        buyGreenButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        buyGreenButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 220.f));
        buyGreenButton.rect.setFillColor(sf::Color::Green);
        buyGreenButton.text.setFont(font);
        buyGreenButton.text.setString(greenWallpaperBought ? "Green Wallpaper (Owned)" : "Buy Green Wallpaper: 15 coins");
        buyGreenButton.text.setCharacterSize(18);
        buyGreenButton.text.setFillColor(sf::Color::White);
        buyGreenButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 90.f, 230.f));
        buyGreenButton.action = [this]() {
            if (!greenWallpaperBought && coins >= 15) {
                coins -= 15;
                greenWallpaperBought = true;
                activeWallpaper = "green";
                spaceBackgroundEnabled = false;
                backgroundColor = sf::Color::Green;
                initializeMenus(); // Refresh menu to update button text
            } else if (greenWallpaperBought) {
                activeWallpaper = "green";
                spaceBackgroundEnabled = false;
                backgroundColor = sf::Color::Green;
            }
        };
        shopButtons.push_back(buyGreenButton);


        Button buyRedButton(font);
        buyRedButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        buyRedButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 290.f));
        buyRedButton.rect.setFillColor(sf::Color::Red);
        buyRedButton.text.setFont(font);
        buyRedButton.text.setString(redWallpaperBought ? "Red Wallpaper (Owned)" : "Buy Red Wallpaper: 20 coins");
        buyRedButton.text.setCharacterSize(18);
        buyRedButton.text.setFillColor(sf::Color::White);
        buyRedButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 90.f, 300.f));
        buyRedButton.action = [this]() {
            if (!redWallpaperBought && coins >= 20) {
                coins -= 20;
                redWallpaperBought = true;
                activeWallpaper = "red";
                spaceBackgroundEnabled = false;
                backgroundColor = sf::Color::Red;
                initializeMenus(); // Refresh menu to update button text
            } else if (redWallpaperBought) {
                activeWallpaper = "red";
                spaceBackgroundEnabled = false;
                backgroundColor = sf::Color::Red;
            }
        };
        shopButtons.push_back(buyRedButton);

        // Add toggle space background button
        Button toggleSpaceBgButton(font);
        toggleSpaceBgButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        toggleSpaceBgButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 360.f));
        toggleSpaceBgButton.rect.setFillColor(sf::Color(100, 100, 100));
        toggleSpaceBgButton.text.setFont(font);
        toggleSpaceBgButton.text.setString("Space Background");
        toggleSpaceBgButton.text.setCharacterSize(18);
        toggleSpaceBgButton.text.setFillColor(sf::Color::White);
        toggleSpaceBgButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 70.f, 370.f));
        toggleSpaceBgButton.action = [this]() {
            activeWallpaper = "space";
            spaceBackgroundEnabled = true;
            backgroundColor = sf::Color::Black;
        };
        shopButtons.push_back(toggleSpaceBgButton);

        Button shopBackButton(font);
        shopBackButton.rect.setSize(sf::Vector2f(200.f, 50.f));
        shopBackButton.rect.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100.f, 400.f));
        shopBackButton.rect.setFillColor(sf::Color(128, 128, 128));
        shopBackButton.text.setFont(font);
        shopBackButton.text.setString("Back");
        shopBackButton.text.setCharacterSize(24);
        shopBackButton.text.setFillColor(sf::Color::White);
        shopBackButton.text.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 25.f, 410.f));
        shopBackButton.action = [this]() { gameState = GameState::MainMenu; };
        shopButtons.push_back(shopBackButton);
    }

    void handleMenuClick(const std::vector<Button>& buttons) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2i globalMouse = sf::Mouse::getPosition();
        sf::Vector2i windowPos = window.getPosition();

        for (size_t i = 0; i < buttons.size(); ++i) {
            const auto& button = buttons[i];
            sf::FloatRect bounds = button.rect.getGlobalBounds();

            if (bounds.contains(static_cast<sf::Vector2f>(mousePos))) {
                if (button.action) {
                    button.action();
                }
                break;
            }
        }
    }

    void handleEvents() {
        std::optional<sf::Event> event;
        while (event = window.pollEvent()) {
            if (!event.has_value()) {
                break;
            }
        if (event->is<sf::Event::Closed>()) {
            saveCoins();
            window.close();
        } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            initializeMenus();
        } else if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mousePosF = static_cast<sf::Vector2f>(mousePos);

                bool handledClick = false;

                if (gameState == GameState::MainMenu) {
                    handleMenuClick(mainButtons);
                    handledClick = true;
                } else if (gameState == GameState::Options) {
                    // Check sliders first
                    bool sliderClicked = false;
                    for (auto& slider : sliders) {
                        if (slider.contains(mousePosF)) {
                            slider.isDragging = true;
                            slider.updateValue(mousePosF);
                            sliderClicked = true;
                            handledClick = true;

                            // Update music volume in real-time
                            if (musicLoaded && slider.value == &soundVolume && tetrisMusic.has_value()) {
                                tetrisMusic->setVolume(soundVolume * 100.0f);
                            }
                            break;
                        }
                    }
                    if (!sliderClicked) {
                        handleMenuClick(optionsButtons);
                        handledClick = true;
                    }
                } else if (gameState == GameState::ModMenu) {
                    handleMenuClick(modButtons);
                    handledClick = true;
                } else if (gameState == GameState::GameOver) {
                    handleMenuClick(gameOverButtons);
                    handledClick = true;
                } else if (gameState == GameState::Shop) {
                    handleMenuClick(shopButtons);
                    handledClick = true;
                } else if (gameState == GameState::Game && backRect.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                    saveCoins();
                    gameState = GameState::MainMenu;
                    handledClick = true;
                }

                // Only enable window dragging if no UI element was clicked
                if (!handledClick) {
                    if (gameState == GameState::Options) {
                        // Don't start window dragging if we're interacting with sliders
                        bool anySliderDragging = false;
                        for (const auto& slider : sliders) {
                            if (slider.isDragging) {
                                anySliderDragging = true;
                                break;
                            }
                        }
                        if (!anySliderDragging) {
                            dragging = true;
                            dragStartPos = sf::Mouse::getPosition(window);
                            windowStartPos = sf::Vector2i(window.getPosition().x, window.getPosition().y);
                        }
                    } else if (gameState != GameState::MainMenu) {
                        // Enable dragging for non-menu states
                        dragging = true;
                        dragStartPos = sf::Mouse::getPosition(window);
                        windowStartPos = sf::Vector2i(window.getPosition().x, window.getPosition().y);
                    }
                }
            }
        } else if (const auto* mouseButtonReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseButtonReleased->button == sf::Mouse::Button::Left) {
                if (dragging && wobbleEnabled) {
                    // Start jello wobble effect
                    wobbleActive = true;
                    wobbleVelocity = wobbleOffset * 0.5f; // Initial velocity based on current offset
                    baseWindowPos = window.getPosition();
                    wobbleClock.restart();
                }
                dragging = false;
                for (auto& slider : sliders) {
                    slider.isDragging = false;
                }
            }
        } else if (event->is<sf::Event::MouseMoved>()) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f mousePosF = static_cast<sf::Vector2f>(mousePos);
            if (gameState == GameState::Options) {
                for (auto& slider : sliders) {
                    if (slider.isDragging) {
                        slider.updateValue(mousePosF);
                        if (musicLoaded && slider.value == &soundVolume && tetrisMusic.has_value()) {
                            tetrisMusic->setVolume(soundVolume * 100.0f);
                        }
                    }
                }
            }
            if (dragging && wobbleEnabled) {
                int deltaX = mousePos.x - dragStartPos.x;
                wobbleOffset += deltaX * wobbleIntensity * 0.05f;
                if (std::abs(wobbleOffset) > 15.f * wobbleIntensity) {
                    wobbleDirection = -wobbleDirection;
                }
                sf::Vector2i newPos(windowStartPos.x + deltaX + static_cast<int>(wobbleOffset), windowStartPos.y);
                window.setPosition(newPos);
            }
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (gameState == GameState::Game) {
                switch (keyPressed->scancode) {
                    case sf::Keyboard::Scancode::Left:
                        if (validPosition(currentPiece, -1, 0)) {
                            currentPiece.x -= 1;
                        }
                        break;
                    case sf::Keyboard::Scancode::Right:
                        if (validPosition(currentPiece, 1, 0)) {
                            currentPiece.x += 1;
                        }
                        break;
                    case sf::Keyboard::Scancode::Up: {
                        int newRotation = (currentPiece.rotation + 1) % 4;
                        if (validPosition(currentPiece, 0, 0, newRotation)) {
                            currentPiece.rotation = newRotation;
                        }
                        break;
                    }
                    case sf::Keyboard::Scancode::Down:
                        if (validPosition(currentPiece, 0, 1)) {
                            currentPiece.y += 1;
                            score += 1; // soft drop bonus
                        }
                        break;
                    case sf::Keyboard::Scancode::S:
                        while (validPosition(currentPiece, 0, 1)) {
                            currentPiece.y += 1;
                        }
                        placePiece();
                        fallClock.restart();
                        break;
                    case sf::Keyboard::Scancode::R:
                        if (keyPressed->control) {
                            resetGame();
                        }
                        break;
                    case sf::Keyboard::Scancode::Escape:
                        saveCoins();
                        gameState = GameState::MainMenu;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    }

    void update() {
        if (window.getSize() != currentWindowSize) {
            initializeMenus();
            currentWindowSize = window.getSize();
        }
        
        // Handle jello wobble effect
        if (wobbleActive && wobbleEnabled) {
            float elapsedTime = wobbleClock.getElapsedTime().asSeconds();
            if (elapsedTime < 2.0f) { // Wobble for 2 seconds
                // Apply spring physics for jello effect
                float springForce = -wobbleOffset * 0.3f; // Spring constant
                wobbleVelocity += springForce;
                wobbleVelocity *= wobbleDamping; // Apply damping
                wobbleOffset += wobbleVelocity;
                
                // Update window position
                sf::Vector2i newPos(baseWindowPos.x + static_cast<int>(wobbleOffset), baseWindowPos.y);
                window.setPosition(newPos);
            } else {
                // Stop wobbling after 2 seconds
                wobbleActive = false;
                wobbleOffset = 0.f;
                wobbleVelocity = 0.f;
                window.setPosition(baseWindowPos);
            }
        }
        
        // Handle music playback based on game state
        if (musicLoaded && tetrisMusic.has_value()) {
            if ((gameState == GameState::Game || gameState == GameState::MainMenu) && tetrisMusic->getStatus() != sf::SoundSource::Status::Playing) {
                tetrisMusic->play();
            } else if (gameState != GameState::Game && gameState != GameState::MainMenu && tetrisMusic->getStatus() == sf::SoundSource::Status::Playing) {
                tetrisMusic->pause();
            }
        }
        
        if (fallClock.getElapsedTime().asMilliseconds() >= fallSpeed) {
            if (gameState == GameState::Game) {
                if (validPosition(currentPiece, 0, 1)) {
                    currentPiece.y += 1;
                } else {
                    placePiece();
                }
            }
            fallClock.restart();
        }
    }

    void draw() {
        // Set background color based on active wallpaper
        if (activeWallpaper == "space" && spaceBackgroundEnabled) {
            window.clear(sf::Color::Black);
            // Draw space background stars
            for (const auto& star : stars) {
                sf::CircleShape starShape(1.f);
                starShape.setPosition(star);
                starShape.setFillColor(sf::Color::White);
                window.draw(starShape);
            }
        } else {
            window.clear(backgroundColor);
        }

        switch (gameState) {
            case GameState::MainMenu:
                if (titleText.has_value()) window.draw(*titleText);
                if (subtitleText.has_value()) window.draw(*subtitleText);
                for (auto& button : mainButtons) {
                    window.draw(button.rect);
                    window.draw(button.text);
                }
                if (mainMenuCoinsText.has_value()) {
                    mainMenuCoinsText->setString("$ " + std::to_string(coins));
                    window.draw(*mainMenuCoinsText);
                }
                break;
            case GameState::Options:
                // Draw sliders
                for (auto& slider : sliders) {
                    // Update slider labels with current values
                    if (slider.value == &brightness) {
                        slider.label.setString("Brightness: " + std::to_string(static_cast<int>(brightness * 100)) + "%");
                    } else if (slider.value == &soundVolume) {
                        slider.label.setString("Volume: " + std::to_string(static_cast<int>(soundVolume * 100)) + "%");
                    } else if (slider.value == &rainbowSpeed) {
                        slider.label.setString("Rainbow Speed: " + std::to_string(static_cast<int>(rainbowSpeed * 100)) + "%");
                    }
                    slider.updateHandle();
                    window.draw(slider.track);
                    window.draw(slider.handle);
                    window.draw(slider.label);
                }
                
                // Draw other option buttons
                for (auto& button : optionsButtons) {
                    if (button.text.getString().find("Window Wobble") != std::string::npos) {
                        button.text.setString(wobbleEnabled ? "Window Wobble: On" : "Window Wobble: Off");
                    }
                    window.draw(button.rect);
                    window.draw(button.text);
                }
                break;
            case GameState::ModMenu:
                for (auto& button : modButtons) {
                    if (button.text.getString().find("Rainbow Mode") != std::string::npos) {
                        button.text.setString(modRainbow ? "Rainbow Mode: On" : "Rainbow Mode: Off");
                    }
                    window.draw(button.rect);
                    window.draw(button.text);
                }
                break;
            case GameState::Shop:
                for (auto& button : shopButtons) {
                    window.draw(button.rect);
                    window.draw(button.text);
                }
                break;
            case GameState::Game: {
                // Draw borders
                // Removed boardBorder drawing to remove playing field border
                // Keep scoreBorder to separate playing field from score
                sf::RectangleShape scoreBorder(sf::Vector2f(WINDOW_WIDTH - BOARD_WIDTH * CELL_SIZE - 2, WINDOW_HEIGHT - 2));
                scoreBorder.setPosition(sf::Vector2f(BOARD_WIDTH * CELL_SIZE + 1, -1));
                scoreBorder.setFillColor(sf::Color::Transparent);
                scoreBorder.setOutlineColor(sf::Color::White);
                scoreBorder.setOutlineThickness(1);
                window.draw(scoreBorder);

                // Draw board
                for (int y = 0; y < BOARD_HEIGHT; ++y) {
                    for (int x = 0; x < BOARD_WIDTH; ++x) {
                        if (board[y][x] != sf::Color::Black) {
                            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                            cell.setPosition(sf::Vector2f(x * CELL_SIZE, y * CELL_SIZE));
                            sf::Color adjusted = board[y][x];
                            adjusted.r = static_cast<uint8_t>(std::min(255.0f, adjusted.r * brightness));
                            adjusted.g = static_cast<uint8_t>(std::min(255.0f, adjusted.g * brightness));
                            adjusted.b = static_cast<uint8_t>(std::min(255.0f, adjusted.b * brightness));
                            cell.setFillColor(adjusted);
                            window.draw(cell);
                        }
                    }
                }
                // Draw current piece
                ShapeMatrix matrix = getShapeMatrix(currentPiece);
                sf::Color pieceColor = currentPiece.color;
                if (modRainbow) {
                    pieceColor = getRainbowColor();
                }
                sf::Color adjustedPiece = pieceColor;
                adjustedPiece.r = static_cast<uint8_t>(std::min(255.0f, adjustedPiece.r * brightness));
                adjustedPiece.g = static_cast<uint8_t>(std::min(255.0f, adjustedPiece.g * brightness));
                adjustedPiece.b = static_cast<uint8_t>(std::min(255.0f, adjustedPiece.b * brightness));
                for (int y = 0; y < (int)matrix.size(); ++y) {
                    for (int x = 0; x < (int)matrix[y].size(); ++x) {
                        if (matrix[y][x]) {
                            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                            cell.setPosition(sf::Vector2f((currentPiece.x + x) * CELL_SIZE, (currentPiece.y + y) * CELL_SIZE));
                            cell.setFillColor(adjustedPiece);
                            window.draw(cell);
                        }
                    }
                }
                // Draw next piece
                sf::Text nextText(font, "Next:", 24);
                nextText.setFillColor(sf::Color::White);
                nextText.setPosition(sf::Vector2f(BOARD_WIDTH * CELL_SIZE + 10, 200));
                window.draw(nextText);

                ShapeMatrix nextMatrix = getShapeMatrix(nextPiece);
                sf::Color nextColor = nextPiece.color;
                if (modRainbow) {
                    nextColor = getRainbowColor();
                }
                for (int y = 0; y < (int)nextMatrix.size(); ++y) {
                    for (int x = 0; x < (int)nextMatrix[y].size(); ++x) {
                        if (nextMatrix[y][x]) {
                            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                            cell.setPosition(sf::Vector2f(BOARD_WIDTH * CELL_SIZE + 50 + x * CELL_SIZE, 230 + y * CELL_SIZE));
                            cell.setFillColor(nextColor);
                            window.draw(cell);
                        }
                    }
                }

                // Draw UI
            if (scoreText.has_value()) {
                scoreText->setString("Score: " + std::to_string(score));
                window.draw(*scoreText);
            }
            if (levelText.has_value()) {
                levelText->setString("Level: " + std::to_string(level));
                window.draw(*levelText);
            }
            if (linesText.has_value()) {
                linesText->setString("Lines: " + std::to_string(linesCleared));
                window.draw(*linesText);
            }
            if (coinsText.has_value()) {
                coinsText->setString("$ " + std::to_string(coins));
                window.draw(*coinsText);
            }
            if (backText.has_value()) window.draw(*backText);
            break;
            }
            case GameState::GameOver:
                if (titleText.has_value()) window.draw(*titleText);
                if (subtitleText.has_value()) window.draw(*subtitleText);
                sf::Text gameOverText(font, "Game Over", 48);
                gameOverText.setFillColor(sf::Color::Red);
                gameOverText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 120.f, 150.f));
                window.draw(gameOverText);
                for (auto& button : gameOverButtons) {
                    window.draw(button.rect);
                    window.draw(button.text);
                }
                break;
        }
        window.display();
    }
};

int main() {
    TetrisApp app;
    app.run();
    return 0;
}
