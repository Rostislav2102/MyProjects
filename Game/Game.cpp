// Game.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SIZE = 50;
const int PLAYER_SPEED = 200;
const int ENEMY_SIZE = 10;
const int BOX_SIZE = 40;
const int SPIRAL_SIZE = 100;
const int ZIGZAG_AMPLITUDE = 50;
const int ENEMY_CREATION_DISTANCE_COMPONENT = 100;
const float ANGLE_SPEED_CHANGE = 5.0f;

class GameObject {
protected:
    float x, y;
    int width, height;
public:
    GameObject(float x, float y, int width, int height)
        : x(x), y(y), width(width), height(height) {
    }

    virtual void update(float deltaTime, float x, float y) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;

    bool checkCollision(const GameObject& other) const {
        return (x < other.x + other.width &&
            x + width > other.x &&
            y < other.y + other.height &&
            y + height > other.y);
    }

    float getX() const { return x; }
    float getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

class Box : public GameObject {
private:
    float offsetX; // Смещение коробки по X относительно игрока
    float offsetY; // Смещение коробки по Y относительно игрока
public:
    Box(float x, float y) : GameObject(x, y, BOX_SIZE, BOX_SIZE), offsetX (-BOX_SIZE), offsetY(-BOX_SIZE){}

    void update(float deltaTime, float playerX, float playerY) override {
        const Uint8* keys = SDL_GetKeyboardState(nullptr);

        // Дополнительное перемещение коробки относительно игрока (Up, Down, Left, Right)
        if (keys[SDL_SCANCODE_UP]) {
            offsetY -= PLAYER_SPEED * deltaTime;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            offsetY += PLAYER_SPEED * deltaTime;
        }
        if (keys[SDL_SCANCODE_LEFT]) {
            offsetX -= PLAYER_SPEED * deltaTime;
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            offsetX += PLAYER_SPEED * deltaTime;
        }

        // Ограничение смещения коробки в пределах её размеров
        offsetX = std::max(-(float)BOX_SIZE, std::min((float)offsetX, (float)PLAYER_SIZE));
        offsetY = std::max(-(float)BOX_SIZE, std::min((float)offsetY, (float)PLAYER_SIZE));

        // Обновляем позицию коробки с учётом смещения
        x = playerX + offsetX;
        y = playerY + offsetY;
    }

    void render(SDL_Renderer* renderer) override {
        SDL_Rect rect = { (int)x, (int)y, width, height };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Красный цвет
        SDL_RenderFillRect(renderer, &rect);
    }
};

class Player : public GameObject {
public:
    Player(float x, float y) : GameObject(x, y, PLAYER_SIZE, PLAYER_SIZE) {}

    void update(float deltaTime, float x, float y) override {
        // Управление игроком
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_W]) this->y -= PLAYER_SPEED * deltaTime; // Используем this->y
        if (keys[SDL_SCANCODE_S]) this->y += PLAYER_SPEED * deltaTime; // Используем this->y
        if (keys[SDL_SCANCODE_A]) this->x -= PLAYER_SPEED * deltaTime; // Используем this->x
        if (keys[SDL_SCANCODE_D]) this->x += PLAYER_SPEED * deltaTime; // Используем this->x

        // Ограничение движения в пределах экрана
        this->x = std::max(0.0f, std::min(this->x, (float)SCREEN_WIDTH - width));
        this->y = std::max(0.0f, std::min(this->y, (float)SCREEN_HEIGHT - height));
    }

    void render(SDL_Renderer* renderer) override {
        SDL_Rect rect = { (int)x, (int)y, width, height };
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Зеленый цвет
        SDL_RenderFillRect(renderer, &rect);
    }
};

class Enemy : public GameObject {
protected:
    float speed;
public:
    Enemy(float x, float y, float speed) : GameObject(x, y, ENEMY_SIZE, ENEMY_SIZE), speed(speed) {}

    virtual void update(float deltaTime, float playerX, float playerY) = 0;

    void render(SDL_Renderer* renderer) override {
        SDL_Rect rect = { (int)x, (int)y, width, height };
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Синий цвет
        SDL_RenderFillRect(renderer, &rect);
    }
};

class StraightEnemy : public Enemy {
public:
    StraightEnemy(float x, float y, float speed) : Enemy(x, y, speed) {}

    void update(float deltaTime, float playerX, float playerY) override {
        // Движение по прямой к игроку
        float dx = playerX - x;
        float dy = playerY - y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            x += (dx / distance) * speed * deltaTime;
            y += (dy / distance) * speed * deltaTime;
        }
    }
};

class ZigZagEnemy : public Enemy {
private:
    float angle = 0.0f; // Угол для зигзага
public:
    ZigZagEnemy(float x, float y, float speed) : Enemy(x, y, speed) {}

    void update(float deltaTime, float playerX, float playerY) override {
        // Движение зигзагом
        angle += ANGLE_SPEED_CHANGE * deltaTime; // Увеличиваем угол
        float dx = playerX - x;
        float dy = playerY - y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            x += (dx / distance) * speed * deltaTime + std::cos(angle) * ZIGZAG_AMPLITUDE * deltaTime;
            y += (dy / distance) * speed * deltaTime;
        }
    }
};

