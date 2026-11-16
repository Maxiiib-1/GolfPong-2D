// CHADPONG.cpp : Hockey Pong con SDL2, SDL_mixer, SDL_image y SDL_ttf
//
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <math.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <vector>
#include <sstream>

using namespace std;

// =====================
// ESTRUCTURAS
// =====================
struct GameRecord
{
    string fecha;
    string hora;
    string dificultad;
    string resultado;
    int puntajeJugador;
    int puntajeIA;
    int tiempoRestante;
};

// =====================
// VARIABLES DE AUDIO
// =====================
Mix_Music* backgroundMusic = nullptr;

Mix_Chunk* sfxHit1 = nullptr;
Mix_Chunk* sfxHit2 = nullptr;

Mix_Chunk* sfxWin = nullptr;
Mix_Chunk* sfxLose = nullptr;
Mix_Chunk* sfxDraw = nullptr;

Mix_Chunk* sfxAdrianWin = nullptr;
Mix_Chunk* sfxTomasWin = nullptr;
Mix_Chunk* sfxGolfDraw = nullptr;

Mix_Chunk* sfxPhrase1 = nullptr;
Mix_Chunk* sfxPhrase2 = nullptr;

Mix_Chunk* sfxGoal = nullptr;
Mix_Chunk* sfxSmash = nullptr;

bool resultSoundPlayed = false;

Uint32 lastPhraseTime = 0;
Uint32 nextPhraseDelay = 0;

// =====================
// VARIABLES GLOBALES
// =====================
int pauseOption = 0;
int pauseSettingsOption = 0;

bool pauseSettings = false;
int powerMeter = 0;
bool smashReady = false;

bool chadrianMode = false;
bool finalBossMode = false;
bool scoreSaved = false;

int difficulty = 1;

// Volumen de musica y efectos por separado
int musicVolume = 100;   // Volumen de musica de fondo
int sfxVolume = 100;     // Volumen de efectos de sonido

const int MUSIC_DUCK_PERCENT = 40; // % del volumen de musica mientras se juega (ducking)
bool musicDucked = false;

int selectedOption = 0;
int settingsOption = 0;
int historialOption = 0;

vector<GameRecord> gameHistory;

const int SCOREBOARD_X = 680;
const int SCOREBOARD_Y = 40;

const int RINK_LEFT = 35;
const int RINK_RIGHT = 1600;
const int RINK_TOP = 180;
const int RINK_BOTTOM = 1010;

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const int PLAYER_TOP = 180;
const int PLAYER_BOTTOM = SCREEN_HEIGHT;
const int AI_TOP = 180;
const int AI_BOTTOM = SCREEN_HEIGHT;

const int GAME_AREA_TOP = 0;
const int GAME_AREA_BOTTOM = SCREEN_HEIGHT;

int playerScore = 0;
int aiScore = 0;
const int MAX_SCORE = 7;

const int GAME_TIME = 120;

Uint32 startTime = 0;
bool gameOver = false;

enum GameState { MENU, PLAYING, PAUSED, SETTINGS, HISTORY, RESULT };

GameState gameState = MENU;

SDL_Texture* golfBackgroundTexture = nullptr;
SDL_Texture* adrianBlueTexture = nullptr;
SDL_Texture* adrianRedTexture = nullptr;
SDL_Texture* ballTexture = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* menuTexture = nullptr;

// Texturas de resultado
SDL_Texture* victoryTexture = nullptr;
SDL_Texture* defeatTexture = nullptr;
SDL_Texture* drawTexture = nullptr;

// Texturas de resultado ADRIAN
SDL_Texture* victoryGolfTexture = nullptr;
SDL_Texture* defeatGolfTexture = nullptr;
SDL_Texture* drawGolfTexture = nullptr;

int resultType = 0; // 1 = victoria, 2 = derrota, 3 = empate

SDL_Texture* leftPaddleTexture = nullptr;
SDL_Texture* rightPaddleTexture = nullptr;
SDL_Rect leftPaddle;
SDL_Rect rightPaddle;
SDL_Rect leftHole;
SDL_Rect rightHole;
SDL_Rect leftWall;
SDL_Rect rightWall;
SDL_Rect bottomWall;
SDL_Rect leftPaddleHitbox;
SDL_Rect rightPaddleHitbox;

SDL_Rect ball;
SDL_Rect ballHitbox;

int paddleSpeed = 10;
int aiSpeed = 7;

int ballVelX = 8;
int ballVelY = 8;

// =====================
// PROTOTIPOS
// =====================
void SaveGameRecord();
void LoadGameHistory();

// =====================
// FUNCIONES DE AUDIO
// =====================

int GetSDLMusicVolume()
{
    int v = (musicVolume * MIX_MAX_VOLUME) / 100;

    // Si la musica esta "duckeada" (estamos jugando), bajamos mas
    if (musicDucked)
        v = (v * MUSIC_DUCK_PERCENT) / 100;

    return v;
}

int GetSDLVolume() // efectos de sonido
{
    return (sfxVolume * MIX_MAX_VOLUME) / 100;
}

void UpdateMusicVolume()
{
    Mix_VolumeMusic(GetSDLMusicVolume());
}

void UpdateSfxVolume()
{
    Mix_Volume(-1, GetSDLVolume());
}

void UpdateVolume()
{
    UpdateMusicVolume();
    UpdateSfxVolume();
}

void DuckMusic(bool duck)
{
    musicDucked = duck;
    UpdateMusicVolume();
}

void PlayHitSound()
{
    int sdlVol = GetSDLVolume();

    if (rand() % 2 == 0)
    {
        if (sfxHit1)
        {
            Mix_VolumeChunk(sfxHit1, sdlVol);
            Mix_PlayChannel(-1, sfxHit1, 0);
        }
    }
    else
    {
        if (sfxHit2)
        {
            Mix_VolumeChunk(sfxHit2, sdlVol);
            Mix_PlayChannel(-1, sfxHit2, 0);
        }
    }
}

void PlayGoalSound()
{
    int sdlVol = GetSDLVolume();
    if (sfxGoal)
    {
        Mix_VolumeChunk(sfxGoal, sdlVol);
        Mix_PlayChannel(-1, sfxGoal, 0);
    }
}

