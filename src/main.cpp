#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstdio>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SPEED = 5;
const int BULLET_SPEED = 7;
const int ENEMY_SPEED = 2;

struct Object {
    int x, y, w, h;
    bool active;
};

// SDL Variables
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* background = nullptr;
SDL_Texture* playerTexture = nullptr;
SDL_Texture* bulletTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;  // Thêm texture cho kẻ địch

Object player = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true};
vector<Object> bullets;
vector<Object> enemies;

// Hàm tải ảnh thành SDL_Texture
SDL_Texture* loadTexture(const char* filename) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(filename);
    if (loadedSurface) {
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
    } else {
        printf("Failed to load image %s: %s\n", filename, IMG_GetError());
    }
    return texture;
}

// Khởi tạo SDL và load ảnh
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) return false;

    window = SDL_CreateWindow("Shooter Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    background = loadTexture("space.jpg");
    playerTexture = loadTexture("player.png");
    bulletTexture = loadTexture("bullet.png");
    enemyTexture = loadTexture("enemy.png");  // Load ảnh kẻ địch

    return background && playerTexture && bulletTexture && enemyTexture;
}

// Tạo kẻ địch
void spawnEnemy() {
    Object enemy = {rand() % (SCREEN_WIDTH - 40), 0, 40, 40, true};
    enemies.push_back(enemy);
}

// Kiểm tra va chạm
bool checkCollision(const Object& a, const Object& b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

// Cập nhật trạng thái game
void update() {
    for (auto& bullet : bullets) bullet.y -= BULLET_SPEED;
    for (auto& enemy : enemies) enemy.y += ENEMY_SPEED;

    // Kiểm tra va chạm đạn & kẻ địch
    for (auto& bullet : bullets) {
        for (auto& enemy : enemies) {
            if (checkCollision(bullet, enemy)) {
                enemy.active = false;
                bullet.active = false;
            }
        }
    }

    // Kiểm tra va chạm kẻ địch với nhau
    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            if (checkCollision(enemies[i], enemies[j])) {
                enemies[j].active = false;
            }
        }
    }

    // Kiểm tra kẻ địch va chạm nhân vật
    for (auto& enemy : enemies) {
        if (checkCollision(enemy, player)) {
            player.active = false;
        }
    }

    // Xóa đối tượng không còn hoạt động
    enemies.erase(remove_if(enemies.begin(), enemies.end(), [](Object& e) { return e.y > SCREEN_HEIGHT || !e.active; }), enemies.end());
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](Object& b) { return b.y < 0 || !b.active; }), bullets.end());
}

// Vẽ đối tượng lên màn hình
void render() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, nullptr, nullptr);

    // Vẽ nhân vật chính
    if (player.active && playerTexture) {
        SDL_Rect pRect = {player.x, player.y, player.w, player.h};
        SDL_RenderCopy(renderer, playerTexture, NULL, &pRect);
    }

    // Vẽ đạn
    for (auto& bullet : bullets) {
        SDL_Rect bRect = {bullet.x, bullet.y, bullet.w, bullet.h};
        SDL_RenderCopy(renderer, bulletTexture, NULL, &bRect);
    }

    // Vẽ kẻ địch
    for (auto& enemy : enemies) {
        if (enemy.active && enemyTexture) {
            SDL_Rect eRect = {enemy.x, enemy.y, enemy.w, enemy.h};
            SDL_RenderCopy(renderer, enemyTexture, NULL, &eRect);
        }
    }

    SDL_RenderPresent(renderer);
}

// Xử lý sự kiện người chơi
void handleEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;
        if (player.active && e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_LEFT && player.x > 0) player.x -= PLAYER_SPEED;
            if (e.key.keysym.sym == SDLK_RIGHT && player.x < SCREEN_WIDTH - player.w) player.x += PLAYER_SPEED;
            if (e.key.keysym.sym == SDLK_SPACE) bullets.push_back({player.x + player.w / 2 - 5, player.y, 10, 20, true});
        }
    }
}

// Giải phóng bộ nhớ
void clean() {
    if (background) SDL_DestroyTexture(background);
    if (playerTexture) SDL_DestroyTexture(playerTexture);
    if (bulletTexture) SDL_DestroyTexture(bulletTexture);
    if (enemyTexture) SDL_DestroyTexture(enemyTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

// Chạy chương trình chính
int main(int argc, char* argv[]) {
    srand(time(0));
    if (!init()) return -1;
    bool running = true;
    while (running) {
        handleEvents(running);
        update();
        if (player.active && rand() % 100 < 2) spawnEnemy();
        render();
        SDL_Delay(16);
    }
    clean();
    return 0;
}
