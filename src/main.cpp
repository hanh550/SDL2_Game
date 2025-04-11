#include <SDL2/SDL.h> // Thư viện SDL2 để xử lý đồ họa, sự kiện và cửa sổ
#include <SDL2/SDL_image.h> // Thư viện SDL2_image để tải và xử lý hình ảnh
#include <SDL2/SDL_ttf.h> // Thư viện SDL2_ttf để hiển thị văn bản với font
#include <SDL2/SDL_mixer.h> // Thư viện SDL2_mixer để xử lý âm thanh
#include <vector> // Thư viện vector để lưu trữ danh sách các đối tượng (đạn, kẻ địch, ...)
#include <cstdlib> // Thư viện C++ cho các hàm rand() và srand()
#include <ctime> // Thư viện để lấy thời gian hiện tại
#include <algorithm> // Thư viện để sử dụng các hàm như remove_if
#include <cstdio> // Thư viện để sử dụng printf

using namespace std;

// Các hằng số cấu hình trò chơi
const int SCREEN_WIDTH = 800; // Chiều rộng màn hình
const int SCREEN_HEIGHT = 600; // Chiều cao màn hình
const int PLAYER_SPEED = 5; // Tốc độ di chuyển của người chơi
const int BULLET_SPEED = 7; // Tốc độ bay của viên đạn
const int ENEMY_SPEED = 2; // Tốc độ di chuyển của kẻ địch
const int BOSS_SPEED = 3; // Tốc độ di chuyển của boss

enum TrangThaiGame { MENU, HUONG_DAN, DANG_CHOI, GAME_OVER, THANG, THOAT }; 
TrangThaiGame trangThaiGame = MENU; 

SDL_Window* cuaSo = nullptr; 
SDL_Renderer* boVe = nullptr; 
SDL_Texture* nenMenu = nullptr; 
SDL_Texture* nenTroChoi = nullptr; 
SDL_Texture* nguoiChoi1Texture = nullptr; 
SDL_Texture* nguoiChoi2Texture = nullptr; 
SDL_Texture* danTexture = nullptr; 
SDL_Texture* keDichTexture = nullptr; 
SDL_Texture* keDich2Texture = nullptr; 
SDL_Texture* keDich3Texture = nullptr; 
SDL_Texture* keDich4Texture = nullptr; 
SDL_Texture* keDich5Texture = nullptr; 
SDL_Texture* bossTexture = nullptr; 
SDL_Texture* bomTexture = nullptr; 
SDL_Texture* diemSoTexture = nullptr; 
SDL_Texture* noTexture = nullptr; 
SDL_Texture* gameOverTexture = nullptr; 
SDL_Texture* thangTexture = nullptr; 
SDL_Texture* choiLaiTexture = nullptr; 
SDL_Texture* thoatTexture = nullptr; 
SDL_Texture* motNguoiChoiTexture = nullptr; 
SDL_Texture* haiNguoiChoiTexture = nullptr; 
SDL_Texture* huongDanTexture = nullptr; 
SDL_Texture* thoatMenuTexture = nullptr; 
SDL_Texture* huongDanTextTexture1 = nullptr; 
SDL_Texture* huongDanTextTexture2 = nullptr; 
SDL_Texture* quayLaiMenuTexture = nullptr; 
SDL_Texture* backToMenuTexture = nullptr; 
SDL_Rect backToMenuRect; 
bool backToMenuGameOverHover = false;
TTF_Font* phongChu = nullptr; // Added declaration for smaller font
TTF_Font* phongChuLon = nullptr; 
SDL_Color trang = {255, 255, 255, 255}; 
SDL_Color mauDo = {255, 0, 0, 255}; 
SDL_Color vang = {255, 255, 0, 255}; 
SDL_Rect diemSoRect; 
SDL_Rect gameOverRect; 
SDL_Rect thangRect; 
SDL_Rect choiLaiRect; 
SDL_Rect thoatRect; 
SDL_Rect motNguoiChoiRect; 
SDL_Rect haiNguoiChoiRect; 
SDL_Rect huongDanRect; 
SDL_Rect thoatMenuRect; 
SDL_Rect huongDanTextRect1; 
SDL_Rect huongDanTextRect2; 
SDL_Rect quayLaiMenuRect; 

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

// Trạng thái âm thanh (bật mặc định)
bool soundEnabled = true; 
SDL_Texture* soundTexture = nullptr; // Texture cho nút âm thanh
SDL_Rect soundRect; // Vị trí và kích thước của nút âm thanh

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

