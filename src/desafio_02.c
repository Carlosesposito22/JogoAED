#include "desafio_02.h"

#define BUTTON_SIZE 80
#define BUTTON_SPACING 18
#define BUTTON_ROW_SPACING 16
#define NUM_BUTTON_COLS 3
#define NUM_BUTTON_ROWS 3
#define MAX_TENTATIVAS 3
#define SEQ_TOTAL_PASSOS 4
#define FADEOUT_DURACAO 0.8f
#define GEMINI_RECT_PADRAO 550
#define GEMINI_PAD_X 36
#define FASE6_CHRONO_MAX 70.0f
#define FALA_NORMAL_02 "A senha foi criptografada usando uma cifra de deslocamento.\nPara decifrá-la, devo retroceder exatamente duas posições no alfabeto para cada letra!"
#define FALA_VENCEU_02 "JOÃO! A senha está correta! Estou pronto para o próximo desafio."
#define FALA_PERDEU_02 "Não deu tempo... A criptografia me venceu. Hank vai me dispensar..."

static int quadX = 400;
static int quadY = 300;
static bool quadButtonPressed[NUM_BUTTON_ROWS][NUM_BUTTON_COLS] = {false};
static bool quadButtonHovered[NUM_BUTTON_ROWS][NUM_BUTTON_COLS] = {false};
static int passoAtual = 0;
static int tentativasFalhaCount = 0;
static bool perdeu_a_fase = false;
static bool venceu_a_fase = false;
static bool faz_fadeout = false;
static float fadeout_time = 0.0f;
static float derrota_show_timer = 0.0f;
static float vitoria_show_timer = 0.0f;
static const float RESPOSTA_MOSTRA_SEG = 3.3f;
static bool podeAvancarDerrotaOuVitoria = false;
static float blinkVitoriaTimer = 0.0f;
static float animTimer = 0.0f;
static int piscaVermelho = 0;
static float piscaVermelhoDelay = 0.0f;
static int piscaVerde = 0;
static float piscaVerdeDelay = 0.0f;
static float geminiRectW = 550;
static float geminiRectH = 0;
static int geminiTextWidth = 0;
static float geminiRectAnim = 0.0f;
static bool geminiMouseOver = false;
static bool geminiHelpClicked = false;
static float geminiAnimSpeed = 6.0f;
static bool fase_concluida = false;
static char fala_exibida[512];
static float cronometro = 0.0f;
static float cronometro_elapsed = 0.0f;
static const char *gemini_help_msg_default = "Clique aqui caso precise de ajuda!";
static const char *gemini_help_msg_ajuda = "Visualize o alfabeto completo: A B C D E F G H I J K L M N O P Q R S T U V W X Y Z.";

static char palavraDigitada[32] = "";
static int palavraLen = 0;
static const char teclaCharMap[9] = {'J', 'I', 'Y', 'A', 'O', 'T', 'P', 'Q', 'U'};

static SpriteStatus spriteStatus = SPRITE_NORMAL;
static LedEstadoCor ledStatus[8] = {LED_ESTADO_APAGADO};
static TeclasQuadrado SEQUENCIA[] = {TECLA1, TECLA5, TECLA4, TECLA5};
static TypeWriter writer;

static Texture2D fundo;
static Texture2D pergunta_img;
static Texture2D sprJoao, sprJoao2, sprJoao3;
static Texture2D sprMateus, sprMateus2, sprMateus3;
static Texture2D sprCarlos, sprCarlos2, sprCarlos3;
static Texture2D sprMamede, sprMamede2, sprMamede3;
static Texture2D sprGemini;
static Texture2D sprEnterButton;
static Texture2D heartSheet;
static Rectangle heartFullRec;
static Rectangle heartEmptyRec;

static Music musicaFase;
static bool  audioAtivo = false;

static void AtualizaTamanhoGeminiBox(void)
{
    float geminiScale = 0.1f;
    float geminiH = sprGemini.height * geminiScale;
    int txtSize = 20;
    const char *gemini_msg = !geminiHelpClicked ? gemini_help_msg_default : gemini_help_msg_ajuda;
    geminiTextWidth = MeasureText(gemini_msg, txtSize);
    float minW = GEMINI_RECT_PADRAO;
    float maxW = GetScreenWidth() - 140;
    geminiRectW = minW;
    if (geminiTextWidth + 2 * GEMINI_PAD_X > minW)
        geminiRectW = (geminiTextWidth + 2 * GEMINI_PAD_X > maxW) ? maxW : geminiTextWidth + 2 * GEMINI_PAD_X;
    geminiRectH = geminiH * 0.75f;
}

