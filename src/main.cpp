#include <SDL2/SDL.h> // SDL2 cơ bản để xử lý đồ họa, sự kiện và cửa sổ
#include <SDL2/SDL_image.h> // để tải và xử lý hình ảnh 
#include <SDL2/SDL_ttf.h> // thêm SDL_ttf để thêm văn bản bằng font
#include <SDL2/SDL_mixer.h> // Thêm SDL_mixer để xử lý âm thanh 
#include <vector> // để lưu trữ danh sách(đạn, kẻ địch ...)
#include <cstdlib> // thư viện c++ cho rand() và srand()
#include <ctime> // thư viện để lấy thời  gian hiện tại 
#include <algorithm> // thư viện để sử dụng remove_if
#include <cstdio> // thư viện để dùng printf

using namespace std;

const int SCREEN_WIDTH = 800; // chiều rộng màn hình là 800 pixel
const int SCREEN_HEIGHT = 600; // chiều cao màn hình là 600 pixel
const int PLAYER_SPEED = 5; //tốc độ di chuyển của người chơi là 5 pixel / giây 
const int BULLET_SPEED = 7; // tốc độ bay của viên đạn là 7 pixel trên giây
const int ENEMY_SPEED = 2; //tốc độ di chuyển của địch là 2 pixel / giây
const int BOSS_SPEED = 3; // tốc độ di chuyển của boss là 2 pixel/ giây

enum GameState { MENU, TUTORIAL, PLAYING, GAME_OVER, WIN, EXIT }; // định nghĩa các trạng thái của game
GameState gameState = MENU; // trạng thái ban đầu là menu 

SDL_Window* window = nullptr; // con trỏ trỏ đến cửa sổ trò chơi, khơi tạo là null
SDL_Renderer* renderer = nullptr; // con trỏ trỏ trới renderer để vẽ cửa sổ, khởi tạo là null
SDL_Texture* menuBackground = nullptr; // texture cho nền menu, khởi tạo là null 
SDL_Texture* background = nullptr; // texture cho nền trò chơi khi chơi
SDL_Texture* player1Texture = nullptr; // texture cho người chơi 1
SDL_Texture* player2Texture = nullptr;// texture cho người chơi 2
SDL_Texture* bulletTexture = nullptr;//texture của viên đạn
SDL_Texture* enemyTexture = nullptr;// texture kẻ địch
SDL_Texture* enemy2Texture = nullptr;// texture kẻ địch 2
SDL_Texture* enemy3Texture = nullptr;// texture kẻ địch 3
SDL_Texture* enemy4Texture = nullptr;// texture kẻ địch4
SDL_Texture* enemy5Texture = nullptr;// texture kẻ địch 5
SDL_Texture* bossTexture = nullptr; // texture boss
SDL_Texture* bomTexture = nullptr; // texture đạn của địch
SDL_Texture* scoreTexture = nullptr; // texture hiển thị điểm số
SDL_Texture* explosionTexture = nullptr; // texture ảnh vụ nổ 
SDL_Texture* gameOverTexture = nullptr; // texture cho văn bản gameover
SDL_Texture* winTexture = nullptr; // texture cho văn bản you win 
SDL_Texture* playAgainTexture = nullptr; //texture cho văn bản playagain
SDL_Texture* exitTexture = nullptr; // texture cho văn bản exit 
SDL_Texture* onePlayerTexture = nullptr; // texture cho văn bản 1 player
SDL_Texture* twoPlayersTexture = nullptr; // texture cho văn bản 2 players
SDL_Texture* tutorialTexture = nullptr; // texture cho hướng dẫn
SDL_Texture* menuExitTexture = nullptr; // texture cho văn bản exit trong menu 
SDL_Texture* tutorialTextTexture1 = nullptr; //
SDL_Texture* tutorialTextTexture2 = nullptr;
SDL_Texture* backToMenuTexture = nullptr;
TTF_Font* font = nullptr;
TTF_Font* bigFont = nullptr;
SDL_Color white = {255, 255, 255, 255};
SDL_Color red = {255, 0, 0, 255};
SDL_Color yellow = {255, 255, 0, 255};
SDL_Rect scoreRect;
SDL_Rect gameOverRect;
SDL_Rect winRect;
SDL_Rect playAgainRect;
SDL_Rect exitRect;
SDL_Rect onePlayerRect;
SDL_Rect twoPlayersRect;
SDL_Rect tutorialRect;
SDL_Rect menuExitRect;
SDL_Rect tutorialTextRect1;
SDL_Rect tutorialTextRect2;
SDL_Rect backToMenuRect;

