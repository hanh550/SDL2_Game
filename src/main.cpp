#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
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
const int BOSS_SPEED = 3; // Tốc độ di chuyển ngang của boss

enum GameState { MENU, PLAYING, GAME_OVER, WIN, EXIT };
GameState gameState = MENU;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* menuBackground = nullptr;
SDL_Texture* background = nullptr;
SDL_Texture* player1Texture = nullptr;
SDL_Texture* player2Texture = nullptr;
SDL_Texture* bulletTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* enemy2Texture = nullptr;
SDL_Texture* enemy3Texture = nullptr;
SDL_Texture* enemy4Texture = nullptr;
SDL_Texture* enemy5Texture = nullptr;
SDL_Texture* bossTexture = nullptr;
SDL_Texture* bomTexture = nullptr;
SDL_Texture* scoreTexture = nullptr;
SDL_Texture* explosionTexture = nullptr;
SDL_Texture* gameOverTexture = nullptr;
SDL_Texture* winTexture = nullptr;
SDL_Texture* playAgainTexture = nullptr;
SDL_Texture* exitTexture = nullptr;
SDL_Texture* onePlayerTexture = nullptr;
SDL_Texture* twoPlayersTexture = nullptr;
SDL_Texture* menuExitTexture = nullptr;
TTF_Font* font = nullptr;
TTF_Font* bigFont = nullptr;
SDL_Color white = {255, 255, 255, 255};
SDL_Color red = {255, 0, 0, 255};
SDL_Rect scoreRect;
SDL_Rect gameOverRect;
SDL_Rect winRect;
SDL_Rect playAgainRect;
SDL_Rect exitRect;
SDL_Rect onePlayerRect;
SDL_Rect twoPlayersRect;
SDL_Rect menuExitRect;

struct Explosion {
    int x, y, w, h;
    int frame;
    bool active;
};

struct Object {
    int x, y, w, h;
    bool active;
    int enemyType; // 1-5: kẻ địch thường, 6: boss
    int health;   // Thanh máu
    int direction; // Thêm biến để theo dõi hướng di chuyển của boss (1: phải, -1: trái)
};

Object player1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
Object player2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0, 1, 0};
vector<Object> bullets;
vector<Object> enemies;
vector<Object> enemyBullets;
vector<Explosion> explosions;
int score = 0;
int enemySpawnTimer = 100;
int bulletCooldown1 = 0;
int bulletCooldown2 = 0;
bool isTwoPlayers = false;
bool bossSpawned = false;

SDL_Texture* loadTexture(const char* filename) {
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) {
        printf("Error loading image %s: %s\n", filename, IMG_GetError());
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("Error creating texture from %s: %s\n", filename, SDL_GetError());
    }
    return texture;
}

