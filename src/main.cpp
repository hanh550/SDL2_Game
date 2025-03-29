//ngohoanganh
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

enum GameState { MENU, PLAYING, GAME_OVER, EXIT };
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
SDL_Texture* bomTexture = nullptr;
SDL_Texture* scoreTexture = nullptr;
SDL_Texture* explosionTexture = nullptr;
SDL_Texture* gameOverTexture = nullptr;
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
    int enemyType;  // Thêm để phân biệt các loại enemy (1-5)
};

Object player1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0};
Object player2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0};
vector<Object> bullets;
vector<Object> enemies;
vector<Object> enemyBullets;
vector<Explosion> explosions;
int score = 0;
int enemySpawnTimer = 100;
int bulletCooldown1 = 0;
int bulletCooldown2 = 0;
bool isTwoPlayers = false;

SDL_Texture* loadTexture(const char* filename) {
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) {
        printf("Error loading image %s: %s\n", filename, IMG_GetError());
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Error creating texture from %s: %s\n", filename, SDL_GetError());
    }
    SDL_FreeSurface(surface);
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
    if (!texture) {
        printf("Error creating texture from text %s: %s\n", text, SDL_GetError());
    }
    SDL_FreeSurface(surface);
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
    background = loadTexture("space.jpg");
    player1Texture = loadTexture("tank.png");
    player2Texture = loadTexture("player2.png");
    bulletTexture = loadTexture("bullet.png");
    enemyTexture = loadTexture("enemy.png");
    enemy2Texture = loadTexture("enemy2.png");
    enemy3Texture = loadTexture("enemy3.png");
    enemy4Texture = loadTexture("enemy4.png");
    enemy5Texture = loadTexture("enemy5.png");
    bomTexture = loadTexture("egg.png");
    explosionTexture = loadTexture("explosion.png");

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
    scoreRect.x = SCREEN_WIDTH - 200;
    scoreRect.y = 10;
    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreRect.w, &scoreRect.h);

    gameOverTexture = renderText("Game Over", bigFont, white);
    if (!gameOverTexture) return false;
    SDL_QueryTexture(gameOverTexture, NULL, NULL, &gameOverRect.w, &gameOverRect.h);
    gameOverRect.x = (SCREEN_WIDTH - gameOverRect.w) / 2;
    gameOverRect.y = SCREEN_HEIGHT / 4;

    playAgainTexture = renderText("Play Again?", font, white);
    if (!playAgainTexture) return false;
    SDL_QueryTexture(playAgainTexture, NULL, NULL, &playAgainRect.w, &playAgainRect.h);
    playAgainRect.x = (SCREEN_WIDTH - playAgainRect.w) / 2;
    playAgainRect.y = gameOverRect.y + gameOverRect.h + 100;

    exitTexture = renderText("Exit", font, white);
    if (!exitTexture) return false;
    SDL_QueryTexture(exitTexture, NULL, NULL, &exitRect.w, &exitRect.h);
    exitRect.x = (SCREEN_WIDTH - exitRect.w) / 2;
    exitRect.y = playAgainRect.y + playAgainRect.h + 50;

    onePlayerTexture = renderText("1 Player", font, red);
    if (!onePlayerTexture) return false;
    SDL_QueryTexture(onePlayerTexture, NULL, NULL, &onePlayerRect.w, &onePlayerRect.h);
    onePlayerRect.x = (SCREEN_WIDTH - onePlayerRect.w) / 2;
    onePlayerRect.y = SCREEN_HEIGHT / 4;

    twoPlayersTexture = renderText("2 Players", font, red);
    if (!twoPlayersTexture) return false;
    SDL_QueryTexture(twoPlayersTexture, NULL, NULL, &twoPlayersRect.w, &twoPlayersRect.h);
    twoPlayersRect.x = (SCREEN_WIDTH - twoPlayersRect.w) / 2;
    twoPlayersRect.y = onePlayerRect.y + onePlayerRect.h + 50;

    menuExitTexture = renderText("Exit", font, red);
    if (!menuExitTexture) return false;
    SDL_QueryTexture(menuExitTexture, NULL, NULL, &menuExitRect.w, &menuExitRect.h);
    menuExitRect.x = (SCREEN_WIDTH - menuExitRect.w) / 2;
    menuExitRect.y = twoPlayersRect.y + twoPlayersRect.h + 50;

    return true;
}

