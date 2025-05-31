#include "brute-force.h"
#include "generalFunctions.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <unistd.h>
#include <dirent.h>

// Recursos
static Texture2D wallpaper;
static Texture2D background;
static Texture2D terminalIcon;
static Texture2D geminiIcon;
static Texture2D firefoxIcon; // << NOVO!
static Texture2D folderIcon;
static Font geminiFont;

// Fade inicial
static float fadeTimer = 0.0f;
static float fadeDuration = 3.0f;
static float fadePause = 1.0f;
static const float esperaPreta = 2.0f;

// Geral
static bool showBackground = false;
static bool bootSoundPlayed = false;
static bool terminalChamado = false;
static bool fase_concluida = false;
static float tempoEspera = 0.0f;
static bool esperaCompleta = false;

static Rectangle folderBounds;
static bool iniciandoTransicao = false;
static float tempoFadeOut = 0.0f;
static float tempoAposFade = 0.0f;

// static bool iniciandoTransicao = false;
// static float tempoFadeOut = 0.0f;
// static float tempoAposFade = 0.0f;
// GEMINI DIREITO INFERIOR (animado)
static Vector2 geminiFinalPos_infd;
static Vector2 geminiAnimPos_infd;
static bool geminiAnimDone_infd = false;
static bool geminiAnimStarted_infd = false;
static float geminiAnimCooldown_infd = 1.0f;
static float geminiAnimTimer_infd = 0.0f;
static bool mostrarCaixaDialogo_infd = false;
static float tempoPosAnimacao_infd = 0.0f;
static float delayCaixaDialogo_infd = 1.0f;
static int estadoCaixa_infd = 0;
static float tempoCaixaDialogo_infd = 0.0f;
static const float trocaMensagemDelay_infd = 3.0f;

// FIREFOX SUPERIOR ESQUERDO (estático)
static const float firefoxSideScale_supesq = 0.12f;
static Vector2 firefoxPos_supesq;
static bool mostrarCaixaDialogo_supesq = false;
static int estadoCaixa_supesq = 0;
static float tempoCaixaDialogo_supesq = 0.0f;
static const float trocaMensagemDelay_supesq = 3.0f;

// Mensagem final
static float tempoMensagemFinal = 0.0f;
static bool aguardandoMensagemFinal = false;

// ==== FUNÇÃO AUXILIAR PARA DRAW CAIXA DIALOGO ====
static void DrawDialogBox(Font font, const char *texto, Vector2 anchor, int alignRight, int fontSize, int padding, Color bg, Color fg)
{
    Vector2 textSize = MeasureTextEx(font, texto, fontSize, 1);
    int largura = (int)textSize.x + padding * 2;
    int altura = (int)textSize.y + padding * 2;
    int x = anchor.x;
    if (alignRight)
        x -= largura + 20; // direita
    int y = anchor.y - altura / 2;
    DrawRectangleRounded((Rectangle){x, y, largura, altura}, 0.3f, 16, bg);
    DrawTextEx(font, texto, (Vector2){x + padding, y + padding}, fontSize, 1, fg);
}

void Init_BruteForce(void)
{
    wallpaper = LoadTexture("src/sprites/os/wallpaper.png");
    background = LoadTexture("src/sprites/os/background.jpg");
    terminalIcon = LoadTexture("src/sprites/os/terminal_icon.png");
    geminiIcon = LoadTexture("src/sprites/os/gemini.png");
    firefoxIcon = LoadTexture("src/sprites/os/firefox.png"); // << NOVO!
    folderIcon = LoadTexture("src/sprites/os/folder.png");
    geminiFont = LoadFont("src/fonts/GoogleSansMono.ttf");

    fadeTimer = 0.0f;
    showBackground = false;
    bootSoundPlayed = true;

    // Gemini inferior direito (animado)
    float geminiAnimScale_infd = 1.0f / 13.5f;
    geminiFinalPos_infd = (Vector2){
        GetScreenWidth() - geminiIcon.width * geminiAnimScale_infd - 20,
        GetScreenHeight() - (geminiIcon.height * geminiAnimScale_infd - 20) - 100};
    geminiAnimPos_infd = (Vector2){GetScreenWidth(), geminiFinalPos_infd.y};
    geminiAnimDone_infd = false;
    geminiAnimStarted_infd = false;
    geminiAnimTimer_infd = 0.0f;
    mostrarCaixaDialogo_infd = false;
    tempoPosAnimacao_infd = 0.0f;
    tempoCaixaDialogo_infd = 0.0f;
    estadoCaixa_infd = 0;

    // Firefox superior esquerdo
    int iconMargin = 10;
    float yTerminal = iconMargin;
    float yFirefox = yTerminal + terminalIcon.height * 1.3f + 8;
    firefoxPos_supesq = (Vector2){iconMargin + 1, yFirefox};
    mostrarCaixaDialogo_supesq = false;
    estadoCaixa_supesq = 0;
    tempoCaixaDialogo_supesq = 0.0f;

    fase_concluida = false;
    iniciandoTransicao = false;
    tempoFadeOut = 0.0f;
    tempoAposFade = 0.0f;
    terminalChamado = false;
    aguardandoMensagemFinal = false;
}