SDL_Texture* renderText(const char* text, TTF_Font* textFont, SDL_Color color) {
    if (!textFont) {
        printf("Font is null!\n");
        return nullptr;
    }
    SDL_Surface* surface = TTF_RenderText_Solid(textFont, text, color);
    if (!surface) {
        printf("Error rendering text %s: %s\n", text, TTF_GetError());
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("Error creating texture from text %s: %s\n", text, SDL_GetError());
    }
    return texture;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        return false;
    }
    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return false;
    }

    window = SDL_CreateWindow("Shooter Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }

    menuBackground = loadTexture("chickenbackground.jpg");
    if (!menuBackground) return false;
    background = loadTexture("space.jpg");
    if (!background) return false;
    player1Texture = loadTexture("tank.png");
    if (!player1Texture) return false;
    player2Texture = loadTexture("player2.png");
    if (!player2Texture) return false;
    bulletTexture = loadTexture("bullet.png");
    if (!bulletTexture) return false;
    enemyTexture = loadTexture("enemy.png");
    if (!enemyTexture) return false;
    enemy2Texture = loadTexture("enemy2.png");
    if (!enemy2Texture) return false;
    enemy3Texture = loadTexture("enemy3.png");
    if (!enemy3Texture) return false;
    enemy4Texture = loadTexture("enemy4.png");
    if (!enemy4Texture) return false;
    enemy5Texture = loadTexture("enemy5.png");
    if (!enemy5Texture) return false;
    bossTexture = loadTexture("boss.png");
    if (!bossTexture) return false;
    bomTexture = loadTexture("egg.png");
    if (!bomTexture) return false;
    explosionTexture = loadTexture("explosion.png");
    if (!explosionTexture) return false;

    font = TTF_OpenFont("arial.ttf", 36);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return false;
    }
    bigFont = TTF_OpenFont("arial.ttf", 72);
    if (!bigFont) {
        printf("Failed to load big font: %s\n", TTF_GetError());
        return false;
    }

    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);
    scoreTexture = renderText(scoreText, font, white);
    if (!scoreTexture) return false;
    scoreRect = {SCREEN_WIDTH - 200, 10, 0, 0};
    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreRect.w, &scoreRect.h);

    gameOverTexture = renderText("Game Over", bigFont, white);
    if (!gameOverTexture) return false;
    SDL_QueryTexture(gameOverTexture, NULL, NULL, &gameOverRect.w, &gameOverRect.h);
    gameOverRect = {(SCREEN_WIDTH - gameOverRect.w) / 2, SCREEN_HEIGHT / 4, gameOverRect.w, gameOverRect.h};

    winTexture = renderText("You Win!", bigFont, white);
    if (!winTexture) return false;
    SDL_QueryTexture(winTexture, NULL, NULL, &winRect.w, &winRect.h);
    winRect = {(SCREEN_WIDTH - winRect.w) / 2, SCREEN_HEIGHT / 4, winRect.w, winRect.h};

    playAgainTexture = renderText("Play Again?", font, white);
    if (!playAgainTexture) return false;
    SDL_QueryTexture(playAgainTexture, NULL, NULL, &playAgainRect.w, &playAgainRect.h);
    playAgainRect = {(SCREEN_WIDTH - playAgainRect.w) / 2, gameOverRect.y + gameOverRect.h + 100, playAgainRect.w, playAgainRect.h};

    exitTexture = renderText("Exit", font, white);
    if (!exitTexture) return false;
    SDL_QueryTexture(exitTexture, NULL, NULL, &exitRect.w, &exitRect.h);
    exitRect = {(SCREEN_WIDTH - exitRect.w) / 2, playAgainRect.y + playAgainRect.h + 50, exitRect.w, exitRect.h};

    onePlayerTexture = renderText("1 Player", font, red);
    if (!onePlayerTexture) return false;
    SDL_QueryTexture(onePlayerTexture, NULL, NULL, &onePlayerRect.w, &onePlayerRect.h);
    onePlayerRect = {(SCREEN_WIDTH - onePlayerRect.w) / 2, SCREEN_HEIGHT / 4, onePlayerRect.w, onePlayerRect.h};

    twoPlayersTexture = renderText("2 Players", font, red);
    if (!twoPlayersTexture) return false;
    SDL_QueryTexture(twoPlayersTexture, NULL, NULL, &twoPlayersRect.w, &twoPlayersRect.h);
    twoPlayersRect = {(SCREEN_WIDTH - twoPlayersRect.w) / 2, onePlayerRect.y + onePlayerRect.h + 50, twoPlayersRect.w, twoPlayersRect.h};

    menuExitTexture = renderText("Exit", font, red);
    if (!menuExitTexture) return false;
    SDL_QueryTexture(menuExitTexture, NULL, NULL, &menuExitRect.w, &menuExitRect.h);
    menuExitRect = {(SCREEN_WIDTH - menuExitRect.w) / 2, twoPlayersRect.y + twoPlayersRect.h + 50, menuExitRect.w, menuExitRect.h};

    return true;
}