// Âm thanh
Mix_Music* backgroundMusic = nullptr; // Nhạc nền
Mix_Chunk* shootSound = nullptr;     // Âm thanh bắn
Mix_Chunk* explosionSound = nullptr; // Âm thanh nổ
Mix_Chunk* clickSound = nullptr;     // Âm thanh nhấp chuột
Mix_Chunk* winSound = nullptr;       // Âm thanh khi thắng
Mix_Chunk* gameOverSound = nullptr;  // Âm thanh khi thua

// Trạng thái để kiểm soát việc phát âm thanh WIN và GAME_OVER (chỉ phát một lần)
bool winSoundPlayed = false;
bool gameOverSoundPlayed = false;

// Trạng thái hover
bool onePlayerHover = false;
bool twoPlayersHover = false;
bool tutorialHover = false;
bool exitHover = false;
bool backToMenuHover = false;

struct Explosion {
    int x, y, w, h;
    int frame;
    bool active;
};

struct Object {
    int x, y, w, h;
    bool active;
    int enemyType;
    int health;
    int direction;
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
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { // Thêm SDL_INIT_AUDIO
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    // Khởi tạo SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer failed to initialize: %s\n", Mix_GetError());
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

    // Tải các texture
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

    // Tải font
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

    // Tải âm thanh
    // Tải âm thanh
backgroundMusic = Mix_LoadMUS("background_music.mp3");
if (!backgroundMusic) {
    printf("Failed to load background music: %s\n", Mix_GetError());
    return false;
}

shootSound = Mix_LoadWAV("shoot.wav");
if (!shootSound) {
    printf("Failed to load shoot sound: %s\n", Mix_GetError());
    return false;
}

explosionSound = Mix_LoadWAV("explosion.wav");
if (!explosionSound) {
    printf("Failed to load explosion sound: %s\n", Mix_GetError());
    return false;
}

clickSound = Mix_LoadWAV("click.wav");
if (!clickSound) {
    printf("Failed to load click sound: %s\n", Mix_GetError());
    return false;
}

winSound = Mix_LoadWAV("win_sound.wav");
if (!winSound) {
    printf("Failed to load win sound: %s\n", Mix_GetError());
    return false;
}

gameOverSound = Mix_LoadWAV("gameover_sound.wav");
if (!gameOverSound) {
    printf("Failed to load game over sound: %s\n", Mix_GetError());
    return false;
}

    clickSound = Mix_LoadWAV("click.wav");
    if (!clickSound) {
        printf("Failed to load click sound: %s\n", Mix_GetError());
        return false;
    }

    // Phát nhạc nền
    Mix_PlayMusic(backgroundMusic, -1); // -1 để lặp vô hạn

    // Tạo các texture văn bản
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

    // Đặt các mục menu vào 4 góc với khoảng cách lề 20 pixel
    int padding = 20;

    // Góc trên-trái: 1 Player
    onePlayerTexture = renderText("1 Player", font, red);
    if (!onePlayerTexture) return false;
    SDL_QueryTexture(onePlayerTexture, NULL, NULL, &onePlayerRect.w, &onePlayerRect.h);
    onePlayerRect = {padding, padding, onePlayerRect.w, onePlayerRect.h};

    // Góc trên-phải: 2 Players
    twoPlayersTexture = renderText("2 Players", font, red);
    if (!twoPlayersTexture) return false;
    SDL_QueryTexture(twoPlayersTexture, NULL, NULL, &twoPlayersRect.w, &twoPlayersRect.h);
    twoPlayersRect = {SCREEN_WIDTH - twoPlayersRect.w - padding, padding, twoPlayersRect.w, twoPlayersRect.h};

    // Góc dưới-trái: Tutorial
    tutorialTexture = renderText("Tutorial", font, red);
    if (!tutorialTexture) return false;
    SDL_QueryTexture(tutorialTexture, NULL, NULL, &tutorialRect.w, &tutorialRect.h);
    tutorialRect = {padding, SCREEN_HEIGHT - tutorialRect.h - padding, tutorialRect.w, tutorialRect.h};

    // Góc dưới-phải: Exit
    menuExitTexture = renderText("  Exit", font, red);
    if (!menuExitTexture) return false;
    SDL_QueryTexture(menuExitTexture, NULL, NULL, &menuExitRect.w, &menuExitRect.h);
    menuExitRect = {SCREEN_WIDTH - menuExitRect.w - padding, SCREEN_HEIGHT - menuExitRect.h - padding, menuExitRect.w, menuExitRect.h};

    // Nội dung tutorial
    tutorialTextTexture1 = renderText("Tutorial: Use A/D to move Player 1, Space to shoot.", font, white);
    if (!tutorialTextTexture1) return false;
    SDL_QueryTexture(tutorialTextTexture1, NULL, NULL, &tutorialTextRect1.w, &tutorialTextRect1.h);
    tutorialTextRect1 = {(SCREEN_WIDTH - tutorialTextRect1.w) / 2, SCREEN_HEIGHT / 3, tutorialTextRect1.w, tutorialTextRect1.h};

    tutorialTextTexture2 = renderText("For Player 2: Left/Right to move, Enter to shoot.", font, white);
    if (!tutorialTextTexture2) return false;
    SDL_QueryTexture(tutorialTextTexture2, NULL, NULL, &tutorialTextRect2.w, &tutorialTextRect2.h);
    tutorialTextRect2 = {(SCREEN_WIDTH - tutorialTextRect2.w) / 2, tutorialTextRect1.y + 50, tutorialTextRect2.w, tutorialTextRect2.h};

    backToMenuTexture = renderText("Back to Menu", font, white);
    if (!backToMenuTexture) return false;
    SDL_QueryTexture(backToMenuTexture, NULL, NULL, &backToMenuRect.w, &backToMenuRect.h);
    backToMenuRect = {(SCREEN_WIDTH - backToMenuRect.w) / 2, tutorialTextRect2.y + 100, backToMenuRect.w, backToMenuRect.h};

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

    // Hiển thị các mục menu với hiệu ứng hover
    SDL_DestroyTexture(onePlayerTexture);
    onePlayerTexture = renderText("1 Player", font, onePlayerHover ? yellow : red);
    SDL_RenderCopy(renderer, onePlayerTexture, nullptr, &onePlayerRect);

    SDL_DestroyTexture(twoPlayersTexture);
    twoPlayersTexture = renderText("2 Players", font, twoPlayersHover ? yellow : red);
    SDL_RenderCopy(renderer, twoPlayersTexture, nullptr, &twoPlayersRect);

    SDL_DestroyTexture(tutorialTexture);
    tutorialTexture = renderText("Tutorial", font, tutorialHover ? yellow : red);
    SDL_RenderCopy(renderer, tutorialTexture, nullptr, &tutorialRect);

    SDL_DestroyTexture(menuExitTexture);
    menuExitTexture = renderText("  Exit", font, exitHover ? yellow : red);
    SDL_RenderCopy(renderer, menuExitTexture, nullptr, &menuExitRect);

    SDL_RenderPresent(renderer);
}

void renderTutorial() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, nullptr, nullptr);
    SDL_RenderCopy(renderer, tutorialTextTexture1, nullptr, &tutorialTextRect1);
    SDL_RenderCopy(renderer, tutorialTextTexture2, nullptr, &tutorialTextRect2);

    SDL_DestroyTexture(backToMenuTexture);
    backToMenuTexture = renderText("Back to Menu", font, backToMenuHover ? yellow : white);
    SDL_RenderCopy(renderer, backToMenuTexture, nullptr, &backToMenuRect);

    SDL_RenderPresent(renderer);
}