void PlaySmashSound()
{
    int sdlVol = GetSDLVolume();
    if (sfxSmash)
    {
        Mix_VolumeChunk(sfxSmash, sdlVol);
        Mix_PlayChannel(-1, sfxSmash, 0);
    }
}

void PlayResultSound()
{
    if (resultSoundPlayed)
        return;

    resultSoundPlayed = true;

    int sdlVol = GetSDLVolume();

    if (difficulty == 3)
    {
        if (resultType == 1)
        {
            if (sfxTomasWin)
            {
                Mix_VolumeChunk(sfxTomasWin, sdlVol);
                Mix_PlayChannel(-1, sfxTomasWin, 0);
            }
        }
        else if (resultType == 2)
        {
            if (sfxAdrianWin)
            {
                Mix_VolumeChunk(sfxAdrianWin, sdlVol);
                Mix_PlayChannel(-1, sfxAdrianWin, 0);
            }
        }
        else if (resultType == 3)
        {
            if (sfxGolfDraw)
            {
                Mix_VolumeChunk(sfxGolfDraw, sdlVol);
                Mix_PlayChannel(-1, sfxGolfDraw, 0);
            }
        }
    }
    else
    {
        if (resultType == 1)
        {
            if (sfxWin)
            {
                Mix_VolumeChunk(sfxWin, sdlVol);
                Mix_PlayChannel(-1, sfxWin, 0);
            }
        }
        else if (resultType == 2)
        {
            if (sfxLose)
            {
                Mix_VolumeChunk(sfxLose, sdlVol);
                Mix_PlayChannel(-1, sfxLose, 0);
            }
        }
        else if (resultType == 3)
        {
            if (sfxDraw)
            {
                Mix_VolumeChunk(sfxDraw, sdlVol);
                Mix_PlayChannel(-1, sfxDraw, 0);
            }
        }
    }
}

void ResetPhraseTimer()
{
    lastPhraseTime = SDL_GetTicks();
    nextPhraseDelay = 15000 + (rand() % 15001);
}

void UpdateAdrianPhrases()
{
    if (difficulty != 3)
        return;

    if (gameState != PLAYING)
        return;

    Uint32 now = SDL_GetTicks();

    if (now - lastPhraseTime < nextPhraseDelay)
        return;

    if (Mix_Playing(1))
        return;

    int sdlVol = GetSDLVolume();

    if (rand() % 2 == 0)
    {
        if (sfxPhrase1)
        {
            Mix_VolumeChunk(sfxPhrase1, sdlVol);
            Mix_PlayChannel(1, sfxPhrase1, 0);
        }
    }
    else
    {
        if (sfxPhrase2)
        {
            Mix_VolumeChunk(sfxPhrase2, sdlVol);
            Mix_PlayChannel(1, sfxPhrase2, 0);
        }
    }

    ResetPhraseTimer();
}