void updateScoreTexture() {
    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);
    if (scoreTexture) SDL_DestroyTexture(scoreTexture);
    scoreTexture = renderText(scoreText, font, white);
    if (scoreTexture) {
        scoreRect = {SCREEN_WIDTH - 200, 10, 0, 0};
        SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreRect.w, &scoreRect.h);
    }
}

void renderMenu() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, menuBackground, nullptr, nullptr);
    SDL_RenderCopy(renderer, onePlayerTexture, nullptr, &onePlayerRect);
    SDL_RenderCopy(renderer, twoPlayersTexture, nullptr, &twoPlayersRect);
    SDL_RenderCopy(renderer, menuExitTexture, nullptr, &menuExitRect);
    SDL_RenderPresent(renderer);
}

void renderGameOver() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, nullptr, nullptr);
    SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
    SDL_RenderCopy(renderer, playAgainTexture, nullptr, &playAgainRect);
    SDL_RenderCopy(renderer, exitTexture, nullptr, &exitRect);
    SDL_RenderPresent(renderer);
}

void renderWin() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, nullptr, nullptr);
    SDL_RenderCopy(renderer, winTexture, nullptr, &winRect);
    SDL_RenderCopy(renderer, playAgainTexture, nullptr, &playAgainRect);
    SDL_RenderCopy(renderer, exitTexture, nullptr, &exitRect);
    SDL_RenderPresent(renderer);
}

void handleEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
            gameState = EXIT;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (gameState == MENU) {
                if (mouseX >= onePlayerRect.x && mouseX <= onePlayerRect.x + onePlayerRect.w &&
                    mouseY >= onePlayerRect.y && mouseY <= onePlayerRect.y + onePlayerRect.h) {
                    isTwoPlayers = false;
                    player1 = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                    player2.active = false;
                    gameState = PLAYING;
                }
                else if (mouseX >= twoPlayersRect.x && mouseX <= twoPlayersRect.x + twoPlayersRect.w &&
                         mouseY >= twoPlayersRect.y && mouseY <= twoPlayersRect.y + twoPlayersRect.h) {
                    isTwoPlayers = true;
                    player1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                    player2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0, 1, 0};
                    gameState = PLAYING;
                }
                else if (mouseX >= menuExitRect.x && mouseX <= menuExitRect.x + menuExitRect.w &&
                         mouseY >= menuExitRect.y && mouseY <= menuExitRect.y + menuExitRect.h) {
                    gameState = EXIT;
                    running = false;
                }
            }
            else if (gameState == GAME_OVER || gameState == WIN) {
                if (mouseX >= playAgainRect.x && mouseX <= playAgainRect.x + playAgainRect.w &&
                    mouseY >= playAgainRect.y && mouseY <= playAgainRect.y + playAgainRect.h) {
                    if (isTwoPlayers) {
                        player1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                        player2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0, 1, 0};
                    } else {
                        player1 = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                        player2.active = false;
                    }
                    bullets.clear();
                    enemies.clear();
                    enemyBullets.clear();
                    explosions.clear();
                    score = 0;
                    enemySpawnTimer = 100;
                    bulletCooldown1 = 0;
                    bulletCooldown2 = 0;
                    bossSpawned = false;
                    updateScoreTexture();
                    gameState = PLAYING;
                }
                else if (mouseX >= exitRect.x && mouseX <= exitRect.x + exitRect.w &&
                         mouseY >= exitRect.y && mouseY <= exitRect.y + exitRect.h) {
                    gameState = EXIT;
                    running = false;
                }
            }
        }
    }
}

