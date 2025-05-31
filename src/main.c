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

AppState state = APP_CUTSCENES;
AppState PFP_Iterrogatorio;
AppState PFP_Loading;
AppState PFP_Trasicao;
AppState PFP_Loading_Iterrogatorio;

PlayerStats playerStats;

typedef struct { const char *audio; const char *texto; } RoteiroHank;
static const RoteiroHank roteiros[] = {
    {
        "src/music/fala_hank_desafio01.mp3",
        "Muito bem! Você mostrou reflexos afiados e domínio da situação. "
        "Conseguiu destruir os blocos certos sem ser pego pelos antivírus, "
        "o que demonstra atenção aos detalhes e controle sob pressão. "
        "É exatamente esse tipo de sangue frio que precisamos por aqui. [ENTER]"
    },
    { 
        "src/music/fala_hank_desafio02.mp3",
        "Muito bem! Você demonstrou inteligência ao reconhecer rapidamente que o texto estava "
        "cifrado e perspicácia ao identificar a técnica correta para decifrá-lo. Essa habilidade "
        "em lidar com criptografia é essencial para proteger a integridade dos dados. [ENTER]"
    },
    { 
        "src/music/fala_hank_desafio03.mp3",
        "Impressionante! Você conseguiu identificar rapidamente as prioridades do sistema "
        "e realocar os recursos com precisão, mantendo a calma mesmo sob pressão. "
        "Certamente uma habilidade essencial para lidar com situações críticas. [ENTER]"
    },
    { 
        "src/music/fala_hank_desafio04.mp3",
        "Muito bom! Você soube coletar os antivírus enquanto evitava os vírus, mostrando que entende "
        "a importância de manter o sistema protegido. Essa postura preventiva é essencial [ENTER]"
    }
};

