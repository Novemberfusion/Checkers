#include <SFML/Graphics.hpp>
#include <vector>
#include <set>
#include <iostream>
#include <tuple>

const int boardSize = 8; // размер доски 8x8
const int tileSize = 60; // размер клетки

sf::Color lightColor(235, 236, 208);  // светлая клетка
sf::Color darkColor(119, 148, 85);    // темная клетка
sf::Color highlightColor(255, 255, 0, 128); // цвет подсветки

enum class PieceType { None, Pawn, King };

// Структура для представления шашки
struct Piece {
    PieceType type;
    bool isWhite;

    Piece() : type(PieceType::None), isWhite(false) {}
    Piece(PieceType t, bool white) : type(t), isWhite(white) {}
};

// Класс для игры в шашки
class CheckersGame {
public:
    CheckersGame() {
        board.resize(boardSize, std::vector<Piece>(boardSize));
        setupBoard();
    }

    void setupBoard() {
        for (int row = 0; row < boardSize; row++) {
            for (int col = 0; col < boardSize; col++) {
                if ((row + col) % 2 == 1) {
                    if (row < 3) {
                        board[row][col] = Piece(PieceType::Pawn, false);
                    }
                    else if (row > 4) {
                        board[row][col] = Piece(PieceType::Pawn, true);
                    }
                }
            }
        }
    }
    void handleClick(int x, int y) {
        int col = x / tileSize;
        int row = y / tileSize;

        if (selectedPiece.x == -1 && board[row][col].type != PieceType::None &&
            board[row][col].isWhite == isWhiteTurn) {
            // Выделяем шашку, если она принадлежит текущему игроку
            selectedPiece = { row, col };
            calculateValidMoves(row, col);

            // Если у выбранной шашки нет доступных ходов, сбрасываем выбор
            if (validMoves.empty()) {
                selectedPiece = { -1, -1 };  // Сбрасываем выбор
                // Можно добавить уведомление или подсветку, что шашка не может двигаться
                std::cout << "No valid moves for this piece!" << std::endl;  // Например, выводим ошибку в консоль
            }
        }
        else if (selectedPiece.x != -1) {
            // Если уже выбрана шашка, проверяем, можно ли пойти
            if (validMoves.count({ row, col })) {
                bool wasJump = abs(row - selectedPiece.x) == 2;

                movePiece(selectedPiece.x, selectedPiece.y, row, col);

                if (wasJump) {
                    // Если был прыжок, проверим возможность цепной атаки
                    selectedPiece = { row, col };
                    calculateValidMoves(row, col);

                    // Если есть еще атаки, продолжаем ход
                    if (canMakeAnotherJump(row, col)) {
                        return; // Ожидаем еще один прыжок
                    }
                }

                // Завершаем ход
                selectedPiece = { -1, -1 };
                validMoves.clear();
                isWhiteTurn = !isWhiteTurn; // Меняем ход
            }
            else {
                // Если нельзя двигать выбранную шашку, сбрасываем выбор
                selectedPiece = { -1, -1 };
                validMoves.clear();
            }
        }
    }

    bool canMakeAnotherJump(int row, int col) {
        // Проверяем, может ли текущая шашка снова сделать прыжок
        int direction = board[row][col].isWhite ? -1 : 1;

        // Проверка на возможность прыжка
        if (board[row][col].type == PieceType::Pawn || board[row][col].type == PieceType::King) {
            if (checkJumpAvailability(row, col, row + direction, col - 1, row + 2 * direction, col - 2) ||
                checkJumpAvailability(row, col, row + direction, col + 1, row + 2 * direction, col + 2)) {
                return true;
            }
        }

        // Проверка на возможность прыжка для короля
        if (board[row][col].type == PieceType::King) {
            if (checkJumpAvailability(row, col, row - direction, col - 1, row - 2 * direction, col - 2) ||
                checkJumpAvailability(row, col, row - direction, col + 1, row - 2 * direction, col + 2)) {
                return true;
            }
        }

        return false;
    }

    bool checkJumpAvailability(int srcRow, int srcCol, int midRow, int midCol, int destRow, int destCol) {
        return midRow >= 0 && midRow < boardSize && midCol >= 0 && midCol < boardSize &&
            destRow >= 0 && destRow < boardSize && destCol >= 0 && destCol < boardSize &&
            board[midRow][midCol].type != PieceType::None &&
            board[midRow][midCol].isWhite != board[srcRow][srcCol].isWhite &&
            board[destRow][destCol].type == PieceType::None;
    }