void updateGame() {
    if ((!player1.active && !isTwoPlayers) || (!player1.active && !player2.active && isTwoPlayers)) {
        gameState = GAME_OVER;
        return;
    }

    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_A] && player1.x > 0)
        player1.x -= PLAYER_SPEED;
    if (keystates[SDL_SCANCODE_D] && player1.x < SCREEN_WIDTH - player1.w)
        player1.x += PLAYER_SPEED;
    if (keystates[SDL_SCANCODE_SPACE] && bulletCooldown1 == 0 && player1.active) {
        bullets.push_back({player1.x + player1.w / 2 - 5, player1.y, 10, 20, true, 0, 1, 0});
        bulletCooldown1 = 10;
    }
    if (bulletCooldown1 > 0) bulletCooldown1--;

    if (isTwoPlayers) {
        if (keystates[SDL_SCANCODE_LEFT] && player2.x > 0)
            player2.x -= PLAYER_SPEED;
        if (keystates[SDL_SCANCODE_RIGHT] && player2.x < SCREEN_WIDTH - player2.w)
            player2.x += PLAYER_SPEED;
        if (keystates[SDL_SCANCODE_RETURN] && bulletCooldown2 == 0 && player2.active) {
            bullets.push_back({player2.x + player2.w / 2 - 5, player2.y, 10, 20, true, 0, 1, 0});
            bulletCooldown2 = 10;
        }
        if (bulletCooldown2 > 0) bulletCooldown2--;
    }

    for (auto& bullet : bullets) bullet.y -= BULLET_SPEED;
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Object& b) { return b.y < -b.h; }), bullets.end());

    if (!bossSpawned) {
        if (score >= 500 && enemies.empty()) {
            enemies.push_back({SCREEN_WIDTH / 2 - 50, 50, 100, 100, true, 6, 50, 1}); // Boss bắt đầu ở y = 50, direction = 1 (phải)
            bossSpawned = true;
        } else if (score < 500) {
            int baseEnemies = 1;
            int extraEnemies = (score / 100) * 2;
            int totalEnemies = baseEnemies + extraEnemies;

            if (--enemySpawnTimer <= 0) {
                for (int i = 0; i < totalEnemies; i++) {
                    int type = (rand() % 5) + 1;
                    enemies.push_back({rand() % (SCREEN_WIDTH - 50), 0, 50, 50, true, type, 1, 0});
                }
                enemySpawnTimer = max(30, 100 - (score / 100) * 10);
            }
        }
    }

    for (auto& enemy : enemies) {
        if (enemy.enemyType == 6) { // Logic cho boss
            // Di chuyển ngang
            enemy.x += BOSS_SPEED * enemy.direction;
            if (enemy.x <= 0) enemy.direction = 1; // Đổi hướng sang phải khi chạm biên trái
            if (enemy.x >= SCREEN_WIDTH - enemy.w) enemy.direction = -1; // Đổi hướng sang trái khi chạm biên phải

            // Xả đạn liên tục
            int shootChance = 10; // Giảm shootChance để bắn liên tục hơn
            if (rand() % shootChance == 0 && enemy.active) {
                // Bắn 3 viên đạn từ các vị trí khác nhau trên boss
                enemyBullets.push_back({enemy.x + enemy.w / 4 - 5, enemy.y + enemy.h, 10, 20, true, 0, 1, 0});
                enemyBullets.push_back({enemy.x + enemy.w / 2 - 5, enemy.y + enemy.h, 10, 20, true, 0, 1, 0});
                enemyBullets.push_back({enemy.x + 3 * enemy.w / 4 - 5, enemy.y + enemy.h, 10, 20, true, 0, 1, 0});
            }
        } else { // Logic cho kẻ địch thường
            enemy.y += ENEMY_SPEED;
            int shootChance = 100;
            if (rand() % shootChance == 0 && enemy.active) {
                enemyBullets.push_back({enemy.x + enemy.w / 2 - 5, enemy.y + enemy.h, 10, 20, true, 0, 1, 0});
            }
        }
    }
    enemies.erase(remove_if(enemies.begin(), enemies.end(), [](const Object& e) { return e.y > SCREEN_HEIGHT && e.enemyType != 6; }), enemies.end());

    for (auto& bomb : enemyBullets) bomb.y += BULLET_SPEED;
    enemyBullets.erase(remove_if(enemyBullets.begin(), enemyBullets.end(), [](const Object& b) { return b.y > SCREEN_HEIGHT; }), enemyBullets.end());

    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = 0; j < bullets.size(); ++j) {
            if (bullets[j].active && enemies[i].active &&
                bullets[j].x < enemies[i].x + enemies[i].w && bullets[j].x + bullets[j].w > enemies[i].x &&
                bullets[j].y < enemies[i].y + enemies[i].h && bullets[j].y + bullets[j].h > enemies[i].y) {
                bullets[j].active = false;
                enemies[i].health--;
                if (enemies[i].health <= 0) {
                    enemies[i].active = false;
                    score += (enemies[i].enemyType == 6) ? 50 : 10;
                    updateScoreTexture();
                    explosions.push_back({enemies[i].x, enemies[i].y, 50, 50, 20, true});
                    if (enemies[i].enemyType == 6) {
                        gameState = WIN;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < enemies.size(); ++i) {
        if (player1.active && enemies[i].active &&
            player1.x < enemies[i].x + enemies[i].w && player1.x + player1.w > enemies[i].x &&
            player1.y < enemies[i].y + enemies[i].h && player1.y + player1.h > enemies[i].y) {
            player1.active = false;
            explosions.push_back({player1.x, player1.y, 50, 50, 20, true});
            break;
        }
    }

    if (isTwoPlayers) {
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (player2.active && enemies[i].active &&
                player2.x < enemies[i].x + enemies[i].w && player2.x + player2.w > enemies[i].x &&
                player2.y < enemies[i].y + enemies[i].h && player2.y + player2.h > enemies[i].y) {
                player2.active = false;
                explosions.push_back({player2.x, player2.y, 50, 50, 20, true});
                break;
            }
        }
    }

    for (size_t i = 0; i < enemyBullets.size(); ++i) {
        if (player1.active && enemyBullets[i].active &&
            player1.x < enemyBullets[i].x + enemyBullets[i].w && player1.x + player1.w > enemyBullets[i].x &&
            player1.y < enemyBullets[i].y + enemyBullets[i].h && player1.y + player1.h > enemyBullets[i].y) {
            player1.active = false;
            enemyBullets[i].active = false;
            explosions.push_back({player1.x, player1.y, 50, 50, 20, true});
            break;
        }
    }

    if (isTwoPlayers) {
        for (size_t i = 0; i < enemyBullets.size(); ++i) {
            if (player2.active && enemyBullets[i].active &&
                player2.x < enemyBullets[i].x + enemyBullets[i].w && player2.x + player2.w > enemyBullets[i].x &&
                player2.y < enemyBullets[i].y + enemyBullets[i].h && player2.y + player2.h > enemyBullets[i].y) {
                player2.active = false;
                enemyBullets[i].active = false;
                explosions.push_back({player2.x, player2.y, 50, 50, 20, true});
                break;
            }
        }
    }

    // Cập nhật và xóa các vụ nổ
    for (auto& explosion : explosions) {
        if (explosion.active) {
            explosion.frame--;
            if (explosion.frame <= 0) {
                explosion.active = false;
            }
        }
    }
    explosions.erase(
        remove_if(explosions.begin(), explosions.end(), 
            [](const Explosion& e) { return !e.active; }), 
        explosions.end()
    );

    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Object& b) { return !b.active; }), bullets.end());
    enemies.erase(remove_if(enemies.begin(), enemies.end(), [](const Object& e) { return !e.active; }), enemies.end());
    enemyBullets.erase(remove_if(enemyBullets.begin(), enemyBullets.end(), [](const Object& b) { return !b.active; }), enemyBullets.end());
}

