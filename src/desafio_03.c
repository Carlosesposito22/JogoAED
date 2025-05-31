#include "desafio_03.h"
#define NUM_SWITCHES 8
#define QUADRO_SHIFT_Y (-40)
#define MIN_Y_BASE 536
#define MAX_Y_BASE 700
#define RESPOSTA_MOSTRA_SEG 5.0f
#define LIMPEZA_TELA_RESULTADO 5.0f
#define GEMINI_RECT_PADRAO 550
#define GEMINI_PAD_X 36
#define FASE3_FADEOUT_DURACAO 0.8f
#define FALA_NORMAL_01 "Preciso realocar os recursos para manter os sistemas críticos ativos. Não posso desligar os essenciais!"
#define FALA_ACERTO_01 "Consegui! Os sistemas prioritários estão ativos e funcionando corretamente."
#define FALA_ERRO_01 "Algo deu errado... Desativei um sistema essencial! Isso pode comprometer o funcionamento!"
#define FALA_JOAO_ACERTO "Tudo certo! Os recursos estão bem distribuídos e a operação está estável."
#define FALA_JOAO_ERRO "Droga! Desativei um sistema essencial! Isso pode atrapalhar o andamento da operação!"
#define FALA_CARLOS_ACERTO "Recursos alocados corretamente! A operação segue estável."
#define FALA_CARLOS_ERRO "Ih, desliguei um sistema importante! Isso vai prejudicar o funcionamento!"
#define FALA_MAMEDE_ACERTO "Boa! Mantive os recursos essenciais ativos e a operação estável."
#define FALA_MAMEDE_ERRO "Que problema! Desliguei algo crítico! A estabilidade ficou comprometida."
#define FALA_EXPLODIU "O que foi isso? Parece que algum sistema falhou! Preciso verificar os recursos novamente!"

static Texture2D fundo;
static Texture2D pergunta_img;
static Texture2D sprJoao, sprJoao2, sprJoao3;
static Texture2D sprMateus, sprMateus2, sprMateus3;
static Texture2D sprCarlos, sprCarlos2, sprCarlos3;
static Texture2D sprMamede, sprMamede2, sprMamede3;
static Texture2D sprEnterButton;
static Texture2D quadroEnergia;
static Texture2D imgSwitch;
static Texture2D sprGemini;
static Texture2D sprCadeadoFechadoCortado;
static Texture2D sprCadeadoAbertoCortado;
static TypeWriter writer;

static const char* gemini_help_msg_default = "Clique aqui caso precise de ajuda!";
static const char* gemini_help_msg_ajuda = "Certifique-se de manter ativos: energia, firewall, antivírus, rede.";
static float geminiRectW = 550;
static float geminiRectH = 0;
static int   geminiTextWidth = 0;
static float geminiRectAnim = 0.0f;
static bool geminiMouseOver = false;
static bool geminiHelpClicked = false;
static float geminiAnimSpeed = 6.0f;
static bool fase_concluida = false; 
static float timer_total = 60.0f;
static float timer_restante = 0.0f;
static bool timer_explodiu = false;
static float switchX[NUM_SWITCHES];
static float switchY[NUM_SWITCHES];
static bool draggingSwitch[NUM_SWITCHES];
static float mouseOffsetY[NUM_SWITCHES];
static float escalaSwitch = 0.85f;
static float minY, maxY;
static bool fase3_fadeout = false;
static float fase3_fadeout_time = 0.0f;
static char fala_exibida[512];
static bool podeAvancar = false;
static bool resposta_entregue = false;
static bool acertou_resposta = false;
static bool mostrar_feedback = false;
static float timer_feedback = 0.0f;
static float respostaShowTimer = 0.0f;