void Update_BruteForce(void)
{
    float dt = GetFrameTime();

    if (IsKeyPressed(KEY_KP_SUBTRACT))
    {
        fase_concluida = true;
    }
    

    // Fade inicial
    fadeTimer += dt;
    if (!showBackground && fadeTimer >= (fadeDuration + fadePause))
        showBackground = true;

    // Gemini INFERIOR DIREITO (animado/aparece depois do fade)
    if (bootSoundPlayed && !geminiAnimStarted_infd)
    {
        geminiAnimTimer_infd += dt;
        if (geminiAnimTimer_infd >= geminiAnimCooldown_infd)
            geminiAnimStarted_infd = true;
    }
    if (geminiAnimStarted_infd && !geminiAnimDone_infd)
    {
        float speed = 600.0f * dt;
        if (geminiAnimPos_infd.x > geminiFinalPos_infd.x)
        {
            geminiAnimPos_infd.x -= speed;
            if (geminiAnimPos_infd.x < geminiFinalPos_infd.x)
                geminiAnimPos_infd.x = geminiFinalPos_infd.x;
        }
        else
        {
            geminiAnimDone_infd = true;
            tempoPosAnimacao_infd = 0.0f;
        }
    }
    if (geminiAnimDone_infd && !mostrarCaixaDialogo_infd)
    {
        tempoPosAnimacao_infd += dt;
        if (tempoPosAnimacao_infd >= delayCaixaDialogo_infd)
            mostrarCaixaDialogo_infd = true;
    }
    if (mostrarCaixaDialogo_infd && estadoCaixa_infd == 0)
    {
        tempoCaixaDialogo_infd += dt;
        if (tempoCaixaDialogo_infd >= trocaMensagemDelay_infd)
            estadoCaixa_infd = 1;
    }

    // Firefox SUPERIOR ESQUERDO lógica do dialogo (se/quando usar)
    // Ligar mostrarCaixaDialogo_supesq conforme necessário
    if (mostrarCaixaDialogo_supesq && estadoCaixa_supesq == 0)
    {
        tempoCaixaDialogo_supesq += dt;
        if (tempoCaixaDialogo_supesq >= trocaMensagemDelay_supesq)
            estadoCaixa_supesq = 1;
    }

    // Clique Abrir Pasta
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, folderBounds))
        {
            const char *exeDir = GetApplicationDirectory();
            char fullPath[512];
            snprintf(fullPath, sizeof(fullPath), "%s..\\hackingFiles", exeDir);
            char command[600];
            snprintf(command, sizeof(command), "explorer \"%s\"", fullPath);
            system(command);
        }
    }

    // Clique Terminal
    if (showBackground && !terminalChamado && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse = GetMousePosition();
        Rectangle terminalIconBounds = {10, 10, terminalIcon.width * 1.5f, terminalIcon.height * 2.0f};
        if (CheckCollisionPointRec(mouse, terminalIconBounds))
        {
            char cwd[512];
            if (_getcwd(cwd, sizeof(cwd)) != NULL)
            {
                char command[1024];
                snprintf(command, sizeof(command),
                         "start \"\" \"%s\\bruteForce_terminal.bat\"", cwd);
                system(command);
                terminalChamado = true;
            }
        }
    }

    if (showBackground && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse = GetMousePosition();
        Rectangle firefoxIconBounds = {firefoxPos_supesq.x, firefoxPos_supesq.y, firefoxIcon.width * firefoxSideScale_supesq, firefoxIcon.height * firefoxSideScale_supesq};
        if (CheckCollisionPointRec(mouse, firefoxIconBounds))
        {
            system("start https://cybertechinc.vercel.app/");
        }
    }

    // Final/fase concluída/transição
    if (iniciandoTransicao)
    {
        tempoFadeOut += dt;
        if (tempoFadeOut >= 1.0f)
        {
            tempoAposFade += dt;
            if (tempoAposFade >= esperaPreta)
            {
                fase_concluida = true;
            }
        }
    }

    // Checar "dadosBruteForce.txt" e iniciar mensagem final
    DIR *d = opendir(".");
    struct dirent *dir;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if ((strcmp(dir->d_name, "dadosBruteForce.txt") == 0))
            {
                remove("dadosBruteForce.txt");
                estadoCaixa_infd = 2;
                tempoMensagemFinal = 0.0f;
                aguardandoMensagemFinal = true;
                iniciandoTransicao = true;
                tempoFadeOut = 0.0f;
                break;
            }
        }
        closedir(d);
    }

    if (iniciandoTransicao)
    {
        // Verificar se a espera já terminou
        if (!esperaCompleta)
        {
            // Incrementa o tempo de espera
            tempoEspera += GetFrameTime();

            // Verifica se os 4 segundos já passaram
            if (tempoEspera >= 4.0f)
            {
                esperaCompleta = true;
            }
        }
        else
        {
            // Após a espera, inicia o fade-out
            tempoFadeOut += GetFrameTime();
            float alpha = (tempoFadeOut < 1.0f) ? tempoFadeOut / 1.0f : 1.0f;
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          (Color){0, 0, 0, (unsigned char)(alpha * 255)});
        }
    }
}