void LoadAudio()
{
    // Musica de fondo
    backgroundMusic = Mix_LoadMUS(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/musicadefondo.wav");
    if (!backgroundMusic)
        cout << "ERROR: No se pudo cargar musicadefondo.wav" << endl;
    else
        cout << "OK: musicadefondo.wav cargado" << endl;

    // Golpes
    sfxHit1 = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/golpepelota1.wav");
    if (!sfxHit1)
        cout << "ERROR: No se pudo cargar golpepelota1.wav" << endl;
    else
        cout << "OK: golpepelota1.wav cargado" << endl;

    sfxHit2 = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/golpepelota2.wav");
    if (!sfxHit2)
        cout << "ERROR: No se pudo cargar golpepelota2.wav" << endl;
    else
        cout << "OK: golpepelota2.wav cargado" << endl;

    // Gol
    sfxGoal = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/gol.wav");
    if (!sfxGoal)
        cout << "WARNING: No se pudo cargar gol.wav" << endl;
    else
        cout << "OK: gol.wav cargado" << endl;

    // Smash
    sfxSmash = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/smash.wav");
    if (!sfxSmash)
        cout << "WARNING: No se pudo cargar smash.wav" << endl;
    else
        cout << "OK: smash.wav cargado" << endl;

    // Resultados normales
    sfxWin = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/GANAR.wav");
    if (!sfxWin)
        cout << "ERROR: No se pudo cargar GANAR.wav" << endl;
    else
        cout << "OK: GANAR.wav cargado" << endl;

    sfxLose = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/PERDER.wav");
    if (!sfxLose)
        cout << "ERROR: No se pudo cargar PERDER.wav" << endl;
    else
        cout << "OK: PERDER.wav cargado" << endl;

    sfxDraw = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/EMPATE.wav");
    if (!sfxDraw)
        cout << "ERROR: No se pudo cargar EMPATE.wav" << endl;
    else
        cout << "OK: EMPATE.wav cargado" << endl;

    // Resultados ADRIAN
    sfxAdrianWin = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/ganoadrian.wav");
    if (!sfxAdrianWin)
        cout << "ERROR: No se pudo cargar ganoadrian.wav" << endl;
    else
        cout << "OK: ganoadrian.wav cargado" << endl;

    sfxTomasWin = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/ganotomas.wav");
    if (!sfxTomasWin)
        cout << "ERROR: No se pudo cargar ganotomas.wav" << endl;
    else
        cout << "OK: ganotomas.wav cargado" << endl;

    sfxGolfDraw = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/empategolf.wav");
    if (!sfxGolfDraw)
        cout << "ERROR: No se pudo cargar empategolf.wav" << endl;
    else
        cout << "OK: empategolf.wav cargado" << endl;

    // Frases ADRIAN
    sfxPhrase1 = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/enmistiempos.wav");
    if (!sfxPhrase1)
        cout << "WARNING: No se pudo cargar enmistiempos.wav" << endl;
    else
        cout << "OK: enmistiempos.wav cargado" << endl;

    sfxPhrase2 = Mix_LoadWAV(
        "C:/Users/maxim/source/repos/CHADPONG/assets/sonidos/momentocultural1.wav");
    if (!sfxPhrase2)
        cout << "WARNING: No se pudo cargar momentocultural1.wav" << endl;
    else
        cout << "OK: momentocultural1.wav cargado" << endl;
}

void FreeAudio()
{
    if (backgroundMusic)
    {
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
    if (sfxHit1)
    {
        Mix_FreeChunk(sfxHit1);
        sfxHit1 = nullptr;
    }
    if (sfxHit2)
    {
        Mix_FreeChunk(sfxHit2);
        sfxHit2 = nullptr;
    }
    if (sfxGoal)
    {
        Mix_FreeChunk(sfxGoal);
        sfxGoal = nullptr;
    }
    if (sfxSmash)
    {
        Mix_FreeChunk(sfxSmash);
        sfxSmash = nullptr;
    }
    if (sfxWin)
    {
        Mix_FreeChunk(sfxWin);
        sfxWin = nullptr;
    }
    if (sfxLose)
    {
        Mix_FreeChunk(sfxLose);
        sfxLose = nullptr;
    }
    if (sfxDraw)
    {
        Mix_FreeChunk(sfxDraw);
        sfxDraw = nullptr;
    }
    if (sfxAdrianWin)
    {
        Mix_FreeChunk(sfxAdrianWin);
        sfxAdrianWin = nullptr;
    }
    if (sfxTomasWin)
    {
        Mix_FreeChunk(sfxTomasWin);
        sfxTomasWin = nullptr;
    }
    if (sfxGolfDraw)
    {
        Mix_FreeChunk(sfxGolfDraw);
        sfxGolfDraw = nullptr;
    }
    if (sfxPhrase1)
    {
        Mix_FreeChunk(sfxPhrase1);
        sfxPhrase1 = nullptr;
    }
    if (sfxPhrase2)
    {
        Mix_FreeChunk(sfxPhrase2);
        sfxPhrase2 = nullptr;
    }
}

// =====================
// FUNCIONES DE HISTORIAL
// =====================

void LoadGameHistory()
{
    gameHistory.clear();
    ifstream file("gamehistory.csv");

    if (!file.is_open())
    {
        cout << "Archivo gamehistory.csv no encontrado (primera partida)" << endl;
        return;
    }

    string line;
    while (getline(file, line))
    {
        if (line.empty()) continue;

        stringstream ss(line);
        string fecha, hora, dificultad, resultado, pj, pia, tr;

        getline(ss, fecha, ',');
        getline(ss, hora, ',');
        getline(ss, dificultad, ',');
        getline(ss, resultado, ',');
        getline(ss, pj, ',');
        getline(ss, pia, ',');
        getline(ss, tr, ',');

        GameRecord record;
        record.fecha = fecha;
        record.hora = hora;
        record.dificultad = dificultad;
        record.resultado = resultado;
        record.puntajeJugador = stoi(pj);
        record.puntajeIA = stoi(pia);
        record.tiempoRestante = stoi(tr);

        gameHistory.insert(gameHistory.begin(), record);
    }

    file.close();
    cout << "Historial cargado: " << gameHistory.size() << " partidas" << endl;
}

void SaveGameRecord()
{
    string dificultadStr;
    switch (difficulty)
    {
    case 0: dificultadStr = "Facil"; break;
    case 1: dificultadStr = "Normal"; break;
    case 2: dificultadStr = "Dificil"; break;
    case 3: dificultadStr = "ADRIAN"; break;
    default: dificultadStr = "Normal"; break;
    }

    string resultadoStr;
    if (resultType == 1)
        resultadoStr = "Victoria";
    else if (resultType == 2)
        resultadoStr = "Derrota";
    else
        resultadoStr = "Empate";

    time_t now = time(0);
    tm localTime;
    localtime_s(&localTime, &now);

    char fecha[20], hora[20];
    strftime(fecha, sizeof(fecha), "%d/%m/%Y", &localTime);
    strftime(hora, sizeof(hora), "%H:%M:%S", &localTime);

    Uint32 elapsedTime = (SDL_GetTicks() - startTime) / 1000;
    int remainingTime = GAME_TIME - elapsedTime;
    if (remainingTime < 0) remainingTime = 0;

    // Crear registro
    GameRecord record;
    record.fecha = fecha;
    record.hora = hora;
    record.dificultad = dificultadStr;
    record.resultado = resultadoStr;
    record.puntajeJugador = playerScore;
    record.puntajeIA = aiScore;
    record.tiempoRestante = remainingTime;

    // Agregar al principio del historial (mas recientes primero)
    gameHistory.insert(gameHistory.begin(), record);

    // Guardar en archivo
    ofstream file("gamehistory.csv");
    if (!file.is_open())
    {
        cout << "Error: No se pudo guardar el historial" << endl;
        return;
    }

    for (const auto& r : gameHistory)
    {
        file << r.fecha << ","
            << r.hora << ","
            << r.dificultad << ","
            << r.resultado << ","
            << r.puntajeJugador << ","
            << r.puntajeIA << ","
            << r.tiempoRestante << "\n";
    }

    file.close();
    cout << "Partida guardada en historial" << endl;
}

// =====================
// INITIALIZE GAME
// =====================
void InitializeGame()
{
    playerScore = 0;
    aiScore = 0;
    gameOver = false;
    scoreSaved = false;
    chadrianMode = false;
    finalBossMode = false;

    pauseOption = 0;
    pauseSettingsOption = 0;
    pauseSettings = false;

    powerMeter = 0;
    smashReady = false;

    resultType = 0;
    resultSoundPlayed = false;
    gameState = PLAYING;

    startTime = SDL_GetTicks();

    // Reiniciar musica
    Mix_HaltMusic();
    if (backgroundMusic)
    {
        Mix_PlayMusic(backgroundMusic, -1);
    }

    // Bajar la musica mientras se juega para resaltar los efectos
    DuckMusic(true);

    ResetPhraseTimer();

    if (difficulty == 3)
    {
        leftWall = { 0, 180, 120, 700 };
        rightWall = { 1800, 180, 120, 700 };
        bottomWall = { 0, 920, 1920, 160 };

        leftHole = { 200, 500, 120, 120 };
        rightHole = { 1600, 500, 120, 120 };
    }
    else
    {
        leftHole = { 0, 0, 0, 0 };
        rightHole = { 0, 0, 0, 0 };
    }

    if (difficulty == 3)
    {
        leftPaddle.x = 400;
        rightPaddle.x = SCREEN_WIDTH - 500;
    }
    else
    {
        leftPaddle.x = 150;
        rightPaddle.x = SCREEN_WIDTH - 250;
    }

    leftPaddle.y = GAME_AREA_TOP + (GAME_AREA_BOTTOM - GAME_AREA_TOP) / 2 - 50;
    leftPaddle.w = 100;
    leftPaddle.h = 200;

    rightPaddle.y = GAME_AREA_TOP + (GAME_AREA_BOTTOM - GAME_AREA_TOP) / 2 - 50;
    rightPaddle.w = 100;
    rightPaddle.h = 200;

    ball.x = SCREEN_WIDTH / 2;
    ball.y = GAME_AREA_TOP + (GAME_AREA_BOTTOM - GAME_AREA_TOP) / 2;
    ball.w = 60;
    ball.h = 40;

    ballVelX = 8;
    ballVelY = 8;

    switch (difficulty)
    {
    case 0: aiSpeed = 4; break;
    case 1: aiSpeed = 7; break;
    case 2: aiSpeed = 10; break;
    case 3: aiSpeed = 20; break;
    default: aiSpeed = 7; break;
    }
}

// =====================
// DRAWTEXT
// =====================
void DrawText(SDL_Renderer* renderer,
    TTF_Font* font,
    const string& text,
    int x,
    int y)
{
    SDL_Color color = { 255, 255, 255, 255 };

    SDL_Surface* surface =
        TTF_RenderText_Solid(font, text.c_str(), color);

    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// =====================
// HANDLE INPUT
// =====================
void HandleInput(bool& running)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            running = false;
        }

        // RESULT
        if (gameState == RESULT)
        {
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    InitializeGame();
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    gameState = MENU;
                    gameOver = false;
                    resultType = 0;
                    resultSoundPlayed = false;
                    DuckMusic(false);
                }
            }
            return;
        }

        // PLAYING
        if (gameState == PLAYING)
        {
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    gameState = PAUSED;
                    return;
                }
            }
        }

        // PAUSA
        if (gameState == PAUSED)
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    if (!pauseSettings)
                        pauseOption--;
                    else
                        pauseSettingsOption--;
                    break;

                case SDLK_s:
                    if (!pauseSettings)
                        pauseOption++;
                    else
                        pauseSettingsOption++;
                    break;

                case SDLK_RETURN:
                    if (!pauseSettings)
                    {
                        if (pauseOption == 0)
                            gameState = PLAYING;

                        if (pauseOption == 1)
                            pauseSettings = true;

                        if (pauseOption == 2)
                        {
                            pauseSettings = false;
                            pauseOption = 0;
                            pauseSettingsOption = 0;
                            gameOver = false;
                            resultSoundPlayed = false;
                            gameState = MENU;
                            DuckMusic(false);
                        }
                    }
                    else
                    {
                        if (pauseSettingsOption == 2)
                            pauseSettings = false;
                    }
                    break;

                case SDLK_a:
                    if (pauseSettings)
                    {
                        if (pauseSettingsOption == 0)
                        {
                            musicVolume -= 10;
                            if (musicVolume < 0) musicVolume = 0;
                            UpdateMusicVolume();
                        }
                        else if (pauseSettingsOption == 1)
                        {
                            sfxVolume -= 10;
                            if (sfxVolume < 0) sfxVolume = 0;
                            UpdateSfxVolume();
                        }
                    }
                    break;

                case SDLK_d:
                    if (pauseSettings)
                    {
                        if (pauseSettingsOption == 0)
                        {
                            musicVolume += 10;
                            if (musicVolume > 100) musicVolume = 100;
                            UpdateMusicVolume();
                        }
                        else if (pauseSettingsOption == 1)
                        {
                            sfxVolume += 10;
                            if (sfxVolume > 100) sfxVolume = 100;
                            UpdateSfxVolume();
                        }
                    }
                    break;

                case SDLK_ESCAPE:
                    if (pauseSettings)
                        pauseSettings = false;
                    else
                        gameState = PLAYING;
                    break;
                }
            }

            if (!pauseSettings)
            {
                if (pauseOption < 0) pauseOption = 2;
                if (pauseOption > 2) pauseOption = 0;
            }
            else
            {
                if (pauseSettingsOption < 0) pauseSettingsOption = 2;
                if (pauseSettingsOption > 2) pauseSettingsOption = 0;
            }

            return;
        }

        // MENU
        if (gameState == MENU)
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    selectedOption--;
                    break;

                case SDLK_s:
                    selectedOption++;
                    break;

                case SDLK_RETURN:
                    if (selectedOption == 0)
                        InitializeGame();

                    if (selectedOption == 1)
                    {
                        historialOption = 0;
                        gameState = HISTORY;
                    }

                    if (selectedOption == 2)
                        gameState = SETTINGS;

                    if (selectedOption == 3)
                        running = false;

                    break;
                }
            }

            if (selectedOption < 0) selectedOption = 3;
            if (selectedOption > 3) selectedOption = 0;
            return;
        }

        // HISTORY
        if (gameState == HISTORY)
        {
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    gameState = MENU;
                }
                else if (event.key.keysym.sym == SDLK_w)
                {
                    if (historialOption > 0)
                        historialOption--;
                }
                else if (event.key.keysym.sym == SDLK_s)
                {
                    if (historialOption < (int)gameHistory.size() - 1)
                        historialOption++;
                }
            }
            return;
        }

        // SETTINGS
        if (gameState == SETTINGS)
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    settingsOption--;
                    break;

                case SDLK_s:
                    settingsOption++;
                    break;

                case SDLK_a:
                    if (settingsOption == 0)
                    {
                        difficulty--;
                        if (difficulty < 0) difficulty = 0;
                    }
                    else if (settingsOption == 1)
                    {
                        musicVolume -= 10;
                        if (musicVolume < 0) musicVolume = 0;
                        UpdateMusicVolume();
                    }
                    else if (settingsOption == 2)
                    {
                        sfxVolume -= 10;
                        if (sfxVolume < 0) sfxVolume = 0;
                        UpdateSfxVolume();
                    }
                    break;

                case SDLK_d:
                    if (settingsOption == 0)
                    {
                        difficulty++;
                        if (difficulty > 3) difficulty = 0;
                    }
                    else if (settingsOption == 1)
                    {
                        musicVolume += 10;
                        if (musicVolume > 100) musicVolume = 100;
                        UpdateMusicVolume();
                    }
                    else if (settingsOption == 2)
                    {
                        sfxVolume += 10;
                        if (sfxVolume > 100) sfxVolume = 100;
                        UpdateSfxVolume();
                    }
                    break;

                case SDLK_RETURN:
                    if (settingsOption == 3)
                        gameState = MENU;
                    break;
                }
            }

            if (settingsOption < 0) settingsOption = 3;
            if (settingsOption > 3) settingsOption = 0;

            return;
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (gameState != PLAYING)
        return;

    if (keys[SDL_SCANCODE_SPACE] && smashReady)
    {
        smashReady = false;
        powerMeter = 0;
        ballVelX *= 2;
        ballVelY *= 2;
        PlaySmashSound();
    }

    if (keys[SDL_SCANCODE_W])
        leftPaddle.y -= paddleSpeed;

    if (keys[SDL_SCANCODE_S])
        leftPaddle.y += paddleSpeed;

    if (leftPaddle.y < PLAYER_TOP)
        leftPaddle.y = PLAYER_TOP;

    if (leftPaddle.y + leftPaddle.h > PLAYER_BOTTOM)
        leftPaddle.y = PLAYER_BOTTOM - leftPaddle.h;
}