static void AtualizaTamanhoGeminiBox(void)
{
    float geminiScale = 0.1f;
    float geminiH = sprGemini.height * geminiScale;
    int txtSize = 20;
    const char* gemini_msg = !geminiHelpClicked ? gemini_help_msg_default : gemini_help_msg_ajuda;
    geminiTextWidth = MeasureText(gemini_msg, txtSize);

    float minW = GEMINI_RECT_PADRAO;
    float maxW = GetScreenWidth() - 140;
    geminiRectW = minW;
    
    if (geminiTextWidth + 2 * GEMINI_PAD_X > minW)
        geminiRectW = (geminiTextWidth + 2 * GEMINI_PAD_X > maxW) ? maxW : geminiTextWidth + 2 * GEMINI_PAD_X;
    geminiRectH = geminiH * 0.75f;
}

static void InitSwitches(void)
{
    float baseX = 696;
    float spacing = 75.7;
    minY = MIN_Y_BASE + QUADRO_SHIFT_Y;
    maxY = MAX_Y_BASE + QUADRO_SHIFT_Y;
    for (int i = 0; i < NUM_SWITCHES; i++) {
        switchX[i] = baseX + i * spacing;
        switchY[i] = minY;
        draggingSwitch[i] = false;
        mouseOffsetY[i] = 0.0f;
    }
}

static const char* FalaPorResultado(const char* name, bool acerto)
{
    if (!name || !name[0]) name = "Dante";
    if (strcmp(name, "Alice") == 0)      return acerto ? FALA_JOAO_ACERTO   : FALA_JOAO_ERRO;
    if (strcmp(name, "Jade") == 0)    return acerto ? FALA_CARLOS_ACERTO : FALA_CARLOS_ERRO;
    if (strcmp(name, "Levi") == 0)    return acerto ? FALA_MAMEDE_ACERTO : FALA_MAMEDE_ERRO;
    return acerto ? FALA_ACERTO_01 : FALA_ERRO_01;
}

void Init_Desafio_03(void)
{
    fundo      = LoadTexture("src/sprites/empresa3.png");
    pergunta_img=LoadTexture("src/sprites/pergunta3.png");
    sprJoao    = LoadTexture("src/sprites/joaoSprite.png");
    sprJoao2   = LoadTexture("src/sprites/joao2.png");
    sprJoao3   = LoadTexture("src/sprites/joao3.png");
    sprMateus  = LoadTexture("src/sprites/mateusSprite.png");
    sprMateus2 = LoadTexture("src/sprites/mateus2.png");
    sprMateus3 = LoadTexture("src/sprites/mateus3.png");
    sprCarlos  = LoadTexture("src/sprites/carlosSprite.png");
    sprCarlos2 = LoadTexture("src/sprites/carlos2.png");
    sprCarlos3 = LoadTexture("src/sprites/carlos3.png");
    sprMamede  = LoadTexture("src/sprites/mamedeSprite.png");
    sprMamede2 = LoadTexture("src/sprites/mamede2.png");
    sprMamede3 = LoadTexture("src/sprites/mamede3.png");
    sprEnterButton= LoadTexture("src/sprites/enter_button.png");
    quadroEnergia = LoadTexture("src/sprites/quadroEnergia.png");
    imgSwitch     = LoadTexture("src/sprites/switch.png");
    sprGemini     = LoadTexture("src/sprites/os/gemini.png");
    sprCadeadoFechadoCortado = LoadTexture("src/sprites/cadeado_fechado_cortado.png");
    sprCadeadoAbertoCortado  = LoadTexture("src/sprites/cadeado_aberto_cortado.png");

    strcpy(fala_exibida, FALA_NORMAL_01);
    InitTypeWriter(&writer, fala_exibida, 19.5f);
    InitSwitches();
    AtualizaTamanhoGeminiBox();
    
    podeAvancar = false;
    fase3_fadeout = false;
    timer_explodiu = false;
    geminiHelpClicked = false;
    geminiMouseOver = false;
    resposta_entregue = false;
    acertou_resposta = false;
    mostrar_feedback = false;
    fase_concluida = false;

    timer_restante = timer_total;
    fase3_fadeout_time = 0.0f;
    timer_total = 40.0f;
    geminiRectAnim = 0.0f;
    timer_feedback = 0.0f;
    respostaShowTimer = 0.0f;
}