void updateScoreTexture() {
    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);
    if (scoreTexture) {
        SDL_DestroyTexture(scoreTexture);
        scoreTexture = nullptr;
    }
    scoreTexture = renderText(scoreText, font, white);
    if (scoreTexture) {
        scoreRect.x = SCREEN_WIDTH - 200;
        scoreRect.y = 10;
        SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreRect.w, &scoreRect.h);
    }
}

void renderMenu() {
    SDL_RenderClear(renderer);
    if (menuBackground) SDL_RenderCopy(renderer, menuBackground, nullptr, nullptr);
    if (onePlayerTexture) SDL_RenderCopy(renderer, onePlayerTexture, NULL, &onePlayerRect);
    if (twoPlayersTexture) SDL_RenderCopy(renderer, twoPlayersTexture, NULL, &twoPlayersRect);
    if (menuExitTexture) SDL_RenderCopy(renderer, menuExitTexture, NULL, &menuExitRect);
    SDL_RenderPresent(renderer);
}

void renderGameOver() {
    SDL_RenderClear(renderer);
    if (background) SDL_RenderCopy(renderer, background, nullptr, nullptr);
    if (gameOverTexture) SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
    if (playAgainTexture) SDL_RenderCopy(renderer, playAgainTexture, NULL, &playAgainRect);
    if (exitTexture) SDL_RenderCopy(renderer, exitTexture, NULL, &exitRect);
    SDL_RenderPresent(renderer);
}

void handleEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (gameState == MENU) {
                if (mouseX > onePlayerRect.x && mouseX < onePlayerRect.x + onePlayerRect.w &&
                    mouseY > onePlayerRect.y && mouseY < onePlayerRect.y + onePlayerRect.h) {
                    isTwoPlayers = false;
                    player1 = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0};
                    player2.active = false;
                    gameState = PLAYING;
                }
                if (mouseX > twoPlayersRect.x && mouseX < twoPlayersRect.x + twoPlayersRect.w &&
                    mouseY > twoPlayersRect.y && mouseY < twoPlayersRect.y + twoPlayersRect.h) {
                    isTwoPlayers = true;
                    player1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0};
                    player2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0};
                    gameState = PLAYING;
                }
                if (mouseX > menuExitRect.x && mouseX < menuExitRect.x + menuExitRect.w &&
                    mouseY > menuExitRect.y && mouseY < menuExitRect.y + menuExitRect.h) {
                    gameState = EXIT;
                    running = false;
                }
            } else if (gameState == GAME_OVER) {
                if (mouseX > playAgainRect.x && mouseX < playAgainRect.x + playAgainRect.w &&
                    mouseY > playAgainRect.y && mouseY < playAgainRect.y + playAgainRect.h) {
                    if (isTwoPlayers) {
                        player1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0};
                        player2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0};
                    } else {
                        player1 = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0};
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
                    updateScoreTexture();
                    gameState = PLAYING;
                }
                if (mouseX > exitRect.x && mouseX < exitRect.x + exitRect.w &&
                    mouseY > exitRect.y && mouseY < exitRect.y + exitRect.h) {
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
        bullets.push_back({player1.x + player1.w / 2 - 5, player1.y, 10, 20, true, 0});
        bulletCooldown1 = 10;
    }
    if (bulletCooldown1 > 0) bulletCooldown1--;

    if (isTwoPlayers) {
        if (keystates[SDL_SCANCODE_LEFT] && player2.x > 0)
            player2.x -= PLAYER_SPEED;
        if (keystates[SDL_SCANCODE_RIGHT] && player2.x < SCREEN_WIDTH - player2.w)
            player2.x += PLAYER_SPEED;
        if (keystates[SDL_SCANCODE_RETURN] && bulletCooldown2 == 0 && player2.active) {
            bullets.push_back({player2.x + player2.w / 2 - 5, player2.y, 10, 20, true, 0});
            bulletCooldown2 = 10;
        }
        if (bulletCooldown2 > 0) bulletCooldown2--;
    }

    for (auto& bullet : bullets) bullet.y -= BULLET_SPEED;
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Object& b) { return b.y < -b.h; }), bullets.end());

    int baseEnemies = 1;
    int extraEnemies = (score / 100) * (10 + rand() % 11);
    int totalEnemies = baseEnemies + extraEnemies;

    if (--enemySpawnTimer <= 0) {
        for (int i = 0; i < totalEnemies; i++) {
            int type = (rand() % 5) + 1;  // Chọn ngẫu nhiên từ 1-5
            enemies.push_back({rand() % (SCREEN_WIDTH - 50), 0, 50, 50, true, type});
        }
        enemySpawnTimer = max(30, 100 - (score / 100) * 10);
    }

    for (auto& enemy : enemies) {
        enemy.y += ENEMY_SPEED;
        if (rand() % 100 == 0 && enemy.active) {
            enemyBullets.push_back({enemy.x + enemy.w / 2 - 5, enemy.y + enemy.h, 10, 20, true, 0});
        }
    }
    enemies.erase(remove_if(enemies.begin(), enemies.end(), [](const Object& e) { return e.y > SCREEN_HEIGHT; }), enemies.end());

    for (auto& bomb : enemyBullets) bomb.y += BULLET_SPEED;
    enemyBullets.erase(remove_if(enemyBullets.begin(), enemyBullets.end(), [](const Object& b) { return b.y > SCREEN_HEIGHT; }), enemyBullets.end());

    for (auto& enemy : enemies) {
        for (auto& bullet : bullets) {
            if (bullet.active && enemy.active &&
                bullet.x < enemy.x + enemy.w && bullet.x + bullet.w > enemy.x &&
                bullet.y < enemy.y + enemy.h && bullet.y + bullet.h > enemy.y) {
                bullet.active = false;
                enemy.active = false;
                score += 10;
                updateScoreTexture();
                explosions.push_back({enemy.x, enemy.y, 50, 50, 20, true});
            }
        }
    }

    for (auto& enemy : enemies) {
        if (player1.active && enemy.active &&
            player1.x < enemy.x + enemy.w && player1.x + player1.w > enemy.x &&
            player1.y < enemy.y + enemy.h && player1.y + player1.h > enemy.y) {
            player1.active = false;
            explosions.push_back({player1.x, player1.y, 50, 50, 20, true});
            break;
        }
    }

    if (isTwoPlayers) {
        for (auto& enemy : enemies) {
            if (player2.active && enemy.active &&
                player2.x < enemy.x + enemy.w && player2.x + player2.w > enemy.x &&
                player2.y < enemy.y + enemy.h && player2.y + player2.h > enemy.y) {
                player2.active = false;
                explosions.push_back({player2.x, player2.y, 50, 50, 20, true});
                break;
            }
        }
    }

    for (auto& bomb : enemyBullets) {
        if (player1.active && bomb.active &&
            player1.x < bomb.x + bomb.w && player1.x + player1.w > bomb.x &&
            player1.y < bomb.y + bomb.h && player1.y + bomb.h > bomb.y) {
            player1.active = false;
            bomb.active = false;
            explosions.push_back({player1.x, player1.y, 50, 50, 20, true});
            break;
        }
    }

    if (isTwoPlayers) {
        for (auto& bomb : enemyBullets) {
            if (player2.active && bomb.active &&
                player2.x < bomb.x + bomb.w && player2.x + player2.w > bomb.x &&
                player2.y < bomb.y + bomb.h && player2.y + bomb.h > bomb.y) {
                player2.active = false;
                bomb.active = false;
                explosions.push_back({player2.x, player2.y, 50, 50, 20, true});
                break;
            }
        }
    }

    for (auto& explosion : explosions) {
        if (explosion.active) {
            explosion.frame--;
            if (explosion.frame <= 0) {
                explosion.active = false;
            }
        }
    }
    explosions.erase(remove_if(explosions.begin(), explosions.end(), 
        [](const Explosion& e) { return !e.active; }), explosions.end());

    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Object& b) { return !b.active; }), bullets.end());
    enemies.erase(remove_if(enemies.begin(), enemies.end(), [](const Object& e) { return !e.active; }), enemies.end());
    enemyBullets.erase(remove_if(enemyBullets.begin(), enemyBullets.end(), [](const Object& b) { return !b.active; }), enemyBullets.end());
}