static void DrawStylizedLedColor(int cx, int cy, int raio, LedEstadoCor estado)
{
    Color apagado = (Color){50, 50, 50, 255};
    Color cor_preench = apagado;
    if (estado == LED_ESTADO_VERDE)
        cor_preench = (Color){41, 221, 61, 255};
    else if (estado == LED_ESTADO_VERMELHO)
        cor_preench = (Color){220, 40, 45, 255};
    else if (estado == LED_ESTADO_APAGADO)
        cor_preench = apagado;
    DrawCircle(cx + raio / 4, cy + raio / 2, raio * 1.08f, (Color){24, 38, 23, 65});
    DrawCircle(cx, cy, raio, (Color){10, 20, 11, 255});
    DrawCircle(cx, cy, raio * 0.93f, cor_preench);
    for (float frac = 0.90; frac > 0.22f; frac -= 0.10f)
    {
        int r = cor_preench.r + (int)(frac * 14);
        int g = cor_preench.g + (int)(frac * 11);
        int b = cor_preench.b + (int)(frac * 9);
        if (r > 255)
            r = 255;
        if (g > 255)
            g = 255;
        if (b > 255)
            b = 255;
        DrawCircle(cx, cy, raio * frac, (Color){r, g, b, 180});
    }
    DrawEllipse(cx - raio / 6, cy - raio / 3, raio * 0.48f, raio * 0.24f, (Color){226, 236, 236, 120});
    DrawCircle(cx - raio / 3, cy - raio / 2, raio * 0.11f, (Color){255, 255, 255, 190});
    DrawCircleLines(cx, cy, raio * 0.93f, (Color){44, 85, 59, 140});
}

static inline char GetLetraFromTecla(TeclasQuadrado t)
{
    return (t >= 0 && t < 9) ? teclaCharMap[t] : '?';
}

void Init_Desafio_02(void)
{
    fundo = LoadTexture("src/sprites/senhas1.png");
    pergunta_img = LoadTexture("src/sprites/pergunta3.png");
    sprJoao = LoadTexture("src/sprites/joaoSprite.png");
    sprJoao2 = LoadTexture("src/sprites/joao2.png");
    sprJoao3 = LoadTexture("src/sprites/joao3.png");
    sprMateus = LoadTexture("src/sprites/mateusSprite.png");
    sprMateus2 = LoadTexture("src/sprites/mateus2.png");
    sprMateus3 = LoadTexture("src/sprites/mateus3.png");
    sprCarlos = LoadTexture("src/sprites/carlosSprite.png");
    sprCarlos2 = LoadTexture("src/sprites/carlos2.png");
    sprCarlos3 = LoadTexture("src/sprites/carlos3.png");
    sprMamede = LoadTexture("src/sprites/mamedeSprite.png");
    sprMamede2 = LoadTexture("src/sprites/mamede2.png");
    sprMamede3 = LoadTexture("src/sprites/mamede3.png");
    sprEnterButton = LoadTexture("src/sprites/enter_button.png");
    sprGemini = LoadTexture("src/sprites/os/gemini.png");
    heartSheet = LoadTexture("src/sprites/bateria.png");
    heartFullRec = (Rectangle){930, 20, 149, 264};  // Coração cheio
    heartEmptyRec = (Rectangle){748, 21, 150, 263}; // Coração vazio
    musicaFase = LoadMusicStream("src/music/desafio-02.mp3"); 
    SetMusicVolume(musicaFase, 1.2f);
    PlayMusicStream(musicaFase);
    audioAtivo = true;

    strcpy(fala_exibida, FALA_NORMAL_02);
    InitTypeWriter(&writer, fala_exibida, 18.5f);
    AtualizaTamanhoGeminiBox();

    for (int row = 0; row < NUM_BUTTON_ROWS; row++)
    {
        for (int col = 0; col < NUM_BUTTON_COLS; col++)
        {
            quadButtonPressed[row][col] = false;
            quadButtonHovered[row][col] = false;
        }
    }
    for (int i = 0; i < 8; i++)
        ledStatus[i] = LED_ESTADO_APAGADO;

    geminiHelpClicked = false;
    geminiRectAnim = 0.0f;
    geminiMouseOver = false;
    quadX = 635;
    quadY = 367;
    passoAtual = 0;
    animTimer = 0.0f;
    piscaVermelho = 0;
    piscaVermelhoDelay = 0.0f;
    piscaVerde = 0;
    piscaVerdeDelay = 0.0f;
    tentativasFalhaCount = 0;
    perdeu_a_fase = false;
    venceu_a_fase = false;
    faz_fadeout = false;
    fadeout_time = 0.0f;
    derrota_show_timer = 0.0f;
    vitoria_show_timer = 0.0f;
    podeAvancarDerrotaOuVitoria = false;
    blinkVitoriaTimer = 0.0f;
    spriteStatus = SPRITE_NORMAL;
    cronometro = FASE6_CHRONO_MAX;
    cronometro_elapsed = 0.0f;
    fase_concluida = false;
    palavraDigitada[0] = '\0';
    palavraLen = 0;
}