class SpiralEnemy : public Enemy {
private:
    float angle = 0.0f; // Угол для спирали
public:
    SpiralEnemy(float x, float y, float speed) : Enemy(x, y, speed) {}

    void update(float deltaTime, float playerX, float playerY) override {
        // Движение по спирали
        angle += ANGLE_SPEED_CHANGE * deltaTime; // Увеличиваем угол
        float dx = playerX - x;
        float dy = playerY - y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            x += (dx / distance) * speed * deltaTime + std::cos(angle) * SPIRAL_SIZE * deltaTime;
            y += (dy / distance) * speed * deltaTime + std::sin(angle) * SPIRAL_SIZE * deltaTime;
        }
    }
};

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Player player;
    Box box;
    std::vector<Enemy*> enemies;
    int score;
    float enemy_speed; 
    bool running;

public:
    Game() : player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), box(player.getX(), player.getY()), score(0), running(true) {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("Catch the Enemies", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        srand((unsigned int)time(nullptr));

        // скорость прямо пропорциональна счету
        enemy_speed = (float)score;
        // Создаем противников
        enemies.push_back(new StraightEnemy(player.getX () - ENEMY_CREATION_DISTANCE_COMPONENT, player.getY() - ENEMY_CREATION_DISTANCE_COMPONENT, enemy_speed));
        enemies.push_back(new ZigZagEnemy(player.getX() + ENEMY_CREATION_DISTANCE_COMPONENT, player.getY() - ENEMY_CREATION_DISTANCE_COMPONENT, enemy_speed));
        enemies.push_back(new SpiralEnemy(player.getX() - ENEMY_CREATION_DISTANCE_COMPONENT, player.getY() + ENEMY_CREATION_DISTANCE_COMPONENT, enemy_speed));
    }

    ~Game() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void clearConsole() {
        system("cls"); // Очистка консоли
    }

    void run() {
        Uint32 lastTime = SDL_GetTicks();
        const Uint8* keys = SDL_GetKeyboardState(nullptr);;
        bool is_pause = false;
        bool pauseKeyPressed = false;
        while (running) {
            Uint32 currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            // Обработка событий SDL
            handleEvents();

            // Обновление состояния клавиш
            SDL_PumpEvents();

            // Проверка нажатия клавиши PAUSE
            if (keys[SDL_SCANCODE_PAUSE] || keys[SDL_SCANCODE_ESCAPE]) {
                if (!pauseKeyPressed) { // Если клавиша только что нажата
                    is_pause = !is_pause; // Переключаем состояние паузы
                    pauseKeyPressed = true; // Устанавливаем флаг нажатия
                }
            }
            else {
                pauseKeyPressed = false; // Сбрасываем флаг, если клавиша отпущена
            }

            if (is_pause)
                continue;

            update(deltaTime);
            render();
            clearConsole(); // Очищаем консоль
            std::cout << "Score: " << score << std::endl; // Выводим счет

        }

        std::cout << "Game is over with a score: " << score << std::endl; // Выводим сообщение о завершении игры
        int n;
        std::cin >> n;
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    void update(float deltaTime) {
        player.update(deltaTime, player.getX(), player.getY());
        box.update(deltaTime, player.getX(), player.getY());

        if (!enemies.empty()) {
            for (auto it = enemies.begin(); it != enemies.end();) {
                (*it)->update(deltaTime, player.getX(), player.getY());

                // Проверка столкновения с коробкой
                if (box.checkCollision(**it)) {
                    score++;
                    delete* it; // Удаляем противника
                    it = enemies.erase(it); // Убираем его из вектора и обновляем итератор

                    // Создаем одного нового случайного противника

                    // скорость прямо пропорциональна счету
                    enemy_speed = (float)score;
                    int type = rand() % 3;
                    switch (type) {
                    case 0:
                        enemies.push_back(new StraightEnemy(player.getX() - ENEMY_CREATION_DISTANCE_COMPONENT, player.getY() - ENEMY_CREATION_DISTANCE_COMPONENT, enemy_speed));
                        break;
                    case 1:
                        enemies.push_back(new ZigZagEnemy(player.getX() + ENEMY_CREATION_DISTANCE_COMPONENT, player.getY() - ENEMY_CREATION_DISTANCE_COMPONENT, enemy_speed));
                        break;
                    case 2:
                        enemies.push_back(new SpiralEnemy(player.getX() - ENEMY_CREATION_DISTANCE_COMPONENT, player.getY() + ENEMY_CREATION_DISTANCE_COMPONENT, enemy_speed));
                        break;
                    }
                }
                else {
                    // Проверка столкновения с игроком
                    if (player.checkCollision(**it)) {
                        running = false; // Завершение игры
                    }
                    ++it; // Переходим к следующему элементу
                }
            }
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        player.render(renderer);
        box.render(renderer);
        for (auto& enemy : enemies) {
            enemy->render(renderer);
        }

        SDL_RenderPresent(renderer);
    }
};


int main(int argc, char* argv[]) {
    Game game;
    game.run();
    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
