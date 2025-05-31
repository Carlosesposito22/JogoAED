#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H

#include <stdbool.h>
#include "raylib.h"
#include <string.h>
#include "playerStats.h"

typedef enum
{
    APP_CUTSCENES,
    APP_MENU,
    APP_LOADING_SCREEN,
    APP_INTRO,
    APP_LIGACAO_DESCONHECIDO,
    APP_FIREWALL,
    APP_PORTA_BATENDO,
    INTERROGATORIO,
    APP_TRANSICAO,
    APP_DESAFIO_01,
    APP_PROXY_3D,
    APP_TRANSICAO_PROXY,
    APP_PROXY_UBUNTU,
    APP_TRANSICAO_PROXY2,
    APP_DESAFIO_02,
    APP_KEYLOGGER_3D,
    APP_KEYLOGGER_UBUNTU,
    APP_DESAFIO_03,
    APP_PENDRIVE,
    APP_BRUTEFORCE,
    APP_DESAFIO_04,
    APP_SHELL_3D_PART1,
    APP_SHELL_BOX,
    APP_SHELL_3D_PART2,
    APP_SHELL_UBUNTU,
    APP_FINAL_JOGO,
    APP_RANKING,
    // END GAME
    APP_FASE4,
    APP_DEBUG
} AppState;

typedef struct {
    const char* text; int length; int drawnChars; float accum; float speed; bool done;
} TypeWriter;

typedef struct {
    const char* text; int length; int drawnChars; float accum; float speed; bool done;
} TypeEraser;

typedef struct {
    const char *audioWin;
    const char *textoWin;
    const char *audioLose;
    const char *textoLose;
} RoteiroHank;

void InitTypeWriter(TypeWriter* tw, const char* text, float speed);
void UpdateTypeWriter(TypeWriter* tw, float deltaTime, bool skip);
void SkipTypeWriter(TypeWriter* tw);
void InitTypeEraser(TypeEraser* te, const char* text, float speed);
void UpdateTypeEraser(TypeEraser* te, float deltaTime, bool skip);
void SkipTypeEraser(TypeEraser* te);
float UpdateFade(float dt, float duration, bool fadeIn);
void DrawChronometer(float timer_restante, float timer_total, int x, int y, int radius);
void DrawDica(float posX, float posY, const char *text);
void DrawPromptBox(Rectangle rec, Color base, const char *txt, bool pulse);
void RaylibSleep(float seconds);
bool DesafioPassou(int idx, const PlayerStats *ps);

#endif