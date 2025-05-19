#include "ubuntu_provisorio.h"
#include "generalFunctions.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <unistd.h>
static Texture2D wallpaper;
static Texture2D background;
static Texture2D terminalIcon;
static Texture2D geminiIcon;
static Texture2D folderIcon;
static Font geminiFont;

static float fadeTimer = 0.0f;
static float fadeDuration = 3.0f;
static float fadePause = 1.0f;

static bool showBackground = false;
static bool bootSoundPlayed = false;

static bool terminalChamado = false;

static Vector2 geminiFinalPos;
static Vector2 geminiAnimPos;
static bool geminiAnimDone = false;
static bool geminiAnimStarted = false;
static float geminiAnimCooldown = 1.0f;
static float geminiAnimTimer = 0.0f;

static bool mostrarCaixaDialogo = false;
static float tempoPosAnimacao = 0.0f;
static float delayCaixaDialogo = 1.0f;

static int estadoCaixa = 0;
static float tempoCaixaDialogo = 0.0f;
static const float trocaMensagemDelay = 3.0f;

static bool fase_concluida = false; 

static Rectangle folderBounds;

void Init_ShellUbuntu(void)
{
    wallpaper = LoadTexture("src/sprites/os/wallpaper.png");
    background = LoadTexture("src/sprites/os/background.jpg");
    terminalIcon = LoadTexture("src/sprites/os/terminal_icon.png");
    geminiIcon = LoadTexture("src/sprites/os/gemini.png");
    folderIcon = LoadTexture("src/sprites/os/folder.png");
    geminiFont = LoadFont("src/fonts/GoogleSansMono.ttf");

    fadeTimer = 0.0f;
    showBackground = false;
    bootSoundPlayed = true;

    float geminiAnimScale = 1.0f / 13.5f;
    geminiFinalPos = (Vector2){
        GetScreenWidth() - geminiIcon.width * geminiAnimScale - 20,
        GetScreenHeight() - geminiIcon.height * geminiAnimScale - 20};

    geminiAnimPos = (Vector2){GetScreenWidth(), geminiFinalPos.y};

    geminiAnimDone = false;
    geminiAnimStarted = false;
    geminiAnimTimer = 0.0f;
    mostrarCaixaDialogo = false;
    tempoPosAnimacao = 0.0f;
    tempoCaixaDialogo = 0.0f;
    estadoCaixa = 0;

    fase_concluida = false;
}

void Update_ShellUbuntu(void)
{
    float dt = GetFrameTime();
    fadeTimer += dt;

    if (!showBackground && fadeTimer >= (fadeDuration + fadePause))
        showBackground = true;

    if (bootSoundPlayed && !geminiAnimStarted)
    {
        geminiAnimTimer += dt;
        if (geminiAnimTimer >= geminiAnimCooldown)
            geminiAnimStarted = true;
    }

    if (geminiAnimStarted && !geminiAnimDone)
    {
        float speed = 600.0f * dt;
        if (geminiAnimPos.x > geminiFinalPos.x)
        {
            geminiAnimPos.x -= speed;
            if (geminiAnimPos.x < geminiFinalPos.x)
                geminiAnimPos.x = geminiFinalPos.x;
        }
        else
        {
            geminiAnimDone = true;
            tempoPosAnimacao = 0.0f;
        }
    }

    if (geminiAnimDone && !mostrarCaixaDialogo)
    {
        tempoPosAnimacao += dt;
        if (tempoPosAnimacao >= delayCaixaDialogo)
            mostrarCaixaDialogo = true;
    }

    if (mostrarCaixaDialogo && estadoCaixa == 0)
    {
        tempoCaixaDialogo += dt;
        if (tempoCaixaDialogo >= trocaMensagemDelay)
            estadoCaixa = 1;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, folderBounds))
        {
            const char *exeDir = GetApplicationDirectory();

            // Caminho final: [diretório do exe]/../hackingFiles
            char fullPath[512];
            snprintf(fullPath, sizeof(fullPath), "%s..\\reverseShellFiles", exeDir);

            // Log para debug
            TraceLog(LOG_INFO, "Abrindo pasta: %s", fullPath);

            // Executar sem verificar, para garantir abertura mesmo com acentos
            char command[600];
            snprintf(command, sizeof(command), "explorer \"%s\"", fullPath);
            system(command);
        }
    }

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

    if (IsKeyPressed(KEY_A))
    {
        fase_concluida = true;
    }

    // (fase_concluida = true;)
    // isso define que a fase acabou, quando tiver essa lógica
    // coloque isso, ao inves de trocar o state, Carlos o gay agradeçe!
}

void Draw_ShellUbuntu(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

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

    DrawRectangle(0, 0, 80, GetScreenHeight(), (Color){0, 0, 0, 76});

    if (showBackground)
    {
        int iconMargin = 10;
        float terminalScale = 1.3f;
        float geminiSideScale = 0.06f;

        float yTerminal = iconMargin;
        float yGemini = yTerminal + terminalIcon.height * terminalScale + 8;
        float yFolder = yGemini + geminiIcon.height * geminiSideScale + 8;

        float folderScale = 0.12f;
        float folderWidth = folderIcon.width * folderScale;
        float folderHeight = folderIcon.height * folderScale;

        folderBounds = (Rectangle){iconMargin + 1, yFolder, folderWidth, folderHeight};

        DrawTextureEx(terminalIcon, (Vector2){iconMargin, yTerminal}, 0.0f, terminalScale, WHITE);
        DrawTextureEx(geminiIcon, (Vector2){iconMargin + 1, yGemini}, 0.0f, geminiSideScale, WHITE);
        DrawTextureEx(folderIcon, (Vector2){folderBounds.x, folderBounds.y}, 0.0f, folderScale, WHITE);

        float geminiAnimScale = 1.0f / 13.5f;

        if (geminiAnimStarted)
            DrawTextureEx(geminiIcon, geminiAnimPos, 0.0f, geminiAnimScale, WHITE);

        if (mostrarCaixaDialogo)
        {
            const char *texto = (estadoCaixa == 0)
                                    ? "Tarefa final detectada."
                                    : "Clique na pasta para ver os seus arquivos.";

            int padding = 20;
            int fontSize = 18;
            Vector2 textSize = MeasureTextEx(geminiFont, texto, fontSize, 1);
            int largura = (int)textSize.x + padding * 2;
            int altura = (int)textSize.y + padding * 2;
            int x = geminiFinalPos.x - largura - 20;
            int y = geminiFinalPos.y + 5 - altura / 2;

            DrawRectangleRounded((Rectangle){x, y, largura, altura}, 0.3f, 16, WHITE);
            DrawTextEx(geminiFont, texto, (Vector2){x + padding, y + padding}, fontSize, 1, DARKGRAY);
        }
    }

    EndDrawing();
}

bool Fase_ShellUbuntu_Concluida(void)
{
    return fase_concluida;
}

void Unload_ShellUbuntu(void)
{
    UnloadTexture(wallpaper);
    UnloadTexture(background);
    UnloadTexture(terminalIcon);
    UnloadTexture(geminiIcon);
    UnloadTexture(folderIcon);
    UnloadFont(geminiFont);
}
