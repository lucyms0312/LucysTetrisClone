                        if (matrix[y][x]) {
                            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                            cell.setPosition(sf::Vector2f((currentPiece.x + x) * CELL_SIZE, (currentPiece.y + y) * CELL_SIZE + TITLEBAR_HEIGHT));
                            cell.setFillColor(adjustedPiece);
                            window.draw(cell);
                        }
                    }
                }
                // Draw next piece
                sf::Text nextText(font, "Next:", 24);
                nextText.setFillColor(sf::Color::White);
                nextText.setPosition(sf::Vector2f(BOARD_WIDTH * CELL_SIZE + 10, 200 + TITLEBAR_HEIGHT));
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
                            cell.setPosition(sf::Vector2f(BOARD_WIDTH * CELL_SIZE + 50 + x * CELL_SIZE, 230 + y * CELL_SIZE + TITLEBAR_HEIGHT));
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
                gameOverText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 120.f, 150.f + TITLEBAR_HEIGHT));
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