Object nguoiChoi1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
Object nguoiChoi2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0, 1, 0};
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

// Hàm loadTexture: Tải hình ảnh từ file và chuyển đổi thành texture SDL
SDL_Texture* loadTexture(const char* filename) {
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) {
        printf("Error loading image %s: %s\n", filename, IMG_GetError());
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(boVe, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("Error creating texture from %s: %s\n", filename, SDL_GetError());
    }
    return texture;
}

// Hàm renderText: Tạo texture từ văn bản với font và màu sắc
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
    SDL_Texture* texture = SDL_CreateTextureFromSurface(boVe, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("Error creating texture from text %s: %s\n", text, SDL_GetError());
    }
    return texture;
}

// Hàm updateScoreTexture: Cập nhật texture hiển thị điểm số
void updateScoreTexture() {
    char scoreText[32];
    sprintf(scoreText, "SCORE: %d", score); // Thay "Diem" thành "Score"
    if (diemSoTexture) SDL_DestroyTexture(diemSoTexture);
    diemSoTexture = renderText(scoreText, phongChu, trang);
    if (diemSoTexture) {
        diemSoRect = {SCREEN_WIDTH - 200, 10, 0, 0};
        SDL_QueryTexture(diemSoTexture, NULL, NULL, &diemSoRect.w, &diemSoRect.h);
    }
}
bool initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

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

    cuaSo = SDL_CreateWindow("Tro Choi Ban Sung", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!cuaSo) {
        printf("Tao cua so that bai: %s\n", SDL_GetError());
        return false;
    }

    boVe = SDL_CreateRenderer(cuaSo, -1, SDL_RENDERER_ACCELERATED);
    if (!boVe) {
        printf("Tao bo ve that bai: %s\n", SDL_GetError());
        return false;
    }

    nenMenu = loadTexture("chickenbackground.jpg");
    if (!nenMenu) return false;
    nenTroChoi = loadTexture("space.jpg");
    if (!nenTroChoi) return false;
    nguoiChoi1Texture = loadTexture("tank.png");
    if (!nguoiChoi1Texture) return false;
    nguoiChoi2Texture = loadTexture("player2.png");
    if (!nguoiChoi2Texture) return false;
    danTexture = loadTexture("bullet.png");
    if (!danTexture) return false;
    keDichTexture = loadTexture("enemy.png");
    if (!keDichTexture) return false;
    keDich2Texture = loadTexture("enemy2.png");
    if (!keDich2Texture) return false;
    keDich3Texture = loadTexture("enemy3.png");
    if (!keDich3Texture) return false;
    keDich4Texture = loadTexture("enemy4.png");
    if (!keDich4Texture) return false;
    keDich5Texture = loadTexture("enemy5.png");
    if (!keDich5Texture) return false;
    bossTexture = loadTexture("boss.png");
    if (!bossTexture) return false;
    bomTexture = loadTexture("egg.png");
    if (!bomTexture) return false;
    noTexture = loadTexture("explosion.png");
    if (!noTexture) return false;

    phongChu = TTF_OpenFont("arial.ttf", 36);
    if (!phongChu) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return false;
    }
    phongChuLon = TTF_OpenFont("arial.ttf", 72);
    if (!phongChuLon) {
        printf("Failed to load big font: %s\n", TTF_GetError());
        return false;
    }

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

    Mix_PlayMusic(backgroundMusic, -1);

    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);
    diemSoTexture = renderText(scoreText, phongChu, trang);
    if (!diemSoTexture) return false;
    diemSoRect = {SCREEN_WIDTH - 200, 10, 0, 0};
    SDL_QueryTexture(diemSoTexture, NULL, NULL, &diemSoRect.w, &diemSoRect.h);

    gameOverTexture = renderText("Game Over", phongChuLon, trang);
    if (!gameOverTexture) return false;
    SDL_QueryTexture(gameOverTexture, NULL, NULL, &gameOverRect.w, &gameOverRect.h);
    gameOverRect = {(SCREEN_WIDTH - gameOverRect.w) / 2, SCREEN_HEIGHT / 4, gameOverRect.w, gameOverRect.h};

    thangTexture = renderText("You Win!", phongChuLon, trang);
    if (!thangTexture) return false;
    SDL_QueryTexture(thangTexture, NULL, NULL, &thangRect.w, &thangRect.h);
    thangRect = {(SCREEN_WIDTH - thangRect.w) / 2, SCREEN_HEIGHT / 4, thangRect.w, thangRect.h};

    choiLaiTexture = renderText("Play Again?", phongChu, trang);
    if (!choiLaiTexture) return false;
    SDL_QueryTexture(choiLaiTexture, NULL, NULL, &choiLaiRect.w, &choiLaiRect.h);
    choiLaiRect = {(SCREEN_WIDTH - choiLaiRect.w) / 2, gameOverRect.y + gameOverRect.h + 100, choiLaiRect.w, choiLaiRect.h};

    thoatTexture = renderText("Exit", phongChu, trang);
    if (!thoatTexture) return false;
    SDL_QueryTexture(thoatTexture, NULL, NULL, &thoatRect.w, &thoatRect.h);
    thoatRect = {(SCREEN_WIDTH - thoatRect.w) / 2, choiLaiRect.y + choiLaiRect.h + 50, thoatRect.w, thoatRect.h};

    backToMenuTexture = renderText("Back to Menu", phongChu, trang);
    if (!backToMenuTexture) return false;
    SDL_QueryTexture(backToMenuTexture, NULL, NULL, &backToMenuRect.w, &backToMenuRect.h);
    backToMenuRect = {(SCREEN_WIDTH - backToMenuRect.w) / 2, thoatRect.y + thoatRect.h + 50, backToMenuRect.w, backToMenuRect.h};

    int padding = 20;

    motNguoiChoiTexture = renderText("1 Player", phongChu, mauDo);
    if (!motNguoiChoiTexture) return false;
    SDL_QueryTexture(motNguoiChoiTexture, NULL, NULL, &motNguoiChoiRect.w, &motNguoiChoiRect.h);
    motNguoiChoiRect = {padding, padding, motNguoiChoiRect.w, motNguoiChoiRect.h};

    haiNguoiChoiTexture = renderText("2 Players", phongChu, mauDo);
    if (!haiNguoiChoiTexture) return false;
    SDL_QueryTexture(haiNguoiChoiTexture, NULL, NULL, &haiNguoiChoiRect.w, &haiNguoiChoiRect.h);
    haiNguoiChoiRect = {SCREEN_WIDTH - haiNguoiChoiRect.w - padding, padding, haiNguoiChoiRect.w, haiNguoiChoiRect.h};

    huongDanTexture = renderText("Tutorial", phongChu, mauDo);
    if (!huongDanTexture) return false;
    SDL_QueryTexture(huongDanTexture, NULL, NULL, &huongDanRect.w, &huongDanRect.h);
    huongDanRect = {padding, SCREEN_HEIGHT - huongDanRect.h - padding, huongDanRect.w, huongDanRect.h};

    thoatMenuTexture = renderText("  Exit", phongChu, mauDo);
    if (!thoatMenuTexture) return false;
    SDL_QueryTexture(thoatMenuTexture, NULL, NULL, &thoatMenuRect.w, &thoatMenuRect.h);
    thoatMenuRect = {SCREEN_WIDTH - thoatMenuRect.w - padding, SCREEN_HEIGHT - thoatMenuRect.h - padding, thoatMenuRect.w, thoatMenuRect.h};

    huongDanTextTexture1 = renderText("Tutorial: Use A/D to move Player 1, Space to shoot.", phongChu, trang);
    if (!huongDanTextTexture1) return false;
    SDL_QueryTexture(huongDanTextTexture1, NULL, NULL, &huongDanTextRect1.w, &huongDanTextRect1.h);
    huongDanTextRect1 = {(SCREEN_WIDTH - huongDanTextRect1.w) / 2, SCREEN_HEIGHT / 3, huongDanTextRect1.w, huongDanTextRect1.h};

    huongDanTextTexture2 = renderText("For Player 2: Left/Right to move, Enter to shoot.", phongChu, trang);
    if (!huongDanTextTexture2) return false;
    SDL_QueryTexture(huongDanTextTexture2, NULL, NULL, &huongDanTextRect2.w, &huongDanTextRect2.h);
    huongDanTextRect2 = {(SCREEN_WIDTH - huongDanTextRect2.w) / 2, huongDanTextRect1.y + 50, huongDanTextRect2.w, huongDanTextRect2.h};

    quayLaiMenuTexture = renderText("Back to Menu", phongChu, trang);
    if (!quayLaiMenuTexture) return false;
    SDL_QueryTexture(quayLaiMenuTexture, NULL, NULL, &quayLaiMenuRect.w, &quayLaiMenuRect.h);
    quayLaiMenuRect = {(SCREEN_WIDTH - quayLaiMenuRect.w) / 2, huongDanTextRect2.y + 100, quayLaiMenuRect.w, quayLaiMenuRect.h};

    soundTexture = renderText("Sound: ON", phongChu, trang);
    if (!soundTexture) return false;
    SDL_QueryTexture(soundTexture, NULL, NULL, &soundRect.w, &soundRect.h);
    soundRect = {(SCREEN_WIDTH - soundRect.w) / 2, 20, soundRect.w, soundRect.h};

    return true;
}