// =====================
// UPDATE
// =====================
void Update()
{
    if (gameState != PLAYING)
        return;

    if (gameOver)
        return;

    UpdateAdrianPhrases();

    ball.x += ballVelX;
    ball.y += ballVelY;

    ballHitbox.w = 22;
    ballHitbox.h = 22;
    ballHitbox.x = ball.x + 19;
    ballHitbox.y = ball.y + 4;

    if (difficulty == 3)
    {
        if (SDL_HasIntersection(&ballHitbox, &leftWall))
            ballVelX = abs(ballVelX);

        if (SDL_HasIntersection(&ballHitbox, &rightWall))
            ballVelX = -abs(ballVelX);

        if (SDL_HasIntersection(&ballHitbox, &bottomWall))
            ballVelY = -abs(ballVelY);
    }

    if (ball.y <= RINK_TOP)
    {
        ball.y = RINK_TOP;
        ballVelY *= -1;
    }

    if (ball.y + ball.h >= RINK_BOTTOM)
    {
        ball.y = RINK_BOTTOM - ball.h;
        ballVelY *= -1;
    }

    if (difficulty == 3)
    {
        leftPaddleHitbox.w = 60;
        leftPaddleHitbox.h = 120;
        rightPaddleHitbox.w = 60;
        rightPaddleHitbox.h = 120;

        leftPaddleHitbox.x = leftPaddle.x + 20;
        leftPaddleHitbox.y = leftPaddle.y + 40;
        rightPaddleHitbox.x = rightPaddle.x + 20;
        rightPaddleHitbox.y = rightPaddle.y + 40;
    }
    else
    {
        leftPaddleHitbox.w = 30;
        leftPaddleHitbox.h = 83;
        rightPaddleHitbox.w = 30;
        rightPaddleHitbox.h = 83;

        leftPaddleHitbox.x = leftPaddle.x + 50;
        leftPaddleHitbox.y = leftPaddle.y + 40;
        rightPaddleHitbox.x = rightPaddle.x + 18;
        rightPaddleHitbox.y = rightPaddle.y + 73;
    }

    Uint32 elapsedTime = (SDL_GetTicks() - startTime) / 1000;
    int remainingTime = GAME_TIME - elapsedTime;

    switch (difficulty)
    {
    case 0: aiSpeed = 4; break;
    case 1: aiSpeed = 7; break;
    case 2: aiSpeed = 10; break;
    case 3: aiSpeed = 20; break;
    }

    if (ballVelX > 0)
    {
        int paddleCenter = rightPaddle.y + rightPaddle.h / 2;
        int ballCenter = ball.y + ball.h / 2 + ballVelY * 50;

        if (difficulty == 3)
        {
            rightPaddle.y = ball.y - 50;
        }
        else
        {
            if (ballCenter < paddleCenter)
                rightPaddle.y -= aiSpeed;
            else if (ballCenter > paddleCenter)
                rightPaddle.y += aiSpeed;
        }
    }

    if (rightPaddle.y < AI_TOP)
        rightPaddle.y = AI_TOP;

    if (rightPaddle.y + rightPaddle.h > AI_BOTTOM)
        rightPaddle.y = AI_BOTTOM - rightPaddle.h;

    // COLISION PALETA DERECHA
    if (SDL_HasIntersection(&ballHitbox, &rightPaddleHitbox))
    {
        ballVelX = -abs(ballVelX);

        int paddleCenter = rightPaddleHitbox.y + rightPaddleHitbox.h / 2;
        int ballCenter = ballHitbox.y + ballHitbox.h / 2;

        ballVelY = (ballCenter - paddleCenter) / 2;

        if (ballVelY > 15) ballVelY = 15;
        if (ballVelY < -15) ballVelY = -15;

        PlayHitSound();
    }

    // COLISION PALETA IZQUIERDA
    if (SDL_HasIntersection(&ballHitbox, &leftPaddleHitbox))
    {
        ballVelX = abs(ballVelX);

        int paddleCenter = leftPaddleHitbox.y + leftPaddleHitbox.h / 2;
        int ballCenter = ballHitbox.y + ballHitbox.h / 2;

        ballVelY = (ballCenter - paddleCenter) / 2;

        if (ballVelY > 15) ballVelY = 15;
        if (ballVelY < -15) ballVelY = -15;

        PlayHitSound();
    }

    // PUNTUACION
    if (difficulty == 3)
    {
        if (SDL_HasIntersection(&ballHitbox, &leftHole))
        {
            aiScore++;
            PlayGoalSound();
            ball.x = SCREEN_WIDTH / 2 - ball.w / 2;
            ball.y = SCREEN_HEIGHT / 2 - ball.h / 2;
            ballVelX = 8;
            ballVelY = 8;
        }

        if (SDL_HasIntersection(&ballHitbox, &rightHole))
        {
            playerScore++;
            PlayGoalSound();
            ball.x = SCREEN_WIDTH / 2 - ball.w / 2;
            ball.y = SCREEN_HEIGHT / 2 - ball.h / 2;
            ballVelX = -8;
            ballVelY = 8;
        }
    }
    else
    {
        if (ball.x < 170)
        {
            aiScore++;
            PlayGoalSound();
            ball.x = SCREEN_WIDTH / 2 - ball.w / 2;
            ball.y = SCREEN_HEIGHT / 2 - ball.h / 2;
            ballVelX = 8;
        }

        if (ball.x > SCREEN_WIDTH - 220)
        {
            playerScore++;
            PlayGoalSound();
            ball.x = SCREEN_WIDTH / 2 - ball.w / 2;
            ball.y = SCREEN_HEIGHT / 2 - ball.h / 2;
            ballVelX = -8;
        }
    }

    if (difficulty == 3 && aiScore >= 3 && !chadrianMode)
    {
        chadrianMode = true;
        aiSpeed = 17;
        ballVelX *= 2;
        ballVelY *= 2;
        cout << "ADRIAN SE ENOJO" << endl;
    }

    if (difficulty == 3 && aiScore >= 6 && !finalBossMode)
    {
        finalBossMode = true;
        aiSpeed = 30;
        cout << "MODO CHADRIAN ACTIVADO" << endl;
    }

    // FIN POR SCORE
    if (playerScore >= MAX_SCORE)
    {
        gameOver = true;
        resultType = 1;
        gameState = RESULT;

        if (!scoreSaved)
        {
            SaveGameRecord();
            scoreSaved = true;
        }
    }

    if (aiScore >= MAX_SCORE)
    {
        gameOver = true;
        resultType = 2;
        gameState = RESULT;

        if (!scoreSaved)
        {
            SaveGameRecord();
            scoreSaved = true;
        }
    }

    // FIN POR TIEMPO
    if (remainingTime <= 0)
    {
        gameOver = true;

        if (playerScore > aiScore)
            resultType = 1;
        else if (aiScore > playerScore)
            resultType = 2;
        else
            resultType = 3;

        gameState = RESULT;

        if (!scoreSaved)
        {
            SaveGameRecord();
            scoreSaved = true;
        }
    }
}

