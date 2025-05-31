#ifndef PLAYER_STATS_H
#define PLAYER_STATS_H

#include <stdbool.h>
#include <string.h>

#define MAX_PERGUNTAS 4
#define MAX_PLAYER_NAME 26

typedef struct {
    /* Desafio 01 */
    bool isPassed_D01;
    int  duration_D01;

    /* Desafio 02 */
    bool isPassed_D02;
    int  duration_D02;
    int  amountOfLives_D02;

    /* Desafio 03 */
    bool isPassed_D03;
    int  duration_D03;

    /* Desafio 04 */
    bool  isPassed_D04;
    int   quantityOfIcons_D04;

    /* Dados gerais */
    char  playerName[MAX_PLAYER_NAME];
    char  characterName[MAX_PLAYER_NAME];
    int   aiOverallScore; 
    bool  isPassouSelecao;
    float notalGeral;
    char  relatorioGeral[8192];
} PlayerStats;

typedef struct {
    char playerName[MAX_PLAYER_NAME];
    char characterName[MAX_PLAYER_NAME];
    float notalGeral;
    bool isPassouSelecao;
} PlayerStatsDTO;

extern int  notasIA[MAX_PERGUNTAS];
extern char relatoriosIA[MAX_PERGUNTAS][512];
extern char relatorioGeralIA[8192];
extern char gPlayerName[MAX_PLAYER_NAME];
extern char gSelectedCharacterName[MAX_PLAYER_NAME];

extern PlayerStats playerStats;

void InitPlayerStats(PlayerStats *stats);
void SetD01Result(PlayerStats *stats, bool passed, int duration);
void SetD02Result(PlayerStats *stats, bool passed, int duration, int lives);
void SetD03Result(PlayerStats *stats, bool passed, int duration);
void SetD04Result(PlayerStats *stats, bool passed, int icons);

void SetPlayerGeneralStats(PlayerStats *ps);

#endif