void renderMenu() {
    SDL_RenderClear(boVe);
    SDL_RenderCopy(boVe, nenMenu, nullptr, nullptr);

    SDL_DestroyTexture(soundTexture);
    soundTexture = renderText(soundEnabled ? "Sound: ON" : "Sound: OFF", phongChu, trang);
    SDL_RenderCopy(boVe, soundTexture, nullptr, &soundRect);

    SDL_DestroyTexture(motNguoiChoiTexture);
    motNguoiChoiTexture = renderText("1 Player", phongChu, onePlayerHover ? vang : mauDo);
    SDL_RenderCopy(boVe, motNguoiChoiTexture, nullptr, &motNguoiChoiRect);

    SDL_DestroyTexture(haiNguoiChoiTexture);
    haiNguoiChoiTexture = renderText("2 Players", phongChu, twoPlayersHover ? vang : mauDo);
    SDL_RenderCopy(boVe, haiNguoiChoiTexture, nullptr, &haiNguoiChoiRect);

    SDL_DestroyTexture(huongDanTexture);
    huongDanTexture = renderText("Tutorial", phongChu, tutorialHover ? vang : mauDo);
    SDL_RenderCopy(boVe, huongDanTexture, nullptr, &huongDanRect);

    SDL_DestroyTexture(thoatMenuTexture);
    thoatMenuTexture = renderText("  Exit", phongChu, exitHover ? vang : mauDo);
    SDL_RenderCopy(boVe, thoatMenuTexture, nullptr, &thoatMenuRect);

    SDL_RenderPresent(boVe);
}