void renderGameOver() {
    // Phát âm thanh game over chỉ một lần khi vào trạng thái này
    if (!gameOverSoundPlayed) {
        Mix_PlayChannel(-1, gameOverSound, 0);
        gameOverSoundPlayed = true;
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background, nullptr, nullptr);
    SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
    SDL_RenderCopy(renderer, playAgainTexture, nullptr, &playAgainRect);
    SDL_RenderCopy(renderer, exitTexture, nullptr, &exitRect);
    SDL_RenderPresent(renderer);
}

void renderWin() {
    // Phát âm thanh win chỉ một lần khi vào trạng thái này
    if (!winSoundPlayed) {
        Mix_PlayChannel(-1, winSound, 0);
        winSoundPlayed = true;
    }

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

        if (e.type == SDL_MOUSEMOTION) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (gameState == MENU) {
                onePlayerHover = (mouseX >= onePlayerRect.x && mouseX <= onePlayerRect.x + onePlayerRect.w &&
                                  mouseY >= onePlayerRect.y && mouseY <= onePlayerRect.y + onePlayerRect.h);
                twoPlayersHover = (mouseX >= twoPlayersRect.x && mouseX <= twoPlayersRect.x + twoPlayersRect.w &&
                                   mouseY >= twoPlayersRect.y && mouseY <= twoPlayersRect.y + twoPlayersRect.h);
                tutorialHover = (mouseX >= tutorialRect.x && mouseX <= tutorialRect.x + tutorialRect.w &&
                                 mouseY >= tutorialRect.y && mouseY <= tutorialRect.y + tutorialRect.h);
                exitHover = (mouseX >= menuExitRect.x && mouseX <= menuExitRect.x + menuExitRect.w &&
                             mouseY >= menuExitRect.y && mouseY <= menuExitRect.y + menuExitRect.h);
            }
            else if (gameState == TUTORIAL) {
                backToMenuHover = (mouseX >= backToMenuRect.x && mouseX <= backToMenuRect.x + backToMenuRect.w &&
                                   mouseY >= backToMenuRect.y && mouseY <= backToMenuRect.y + backToMenuRect.h);
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            // Khai báo và gán giá trị cho mouseX, mouseY ngay đầu khối
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (gameState == MENU) {
                if (mouseX >= onePlayerRect.x && mouseX <= onePlayerRect.x + onePlayerRect.w &&
                    mouseY >= onePlayerRect.y && mouseY <= onePlayerRect.y + onePlayerRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0); // Phát âm thanh nhấp chuột
                    isTwoPlayers = false;
                    player1 = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                    player2.active = false;
                    gameState = PLAYING;
                    // Bỏ Mix_PauseMusic() để nhạc nền tiếp tục phát
                }
                else if (mouseX >= twoPlayersRect.x && mouseX <= twoPlayersRect.x + twoPlayersRect.w &&
                         mouseY >= twoPlayersRect.y && mouseY <= twoPlayersRect.y + twoPlayersRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0); // Phát âm thanh nhấp chuột
                    isTwoPlayers = true;
                    player1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                    player2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0, 1, 0};
                    gameState = PLAYING;
                    // Bỏ Mix_PauseMusic() để nhạc nền tiếp tục phát
                }
                else if (mouseX >= tutorialRect.x && mouseX <= tutorialRect.x + tutorialRect.w &&
                         mouseY >= tutorialRect.y && mouseY <= tutorialRect.y + tutorialRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0); // Phát âm thanh nhấp chuột
                    gameState = TUTORIAL;
                }
                else if (mouseX >= menuExitRect.x && mouseX <= menuExitRect.x + menuExitRect.w &&
                         mouseY >= menuExitRect.y && mouseY <= menuExitRect.y + menuExitRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0); // Phát âm thanh nhấp chuột
                    gameState = EXIT;
                    running = false;
                }
            }
            else if (gameState == TUTORIAL) {
                if (mouseX >= backToMenuRect.x && mouseX <= backToMenuRect.x + backToMenuRect.w &&
                    mouseY >= backToMenuRect.y && mouseY <= backToMenuRect.y + backToMenuRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0); // Phát âm thanh nhấp chuột
                    gameState = MENU;
                }
            }
            else if (gameState == GAME_OVER || gameState == WIN) {
                if (mouseX >= playAgainRect.x && mouseX <= playAgainRect.x + playAgainRect.w &&
                    mouseY >= playAgainRect.y && mouseY <= playAgainRect.y + playAgainRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0); // Phát âm thanh nhấp chuột
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
                    winSoundPlayed = false; // Reset trạng thái âm thanh WIN
                    gameOverSoundPlayed = false; // Reset trạng thái âm thanh GAME_OVER
                    gameState = PLAYING;
                    Mix_ResumeMusic(); // Bật lại nhạc nền khi chơi lại
                }
                else if (mouseX >= exitRect.x && mouseX <= exitRect.x + exitRect.w &&
                         mouseY >= exitRect.y && mouseY <= exitRect.y + exitRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0); // Phát âm thanh nhấp chuột
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
        Mix_PauseMusic(); // Tạm dừng nhạc nền khi game over
        return;
    }

    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_A] && player1.x > 0)
        player1.x -= PLAYER_SPEED;
    if (keystates[SDL_SCANCODE_D] && player1.x < SCREEN_WIDTH - player1.w)
        player1.x += PLAYER_SPEED;
    if (keystates[SDL_SCANCODE_SPACE] && bulletCooldown1 == 0 && player1.active) {
        bullets.push_back({player1.x + player1.w / 2 - 5, player1.y, 10, 20, true, 0, 1, 0});
        Mix_PlayChannel(-1, shootSound, 0); // Phát âm thanh bắn
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
            Mix_PlayChannel(-1, shootSound, 0); // Phát âm thanh bắn
            bulletCooldown2 = 10;
        }
        if (bulletCooldown2 > 0) bulletCooldown2--;
    }

    for (auto& bullet : bullets) bullet.y -= BULLET_SPEED;
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Object& b) { return b.y < -b.h; }), bullets.end());

    if (!bossSpawned) {
        if (score >= 500 && enemies.empty()) {
            enemies.push_back({SCREEN_WIDTH / 2 - 50, 50, 100, 100, true, 6, 50, 1});
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
        if (enemy.enemyType == 6) {
            enemy.x += BOSS_SPEED * enemy.direction;
            if (enemy.x <= 0) enemy.direction = 1;
            if (enemy.x >= SCREEN_WIDTH - enemy.w) enemy.direction = -1;

            int shootChance = 10;
            if (rand() % shootChance == 0 && enemy.active) {
                enemyBullets.push_back({enemy.x + enemy.w / 4 - 5, enemy.y + enemy.h, 10, 20, true, 0, 1, 0});
                enemyBullets.push_back({enemy.x + enemy.w / 2 - 5, enemy.y + enemy.h, 10, 20, true, 0, 1, 0});
                enemyBullets.push_back({enemy.x + 3 * enemy.w / 4 - 5, enemy.y + enemy.h, 10, 20, true, 0, 1, 0});
            }
        } else {
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
                    Mix_PlayChannel(-1, explosionSound, 0); // Phát âm thanh nổ
                    if (enemies[i].enemyType == 6) {
                        gameState = WIN;
                        Mix_PauseMusic(); // Tạm dừng nhạc nền khi thắng
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
            Mix_PlayChannel(-1, explosionSound, 0); // Phát âm thanh nổ
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
                Mix_PlayChannel(-1, explosionSound, 0); // Phát âm thanh nổ
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
            Mix_PlayChannel(-1, explosionSound, 0); // Phát âm thanh nổ
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
                Mix_PlayChannel(-1, explosionSound, 0); // Phát âm thanh nổ
                break;
            }
        }
    }

    for (auto& explosion : explosions) {
        if (explosion.active) {
            explosion.frame--;
            if (explosion.frame <= 0) explosion.active = false;
        }
    }
    explosions.erase(remove_if(explosions.begin(), explosions.end(), [](const Explosion& e) { return !e.active; }), explosions.end());
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
    // Dọn dẹp tài nguyên âm thanh
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);
    if (shootSound) Mix_FreeChunk(shootSound);
    if (explosionSound) Mix_FreeChunk(explosionSound);
    if (clickSound) Mix_FreeChunk(clickSound);
    if (winSound) Mix_FreeChunk(winSound);
    if (gameOverSound) Mix_FreeChunk(gameOverSound);
    Mix_CloseAudio();

    // Dọn dẹp font
    if (font) TTF_CloseFont(font);
    if (bigFont) TTF_CloseFont(bigFont);
    TTF_Quit();

    // Dọn dẹp texture
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
    if (tutorialTexture) SDL_DestroyTexture(tutorialTexture);
    if (menuExitTexture) SDL_DestroyTexture(menuExitTexture);
    if (tutorialTextTexture1) SDL_DestroyTexture(tutorialTextTexture1);
    if (tutorialTextTexture2) SDL_DestroyTexture(tutorialTextTexture2);
    if (backToMenuTexture) SDL_DestroyTexture(backToMenuTexture);

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
            case TUTORIAL:
                renderTutorial();
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