int main(void)
{
    srand(time(NULL));
    int screenWidth = GetMonitorWidth(0);
    int screenHeight = GetMonitorHeight(0);

    InitWindow(screenWidth, screenHeight, "Through The Firewall");
    SetTargetFPS(800);
    SetWindowPosition(0, 0); 

    InitAudioDevice();
    Music music = LoadMusicStream("src/music/musica.mp3");
    SetMusicVolume(music, 0.9f);
    PlayMusicStream(music);

    SelecionarPerguntasAleatorias();
    InitPlayerStats(&playerStats);

    static int perguntaAtual = -1;
    static bool firewall_Initialized = false;
    static bool bruteforce_Initialized = false;
    static bool desafio_03_Initialized = false;
    static bool ligacao_desconhecido_Initialized = false;
    static bool porta_batendo_Initialized = false;
    static bool desafio_01_Initialized = false;
    static bool fase4Initialized = false;
    static bool shellUbuntu_Initialized = false;
    static bool desafio_04_Initialized = false;
    static bool desafio_02_Initialized = false;
    static bool interrogatorio_Initialized = false;
    static bool pendrive_Initialized = false;
    static bool proxy3D_Initialized = false;
    static bool keyloggerUbuntu_Initialized = false;
    static bool proxyUbuntu_Initialized = false;
    static bool keylogger3D_Initialized = false;
    static bool shell3D_01_Initialized = false;
    static bool shell3D_02_Initialized = false;
    static bool shellBox_Initialized = false;
    static bool finalJogo_Initialized = false;
    static bool loading_Initialized = false;
    static bool transicao_Initialized = false;
    static bool ranking_Initialized = false;
    static bool menu_Initialized = false;
    static bool intro_Initialized = false;
    static bool cutscene_Initialized = false;
    static bool debug_Initialized = false;
    static bool transicao_proxy_Initialized = false;
    static bool transicao_proxy2_Initialized = false;

    // DEBUG DE SELEÇÃO DO NOME
    strncpy(gSelectedCharacterName, "Dante", MAX_PLAYER_NAME);
    gSelectedCharacterName[MAX_PLAYER_NAME-1] = '\0';
    strncpy(gPlayerName, "Carlos Eduardo Esposito", MAX_PLAYER_NAME);
    gPlayerName[MAX_PLAYER_NAME-1] = '\0';
    playerStats.isPassouSelecao = false;
    // FIM DO DEBUG DE NOME

    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);

        if (state == APP_CUTSCENES)
        {
            if (!cutscene_Initialized)
            {
                InitCutscenes();
                cutscene_Initialized = true;
            }
            UpdateCutscenes();
            DrawCutscenes();
            if (CutscenesEnded())
            {
                UnloadCutscenes();
                cutscene_Initialized = false;
                state = APP_MENU;
            }
        }
        else if (state == APP_MENU)
        {
            if (!menu_Initialized)
            {
                InitMenu();
                menu_Initialized = true;
                ResumeMusicStream(music);
            }
            UpdateMenu();
            DrawMenu();
            if (MenuShowRanking())
            {
                UnloadMenu();
                menu_Initialized = false;
                ranking_Initialized = false;
                state = APP_RANKING;
            }
            if (MenuStartGame())
            {
                PauseMusicStream(music);
                UnloadMenu();
                menu_Initialized = false;
                state = APP_LOADING_SCREEN;
                PFP_Loading = APP_INTRO;
            }
            // DEBUG KEYS
            if (IsKeyPressed(KEY_ONE))
            {
                PauseMusicStream(music);
                UnloadMenu();
                desafio_01_Initialized = false;
                state = APP_DESAFIO_01;
            }
            if (IsKeyPressed(KEY_TWO))
            {
                PauseMusicStream(music);
                UnloadMenu();
                desafio_02_Initialized = false;
                state = APP_DESAFIO_02;
            }
            if (IsKeyPressed(KEY_THREE))
            {
                PauseMusicStream(music);
                UnloadMenu();
                desafio_03_Initialized = false;
                state = APP_DESAFIO_03;
            }
            if (IsKeyPressed(KEY_FOUR))
            {
                PauseMusicStream(music);
                UnloadMenu();
                desafio_04_Initialized = false;
                state = APP_DESAFIO_04;
            }
            if (IsKeyPressed(KEY_O))
            {
                PauseMusicStream(music);
                UnloadMenu();
                pendrive_Initialized = false;
                state = APP_PENDRIVE;
            }
            if (IsKeyPressed(KEY_K))
            {
                PauseMusicStream(music);
                UnloadMenu();
                fase4Initialized = false;
                state = APP_FASE4;
            }
            if (IsKeyPressed(KEY_L))
            {
                PauseMusicStream(music);
                UnloadMenu();
                shellUbuntu_Initialized = false;
                state = APP_SHELL_UBUNTU;
            }
            if (IsKeyPressed(KEY_Z))
            {
                PauseMusicStream(music);
                UnloadMenu();
                keyloggerUbuntu_Initialized = false;
                state = APP_KEYLOGGER_UBUNTU;
            }
            if (IsKeyPressed(KEY_X))
            {
                PauseMusicStream(music);
                UnloadMenu();
                proxyUbuntu_Initialized = false;
                state = APP_PROXY_UBUNTU;
            }
            if (IsKeyPressed(KEY_C))
            {
                PauseMusicStream(music);
                UnloadMenu();
                keylogger3D_Initialized = false;
                state = APP_KEYLOGGER_3D;
            }
            if (IsKeyPressed(KEY_V))
            {
                PauseMusicStream(music);
                UnloadMenu();
                shell3D_01_Initialized = false;
                state = APP_SHELL_3D_PART1;
            }
            if (IsKeyPressed(KEY_E))
            {
                PauseMusicStream(music);
                UnloadMenu();
                shell3D_02_Initialized = false;
                state = APP_SHELL_3D_PART2;
            }
            if (IsKeyPressed(KEY_J))
            {
                PauseMusicStream(music);
                UnloadMenu();
                proxy3D_Initialized = false;
                state = APP_PROXY_3D;
            }
            if (IsKeyPressed(KEY_M))
            { 
                PauseMusicStream(music);
                UnloadMenu();
                InitDebug();
                state = APP_DEBUG;
            }
            if (IsKeyPressed(KEY_T))
            {
                PauseMusicStream(music);
                UnloadMenu();
                perguntaAtual++;
                Init_Interrogatorio(-1, NULL, NULL);
                interrogatorio_Initialized = true;
                PFP_Iterrogatorio = APP_DESAFIO_01;
                state = INTERROGATORIO;
            }
            if (IsKeyPressed(KEY_B))
            {
                PauseMusicStream(music);
                UnloadMenu();
                shellBox_Initialized = false;
                state = APP_SHELL_BOX;
            }
            if (IsKeyPressed(KEY_G))
            {
                PauseMusicStream(music);
                UnloadMenu();
                finalJogo_Initialized = false;
                state = APP_FINAL_JOGO;
            }
            if (IsKeyPressed(KEY_PERIOD))
            {
                PauseMusicStream(music);
                UnloadMenu();
                transicao_proxy_Initialized = false;
                state = APP_TRANSICAO_PROXY;
            }
            if (IsKeyPressed(KEY_F))
            {
                PauseMusicStream(music);
                UnloadMenu();
                transicao_Initialized = false;
                state = APP_TRANSICAO;
            }
            // FIM DEBUG
        }
        else if (state == APP_RANKING)
        {
            if (!ranking_Initialized)
            {
                Init_Ranking();
                ranking_Initialized = true;
                ResumeMusicStream(music);
            }
            Update_Ranking();
            Draw_Ranking();
            if (Ranking_Concluido()) 
            {
                Unload_Ranking();
                ranking_Initialized = false;
                menu_Initialized = false;
                state = APP_MENU;
            }
        }
        else if (state == APP_LOADING_SCREEN)
        {
            if (!loading_Initialized)
            {
                Init_LoadingScreen(5.0f);
                loading_Initialized = true;
            }
            Update_LoadingScreen();
            Draw_LoadingScreen();
            if (LoadingScreen_Done())
            {
                Unload_LoadingScreen();
                loading_Initialized = false;
                state = PFP_Loading;
            }
        }
        else if (state == APP_INTRO)
        {
            if (!intro_Initialized)
            {
                float temposIntroShow[3] = {11.0f, 8.0f, 6.0f};
                float temposIntroEraser[3] = {1.5f, 1.5f, 2.0f};
                InitIntro(temposIntroShow, temposIntroEraser);
                intro_Initialized = true;
            }
            UpdateIntro();
            DrawIntro();
            if (IntroEnded())
            {
                UnloadIntro();
                intro_Initialized = false;
                state = APP_LIGACAO_DESCONHECIDO;
            }
        }
        else if (state == APP_LIGACAO_DESCONHECIDO)
        {
            if (!ligacao_desconhecido_Initialized)
            {
                Init_Ligacao_Desconhecido();
                ligacao_desconhecido_Initialized = true;
            }
            Update_Ligacao_Desconhecido();
            Draw_Ligacao_Desconhecido();
            if (Fase_Ligacao_Desconhecido_Concluida())
            {
                Unload_Ligacao_Desconhecido();
                ligacao_desconhecido_Initialized = false;
                state = APP_FIREWALL;
            }
        }
        else if (state == APP_FIREWALL)
        {
            if (!firewall_Initialized)
            {
                Init_Firewall();
                firewall_Initialized = true;
            }
            Update_Firewall();
            Draw_Firewall();
            if (Fase_Firewall_Concluida())
            {
                Unload_Firewall();
                firewall_Initialized = false;
                state = APP_PORTA_BATENDO;
            }
        }
        else if (state == APP_PORTA_BATENDO)
        {
            if (!porta_batendo_Initialized)
            {
                Init_Porta_Batendo();
                porta_batendo_Initialized = true;
            }
            Update_Porta_Batendo();
            Draw_Porta_Batendo();
            if (Fase_Porta_Batendo_Concluida())
            {
                Unload_Porta_Batendo();
                porta_batendo_Initialized = false;

                Init_Interrogatorio(-1, NULL, NULL);
                interrogatorio_Initialized = true;
                state = INTERROGATORIO;

                PFP_Iterrogatorio = APP_TRANSICAO;
                PFP_Trasicao = APP_DESAFIO_01;
            }
        }
        else if (state == INTERROGATORIO)
        {
            if (!interrogatorio_Initialized)
            {
                Init_Interrogatorio(perguntaAtual, roteiros[perguntaAtual].audio, roteiros[perguntaAtual].texto);
                interrogatorio_Initialized = true;
            }
            Update_Interrogatorio();
            Draw_Interrogatorio();
            if (Fase_Interrogatorio_Concluida())
            {
                Unload_Interrogatorio();
                interrogatorio_Initialized = false;
                perguntaAtual++;
                state = PFP_Iterrogatorio;
            }
        }
        else if (state == APP_TRANSICAO)
        {
            if (!transicao_Initialized)
            {
                Init_TransitionScreen(1, "PongWall");
                transicao_Initialized = true;
            }
            Update_TransitionScreen();
            Draw_TransitionScreen();
            if (TransitionScreenDone())
            {
                Unload_TransitionScreen();
                transicao_Initialized = false;
                state = PFP_Trasicao;
            }
        }
        else if (state == APP_DESAFIO_01)
        {
            if (!desafio_01_Initialized)
            {
                Init_Desafio_01();
                desafio_01_Initialized = true;
            }
            Update_Desafio_01();
            Draw_Desafio_01();
            if (Fase_Desafio_01_Concluida()) 
            {
                Unload_Desafio_01();
                desafio_01_Initialized = false;

                state = APP_LOADING_SCREEN;
                PFP_Loading = INTERROGATORIO;
                PFP_Iterrogatorio = APP_TRANSICAO_PROXY;
            }
        }
        else if (state == APP_TRANSICAO_PROXY)
        {
            if (!transicao_proxy_Initialized) 
            {
                Init_Transicao_Proxy();
                transicao_proxy_Initialized = true;
            }
            Update_Transicao_Proxy();
            Draw_Transicao_Proxy();
            if (Transicao_Proxy_Done()) 
            {
                Unload_Transicao_Proxy();
                transicao_proxy_Initialized = false;
                state = APP_PROXY_3D;
            }
        }
        else if (state == APP_PROXY_3D)
        {
            if (!proxy3D_Initialized)
            {
                Init_Proxy3D();
                proxy3D_Initialized = true;
            }
            Update_Proxy3D();
            Draw_Proxy3D();
            if (Fase_Proxy3D_Concluida()) 
            {
                Unload_Proxy3D();
                proxy3D_Initialized = false;
                state = APP_PROXY_UBUNTU;
            }
        }
        else if (state == APP_PROXY_UBUNTU)
        {
            if (!proxyUbuntu_Initialized)
            {
                Init_ProxyUbuntu();
                proxyUbuntu_Initialized = true;
            }
            Update_ProxyUbuntu();
            Draw_ProxyUbuntu();
            if (Fase_ProxyUbuntu_Concluida()) 
            {
                Unload_ProxyUbuntu();
                proxyUbuntu_Initialized = false;
                state = APP_TRANSICAO_PROXY2;
            }
        }
        else if (state == APP_TRANSICAO_PROXY2)
        {
            if (!transicao_proxy2_Initialized) {
                Init_Transicao_Proxy2();
                transicao_proxy2_Initialized = true;
            }
            Update_Transicao_Proxy2();
            Draw_Transicao_Proxy2();
            if (Transicao_Proxy2_Done()) {
                Unload_Transicao_Proxy2();
                transicao_proxy2_Initialized = false;

                Init_TransitionScreen(2, "Cifra de Cesar");
                transicao_Initialized = true;
                state = APP_TRANSICAO;
                PFP_Trasicao = APP_DESAFIO_02;
            }
        }
        else if (state == APP_DESAFIO_02)
        {
            if (!desafio_02_Initialized)
            {
                Init_Desafio_02();
                desafio_02_Initialized = true;
            }
            Update_Desafio_02();
            Draw_Desafio_02();
            if (Fase_Desafio_02_Concluida()) 
            {
                Unload_Desafio_02();
                desafio_02_Initialized = false;

                state = APP_LOADING_SCREEN;
                PFP_Loading = INTERROGATORIO;
                PFP_Iterrogatorio = APP_KEYLOGGER_3D;
            }
        }
        else if (state == APP_KEYLOGGER_3D)
        {
            if (!keylogger3D_Initialized)
            {
                Init_Keylogger3D();
                keylogger3D_Initialized = true;
            }
            Update_Keylogger3D();
            Draw_Keylogger3D();
            if (Fase_Keylogger3D_Concluida()) 
            {
                Unload_Keylogger3D();
                keylogger3D_Initialized = false;
                state = APP_KEYLOGGER_UBUNTU;
            }
        }
        else if (state == APP_KEYLOGGER_UBUNTU)
        {
            if (!keyloggerUbuntu_Initialized)
            {
                Init_KeyloggerUbuntu();
                keyloggerUbuntu_Initialized = true;
            }
            Update_KeyloggerUbuntu();
            Draw_KeyloggerUbuntu();
            if (Fase_KeyloggerUbuntu_Concluida()) 
            {
                Unload_KeyloggerUbuntu();
                keyloggerUbuntu_Initialized = false;

                Init_TransitionScreen(3, "Redirecione os sistemas criticos");
                transicao_Initialized = true;
                state = APP_TRANSICAO;
                PFP_Trasicao = APP_DESAFIO_03;
            }
        }
        else if (state == APP_DESAFIO_03)
        {
            if (!desafio_03_Initialized)
            {
                Init_Desafio_03();
                desafio_03_Initialized = true;
            }
            Update_Desafio_03();
            Draw_Desafio_03();
            if (Fase_Desafio_03_Concluida()) 
            {
                Unload_Desafio_03();
                desafio_03_Initialized = false;

                state = APP_LOADING_SCREEN;
                PFP_Loading = INTERROGATORIO;
                PFP_Iterrogatorio = APP_PENDRIVE;
            }
        }
        else if (state == APP_PENDRIVE)
        {
            if (!pendrive_Initialized)
            {
                Init_Pendrive();
                pendrive_Initialized = true;
            }
            Update_Pendrive();
            Draw_Pendrive();
            if (Fase_Pendrive_Concluida())
            {
                Unload_Pendrive();
                pendrive_Initialized = false;
                state = APP_BRUTEFORCE;
            }
        }
        else if (state == APP_BRUTEFORCE)
        {
            if (!bruteforce_Initialized)
            {
                Init_BruteForce();
                bruteforce_Initialized = true;
            }
            Update_BruteForce();
            Draw_BruteForce();
            if (Fase_BruteForce_Concluida())
            {
                Unload_BruteForce();
                bruteforce_Initialized = false;

                Init_TransitionScreen(4, "Adquira Arquivos");
                transicao_Initialized = true;
                state = APP_TRANSICAO;
                PFP_Trasicao = APP_DESAFIO_04;
            }
        }
        else if (state == APP_DESAFIO_04)
        {
            if (!desafio_04_Initialized)
            {
                Init_Desafio_04();
                desafio_04_Initialized = true;
            }
            Update_Desafio_04();
            Draw_Desafio_04();
            if (Fase_Desafio_04_Concluida()) 
            {
                Unload_Desafio_04();
                desafio_04_Initialized = false;

                state = APP_LOADING_SCREEN;
                PFP_Loading = INTERROGATORIO;
                PFP_Iterrogatorio = APP_SHELL_3D_PART1;
            }
        }
        else if (state == APP_SHELL_3D_PART1)
        {
            if (!shell3D_01_Initialized)
            {
                Init_Shell3D_01();
                shell3D_01_Initialized = true;
            }
            Update_Shell3D_01();
            Draw_Shell3D_01();
            if (Fase_Shell3D_01_Concluida()) 
            {
                Unload_Shell3D_01();
                shell3D_01_Initialized = false;
                state = APP_SHELL_BOX;
            }
        }
        else if (state == APP_SHELL_BOX)
        {
            if (!shellBox_Initialized)
            {
                Init_ShellBox();
                shellBox_Initialized = true;
            }
            Update_ShellBox();
            Draw_ShellBox();
            if (Fase_ShellBox_Concluida()) 
            {
                Unload_ShellBox();
                shellBox_Initialized = false;
                state = APP_SHELL_3D_PART2;
            }
        }
        else if (state == APP_SHELL_3D_PART2)
        {
            if (!shell3D_02_Initialized)
            {
                Init_Shell3D_02();
                shell3D_02_Initialized = true;
            }
            Update_Shell3D_02();
            Draw_Shell3D_02();
            if (Fase_Shell3D_02_Concluida())
            {
                Unload_Shell3D_02();
                shell3D_02_Initialized = false;

                SetPlayerGeneralStats(&playerStats);
                AppendPlayerToRankingFile(&playerStats, "ranking.txt");
                RaylibSleep(1.0f);
                state = APP_SHELL_UBUNTU;
            }
        }
        else if (state == APP_SHELL_UBUNTU)
        {
            if (!shellUbuntu_Initialized)
            {
                Init_ShellUbuntu();
                shellUbuntu_Initialized = true;
            }
            Update_ShellUbuntu();
            Draw_ShellUbuntu();
            if (Fase_ShellUbuntu_Concluida()) 
            {
                Unload_ShellUbuntu();
                shellUbuntu_Initialized = false;
                state = APP_FINAL_JOGO;
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

                InitPlayerStats(&playerStats);
                state = APP_RANKING;
            }
        }
        // ===== FIM DO JOGO! =====
        else if (state == APP_FASE4)
        {
            if (!fase4Initialized)
            {
                InitFase4();
                fase4Initialized = true;
            }
            UpdateFase4();
            DrawFase4();
            if (Fase4Concluida()) {
                UnloadFase4();
                fase4Initialized = false;
                PFP_Iterrogatorio = APP_SHELL_UBUNTU;
                state = INTERROGATORIO;
            }
        }
        else if (state == APP_DEBUG)
        {
            if (!debug_Initialized)
            {
                SetPlayerGeneralStats(&playerStats); 
                AppendPlayerToRankingFile(&playerStats, "ranking.txt");
                debug_Initialized = true;
            }
            UpdateDebug();
            DrawDebug();
        }
    }

    // Limpeza final (ao sair do loop)
    if (state == APP_CUTSCENES)
        UnloadCutscenes();
    else if (state == APP_MENU)
        UnloadMenu();
    else if (state == APP_INTRO)
        UnloadIntro();
    else if (state == APP_LIGACAO_DESCONHECIDO)
        Unload_Ligacao_Desconhecido();
    else if (state == APP_DESAFIO_03)
        Unload_Desafio_03();
    else if (state == APP_PORTA_BATENDO)
        Unload_Porta_Batendo();
    else if (state == APP_DESAFIO_01)
        Unload_Desafio_01();
    else if (state == APP_FASE4)
        UnloadFase4();
    else if (state == APP_SHELL_UBUNTU)
        Unload_ShellUbuntu();
    else if (state == APP_DESAFIO_04)
        Unload_Desafio_04();
    else if (state == APP_DESAFIO_02)
        Unload_Desafio_02();
    else if (state == INTERROGATORIO)
        Unload_Interrogatorio();
    else if (state == APP_PENDRIVE)
        Unload_Pendrive();
    else if (state == APP_FIREWALL)
        Unload_Firewall();
    else if (state == APP_BRUTEFORCE)
        Unload_BruteForce();
    else if (state == APP_PROXY_3D)
        Unload_Proxy3D();
    else if (state == APP_DEBUG)
        UnloadDebug();
    else if (state == APP_TRANSICAO_PROXY)
        Unload_Transicao_Proxy();
    else if (state == APP_TRANSICAO_PROXY2)
        Unload_Transicao_Proxy2();

    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}