void renderTutorial() {
    SDL_RenderClear(boVe);
    SDL_RenderCopy(boVe, nenTroChoi, nullptr, nullptr);
    SDL_RenderCopy(boVe, huongDanTextTexture1, nullptr, &huongDanTextRect1);
    SDL_RenderCopy(boVe, huongDanTextTexture2, nullptr, &huongDanTextRect2);

    SDL_DestroyTexture(quayLaiMenuTexture);
    quayLaiMenuTexture = renderText("Back to Menu", phongChu, backToMenuHover ? vang : trang);
    SDL_RenderCopy(boVe, quayLaiMenuTexture, nullptr, &quayLaiMenuRect);

    SDL_RenderPresent(boVe);
}

void renderGameOver() {
    if (!gameOverSoundPlayed) {
        Mix_PlayChannel(-1, gameOverSound, 0);
        gameOverSoundPlayed = true;
    }

    SDL_RenderClear(boVe);
    SDL_RenderCopy(boVe, nenTroChoi, nullptr, nullptr);
    SDL_RenderCopy(boVe, gameOverTexture, nullptr, &gameOverRect);
    SDL_RenderCopy(boVe, choiLaiTexture, nullptr, &choiLaiRect);
    SDL_RenderCopy(boVe, thoatTexture, nullptr, &thoatRect);

    SDL_DestroyTexture(backToMenuTexture);
    backToMenuTexture = renderText("Back to Menu", phongChu, backToMenuGameOverHover ? vang : trang);
    SDL_RenderCopy(boVe, backToMenuTexture, nullptr, &backToMenuRect);

    SDL_RenderPresent(boVe);
}

void renderWin() {
    if (!winSoundPlayed) {
        Mix_PlayChannel(-1, winSound, 0);
        winSoundPlayed = true;
    }

    SDL_RenderClear(boVe);
    SDL_RenderCopy(boVe, nenTroChoi, nullptr, nullptr);
    SDL_RenderCopy(boVe, thangTexture, nullptr, &thangRect);
    SDL_RenderCopy(boVe, choiLaiTexture, nullptr, &choiLaiRect);
    SDL_RenderCopy(boVe, thoatTexture, nullptr, &thoatRect);
    SDL_DestroyTexture(backToMenuTexture);
    backToMenuTexture = renderText("Back to Menu", phongChu, backToMenuGameOverHover ? vang : trang);
    SDL_RenderCopy(boVe, backToMenuTexture, nullptr, &backToMenuRect);

    SDL_RenderPresent(boVe);
}