void render() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, nullptr, nullptr);

    if (player1.active) {
        SDL_Rect pRect = {player1.x, player1.y, player1.w, player1.h};
        SDL_RenderCopy(renderer, player1Texture, nullptr, &pRect);
    }
    if (isTwoPlayers && player2.active) {
        SDL_Rect pRect = {player2.x, player2.y, player2.w, player2.h};
        SDL_RenderCopy(renderer, player2Texture, nullptr, &pRect);
    }

    for (const auto& bullet : bullets) {
        SDL_Rect bRect = {bullet.x, bullet.y, bullet.w, bullet.h};
        SDL_RenderCopy(renderer, bulletTexture, nullptr, &bRect);
    }

    for (const auto& enemy : enemies) {
        SDL_Rect eRect = {enemy.x, enemy.y, enemy.w, enemy.h};
        switch (enemy.enemyType) {
            case 1: SDL_RenderCopy(renderer, enemyTexture, nullptr, &eRect); break;
            case 2: SDL_RenderCopy(renderer, enemy2Texture, nullptr, &eRect); break;
            case 3: SDL_RenderCopy(renderer, enemy3Texture, nullptr, &eRect); break;
            case 4: SDL_RenderCopy(renderer, enemy4Texture, nullptr, &eRect); break;
            case 5: SDL_RenderCopy(renderer, enemy5Texture, nullptr, &eRect); break;
            case 6: SDL_RenderCopy(renderer, bossTexture, nullptr, &eRect); break;
        }
    }

    for (const auto& bomb : enemyBullets) {
        SDL_Rect bombRect = {bomb.x, bomb.y, bomb.w, bomb.h};
        SDL_RenderCopy(renderer, bomTexture, nullptr, &bombRect);
    }

    for (const auto& explosion : explosions) {
        if (explosion.active) {
            SDL_Rect eRect = {explosion.x, explosion.y, explosion.w, explosion.h};
            SDL_RenderCopy(renderer, explosionTexture, nullptr, &eRect);
        }
    }

    SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);
    SDL_RenderPresent(renderer);
}

