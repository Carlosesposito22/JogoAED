/*  main_alt.c  –– Versão enxuta:
    • Executa apenas 4 rodadas de INTERROGATORIO
    • Calcula o resultado final do jogador
    • Entra em APP_DEBUG para mostrar o relatório          

#include "cutscenes.h"
#include "menu.h"
#include "intro.h"
#include "ligacao_desconhecido.h"
#include "firewall.h"
#include "porta_batendo.h"
#include "interrogatorio.h"
#include "desafio_01.h"
#include "proxy3D.h"
#include "desafio_02.h"
#include "desafio_03.h"
#include "pendrive.h"
#include "brute-force.h"
#include "desafio_04.h"
#include "gemini.h"
#include "generalFunctions.h"
#include "debug.h"
#include "fase4.h"
#include "ubuntu_provisorio.h"
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include "keyloggerUbuntu.h"
#include "proxyUbuntu.h"
#include "keylogger3D.h"
#include "shell3D_01.h"
#include "shell3D_02.h"
#include "shellBox.h"
#include "cena_final.h"
#include "loading_screen.h"
#include "transition_screen.h"
#include "playerStats.h"
#include "ranking.h"
#include "transicao_proxy.h"
#include "transicao_proxy2.h"

static AppState state          = INTERROGATORIO;
PlayerStats playerStats = {0};

typedef struct { const char *audio; const char *texto; } RoteiroHank;
static const RoteiroHank roteiros[4] = {
    { "",
      "Impressionante! ..." },
    { "",
      "Muito bem..." },
    { "",
      "Parabéns! .." },
    { "",
      "Muito bom! ..." }
};

int main(void)
{
    srand((unsigned)time(NULL));

    const int screenWidth  = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Blindspot Undercovered - Versão Interrogatório-Only");
    InitAudioDevice();
    SetTargetFPS(800);

    Music music = LoadMusicStream("");
    SetMusicVolume(music, 0.7f);
    PlayMusicStream(music);

    SelecionarPerguntasAleatorias();
    InitPlayerStats(&playerStats);

    strcpy(gPlayerName, "Carlao");
    strcpy(gSelectedCharacterName, "Alice");
    playerStats.isPassed_D01 = false;
    playerStats.isPassed_D02 = true;
    playerStats.isPassed_D03 = true;
    playerStats.isPassed_D04 = true;
    playerStats.amountOfLives_D02 = 2;
    playerStats.quantityOfIcons_D04 = 37;

    bool interrogatorioInitialized = false;
    bool debugInitialized          = false;
    bool ranking_Initialized = false;
    bool finalJogo_Initialized = false;
    int  perguntaAtual             = 0; 
    int  interrogatorioCount       = 0; 

    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);

        if (state == INTERROGATORIO)
        {
            if (!interrogatorioInitialized)
            {
                Init_Interrogatorio(perguntaAtual,
                                    roteiros[perguntaAtual].audio,
                                    roteiros[perguntaAtual].texto);
                interrogatorioInitialized = true;
            }

            Update_Interrogatorio();
            Draw_Interrogatorio();

            if (Fase_Interrogatorio_Concluida())
            {
                Unload_Interrogatorio();
                interrogatorioInitialized = false;

                interrogatorioCount++;
                perguntaAtual++;

                if (interrogatorioCount >= 4)
                {
                    SetPlayerGeneralStats(&playerStats);
                    AppendPlayerToRankingFile(&playerStats, "ranking.txt");

                    state = APP_FINAL_JOGO;
                }
            }
        }
        else if (state == APP_FINAL_JOGO)
        {
            if (!finalJogo_Initialized)
            {
                Init_FinalJogo();
                finalJogo_Initialized = true;
            }
            Update_FinalJogo();
            Draw_FinalJogo();
            if (Fase_FinalJogo_Concluida()) 
            {
                Unload_FinalJogo();
                finalJogo_Initialized = false;
                state = APP_DEBUG;
            }
        }
        else if (state == APP_DEBUG)
        {
            if (!debugInitialized)
            {
                InitDebug();
                debugInitialized = true;
            }
            UpdateDebug();
            DrawDebug();
            if (IsKeyPressed(KEY_ENTER))
            {
                state = APP_RANKING;
            }
        }
        else if (state == APP_RANKING)
        {
            if (!ranking_Initialized)
            {
                Init_Ranking();
                ranking_Initialized = true;
            }
            Update_Ranking();
            Draw_Ranking();
        }
    }

    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
    */