void render() {
    SDL_RenderClear(renderer);
    if (background) SDL_RenderCopy(renderer, background, nullptr, nullptr);

    if (player1.active && player1Texture) {
        SDL_Rect pRect = {player1.x, player1.y, player1.w, player1.h};
        SDL_RenderCopy(renderer, player1Texture, NULL, &pRect);
    }
    if (isTwoPlayers && player2.active && player2Texture) {
        SDL_Rect pRect = {player2.x, player2.y, player2.w, player2.h};
        SDL_RenderCopy(renderer, player2Texture, NULL, &pRect);
    }

    for (const auto& bullet : bullets) {
        if (bulletTexture) {
            SDL_Rect bRect = {bullet.x, bullet.y, bullet.w, bullet.h};
            SDL_RenderCopy(renderer, bulletTexture, NULL, &bRect);
        }
    }

    for (const auto& enemy : enemies) {
        SDL_Rect eRect = {enemy.x, enemy.y, enemy.w, enemy.h};
        switch(enemy.enemyType) {
            case 1:
                if (enemyTexture)
                    SDL_RenderCopy(renderer, enemyTexture, NULL, &eRect);
                break;
            case 2:
                if (enemy2Texture)
                    SDL_RenderCopy(renderer, enemy2Texture, NULL, &eRect);
                break;
            case 3:
                if (enemy3Texture)
                    SDL_RenderCopy(renderer, enemy3Texture, NULL, &eRect);
                break;
            case 4:
                if (enemy4Texture)
                    SDL_RenderCopy(renderer, enemy4Texture, NULL, &eRect);
                break;
            case 5:
                if (enemy5Texture)
                    SDL_RenderCopy(renderer, enemy5Texture, NULL, &eRect);
                break;
        }
    }

    for (const auto& bomb : enemyBullets) {
        if (bomTexture) {
            SDL_Rect bombRect = {bomb.x, bomb.y, bomb.w, bomb.h};
            SDL_RenderCopy(renderer, bomTexture, NULL, &bombRect);
        }
    }

    for (const auto& explosion : explosions) {
        if (explosion.active && explosionTexture) {
            SDL_Rect eRect = {explosion.x, explosion.y, explosion.w, explosion.h};
            SDL_RenderCopy(renderer, explosionTexture, NULL, &eRect);
        }
    }

    if (scoreTexture) {
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    }

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
    if (bomTexture) SDL_DestroyTexture(bomTexture);
    if (scoreTexture) SDL_DestroyTexture(scoreTexture);
    if (explosionTexture) SDL_DestroyTexture(explosionTexture);
    if (gameOverTexture) SDL_DestroyTexture(gameOverTexture);
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
    srand(time(0));
    if (!init()) {
        printf("Initialization failed!\n");
        clean();
        return -1;
    }
    bool running = true;

    while (running) {
        handleEvents(running);
        if (gameState == MENU) {
            renderMenu();
        } else if (gameState == PLAYING) {
            updateGame();
            render();
        } else if (gameState == GAME_OVER) {
            renderGameOver();
        }
        SDL_Delay(16);
    }

    clean();
    return 0;
}