void Draw_BruteForce(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

    // Fundo (fade/wallpaper)
    if (showBackground)
    {
        DrawTexturePro(background, (Rectangle){0, 0, background.width, background.height},
                       (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, (Vector2){0, 0}, 0.0f, WHITE);
    }
    else
    {
        float t = fmodf(fadeTimer, fadeDuration + fadePause);
        float alpha = (t < fadeDuration) ? fabsf(sinf(t / fadeDuration * PI)) : 0.0f;
        Vector2 pos = {(GetScreenWidth() - wallpaper.width * 0.3f) / 2, (GetScreenHeight() - wallpaper.height * 0.3f) / 2};
        DrawTextureEx(wallpaper, pos, 0.0f, 0.3f, (Color){255, 255, 255, (unsigned char)(alpha * 255)});
    }

    // Barra lateral (semi-transp)
    DrawRectangle(0, 0, 80, GetScreenHeight(), (Color){0, 0, 0, 76});

    if (showBackground)
    {
        int iconMargin = 10;
        float terminalScale = 1.3f;
        float folderScale = 0.12f;
        // ícones LADO ESQUERDO:
        float yTerminal = iconMargin;
        float yFirefox = yTerminal + terminalIcon.height * terminalScale + 8;
        float yFolder = yFirefox + firefoxIcon.height * firefoxSideScale_supesq + 8;
        float folderWidth = folderIcon.width * folderScale;
        float folderHeight = folderIcon.height * folderScale;
        folderBounds = (Rectangle){iconMargin + 1, yFolder, folderWidth, folderHeight};
        // Ícones:
        DrawTextureEx(terminalIcon, (Vector2){iconMargin, yTerminal}, 0.0f, terminalScale, WHITE);
        DrawTextureEx(firefoxIcon, firefoxPos_supesq, 0.0f, firefoxSideScale_supesq, WHITE); // << TROCA PARA FIREFOX!
        DrawTextureEx(folderIcon, (Vector2){folderBounds.x, folderBounds.y}, 0.0f, folderScale, WHITE);

        // Gemini INFERIOR DIREITO (animado)
        float geminiAnimScale_infd = 1.0f / 13.5f;
        if (geminiAnimStarted_infd)
            DrawTextureEx(geminiIcon, geminiAnimPos_infd, 0.0f, geminiAnimScale_infd, WHITE);

        // CAIXA DE DIÁLOGO - GEMINI INFERIOR DIREITO
        if (mostrarCaixaDialogo_infd)
        {
            const char *texto;
            if (estadoCaixa_infd == 0)
                texto = "Nova tarefa detectada: Brute Force";
            else if (estadoCaixa_infd == 1)
                texto = "Clique na pasta e use as informacoes dela para inserir os dados no terminal.";
            else
                texto = "Missao concluida! :)";
            Vector2 anchor_infd = (Vector2){geminiFinalPos_infd.x, geminiFinalPos_infd.y + 35};
            int fontSize = 18;
            int padding = 20;
            DrawDialogBox(geminiFont, texto, anchor_infd, 1, fontSize, padding, WHITE, DARKGRAY);
        }

        // CAIXA DE DIÁLOGO - FIREFOX SUPERIOR ESQUERDO (se/quando quiser ativar)
        if (mostrarCaixaDialogo_supesq)
        {
            const char *texto;
            if (estadoCaixa_supesq == 0)
                texto = "Sou o Firefox do painel!";
            else
                texto = "Outra mensagem lateral...";
            // Caixinha ao lado do ícone lateral
            Vector2 anchor_supesq = (Vector2){firefoxPos_supesq.x + firefoxIcon.width * firefoxSideScale_supesq + 12,
                                              firefoxPos_supesq.y + (firefoxIcon.height * firefoxSideScale_supesq) / 2};
            int fontSize = 16;
            int padding = 16;
            DrawDialogBox(geminiFont, texto, anchor_supesq, 0, fontSize, padding, WHITE, DARKGRAY);
        }
    }

    if (iniciandoTransicao)
    {
        float alpha = (tempoFadeOut < 1.0f) ? tempoFadeOut / 1.0f : 1.0f;
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      (Color){0, 0, 0, (unsigned char)(alpha * 255)});
    }

    EndDrawing();
}

bool Fase_BruteForce_Concluida(void)
{
    return fase_concluida;
}

void Unload_BruteForce(void)
{
    UnloadTexture(wallpaper);
    UnloadTexture(background);
    UnloadTexture(terminalIcon);
    UnloadTexture(geminiIcon);
    UnloadTexture(firefoxIcon); // <- NOVO!
    UnloadTexture(folderIcon);
    UnloadFont(geminiFont);
}