// =====================
// RENDER
// =====================
void Render(SDL_Renderer* renderer, TTF_Font* font)
{
    // RESULT
    if (gameState == RESULT)
    {
        SDL_RenderClear(renderer);

        PlayResultSound();

        // Mostrar texturas correspondientes
        if (difficulty == 3)
        {
            if (resultType == 1 && victoryGolfTexture)
                SDL_RenderCopy(renderer, victoryGolfTexture, NULL, NULL);
            else if (resultType == 2 && defeatGolfTexture)
                SDL_RenderCopy(renderer, defeatGolfTexture, NULL, NULL);
            else if (resultType == 3 && drawGolfTexture)
                SDL_RenderCopy(renderer, drawGolfTexture, NULL, NULL);
        }
        else
        {
            if (resultType == 1 && victoryTexture)
                SDL_RenderCopy(renderer, victoryTexture, NULL, NULL);
            else if (resultType == 2 && defeatTexture)
                SDL_RenderCopy(renderer, defeatTexture, NULL, NULL);
            else if (resultType == 3 && drawTexture)
                SDL_RenderCopy(renderer, drawTexture, NULL, NULL);
        }

        DrawText(renderer, font, "ENTER - Jugar de nuevo", 700, 900);
        DrawText(renderer, font, "ESC - Menu principal", 700, 950);

        SDL_RenderPresent(renderer);
        return;
    }

    // HISTORY
    if (gameState == HISTORY)
    {
        SDL_RenderCopy(renderer, menuTexture, NULL, NULL);

        DrawText(renderer, font, "HISTORIAL DE PARTIDAS", 600, 100);

        if (gameHistory.empty())
        {
            DrawText(renderer, font, "Sin historial aun", 750, 400);
        }
        else
        {
            int startY = 250;
            int lineHeight = 60;
            int maxLines = 10;

            for (int i = 0; i < maxLines && i < (int)gameHistory.size(); i++)
            {
                const auto& record = gameHistory[i];

                string line = record.fecha + " " + record.hora + " | " +
                    record.dificultad + " | " + record.resultado + " | " +
                    to_string(record.puntajeJugador) + "-" + to_string(record.puntajeIA);

                SDL_Color textColor = { 255, 255, 255, 255 };
                if (i == historialOption)
                    textColor = { 255, 200, 0, 255 };

                SDL_Surface* surface =
                    TTF_RenderText_Solid(font, line.c_str(), textColor);
                SDL_Texture* texture =
                    SDL_CreateTextureFromSurface(renderer, surface);

                SDL_Rect rect;
                rect.x = 200;
                rect.y = startY + i * lineHeight;
                rect.w = surface->w;
                rect.h = surface->h;

                SDL_RenderCopy(renderer, texture, NULL, &rect);

                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            }
        }

        DrawText(renderer, font, "W/S - Navegar | ESC - Volver", 500, 950);

        SDL_RenderPresent(renderer);
        return;
    }

    // PAUSA
    if (gameState == PAUSED)
    {
        if (difficulty == 3)
            SDL_RenderCopy(renderer, golfBackgroundTexture, NULL, NULL);
        else
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        if (difficulty == 3)
        {
            SDL_RenderCopy(renderer, adrianBlueTexture, NULL, &leftPaddle);
            SDL_RenderCopy(renderer, adrianRedTexture, NULL, &rightPaddle);
        }
        else
        {
            SDL_RenderCopy(renderer, leftPaddleTexture, NULL, &leftPaddle);
            SDL_RenderCopy(renderer, rightPaddleTexture, NULL, &rightPaddle);
        }

        SDL_RenderCopy(renderer, ballTexture, NULL, &ball);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);

        SDL_Rect overlay = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &overlay);

        DrawText(renderer, font, "PAUSA", 800, 200);

        if (!pauseSettings)
        {
            DrawText(renderer, font, "Continuar", 750, 400);
            DrawText(renderer, font, "Opciones", 750, 500);
            DrawText(renderer, font, "Menu Principal", 750, 600);

            if (pauseOption == 0) DrawText(renderer, font, ">", 680, 400);
            if (pauseOption == 1) DrawText(renderer, font, ">", 680, 500);
            if (pauseOption == 2) DrawText(renderer, font, ">", 680, 600);
        }
        else
        {
            DrawText(renderer, font, "OPCIONES", 700, 250);
            DrawText(renderer, font,
                "Musica: " + to_string(musicVolume) + "%", 650, 400);
            DrawText(renderer, font,
                "Efectos: " + to_string(sfxVolume) + "%", 650, 480);
            DrawText(renderer, font, "Volver", 650, 560);

            if (pauseSettingsOption == 0) DrawText(renderer, font, ">", 580, 400);
            if (pauseSettingsOption == 1) DrawText(renderer, font, ">", 580, 480);
            if (pauseSettingsOption == 2) DrawText(renderer, font, ">", 580, 560);
        }

        SDL_RenderPresent(renderer);
        return;
    }

    // SETTINGS
    if (gameState == SETTINGS)
    {
        SDL_RenderCopy(renderer, menuTexture, NULL, NULL);

        DrawText(renderer, font, "CONFIGURACION", 650, 200);

        string difficultyText;
        switch (difficulty)
        {
        case 0: difficultyText = "Facil"; break;
        case 1: difficultyText = "Normal"; break;
        case 2: difficultyText = "Dificil"; break;
        case 3: difficultyText = "ADRIAN"; break;
        }

        DrawText(renderer, font, "Dificultad: " + difficultyText, 650, 380);
        DrawText(renderer, font, "Musica: " + to_string(musicVolume) + "%", 650, 460);
        DrawText(renderer, font, "Efectos: " + to_string(sfxVolume) + "%", 650, 540);
        DrawText(renderer, font, "Volver", 650, 620);

        if (settingsOption == 0) DrawText(renderer, font, ">", 580, 380);
        if (settingsOption == 1) DrawText(renderer, font, ">", 580, 460);
        if (settingsOption == 2) DrawText(renderer, font, ">", 580, 540);
        if (settingsOption == 3) DrawText(renderer, font, ">", 580, 620);

        SDL_RenderPresent(renderer);
        return;
    }

    // MENU
    if (gameState == MENU)
    {
        SDL_RenderCopy(renderer, menuTexture, NULL, NULL);

        DrawText(renderer, font, "PLAY", 850, 350);
        DrawText(renderer, font, "HISTORIAL", 700, 450);
        DrawText(renderer, font, "CONFIGURACION", 700, 550);
        DrawText(renderer, font, "SALIR", 850, 650);

        if (selectedOption == 0) DrawText(renderer, font, ">", 800, 350);
        if (selectedOption == 1) DrawText(renderer, font, ">", 600, 450);
        if (selectedOption == 2) DrawText(renderer, font, ">", 600, 550);
        if (selectedOption == 3) DrawText(renderer, font, ">", 600, 650);

        SDL_RenderPresent(renderer);
        return;
    }

    // PLAYING
    if (difficulty == 3)
        SDL_RenderCopy(renderer, golfBackgroundTexture, NULL, NULL);
    else
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    if (difficulty == 3)
    {
        SDL_RenderCopy(renderer, adrianBlueTexture, NULL, &leftPaddle);
        SDL_RenderCopy(renderer, adrianRedTexture, NULL, &rightPaddle);
    }
    else
    {
        SDL_RenderCopy(renderer, leftPaddleTexture, NULL, &leftPaddle);
        SDL_RenderCopy(renderer, rightPaddleTexture, NULL, &rightPaddle);
    }

    SDL_RenderCopy(renderer, ballTexture, NULL, &ball);

    Uint32 elapsedTime = (SDL_GetTicks() - startTime) / 1000;
    int remainingTime = GAME_TIME - elapsedTime;
    if (remainingTime < 0) remainingTime = 0;

    DrawText(renderer, font,
        to_string(playerScore),
        SCOREBOARD_X + 100,
        SCOREBOARD_Y + 40);

    DrawText(renderer, font,
        to_string(remainingTime),
        SCOREBOARD_X + 250,
        SCOREBOARD_Y + 25);

    DrawText(renderer, font,
        to_string(aiScore),
        SCOREBOARD_X + 425,
        SCOREBOARD_Y + 40);

    SDL_Rect powerBar;
    powerBar.x = 50;
    powerBar.y = 50;
    powerBar.w = powerMeter * 3;
    powerBar.h = 20;

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &powerBar);

    if (smashReady)
        DrawText(renderer, font, "SMASH LISTO!", 50, 80);

    if (chadrianMode)
        DrawText(renderer, font, "ES UN INSULTO AL GOLF", 700, 120);

    SDL_RenderPresent(renderer);
}