void clean() {
    if (font) TTF_CloseFont(font);
    if (bigFont) TTF_CloseFont(bigFont);
    TTF_Quit();

    if (menuBackground) SDL_DestroyTexture(menuBackground);
    if (background) SDL_DestroyTexture(background);
    if (player1Texture) SDL_DestroyTexture(player1Texture);
    if (player2Texture) SDL_DestroyTexture(player2Texture);
    if (bulletTexture) SDL_DestroyTexture(bulletTexture);
    if (enemyTexture) SDL_DestroyTexture(enemyTexture);
    if (enemy2Texture) SDL_DestroyTexture(enemy2Texture);
    if (enemy3Texture) SDL_DestroyTexture(enemy3Texture);
    if (enemy4Texture) SDL_DestroyTexture(enemy4Texture);
    if (enemy5Texture) SDL_DestroyTexture(enemy5Texture);
    if (bossTexture) SDL_DestroyTexture(bossTexture);
    if (bomTexture) SDL_DestroyTexture(bomTexture);
    if (scoreTexture) SDL_DestroyTexture(scoreTexture);
    if (explosionTexture) SDL_DestroyTexture(explosionTexture);
    if (gameOverTexture) SDL_DestroyTexture(gameOverTexture);
    if (winTexture) SDL_DestroyTexture(winTexture);
    if (playAgainTexture) SDL_DestroyTexture(playAgainTexture);
    if (exitTexture) SDL_DestroyTexture(exitTexture);
    if (onePlayerTexture) SDL_DestroyTexture(onePlayerTexture);
    if (twoPlayersTexture) SDL_DestroyTexture(twoPlayersTexture);
    if (menuExitTexture) SDL_DestroyTexture(menuExitTexture);

    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    srand(static_cast<unsigned>(time(0)));
    if (!init()) {
        printf("Initialization failed!\n");
        clean();
        return 1;
    }

    bool running = true;
    while (running) {
        handleEvents(running);
        switch (gameState) {
            case MENU:
                renderMenu();
                break;
            case PLAYING:
                updateGame();
                render();
                break;
            case GAME_OVER:
                renderGameOver();
                break;
            case WIN:
                renderWin();
                break;
            case EXIT:
                running = false;
                break;
        }
        SDL_Delay(16); // ~60 FPS
    }

    clean();
    return 0;
}