    void draw(sf::RenderWindow& window) {
        for (int row = 0; row < boardSize; row++) {
            for (int col = 0; col < boardSize; col++) {
                sf::RectangleShape cell(sf::Vector2f(tileSize, tileSize));
                cell.setPosition(col * tileSize, row * tileSize);
                cell.setFillColor((row + col) % 2 == 0 ? lightColor : darkColor);
                window.draw(cell);

                // Подсвечиваем возможные ходы
                if (validMoves.count({ row, col })) {
                    cell.setFillColor(highlightColor);
                    window.draw(cell);
                }

                if (board[row][col].type != PieceType::None) {
                    sf::CircleShape piece(tileSize / 3);
                    piece.setPosition(col * tileSize + tileSize / 6, row * tileSize + tileSize / 6);
                    piece.setFillColor(board[row][col].isWhite ? sf::Color::White : sf::Color::Black);

                    if (board[row][col].type == PieceType::King) {
                        piece.setOutlineThickness(3);
                        piece.setOutlineColor(sf::Color::Yellow); // Обозначение дамки
                    }

                    window.draw(piece);
                }
            }
        }
    }

private:
    struct Position {
        int x, y;
        bool operator<(const Position& other) const {
            return std::tie(x, y) < std::tie(other.x, other.y);
        }
    };

    std::vector<std::vector<Piece>> board;
    Position selectedPiece = { -1, -1 };
    std::set<Position> validMoves;
    bool isWhiteTurn = true; // Чей сейчас ход

    void calculateValidMoves(int row, int col) {
        validMoves.clear();
        int direction = board[row][col].isWhite ? -1 : 1;

        // Проверка клеток для обычного хода
        if (board[row][col].type == PieceType::Pawn || board[row][col].type == PieceType::King) {
            checkMove(row, col, row + direction, col - 1);
            checkMove(row, col, row + direction, col + 1);

            // Проверка прыжков через противника
            checkJump(row, col, row + direction, col - 1, row + 2 * direction, col - 2);
            checkJump(row, col, row + direction, col + 1, row + 2 * direction, col + 2);
        }

        // Король может двигаться в обе стороны
        if (board[row][col].type == PieceType::King) {
            checkMove(row, col, row - direction, col - 1);
            checkMove(row, col, row - direction, col + 1);
            checkJump(row, col, row - direction, col - 1, row - 2 * direction, col - 2);
            checkJump(row, col, row - direction, col + 1, row - 2 * direction, col + 2);
        }
    }

    void checkMove(int srcRow, int srcCol, int destRow, int destCol) {
        if (isValidMove(destRow, destCol)) {
            validMoves.insert({ destRow, destCol });
        }
    }

    void checkJump(int srcRow, int srcCol, int midRow, int midCol, int destRow, int destCol) {
        if (isValidJump(midRow, midCol, destRow, destCol)) {
            validMoves.insert({ destRow, destCol });
        }
    }

    bool isValidMove(int row, int col) {
        return row >= 0 && row < boardSize && col >= 0 && col < boardSize && board[row][col].type == PieceType::None;
    }

    bool isValidJump(int midRow, int midCol, int destRow, int destCol) {
        return midRow >= 0 && midRow < boardSize && midCol >= 0 && midCol < boardSize &&
            destRow >= 0 && destRow < boardSize && destCol >= 0 && destCol < boardSize &&
            board[midRow][midCol].type != PieceType::None &&
            board[midRow][midCol].isWhite != board[selectedPiece.x][selectedPiece.y].isWhite &&
            board[destRow][destCol].type == PieceType::None;
    }

    void movePiece(int srcRow, int srcCol, int destRow, int destCol) {
        if (abs(destRow - srcRow) == 2) { // Если это прыжок
            int midRow = (srcRow + destRow) / 2;
            int midCol = (srcCol + destCol) / 2;
            board[midRow][midCol] = Piece(); // Убираем съеденную шашку
        }

        board[destRow][destCol] = board[srcRow][srcCol];
        board[srcRow][srcCol] = Piece();

        // Превращение в дамку
        if ((destRow == 0 && board[destRow][destCol].isWhite) ||
            (destRow == boardSize - 1 && !board[destRow][destCol].isWhite)) {
            board[destRow][destCol].type = PieceType::King;
        }
    }
};

// Главная функция
int main() {
    sf::RenderWindow window(sf::VideoMode(boardSize * tileSize, boardSize * tileSize), "Checkers");
    CheckersGame game;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                game.handleClick(event.mouseButton.x, event.mouseButton.y);
            }
        }

        window.clear();
        game.draw(window);
        window.display();
    }

    return 0;
}
//test commit