// =====================
// MAIN
// =====================
int main(int argc, char* argv[])
{
    srand((unsigned int)time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        cout << "ERROR: SDL_Init fallido: " << SDL_GetError() << endl;
        return -1;
    }

    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    if (TTF_Init() == -1)
    {
        cout << "ERROR: TTF_Init fallido: " << TTF_GetError() << endl;
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        cout << "ERROR: Mix_OpenAudio fallido: " << Mix_GetError() << endl;
        return -1;
    }

    Mix_AllocateChannels(8);

    SDL_Window* window = SDL_CreateWindow(
        "Hockey Pong",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        cout << "ERROR: No se pudo crear ventana: " << SDL_GetError() << endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer)
    {
        cout << "ERROR: No se pudo crear renderer: " << SDL_GetError() << endl;
        return -1;
    }

    // CARGAR IMAGENES
    cout << "\n=== CARGANDO IMAGENES ===" << endl;

    SDL_Surface* backgroundSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/fondo.png");
    if (backgroundSurface)
    {
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);
        cout << "OK: fondo.png cargado" << endl;
    }

    SDL_Surface* golfSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/fondoA.png");
    if (golfSurface)
    {
        golfBackgroundTexture = SDL_CreateTextureFromSurface(renderer, golfSurface);
        SDL_FreeSurface(golfSurface);
        cout << "OK: fondoA.png cargado" << endl;
    }

    SDL_Surface* adrianBlueSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/tomy.png");
    if (adrianBlueSurface)
    {
        adrianBlueTexture = SDL_CreateTextureFromSurface(renderer, adrianBlueSurface);
        SDL_FreeSurface(adrianBlueSurface);
        cout << "OK: tomy.png cargado" << endl;
    }

    SDL_Surface* adrianRedSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/adrian.png");
    if (adrianRedSurface)
    {
        adrianRedTexture = SDL_CreateTextureFromSurface(renderer, adrianRedSurface);
        SDL_FreeSurface(adrianRedSurface);
        cout << "OK: adrian.png cargado" << endl;
    }

    SDL_Surface* menuSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/menu.png");
    if (menuSurface)
    {
        menuTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);
        SDL_FreeSurface(menuSurface);
        cout << "OK: menu.png cargado" << endl;
    }

    SDL_Surface* leftSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/paletaA.png");
    if (leftSurface)
    {
        leftPaddleTexture = SDL_CreateTextureFromSurface(renderer, leftSurface);
        SDL_FreeSurface(leftSurface);
        cout << "OK: paletaA.png cargado" << endl;
    }

    SDL_Surface* rightSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/paletaN.png");
    if (rightSurface)
    {
        rightPaddleTexture = SDL_CreateTextureFromSurface(renderer, rightSurface);
        SDL_FreeSurface(rightSurface);
        cout << "OK: paletaN.png cargado" << endl;
    }

    SDL_Surface* ballSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/pelota.png");
    if (ballSurface)
    {
        ballTexture = SDL_CreateTextureFromSurface(renderer, ballSurface);
        SDL_FreeSurface(ballSurface);
        cout << "OK: pelota.png cargado" << endl;
    }

    // Texturas de resultado
    SDL_Surface* victorySurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/victoria.png");
    if (victorySurface)
    {
        victoryTexture = SDL_CreateTextureFromSurface(renderer, victorySurface);
        SDL_FreeSurface(victorySurface);
        cout << "OK: victoria.png cargado" << endl;
    }

    SDL_Surface* defeatSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/derrota.png");
    if (defeatSurface)
    {
        defeatTexture = SDL_CreateTextureFromSurface(renderer, defeatSurface);
        SDL_FreeSurface(defeatSurface);
        cout << "OK: derrota.png cargado" << endl;
    }

    SDL_Surface* drawSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/empate.png");
    if (drawSurface)
    {
        drawTexture = SDL_CreateTextureFromSurface(renderer, drawSurface);
        SDL_FreeSurface(drawSurface);
        cout << "OK: empate.png cargado" << endl;
    }

    // Texturas ADRIAN
    SDL_Surface* victoryGolfSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/victoriagolf.png");
    if (victoryGolfSurface)
    {
        victoryGolfTexture = SDL_CreateTextureFromSurface(renderer, victoryGolfSurface);
        SDL_FreeSurface(victoryGolfSurface);
        cout << "OK: victoriagolf.png cargado" << endl;
    }

    SDL_Surface* defeatGolfSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/derrotagolf.png");
    if (defeatGolfSurface)
    {
        defeatGolfTexture = SDL_CreateTextureFromSurface(renderer, defeatGolfSurface);
        SDL_FreeSurface(defeatGolfSurface);
        cout << "OK: derrotagolf.png cargado" << endl;
    }

    SDL_Surface* drawGolfSurface =
        IMG_Load("C:/Users/maxim/source/repos/CHADPONG/assets/images/empategolf.png");
    if (drawGolfSurface)
    {
        drawGolfTexture = SDL_CreateTextureFromSurface(renderer, drawGolfSurface);
        SDL_FreeSurface(drawGolfSurface);
        cout << "OK: empategolf.png cargado" << endl;
    }

    TTF_Font* font =
        TTF_OpenFont(
            "C:/Users/maxim/source/repos/CHADPONG/assets/ttf/arial.ttf",
            60);

    if (!font)
    {
        cout << "ERROR: No se pudo cargar arial.ttf: " << TTF_GetError() << endl;
        return -1;
    }

    cout << "\n=== CARGANDO AUDIO ===" << endl;
    LoadAudio();

    cout << "\n=== INICIANDO JUEGO ===" << endl;
    LoadGameHistory();

    UpdateVolume();

    if (backgroundMusic)
    {
        Mix_PlayMusic(backgroundMusic, -1);
        cout << "OK: Musica de fondo iniciada" << endl;
    }

    ResetPhraseTimer();

    bool running = true;
    while (running)
    {
        HandleInput(running);
        Update();
        Render(renderer, font);

        SDL_Delay(16);
    }

    cout << "\n=== LIMPIANDO RECURSOS ===" << endl;

    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    FreeAudio();
    Mix_CloseAudio();
    Mix_Quit();

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(ballTexture);
    SDL_DestroyTexture(leftPaddleTexture);
    SDL_DestroyTexture(rightPaddleTexture);
    SDL_DestroyTexture(golfBackgroundTexture);
    SDL_DestroyTexture(adrianBlueTexture);
    SDL_DestroyTexture(adrianRedTexture);
    SDL_DestroyTexture(menuTexture);
    SDL_DestroyTexture(victoryTexture);
    SDL_DestroyTexture(defeatTexture);
    SDL_DestroyTexture(drawTexture);
    SDL_DestroyTexture(victoryGolfTexture);
    SDL_DestroyTexture(defeatGolfTexture);
    SDL_DestroyTexture(drawGolfTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    cout << "OK: Programa finalizado" << endl;

    return 0;
}