void handleEvents(bool& isRunning) {
    SDL_Event suKien;
    while (SDL_PollEvent(&suKien)) {
        if (suKien.type == SDL_QUIT) {
            isRunning = false;
            trangThaiGame = THOAT;
        }

        if (suKien.type == SDL_MOUSEMOTION) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (trangThaiGame == MENU) {
                onePlayerHover = (mouseX >= motNguoiChoiRect.x && mouseX <= motNguoiChoiRect.x + motNguoiChoiRect.w &&
                                  mouseY >= motNguoiChoiRect.y && mouseY <= motNguoiChoiRect.y + motNguoiChoiRect.h);
                twoPlayersHover = (mouseX >= haiNguoiChoiRect.x && mouseX <= haiNguoiChoiRect.x + haiNguoiChoiRect.w &&
                                   mouseY >= haiNguoiChoiRect.y && mouseY <= haiNguoiChoiRect.y + haiNguoiChoiRect.h);
                tutorialHover = (mouseX >= huongDanRect.x && mouseX <= huongDanRect.x + huongDanRect.w &&
                                 mouseY >= huongDanRect.y && mouseY <= huongDanRect.y + huongDanRect.h);
                exitHover = (mouseX >= thoatMenuRect.x && mouseX <= thoatMenuRect.x + thoatMenuRect.w &&
                             mouseY >= thoatMenuRect.y && mouseY <= thoatMenuRect.y + thoatMenuRect.h);
            }
            else if (trangThaiGame == HUONG_DAN) {
                backToMenuHover = (mouseX >= quayLaiMenuRect.x && mouseX <= quayLaiMenuRect.x + quayLaiMenuRect.w &&
                                   mouseY >= quayLaiMenuRect.y && mouseY <= quayLaiMenuRect.y + quayLaiMenuRect.h);
            }
            else if (trangThaiGame == GAME_OVER || trangThaiGame == THANG) {
                backToMenuGameOverHover = (mouseX >= backToMenuRect.x && mouseX <= backToMenuRect.x + backToMenuRect.w &&
                                           mouseY >= backToMenuRect.y && mouseY <= backToMenuRect.y + backToMenuRect.h);
            }
        }

        if (suKien.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (trangThaiGame == MENU) {
                if (mouseX >= soundRect.x && mouseX <= soundRect.x + soundRect.w &&
                    mouseY >= soundRect.y && mouseY <= soundRect.y + soundRect.h) {
                    soundEnabled = !soundEnabled;
                    if (soundEnabled) {
                        Mix_ResumeMusic();
                    } else {
                        Mix_PauseMusic();
                    }
                }
                if (mouseX >= motNguoiChoiRect.x && mouseX <= motNguoiChoiRect.x + motNguoiChoiRect.w &&
                    mouseY >= motNguoiChoiRect.y && mouseY <= motNguoiChoiRect.y + motNguoiChoiRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    isTwoPlayers = false;
                    nguoiChoi1 = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                    nguoiChoi2.active = false;
                    trangThaiGame = DANG_CHOI;
                }
                else if (mouseX >= haiNguoiChoiRect.x && mouseX <= haiNguoiChoiRect.x + haiNguoiChoiRect.w &&
                         mouseY >= haiNguoiChoiRect.y && mouseY <= haiNguoiChoiRect.y + haiNguoiChoiRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    isTwoPlayers = true;
                    nguoiChoi1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                    nguoiChoi2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0, 1, 0};
                    trangThaiGame = DANG_CHOI;
                }
                else if (mouseX >= huongDanRect.x && mouseX <= huongDanRect.x + huongDanRect.w &&
                         mouseY >= huongDanRect.y && mouseY <= huongDanRect.y + huongDanRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    trangThaiGame = HUONG_DAN;
                }
                else if (mouseX >= thoatMenuRect.x && mouseX <= thoatMenuRect.x + thoatMenuRect.w &&
                         mouseY >= thoatMenuRect.y && mouseY <= thoatMenuRect.y + thoatMenuRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    trangThaiGame = THOAT;
                    isRunning = false;
                }
            }
            else if (trangThaiGame == HUONG_DAN) {
                if (mouseX >= quayLaiMenuRect.x && mouseX <= quayLaiMenuRect.x + quayLaiMenuRect.w &&
                    mouseY >= quayLaiMenuRect.y && mouseY <= quayLaiMenuRect.y + quayLaiMenuRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    trangThaiGame = MENU;
                }
            }
            else if (trangThaiGame == GAME_OVER || trangThaiGame == THANG) {
                if (mouseX >= choiLaiRect.x && mouseX <= choiLaiRect.x + choiLaiRect.w &&
                    mouseY >= choiLaiRect.y && mouseY <= choiLaiRect.y + choiLaiRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    if (isTwoPlayers) {
                        nguoiChoi1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                        nguoiChoi2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0, 1, 0};
                    } else {
                        nguoiChoi1 = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                        nguoiChoi2.active = false;
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
                    winSoundPlayed = false;
                    gameOverSoundPlayed = false;
                    trangThaiGame = DANG_CHOI;
                    Mix_ResumeMusic();
                }
                else if (mouseX >= thoatRect.x && mouseX <= thoatRect.x + thoatRect.w &&
                         mouseY >= thoatRect.y && mouseY <= thoatRect.y + thoatRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    trangThaiGame = THOAT;
                    isRunning = false;
                }
                else if (mouseX >= backToMenuRect.x && mouseX <= backToMenuRect.x + backToMenuRect.w &&
                         mouseY >= backToMenuRect.y && mouseY <= backToMenuRect.y + backToMenuRect.h) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    if (isTwoPlayers) {
                        nguoiChoi1 = {SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                        nguoiChoi2 = {3 * SCREEN_WIDTH / 4 - 40, SCREEN_HEIGHT - 80, 50, 50, true, 0, 1, 0};
                    } else {
                        nguoiChoi1 = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50, true, 0, 1, 0};
                        nguoiChoi2.active = false;
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
                    winSoundPlayed = false;
                    gameOverSoundPlayed = false;
                    trangThaiGame = MENU;
                    Mix_ResumeMusic();
                }
            }
        }
    }
}