void Update_Desafio_02(void)
{
    float delta = GetFrameTime();

    if (audioAtivo) UpdateMusicStream(musicaFase);

    if (faz_fadeout)
    {
        fadeout_time += delta;
        return;
    }

    if (!perdeu_a_fase && !venceu_a_fase)
    {
        cronometro_elapsed += delta;
        cronometro = FASE6_CHRONO_MAX - cronometro_elapsed;
        if (cronometro <= 0.0f)
        {
            perdeu_a_fase = true;
            cronometro = 0.0f;
            strcpy(fala_exibida, FALA_PERDEU_02);
            InitTypeWriter(&writer, fala_exibida, 18.5f);
            spriteStatus = SPRITE_DERROTA;
            derrota_show_timer = 0.0f;
            podeAvancarDerrotaOuVitoria = false;
        }
    }

    float geminiX = 49, geminiY = 67, geminiScale = 0.1f;
    float geminiW = sprGemini.width * geminiScale;
    float geminiH = sprGemini.height * geminiScale;
    AtualizaTamanhoGeminiBox();
    float rx = geminiX + geminiW - 20.0f;
    float ry = geminiY + (geminiH - geminiRectH) / 2.0f;
    Rectangle geminiLogoRec = {geminiX, geminiY, geminiW, geminiH};
    Rectangle geminiRectRec = {rx, ry, geminiRectW, geminiRectH};

    Vector2 mouseGem = GetMousePosition();
    bool mouseOverLogo = CheckCollisionPointRec(mouseGem, geminiLogoRec);
    bool mouseOverRect = CheckCollisionPointRec(mouseGem, geminiRectRec);
    geminiMouseOver = mouseOverLogo || mouseOverRect;
    float dir = geminiMouseOver ? 1.0f : -1.0f;

    geminiRectAnim += dir * geminiAnimSpeed * delta;
    if (geminiRectAnim > 1.0f)
        geminiRectAnim = 1.0f;
    if (geminiRectAnim < 0.0f)
        geminiRectAnim = 0.0f;
    if ((mouseOverLogo || mouseOverRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (!geminiHelpClicked)
        {
            geminiHelpClicked = true;
            AtualizaTamanhoGeminiBox();
            geminiRectAnim = 1.0f;
        }
    }

    UpdateTypeWriter(&writer, delta, IsKeyPressed(KEY_SPACE));

    if (perdeu_a_fase)
    {
        derrota_show_timer += delta;
        if (derrota_show_timer > RESPOSTA_MOSTRA_SEG)
            podeAvancarDerrotaOuVitoria = true;
        if (podeAvancarDerrotaOuVitoria &&
            (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
        {
            faz_fadeout = true;
            fadeout_time = 0.0f;

            int tempoGasto = (int)cronometro_elapsed;
            int vidasRestantes = MAX_TENTATIVAS - tentativasFalhaCount;
            SetD02Result(&playerStats, !perdeu_a_fase, tempoGasto, vidasRestantes);

            fase_concluida = true;
        }
        return;
    }

    if (venceu_a_fase)
    {
        vitoria_show_timer += delta;
        if (vitoria_show_timer > RESPOSTA_MOSTRA_SEG)
            podeAvancarDerrotaOuVitoria = true;
        if (podeAvancarDerrotaOuVitoria &&
            (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
        {
            faz_fadeout = true;
            fadeout_time = 0.0f;

            int tempoGasto = (int)cronometro_elapsed;
            int vidasRestantes = MAX_TENTATIVAS - tentativasFalhaCount;
            SetD02Result(&playerStats, venceu_a_fase, tempoGasto, vidasRestantes);

            fase_concluida = true;
        }
        return;
    }

    int velocidade = 5;
    if (IsKeyDown(KEY_RIGHT))
        quadX += velocidade;
    if (IsKeyDown(KEY_LEFT))
        quadX -= velocidade;
    if (IsKeyDown(KEY_DOWN))
        quadY += velocidade;
    if (IsKeyDown(KEY_UP))
        quadY -= velocidade;
    if (quadX < 0)
        quadX = 0;
    if (quadY < 0)
        quadY = 0;
    if (quadX + BUTTON_SIZE > GetScreenWidth())
        quadX = GetScreenWidth() - BUTTON_SIZE;
    if (quadY + BUTTON_SIZE > GetScreenHeight())
        quadY = GetScreenHeight() - BUTTON_SIZE;

    if (piscaVerde)
    {
        piscaVerdeDelay -= delta;
        if (piscaVerdeDelay <= 0)
        {
            piscaVerde = 0;
            if (passoAtual > 0 && (passoAtual - 1) < 4)
            {
                ledStatus[passoAtual - 1] = LED_ESTADO_VERDE;
                ledStatus[passoAtual - 1 + 4] = LED_ESTADO_VERDE;
            }
            piscaVerdeDelay = 0;
        }
        return;
    }
    if (piscaVermelho)
    {
        piscaVermelhoDelay -= delta;
        if (piscaVermelhoDelay <= 0)
        {
            piscaVermelho--;
            int ledAlvo = passoAtual;
            int ledAlvo2 = passoAtual + 4;
            if (piscaVermelho > 0)
            {
                piscaVermelhoDelay = 0.18f;
                LedEstadoCor nextCor = (ledStatus[ledAlvo] == LED_ESTADO_VERMELHO) ? LED_ESTADO_APAGADO : LED_ESTADO_VERMELHO;
                ledStatus[ledAlvo] = nextCor;
                ledStatus[ledAlvo2] = nextCor;
            }
            else
            {
                ledStatus[ledAlvo] = LED_ESTADO_APAGADO;
                ledStatus[ledAlvo2] = LED_ESTADO_APAGADO;
                piscaVermelhoDelay = 0;
            }
        }
        return;
    }

    Vector2 mouse = GetMousePosition();
    for (int row = 0; row < NUM_BUTTON_ROWS; row++)
    {
        float btnY = quadY + row * (BUTTON_SIZE + BUTTON_ROW_SPACING);
        for (int col = 0; col < NUM_BUTTON_COLS; col++)
        {
            float btnX = quadX + col * (BUTTON_SIZE + BUTTON_SPACING);
            Rectangle btnRec = {btnX, btnY, BUTTON_SIZE, BUTTON_SIZE};
            bool mouseOver = CheckCollisionPointRec(mouse, btnRec);
            quadButtonHovered[row][col] = mouseOver;
            if (mouseOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                TeclasQuadrado teclaApertada = (TeclasQuadrado)(row * NUM_BUTTON_COLS + col);
                char nova_fala[256];

                if (passoAtual < SEQ_TOTAL_PASSOS && teclaApertada == SEQUENCIA[passoAtual])
                {
                    char letra = GetLetraFromTecla(teclaApertada);

                    if (palavraLen < (int)sizeof(palavraDigitada) - 1)
                    {
                        palavraDigitada[palavraLen++] = letra;
                        palavraDigitada[palavraLen] = '\0';
                    }

                    snprintf(nova_fala, sizeof(nova_fala), "Palavra: %s", palavraDigitada);
                    strcpy(fala_exibida, nova_fala);
                }

                InitTypeWriter(&writer, fala_exibida, 18.5f);
                int ledAlvo = passoAtual;
                int ledAlvo2 = passoAtual + 4;
                if (passoAtual < SEQ_TOTAL_PASSOS && ledAlvo < 4)
                {
                    if (teclaApertada == SEQUENCIA[passoAtual])
                    {
                        ledStatus[ledAlvo] = LED_ESTADO_VERDE;
                        ledStatus[ledAlvo2] = LED_ESTADO_VERDE;
                        piscaVerde = 1;
                        piscaVerdeDelay = 0.22f;
                        passoAtual++;
                        if (passoAtual == SEQ_TOTAL_PASSOS)
                        {
                            venceu_a_fase = true;
                            strcpy(fala_exibida, FALA_VENCEU_02);
                            InitTypeWriter(&writer, fala_exibida, 18.5f);
                            spriteStatus = SPRITE_VITORIA;
                            vitoria_show_timer = 0.0f;
                            blinkVitoriaTimer = 0.0f;
                            podeAvancarDerrotaOuVitoria = false;
                        }
                    }
                    else
                    {
                        piscaVermelho = 6;
                        piscaVermelhoDelay = 0.18f;
                        ledStatus[ledAlvo] = LED_ESTADO_VERMELHO;
                        ledStatus[ledAlvo2] = LED_ESTADO_VERMELHO;
                        tentativasFalhaCount++;
                        if (tentativasFalhaCount >= MAX_TENTATIVAS)
                        {
                            perdeu_a_fase = true;
                            strcpy(fala_exibida, FALA_PERDEU_02);
                            InitTypeWriter(&writer, fala_exibida, 18.5f);
                            spriteStatus = SPRITE_DERROTA;
                            derrota_show_timer = 0.0f;
                            podeAvancarDerrotaOuVitoria = false;
                        }
                    }
                }
            }
        }
    }
}

void Draw_Desafio_02(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    int boxX = 60;
    int marginBottom = 220;
    int boxY = GetScreenHeight() - marginBottom;
    int boxWidth = GetScreenWidth() - 120;
    int boxHeight = 130;
    int imgW = 1000;
    int imgH = pergunta_img.height - 130;
    int imgX = boxX;
    int imgY = boxY - imgH;
    float delta = GetFrameTime();
    {
        float screenRatio = (float)GetScreenWidth() / (float)GetScreenHeight();
        float imgRatio = (float)fundo.width / (float)fundo.height;
        float scale = (screenRatio > imgRatio)
                          ? (float)GetScreenWidth() / fundo.width
                          : (float)GetScreenHeight() / fundo.height;
        float drawW = fundo.width * scale;
        float drawH = fundo.height * scale;
        float drawX = (GetScreenWidth() - drawW) / 2;
        float drawY = (GetScreenHeight() - drawH) / 2;
        DrawTextureEx(fundo, (Vector2){drawX, drawY}, 0.0f, scale, WHITE);
    }
    const char *name = gSelectedCharacterName;
    DrawTexturePro(pergunta_img, (Rectangle){0, 0, pergunta_img.width, pergunta_img.height},
                   (Rectangle){imgX, imgY, imgW, imgH}, (Vector2){0, 0}, 0.0f, WHITE);
    DrawText((name && name[0]) ? name : "???", imgX + 10, imgY + imgH - 30, 30, WHITE);
    int borderRadius = boxHeight / 2;
    DrawRectangle(boxX, boxY, boxWidth - borderRadius, boxHeight, (Color){20, 20, 20, 220});
    DrawCircle(boxX + boxWidth - borderRadius, boxY + borderRadius, borderRadius, (Color){20, 20, 20, 220});
    if (writer.drawnChars > 0)
    {
        char txtbuf[512];
        strncpy(txtbuf, fala_exibida, writer.drawnChars);
        txtbuf[writer.drawnChars] = '\0';
        DrawText(txtbuf, boxX + 20, boxY + 30, 28, WHITE);
    }
    bool telaFinal = false;
    float timerFinal = 0;
    if (perdeu_a_fase)
    {
        telaFinal = true;
        timerFinal = derrota_show_timer;
    }
    if (venceu_a_fase)
    {
        telaFinal = true;
        timerFinal = vitoria_show_timer;
    }
    {
        Texture2D spr = sprJoao;
        float scale = 0.6f;
        int carlosExtraOffset = 0, mamedeExtraOffset = 0;
        if (strcmp(name, "Dante") == 0)
        {
            if (spriteStatus == SPRITE_VITORIA)
            {
                spr = sprMateus2;
                scale = 0.8f;
            }
            else if (spriteStatus == SPRITE_DERROTA)
            {
                spr = sprMateus3;
                scale = 0.8f;
            }
            else
            {
                spr = sprMateus;
                scale = 1.3f;
            }
        }
        else if (strcmp(name, "Alice") == 0)
        {
            if (spriteStatus == SPRITE_VITORIA)
            {
                spr = sprJoao2;
                scale = 0.95f;
            }
            else if (spriteStatus == SPRITE_DERROTA)
            {
                spr = sprJoao3;
                scale = 0.95f;
            }
            else
            {
                spr = sprJoao;
                scale = 0.6f;
            }
        }
        else if (strcmp(name, "Jade") == 0)
        {
            if (spriteStatus == SPRITE_VITORIA)
            {
                spr = sprCarlos2;
                scale = 1.02f;
                carlosExtraOffset = -70;
            }
            else if (spriteStatus == SPRITE_DERROTA)
            {
                spr = sprCarlos3;
                scale = 1.0f;
                carlosExtraOffset = -44;
            }
            else
            {
                spr = sprCarlos;
                scale = 0.56f;
                carlosExtraOffset = 0;
            }
        }
        else if (strcmp(name, "Jade") == 0)
        {
            if (spriteStatus == SPRITE_VITORIA)
            {
                spr = sprCarlos2;
                scale = 1.02f;
                carlosExtraOffset = -70;
            }
            else if (spriteStatus == SPRITE_DERROTA)
            {
                spr = sprCarlos3;
                scale = 1.0f;
                carlosExtraOffset = -44;
            }
            else
            {
                spr = sprCarlos;
                scale = 0.56f;
                carlosExtraOffset = 0;
            }
        }
        else if (strcmp(name, "Levi") == 0)
        {
            if (spriteStatus == SPRITE_VITORIA)
            {
                spr = sprMamede2;
                scale = 1.0f;
            }
            else if (spriteStatus == SPRITE_DERROTA)
            {
                spr = sprMamede3;
                scale = 1.0f;
            }
            else
            {
                spr = sprMamede;
                scale = 1.0f;
            }
        }
        else if (name[0] == '\0')
        {
            spr = sprJoao;
            scale = 0.6f;
        }
        float tw2 = spr.width * scale;
        float th = spr.height * scale;
        Vector2 pos;
        pos.x = imgX - 330 + (imgW - tw2) / 2.0f;
        pos.y = imgY - th + 210;
        if (strcmp(name, "Dante") == 0)
            pos.x += 24;
        if (strcmp(name, "Alice") == 0)
            pos.x += 30;
        if (strcmp(name, "Jade") == 0)
            pos.x += 100 + carlosExtraOffset;
        else if (strcmp(name, "Levi") == 0)
            pos.x += mamedeExtraOffset;
        DrawTextureEx(spr, pos, 0, scale, WHITE);
    }
    if (!telaFinal || timerFinal <= RESPOSTA_MOSTRA_SEG)
    {
        if (!perdeu_a_fase && !venceu_a_fase && !faz_fadeout)
        {
            float chronoR = 55.0f;
            float scaleHeart = 0.30f;
            float heartW = heartFullRec.width * scaleHeart;
            float heartH = heartFullRec.height * scaleHeart;
            float heartsGap = 6.0f;
            float heartsW = 3 * heartW + 2 * heartsGap;

            float chronoCX = GetScreenWidth() - 80; // centro X (você já usa  -80)
            float chronoCY = 80;                    // centro Y
            float pad = 10.0f;
            float panelH = fmaxf(heartH, chronoR * 2) + pad * 2.0f;
            float panelW = heartsW + chronoR * 2 + pad * 3.0f;
            float panelX = chronoCX - panelW + chronoR;
            float panelY = chronoCY - panelH / 2.0f;
            float extraX = 150.0f;
            float extraY = 50.0f;
            panelW += extraX;
            panelH += extraY;
            panelX -= extraX * 0.3f;
            panelY -= extraY;
            Color fill = (Color){8, 38, 18, 200};
            //Color border = (Color){24, 90, 45, 255};
            float round = 0.18f;
            int segs = 8;

            DrawRectangleRounded((Rectangle){panelX, panelY, panelW, panelH}, round, segs, fill);
            //DrawRectangleRoundedLines((Rectangle){panelX, panelY, panelW, panelH}, round, segs, 5.0f, WHITE);

            DrawChronometer(cronometro, FASE6_CHRONO_MAX, GetScreenWidth() - 80, 80, 55);

            int vidasRestantes = MAX_TENTATIVAS - tentativasFalhaCount;
            float scale = 0.3f;
            float hW = heartFullRec.width * scale;
            float hH = heartFullRec.height * scale;
            float espaco = 6.0f;
            float refX = GetScreenWidth() - 80 - (3 * hW + 2 * espaco) - 100;
            float refY = 80 - hH / 2;

            for (int i = 0; i < 3; i++)
            {
                Rectangle srcRec = (i < vidasRestantes) ? heartFullRec : heartEmptyRec;
                Rectangle dstRec = {refX + i * (hW + espaco), refY, hW, hH};
                DrawTexturePro(heartSheet, srcRec, dstRec, (Vector2){0, 0}, 0.0f, WHITE);
            }
        }
        float geminiX = 49, geminiY = 67, geminiScale = 0.1f;
        float geminiW = sprGemini.width * geminiScale;
        float geminiH = sprGemini.height * geminiScale;
        const char *gemini_msg = !geminiHelpClicked ? gemini_help_msg_default : gemini_help_msg_ajuda;
        int txtSize = 20;
        float rx = geminiX + geminiW - 20.0f;
        float ry = geminiY + (geminiH - geminiRectH) / 2.0f;
        float animW = geminiRectW * geminiRectAnim;
        Color logoCol = geminiMouseOver ? (Color){18, 60, 32, 255} : (Color){26, 110, 51, 255};
        if (geminiRectAnim > 0.01f)
        {
            Color rectCol = (Color){15, 42, 26, (unsigned char)(210 * geminiRectAnim)};
            float round = 0.33f;
            DrawRectangleRounded((Rectangle){rx, ry, animW, geminiRectH}, round, 12, rectCol);
            if (geminiRectAnim > 0.05f)
            {
                if (animW > geminiTextWidth + 2 * GEMINI_PAD_X - 4)
                {
                    DrawText(gemini_msg, rx + GEMINI_PAD_X, ry + geminiRectH / 2 - txtSize / 2, txtSize, WHITE);
                }
            }
        }
        DrawTextureEx(sprGemini, (Vector2){geminiX, geminiY}, 0.0f, geminiScale, logoCol);
        // Color borderColorNormal = (Color){4, 77, 18, 255};
        // Color borderColorHover  = (Color){18, 130, 49, 255};
        // for (int row = 0; row < NUM_BUTTON_ROWS; row++) {
        // float btnY = quadY + row * (BUTTON_SIZE + BUTTON_ROW_SPACING);
        // for (int col = 0; col < NUM_BUTTON_COLS; col++) {
        // float btnX = quadX + col * (BUTTON_SIZE + BUTTON_SPACING);
        // Rectangle btnRec = { btnX, btnY, BUTTON_SIZE, BUTTON_SIZE };
        // Color cor = quadButtonHovered[row][col] ? borderColorHover : borderColorNormal;
        // DrawRectangleRoundedLines(btnRec, 0.23f, 16, cor);
        //       (Rectangle) (float) (int) (Color)
        //}
        //}
        int led_centro_x = GetScreenWidth() / 2 - 280;
        int led_centro_y = GetScreenHeight() / 2 - 210;
        int led_raio = 14;
        int led_espacamento = 40;
        for (int i = 0; i < 4; i++)
        {
            int x = led_centro_x + i * (2 * led_raio + led_espacamento);
            DrawStylizedLedColor(x, led_centro_y, led_raio, ledStatus[i]);
        }
        int ultimo_led1_x = led_centro_x + (3) * (2 * led_raio + led_espacamento);
        int bloco2_inicio_x = ultimo_led1_x + 123 + 2 * led_raio;
        for (int i = 0; i < 4; i++)
        {
            int x = bloco2_inicio_x + i * (2 * led_raio + led_espacamento);
            DrawStylizedLedColor(x, led_centro_y, led_raio, ledStatus[i + 4]);
        }
    }
    if (venceu_a_fase && !faz_fadeout)
    {
        blinkVitoriaTimer += delta;
        int w = GetScreenWidth(), h = GetScreenHeight();
        int layers = 5, layerThick = 20;
        int pisca = ((int)(blinkVitoriaTimer * 4.1f)) % 2;
        for (int i = 0; i < layers; i++)
        {
            int thick = layerThick + i * layerThick;
            int alpha = pisca ? (38 - i * 5) : (25 - i * 5);
            if (alpha < 6)
                alpha = 6;
            Color blurGreen = (Color){40, 195, 70, alpha};
            DrawRectangle(0, 0, w, thick, blurGreen);
            DrawRectangle(0, h - thick, w, thick, blurGreen);
            DrawRectangle(0, 0, thick, h, blurGreen);
            DrawRectangle(w - thick, 0, thick, h, blurGreen);
        }
        if (vitoria_show_timer > RESPOSTA_MOSTRA_SEG)
        {
            float pulse = 0.07f * sinf(GetTime() * 3.0f);
            float btnScaleBase = 0.85f;
            float btnScale = btnScaleBase + pulse;
            float btnW = sprEnterButton.width * btnScale;
            float btnH = sprEnterButton.height * btnScale;
            float btnX = GetScreenWidth() / 2 - btnW / 2 + 630;
            float btnY = GetScreenHeight() / 2 - 90;
            Color sombra = (Color){0, 0, 0, 90};
            DrawRectangleRounded((Rectangle){btnX + 8, btnY + 8, btnW, btnH}, 0.25f, 10, sombra);
            Color brilho = (pulse > 0.04f) ? (Color){255, 255, 255, 75} : WHITE;
            DrawTextureEx(sprEnterButton, (Vector2){btnX, btnY}, 0.0f, btnScale, brilho);
        }
    }
    if (perdeu_a_fase && !faz_fadeout)
    {
        int w = GetScreenWidth(), h = GetScreenHeight();
        int layers = 5, layerThick = 20;
        for (int i = 0; i < layers; i++)
        {
            int thick = layerThick + i * layerThick;
            int alpha = 38 - i * 5;
            if (alpha < 6)
                alpha = 6;
            Color blurRed = (Color){255, 32, 32, alpha};
            DrawRectangle(0, 0, w, thick, blurRed);
            DrawRectangle(0, h - thick, w, thick, blurRed);
            DrawRectangle(0, 0, thick, h, blurRed);
            DrawRectangle(w - thick, 0, thick, h, blurRed);
        }
        if (derrota_show_timer > RESPOSTA_MOSTRA_SEG)
        {
            float pulse = 0.07f * sinf(GetTime() * 3.0f);
            float btnScaleBase = 0.85f;
            float btnScale = btnScaleBase + pulse;
            float btnW = sprEnterButton.width * btnScale;
            float btnH = sprEnterButton.height * btnScale;
            float btnX = GetScreenWidth() / 2 - btnW / 2 + 630;
            float btnY = GetScreenHeight() / 2 - 90;
            Color sombra = (Color){0, 0, 0, 90};
            DrawRectangleRounded((Rectangle){btnX + 8, btnY + 8, btnW, btnH}, 0.25f, 10, sombra);
            Color brilho = (pulse > 0.04f) ? (Color){255, 255, 255, 75} : WHITE;
            DrawTextureEx(sprEnterButton, (Vector2){btnX, btnY}, 0.0f, btnScale, brilho);
        }
    }
    if (faz_fadeout)
    {
        float perc = fadeout_time / FADEOUT_DURACAO;
        if (perc > 1.0f)
            perc = 1.0f;
        int alpha = (int)(255 * perc);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, alpha});
    }
    EndDrawing();
}

bool Fase_Desafio_02_Concluida(void)
{
    return fase_concluida;
}

void Unload_Desafio_02(void)
{
    UnloadTexture(fundo);
    UnloadTexture(pergunta_img);
    UnloadTexture(sprJoao);
    UnloadTexture(sprJoao2);
    UnloadTexture(sprJoao3);
    UnloadTexture(sprMateus);
    UnloadTexture(sprMateus2);
    UnloadTexture(sprMateus3);
    UnloadTexture(sprCarlos);
    UnloadTexture(sprCarlos2);
    UnloadTexture(sprCarlos3);
    UnloadTexture(sprMamede);
    UnloadTexture(sprMamede2);
    UnloadTexture(sprMamede3);
    UnloadTexture(sprEnterButton);
    UnloadTexture(sprGemini);
    UnloadTexture(heartSheet);

    if (audioAtivo) {
        StopMusicStream(musicaFase);
        UnloadMusicStream(musicaFase);
        audioAtivo = false;
    }
}