void Update_Desafio_03(void)
{
    float delta = GetFrameTime();
    float geminiX = 49, geminiY = 67, geminiScale = 0.1f;
    float geminiW = sprGemini.width * geminiScale;
    float geminiH = sprGemini.height * geminiScale;
    AtualizaTamanhoGeminiBox();
    float rx = geminiX + geminiW - 20.0f;
    float ry = geminiY + (geminiH - geminiRectH) / 2.0f;
    Rectangle geminiLogoRec = { geminiX, geminiY, geminiW, geminiH };
    Rectangle geminiRectRec = { rx, ry, geminiRectW, geminiRectH };
    Vector2 mouseGem = GetMousePosition();
    bool mouseOverLogo = CheckCollisionPointRec(mouseGem, geminiLogoRec);
    bool mouseOverRect = CheckCollisionPointRec(mouseGem, geminiRectRec);
    geminiMouseOver = mouseOverLogo || mouseOverRect;
    float dir = geminiMouseOver ? 1.0f : -1.0f;
    geminiRectAnim += dir * geminiAnimSpeed * delta;
    if (geminiRectAnim > 1.0f) geminiRectAnim = 1.0f;
    if (geminiRectAnim < 0.0f) geminiRectAnim = 0.0f;
    if ((mouseOverLogo || mouseOverRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (!geminiHelpClicked) {
            geminiHelpClicked = true;
            AtualizaTamanhoGeminiBox();
            geminiRectAnim = 1.0f;
        }
    }
    float escalaQuadro = 0.65f;
    float quadroW = quadroEnergia.width * escalaQuadro;
    float quadroH = quadroEnergia.height * escalaQuadro;
    float quadroX = (GetScreenWidth() - quadroW) / 2.0f;
    float quadroY = (GetScreenHeight() - quadroH) / 2.0f - 100 + QUADRO_SHIFT_Y;
    float entregaW = quadroW;
    float entregaH = 38;
    float entregaX = quadroX;
    float entregaY = quadroY + quadroH + 12;
    Rectangle entregaBtn = { entregaX, entregaY, entregaW, entregaH };
    const char* name = gSelectedCharacterName;
    if (resposta_entregue || timer_explodiu) respostaShowTimer += delta;
    else respostaShowTimer = 0.0f;
    if (!timer_explodiu && !fase3_fadeout && !resposta_entregue) {
        timer_restante -= delta;
        if (timer_restante < 0.0f) timer_restante = 0.0f;
        if (timer_restante <= 0.0f && !timer_explodiu) {
            timer_explodiu = true;
            resposta_entregue = true;
            bool condicaoGanhei = true;
            for (int i = 0; i < NUM_SWITCHES; i++) {
                if (i == 0 || i == 1 || i == 5 || i == 7) {
                    if (switchY[i] != minY) condicaoGanhei = false;
                } else {
                    if (switchY[i] != maxY) condicaoGanhei = false;
                }                
            }
            acertou_resposta = condicaoGanhei;
            mostrar_feedback = !acertou_resposta;
            timer_feedback = 0.0f;
            if (!acertou_resposta) strcpy(fala_exibida, FALA_EXPLODIU);
            else strcpy(fala_exibida, FalaPorResultado(name, acertou_resposta));
            InitTypeWriter(&writer, fala_exibida, 18.5f);
            respostaShowTimer = 0.0f;
            podeAvancar = false;
        }
    }
    float w = imgSwitch.width * escalaSwitch;
    float h = imgSwitch.height * escalaSwitch;
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < NUM_SWITCHES; i++) {
        if (timer_explodiu || resposta_entregue) continue;
        Rectangle switchBounds = {switchX[i], switchY[i], w, h};
        if (!draggingSwitch[i] && CheckCollisionPointRec(mouse, switchBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            draggingSwitch[i] = true;
            mouseOffsetY[i] = mouse.y - switchY[i];
        }
        if (draggingSwitch[i]) {
            float newY = mouse.y - mouseOffsetY[i];
            if (newY < minY) newY = minY;
            if (newY > maxY) newY = maxY;
            switchY[i] = newY;
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                draggingSwitch[i] = false;
            }
        }
    }
    if (!resposta_entregue && !timer_explodiu) {
        Vector2 mouseBt = GetMousePosition();
        if (CheckCollisionPointRec(mouseBt, entregaBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            resposta_entregue = true;
            bool condicaoGanhei = true;
            for (int i = 0; i < NUM_SWITCHES; i++) {
                if (i == 0 || i == 1 || i == 5 || i == 7) {
                    if (switchY[i] != minY) condicaoGanhei = false;
                } else {
                    if (switchY[i] != maxY) condicaoGanhei = false;
                }                
            }
            acertou_resposta = condicaoGanhei;
            mostrar_feedback = !acertou_resposta;
            timer_feedback = 0.0f;
            strcpy(fala_exibida, FalaPorResultado(name, acertou_resposta));
            InitTypeWriter(&writer, fala_exibida, 18.5f);
            respostaShowTimer = 0.0f;
            podeAvancar = false;
        }
    }
    if (mostrar_feedback) {
        timer_feedback += delta;
        if (timer_feedback >= 2.0f) {
            mostrar_feedback = false;
        }
    }
    UpdateTypeWriter(&writer, delta, IsKeyPressed(KEY_SPACE));
    if ((resposta_entregue || timer_explodiu) && !podeAvancar && writer.done) podeAvancar = true;
    if (podeAvancar && respostaShowTimer > RESPOSTA_MOSTRA_SEG && respostaShowTimer > LIMPEZA_TELA_RESULTADO) {
        float btnScaleBase = 1.2f;
        float btnW = sprEnterButton.width * btnScaleBase;
        float btnH = sprEnterButton.height * btnScaleBase;
        float btnX = GetScreenWidth()/2 - btnW/2 + 120;
        float btnY = GetScreenHeight()/2 - 150;
        Rectangle btnBounds = {btnX, btnY, btnW, btnH};
        Vector2 mouse = GetMousePosition();
        if (!fase3_fadeout) {
            if (
                IsKeyPressed(KEY_ENTER) 
                || IsKeyPressed(KEY_KP_ENTER) 
                || (CheckCollisionPointRec(mouse, btnBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            ) 
            {
                fase3_fadeout = true;
                fase3_fadeout_time = 0.0f;

                SetD03Result(&playerStats, acertou_resposta, timer_total - timer_restante);
                fase_concluida = true;
            }
        }
        if (fase3_fadeout) {
            fase3_fadeout_time += delta;
        }
    }
}

void Draw_Desafio_03(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawTextureEx(fundo, (Vector2){0,0}, 0.0f, (float)GetScreenWidth() / fundo.width, RAYWHITE);
    bool mostrarTudo = !(resposta_entregue || timer_explodiu) ||
                       respostaShowTimer < LIMPEZA_TELA_RESULTADO;
    float escalaQuadro = 0.65f;
    float quadroW = quadroEnergia.width * escalaQuadro;
    float quadroH = quadroEnergia.height * escalaQuadro;
    float quadroX = (GetScreenWidth() - quadroW) / 2.0f;
    float quadroY = (GetScreenHeight() - quadroH) / 2.0f -100 + QUADRO_SHIFT_Y;

    if (mostrarTudo) 
    {
        DrawTextureEx(quadroEnergia, (Vector2){quadroX, quadroY}, 0.0f, escalaQuadro, WHITE);
        float cadeadoScale = 0.303f;
        float cadeadoW = sprCadeadoFechadoCortado.width * cadeadoScale;
        float cadeadoH = sprCadeadoFechadoCortado.height * cadeadoScale;
        float cadeadoX = quadroX + quadroW/2 - cadeadoW/2 + 20;
        float cadeadoY = quadroY + 110;
        float rectX = cadeadoX - 1;
        float rectY = cadeadoY - 50;
        float rectW = cadeadoW + 3; 
        float rectH = cadeadoH + 92; 
        DrawRectangle(rectX, rectY, rectW, rectH, BLACK);
        Texture2D cadeadoAText = sprCadeadoFechadoCortado;
        bool terminou = (resposta_entregue || timer_explodiu);

        if (terminou && acertou_resposta) 
        {
            cadeadoAText = sprCadeadoAbertoCortado;
            cadeadoW = sprCadeadoAbertoCortado.width * cadeadoScale;
            cadeadoH = sprCadeadoAbertoCortado.height * cadeadoScale;
            cadeadoX = quadroX + quadroW/2 - cadeadoW/2 + 20;
            cadeadoY = quadroY + 110;
        }

        DrawTextureEx(cadeadoAText, (Vector2){cadeadoX, cadeadoY}, 0.0f, cadeadoScale, WHITE);
    }

    float entregaW = quadroW;
    float entregaH = 38;
    float entregaX = quadroX;
    float entregaY = quadroY + quadroH + 12;

    if (!resposta_entregue && !timer_explodiu && mostrarTudo)
    {
        float entregaW_final = entregaW - 20;
        float entregaX_final = entregaX + 20;
        Rectangle entregaBtnFinal = { entregaX_final, entregaY, entregaW_final, entregaH };
        Vector2 mouse = GetMousePosition();
        bool hover = CheckCollisionPointRec(mouse, entregaBtnFinal);
        Color corVerdeA = (Color){22, 61, 30, 232};
        Color corVerdeB = (Color){38, 96, 47, 232};
        Color corVerdeHoverA = (Color){34, 72, 41, 255};
        Color corVerdeHoverB = (Color){52, 120, 67, 255};
        Color corSombra = (Color){0, 0, 0, 60};
        DrawRectangleRounded((Rectangle){entregaX_final+3, entregaY+5, entregaW_final, entregaH}, 0.34f, 6, corSombra);
        Color corTop = hover ? corVerdeHoverA : corVerdeA;
        Color corBot = hover ? corVerdeHoverB : corVerdeB;

        for (int i = 0; i < (int)entregaH; ++i) 
        {
            float t = (float)i/entregaH;
            Color row = {
                (unsigned char)(corTop.r*(1-t) + corBot.r*t),
                (unsigned char)(corTop.g*(1-t) + corBot.g*t),
                (unsigned char)(corTop.b*(1-t) + corBot.b*t),
                (unsigned char)(corTop.a*(1-t) + corBot.a*t)
            };
            DrawRectangle(entregaX_final, entregaY + i, entregaW_final, 1, row);
        }
        //DrawRectangleRoundedLines(entregaBtnFinal, 0.32f, 8, 2.0f, corVerdeBorder);  
        if (!hover) {
            DrawRectangleRounded((Rectangle){entregaX_final+6, entregaY+6, entregaW_final-12, entregaH*0.28f}, 0.34f, 6, (Color){255,255,255,31});
        }

        int fonte = 28;
        const char* label = "ENTREGAR";
        int twidth = MeasureText(label, fonte);
        DrawText(label,
                entregaX_final + entregaW_final/2 - twidth/2,
                entregaY + entregaH/2 - fonte/2 + 2,
                fonte,
                (hover ? (Color){230,255,240,255} : (Color){245,255,245,255})
        );
    }

    if (mostrarTudo)
    {
        DrawChronometer(timer_restante, timer_total, GetScreenWidth()-80, 80, 55);
    }

    static float blinkTimerSwitch = 0.0f;
    if (mostrarTudo && ((timer_explodiu && !acertou_resposta) || (resposta_entregue && !acertou_resposta && mostrar_feedback)))
        blinkTimerSwitch += GetFrameTime();

    if (mostrarTudo) 
    {
        for (int i = 0; i < NUM_SWITCHES; i++) 
        {
            float w = imgSwitch.width * escalaSwitch;
            float h = imgSwitch.height * escalaSwitch;
            DrawTextureEx(imgSwitch, (Vector2){switchX[i], switchY[i]}, 0, escalaSwitch, WHITE);
            int sinalR = 7;
            int sinalX = (int)(switchX[i] + w/2);
            int sinalY = (int)(maxY + h + 5);
            Color corLigado   = (Color){39, 174, 96, 255};
            Color corDesligado= (Color){190,190,190,255};
            Color corAlarme   = (Color){255,38,38, 255};
            Color corEstado;
            if((timer_explodiu && !acertou_resposta) || (resposta_entregue && !acertou_resposta && mostrar_feedback)) {
                bool blinkRed = (((int)(blinkTimerSwitch * 5.0f) % 2) == 0);
                corEstado = blinkRed ? corAlarme : corDesligado;
            } else {
                corEstado = (switchY[i] >= maxY) ? corDesligado : corLigado;
            }
            DrawCircle(sinalX, sinalY, sinalR, corEstado);
            DrawCircle(sinalX, sinalY, sinalR-3, (Color){255,255,255,160});
        }
    }

    int boxX = 60;
    int marginBottom = 220;
    int boxY = GetScreenHeight() - marginBottom;
    int boxWidth = GetScreenWidth() - 120;
    int boxHeight = 130;
    int imgW = 1000;
    int imgH = pergunta_img.height - 130;
    int imgX = boxX;
    int imgY = boxY - imgH;

    DrawTexturePro(pergunta_img, (Rectangle){0, 0, pergunta_img.width, pergunta_img.height},
                   (Rectangle){imgX, imgY, imgW, imgH}, (Vector2){0, 0}, 0.0f, WHITE);
    const char *name = gSelectedCharacterName;
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

    Texture2D spr = sprJoao;
    float scale = 0.6f;
    int carlosExtraOffset = 0, mamedeExtraOffset = 0;
    bool terminou = (timer_explodiu || resposta_entregue);
    bool ganhou   = terminou && acertou_resposta;
    bool perdeu   = terminou && !acertou_resposta;

    if(strcmp(name, "Dante") == 0) 
    {
        if (ganhou)      { spr = sprMateus2; scale = 0.8f; }
        else if(perdeu)  { spr = sprMateus3; scale = 0.8f; }
        else             { spr = sprMateus;  scale = 1.3f; }
    } 
    else if(strcmp(name, "Alice") == 0) 
    {
        if (ganhou)      { spr = sprJoao2; scale = 0.95f; }
        else if(perdeu)  { spr = sprJoao3; scale = 0.95f; }
        else             { spr = sprJoao;  scale = 0.6f; }
    } 
    else if(strcmp(name, "Jade") == 0) 
    {
        if (ganhou)      { spr = sprCarlos2; scale = 1.02f; carlosExtraOffset = -70;}
        else if(perdeu)  { spr = sprCarlos3; scale = 1.0f;  carlosExtraOffset = -44; }
        else             { spr = sprCarlos;  scale = 0.56f; carlosExtraOffset = 0; }
    } 
    else if(strcmp(name, "Levi") == 0) 
    {
        if (ganhou)      { spr = sprMamede2; scale = 1.0f;}
        else if(perdeu)  { spr = sprMamede3; scale = 1.0f;}
        else             { spr = sprMamede;  scale = 1.0f;}
    } 
    else if(name[0]=='\0') 
    {
        spr = sprJoao; 
        scale = 0.6f;
    }

    float tw2 = spr.width * scale;
    float th  = spr.height * scale;
    Vector2 pos;
    pos.x = imgX - 330 + (imgW - tw2)/2.0f;
    pos.y = imgY - th + 210;

    if (strcmp(name, "Jade") == 0)
        pos.x += 100 + carlosExtraOffset;
    else if (strcmp(name, "Levi") == 0)
        pos.x += mamedeExtraOffset;
    
    DrawTextureEx(spr, pos, 0, scale, WHITE);
    if (mostrarTudo) 
    {
        float geminiX = 49, geminiY = 67, geminiScale = 0.1f;
        float geminiW = sprGemini.width * geminiScale;
        float geminiH = sprGemini.height * geminiScale;
        const char* gemini_msg = !geminiHelpClicked ? gemini_help_msg_default : gemini_help_msg_ajuda;
        int txtSize = 20;
        float rx = geminiX + geminiW - 20.0f;
        float ry = geminiY + (geminiH - geminiRectH)/2.0f;
        float animW = geminiRectW * geminiRectAnim;
        Color logoCol = geminiMouseOver ? (Color){ 18, 60, 32, 255 } : (Color){ 26, 110, 51, 255 };

        if (geminiRectAnim > 0.01f) 
        {
            Color rectCol = (Color){ 15, 42, 26, (unsigned char)( 210 * geminiRectAnim ) };
            float round = 0.33f;
            DrawRectangleRounded((Rectangle){ rx, ry, animW, geminiRectH }, round, 12, rectCol);

            if (geminiRectAnim > 0.05f) {
                if (animW > geminiTextWidth + 2*GEMINI_PAD_X - 4)
                    DrawText(gemini_msg, rx + GEMINI_PAD_X, ry + geminiRectH/2 - txtSize/2, txtSize, WHITE);
            }
        }
        DrawTextureEx(sprGemini, (Vector2){geminiX, geminiY}, 0.0f, geminiScale, logoCol);
    }

    static float blinkTimer = 0.0f;
    blinkTimer += GetFrameTime();
    if (terminou && !acertou_resposta) 
    {
        int w = GetScreenWidth(), h = GetScreenHeight();
        int layers = 5, layerThick = 20;
        bool blink = (((int)(blinkTimer * 5.0f) % 2) == 0);
        if (blink) {
            for (int i = 0; i < layers; i++) {
                int thick = layerThick + i * layerThick;
                int alpha = 38 - i*5;
                if (alpha < 6) alpha = 6;
                Color blurRed = (Color){255, 32, 32, alpha};
                DrawRectangle(0, 0, w, thick, blurRed);
                DrawRectangle(0, h-thick, w, thick, blurRed);
                DrawRectangle(0, 0, thick, h, blurRed);
                DrawRectangle(w-thick, 0, thick, h, blurRed);
            }
        }
    }

    if ((resposta_entregue || timer_explodiu) && writer.done &&
        respostaShowTimer > RESPOSTA_MOSTRA_SEG && respostaShowTimer > LIMPEZA_TELA_RESULTADO)
    {
        float pulse = 0.07f * sinf(GetTime() * 5.0f);
        float btnScaleBase = 0.95f;
        float btnScale = btnScaleBase + pulse;
        float btnW = sprEnterButton.width * btnScale;
        float btnH = sprEnterButton.height * btnScale;
        float btnX = GetScreenWidth()/2 - btnW/2 + 120;
        float btnY = GetScreenHeight()/2 - 150;
        Color sombra = (Color){0, 0, 0, 90};
        DrawRectangleRounded((Rectangle){btnX + 8, btnY + 8, btnW, btnH}, 0.25f, 10, sombra);
        Color brilho = (pulse > 0.04f) ? (Color){255,255,255,75} : WHITE;
        DrawTextureEx(sprEnterButton, (Vector2){btnX, btnY}, 0.0f, btnScale, brilho);
    }

    if (fase3_fadeout) 
    {
        float perc = fase3_fadeout_time / FASE3_FADEOUT_DURACAO;
        if (perc > 1.0f) perc = 1.0f;
        int alpha = (int)(255 * perc);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0,0,0, alpha});
    }
    
    EndDrawing();
}

bool Fase_Desafio_03_Concluida(void)
{
    return fase_concluida;
}

void Unload_Desafio_03(void)
{
    UnloadTexture(fundo);
    UnloadTexture(pergunta_img);
    UnloadTexture(sprJoao);    UnloadTexture(sprJoao2);    UnloadTexture(sprJoao3);
    UnloadTexture(sprMateus);  UnloadTexture(sprMateus2);  UnloadTexture(sprMateus3);
    UnloadTexture(sprCarlos);  UnloadTexture(sprCarlos2);  UnloadTexture(sprCarlos3);
    UnloadTexture(sprMamede);  UnloadTexture(sprMamede2);  UnloadTexture(sprMamede3);
    UnloadTexture(sprEnterButton);
    UnloadTexture(quadroEnergia);
    UnloadTexture(imgSwitch);
    UnloadTexture(sprGemini);
    UnloadTexture(sprCadeadoFechadoCortado);
    UnloadTexture(sprCadeadoAbertoCortado);
}