void updateGame() {
    if ((!nguoiChoi1.active && !isTwoPlayers) || (!nguoiChoi1.active && !nguoiChoi2.active && isTwoPlayers)) {
        trangThaiGame = GAME_OVER;
        Mix_PauseMusic();
        return;
    }

    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_A] && nguoiChoi1.x > 0) {
        nguoiChoi1.x -= PLAYER_SPEED;
    }
    if (keystates[SDL_SCANCODE_D] && nguoiChoi1.x < SCREEN_WIDTH - nguoiChoi1.w) {
        nguoiChoi1.x += PLAYER_SPEED;
    }

    if (keystates[SDL_SCANCODE_SPACE] && bulletCooldown1 == 0 && nguoiChoi1.active) {
        bullets.push_back({nguoiChoi1.x + nguoiChoi1.w / 2 - 5, nguoiChoi1.y, 10, 20, true, 0, 1, 0});
        Mix_PlayChannel(-1, shootSound, 0);
        bulletCooldown1 = 10;
    }
    if (bulletCooldown1 > 0) bulletCooldown1--;

    if (isTwoPlayers) {
        if (keystates[SDL_SCANCODE_LEFT] && nguoiChoi2.x > 0)
            nguoiChoi2.x -= PLAYER_SPEED;
        if (keystates[SDL_SCANCODE_RIGHT] && nguoiChoi2.x < SCREEN_WIDTH - nguoiChoi2.w)
            nguoiChoi2.x += PLAYER_SPEED;
        if (keystates[SDL_SCANCODE_RETURN] && bulletCooldown2 == 0 && nguoiChoi2.active) {
            bullets.push_back({nguoiChoi2.x + nguoiChoi2.w / 2 - 5, nguoiChoi2.y, 10, 20, true, 0, 1, 0});
            Mix_PlayChannel(-1, shootSound, 0);
            bulletCooldown2 = 10;
        }
        if (bulletCooldown2 > 0) bulletCooldown2--;
    }

    for (auto& bullet : bullets) bullet.y -= BULLET_SPEED;
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Object& b) { return b.y < -b.h; }), bullets.end());

    if (!bossSpawned) {
        if (score >= 500 && enemies.empty()) {
            enemies.push_back({SCREEN_WIDTH / 2 - 50, 50, 100, 100, true, 6, 30, 1}); 
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

            int shootChance = 20;
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
                    Mix_PlayChannel(-1, explosionSound, 0);
                    if (enemies[i].enemyType == 6) {
                        trangThaiGame = THANG;
                        Mix_PauseMusic();
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < enemies.size(); ++i) {
        if (nguoiChoi1.active && enemies[i].active &&
            nguoiChoi1.x < enemies[i].x + enemies[i].w && nguoiChoi1.x + nguoiChoi1.w > enemies[i].x &&
            nguoiChoi1.y < enemies[i].y + enemies[i].h && nguoiChoi1.y + nguoiChoi1.h > enemies[i].y) {
            nguoiChoi1.active = false;
            explosions.push_back({nguoiChoi1.x, nguoiChoi1.y, 50, 50, 20, true});
            Mix_PlayChannel(-1, explosionSound, 0);
            break;
        }
    }

    if (isTwoPlayers) {
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (nguoiChoi2.active && enemies[i].active &&
                nguoiChoi2.x < enemies[i].x + enemies[i].w && nguoiChoi2.x + nguoiChoi2.w > enemies[i].x &&
                nguoiChoi2.y < enemies[i].y + enemies[i].h && nguoiChoi2.y + nguoiChoi2.h > enemies[i].y) {
                nguoiChoi2.active = false;
                explosions.push_back({nguoiChoi2.x, nguoiChoi2.y, 50, 50, 20, true});
                Mix_PlayChannel(-1, explosionSound, 0);
                break;
            }
        }
    }

    for (size_t i = 0; i < enemyBullets.size(); ++i) {
        if (nguoiChoi1.active && enemyBullets[i].active &&
            nguoiChoi1.x < enemyBullets[i].x + enemyBullets[i].w && nguoiChoi1.x + nguoiChoi1.w > enemyBullets[i].x &&
            nguoiChoi1.y < enemyBullets[i].y + enemyBullets[i].h && nguoiChoi1.y + nguoiChoi1.h > enemyBullets[i].y) {
            nguoiChoi1.active = false;
            enemyBullets[i].active = false;
            explosions.push_back({nguoiChoi1.x, nguoiChoi1.y, 50, 50, 20, true});
            Mix_PlayChannel(-1, explosionSound, 0);
            break;
        }
    }

    if (isTwoPlayers) {
        for (size_t i = 0; i < enemyBullets.size(); ++i) {
            if (nguoiChoi2.active && enemyBullets[i].active &&
                nguoiChoi2.x < enemyBullets[i].x + enemyBullets[i].w && nguoiChoi2.x + nguoiChoi2.w > enemyBullets[i].x &&
                nguoiChoi2.y < enemyBullets[i].y + enemyBullets[i].h && nguoiChoi2.y + nguoiChoi2.h > enemyBullets[i].y) {
                nguoiChoi2.active = false;
                enemyBullets[i].active = false;
                explosions.push_back({nguoiChoi2.x, nguoiChoi2.y, 50, 50, 20, true});
                Mix_PlayChannel(-1, explosionSound, 0);
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
    SDL_RenderClear(boVe);
    SDL_RenderCopy(boVe, nenTroChoi, nullptr, nullptr);

    if (nguoiChoi1.active) {
        SDL_Rect nguoiChoi1Rect = {nguoiChoi1.x, nguoiChoi1.y, nguoiChoi1.w, nguoiChoi1.h};
        SDL_RenderCopy(boVe, nguoiChoi1Texture, nullptr, &nguoiChoi1Rect);
    }
    if (isTwoPlayers && nguoiChoi2.active) {
        SDL_Rect nguoiChoi2Rect = {nguoiChoi2.x, nguoiChoi2.y, nguoiChoi2.w, nguoiChoi2.h};
        SDL_RenderCopy(boVe, nguoiChoi2Texture, nullptr, &nguoiChoi2Rect);
    }

    for (const auto& bullet : bullets) {
        SDL_Rect bRect = {bullet.x, bullet.y, bullet.w, bullet.h};
        SDL_RenderCopy(boVe, danTexture, nullptr, &bRect);
    }

    for (const auto& enemy : enemies) {
        SDL_Rect eRect = {enemy.x, enemy.y, enemy.w, enemy.h};
        switch (enemy.enemyType) {
            case 1: SDL_RenderCopy(boVe, keDichTexture, nullptr, &eRect); break;
            case 2: SDL_RenderCopy(boVe, keDich2Texture, nullptr, &eRect); break;
            case 3: SDL_RenderCopy(boVe, keDich3Texture, nullptr, &eRect); break;
            case 4: SDL_RenderCopy(boVe, keDich4Texture, nullptr, &eRect); break;
            case 5: SDL_RenderCopy(boVe, keDich5Texture, nullptr, &eRect); break;
            case 6: SDL_RenderCopy(boVe, bossTexture, nullptr, &eRect); break;
        }
    }

    for (const auto& bomb : enemyBullets) {
        SDL_Rect bombRect = {bomb.x, bomb.y, bomb.w, bomb.h};
        SDL_RenderCopy(boVe, bomTexture, nullptr, &bombRect);
    }

    for (const auto& explosion : explosions) {
        if (explosion.active) {
            SDL_Rect eRect = {explosion.x, explosion.y, explosion.w, explosion.h};
            SDL_RenderCopy(boVe, noTexture, nullptr, &eRect);
        }
    }

    SDL_RenderCopy(boVe, diemSoTexture, nullptr, &diemSoRect);
    SDL_RenderPresent(boVe);
}

void clearResources() {
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);
    if (shootSound) Mix_FreeChunk(shootSound);
    if (explosionSound) Mix_FreeChunk(explosionSound);
    if (clickSound) Mix_FreeChunk(clickSound);
    if (winSound) Mix_FreeChunk(winSound);
    if (gameOverSound) Mix_FreeChunk(gameOverSound);
    Mix_CloseAudio();

    if (phongChu) TTF_CloseFont(phongChu);
    if (phongChuLon) TTF_CloseFont(phongChuLon);
    TTF_Quit();

    if (nenMenu) SDL_DestroyTexture(nenMenu);
    if (nenTroChoi) SDL_DestroyTexture(nenTroChoi);
    if (nguoiChoi1Texture) SDL_DestroyTexture(nguoiChoi1Texture);
    if (nguoiChoi2Texture) SDL_DestroyTexture(nguoiChoi2Texture);
    if (danTexture) SDL_DestroyTexture(danTexture);
    if (keDichTexture) SDL_DestroyTexture(keDichTexture);
    if (keDich2Texture) SDL_DestroyTexture(keDich2Texture);
    if (keDich3Texture) SDL_DestroyTexture(keDich3Texture);
    if (keDich4Texture) SDL_DestroyTexture(keDich4Texture);
    if (keDich5Texture) SDL_DestroyTexture(keDich5Texture);
    if (bossTexture) SDL_DestroyTexture(bossTexture);
    if (bomTexture) SDL_DestroyTexture(bomTexture);
    if (diemSoTexture) SDL_DestroyTexture(diemSoTexture);
    if (noTexture) SDL_DestroyTexture(noTexture);
    if (gameOverTexture) SDL_DestroyTexture(gameOverTexture);
    if (thangTexture) SDL_DestroyTexture(thangTexture);
    if (choiLaiTexture) SDL_DestroyTexture(choiLaiTexture);
    if (thoatTexture) SDL_DestroyTexture(thoatTexture);
    if (motNguoiChoiTexture) SDL_DestroyTexture(motNguoiChoiTexture);
    if (haiNguoiChoiTexture) SDL_DestroyTexture(haiNguoiChoiTexture);
    if (huongDanTexture) SDL_DestroyTexture(huongDanTexture);
    if (thoatMenuTexture) SDL_DestroyTexture(thoatMenuTexture);
    if (huongDanTextTexture1) SDL_DestroyTexture(huongDanTextTexture1);
    if (huongDanTextTexture2) SDL_DestroyTexture(huongDanTextTexture2);
    if (quayLaiMenuTexture) SDL_DestroyTexture(quayLaiMenuTexture);
    if (soundTexture) SDL_DestroyTexture(soundTexture);
    if (backToMenuTexture) SDL_DestroyTexture(backToMenuTexture);

    if (boVe) SDL_DestroyRenderer(boVe);
    if (cuaSo) SDL_DestroyWindow(cuaSo);

    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    srand(static_cast<unsigned>(time(0)));
    if (!initialize()) {
        printf("Initialization failed!\n");
        clearResources();
        return 1;
    }

    bool isRunning = true;
    while (isRunning) {
        handleEvents(isRunning);
        switch (trangThaiGame) {
            case MENU:
                renderMenu();
                break;
            case HUONG_DAN:
                renderTutorial();
                break;
            case DANG_CHOI:
                updateGame();
                render();
                break;
            case GAME_OVER:
                renderGameOver();
                break;
            case THANG:
                renderWin();
                break;
            case THOAT:
                isRunning = false;
                break;
        }
        SDL_Delay(16); // ~60 FPS
    }

    clearResources();
    return 0;
}