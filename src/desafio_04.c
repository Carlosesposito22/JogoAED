#include "desafio_04.h"

#define FALA_NORMAL_04   "Preciso coletar apenas os arquivos essenciais! Não posso pegar antivírus!"
#define FALA_VENCEU_04   "Ótimo! Consegui reunir os arquivos certos e garantir a estabilidade do sistema."
#define FALA_PERDEU_04   "Droga! O antivírus detectou minha ação. Preciso ser mais cuidadoso na próxima vez."
#define FALA_INTRO       "Para demonstrar minha eficiência, preciso coletar o máximo de arquivos importantes.\nNão posso permitir que o antivírus me identifique, ou minha candidatura estará comprometida."
#define MAX_ARQUIVOS 7
#define FASE7_CHRONO_MAX 35.0f
#define FADEOUT_DURACAO 0.8f

typedef struct {
    Vector2 pos;
    Vector2 speed;
    float radius;
    bool cut;
    bool active;
    bool animandoParaScore;
    Vector2 animStartPos, animEndPos;
    float animTimer;
    bool antiVirus;
} Arquivo;
static Arquivo arquivos[MAX_ARQUIVOS];

typedef enum { SPRITE_NORMAL, SPRITE_VITORIA, SPRITE_DERROTA } SpriteStatus;

static char fala_exibida[512];
static int numArquivosOnda = 4;
static int rodada = 0;
static float tempoEntreArquivos = 0.30f;
static float tempoDesdeUltimo = 0.0f;
static int arquivosSpawnados = 0;
static int pontos = 0;
static int arquivosDesdeAntiVirus = 0;
static float ubuntuBaseX = 0.0f;
static float ubuntuBaseY = 0.0f;
static float ubuntuBaseW = 0.0f;
static float ubuntuTopY = 0.0f;
static float ubuntuAlturaVisivel = 0.0f;
static float ipadScorePosX = 0.0f;
static float ipadScorePosY = 0.0f;
static float ipadScoreW = 0.0f;
static float ipadScoreH = 0.0f;
static float cronometro = 0.0f;
static float cronometro_elapsed = 0.0f;
static float derrota_show_timer = 0.0f;
static float vitoria_show_timer = 0.0f;
static const float RESPOSTA_MOSTRA_SEG = 3.3f;
static float fadeout_time = 0.0f;

static bool sprEnterButtonLoaded = false;
static bool pergunta_img_loaded = false;
static bool venceu_fase = false;
static bool jogoEncerrado = false;
static bool ubuntuFundoLoaded = false;
static bool antiVirusLoaded = false;
static bool folderSpriteLoaded = false;
static bool ipadLoaded = false;
static bool podeAvancarDerrotaOuVitoria = false;
static bool faz_fadeout = false;
static bool preFalaInicial = true;
static bool fase_concluida = false; 

static TypeWriter writer;
static SpriteStatus spriteStatus = SPRITE_NORMAL;
static Texture2D sprJoao, sprJoao2, sprJoao3;
static Texture2D sprMateus, sprMateus2, sprMateus3;
static Texture2D sprCarlos, sprCarlos2, sprCarlos3;
static Texture2D sprMamede, sprMamede2, sprMamede3;
static Texture2D sprEnterButton;
static Texture2D background;
static Texture2D ubuntuFundo;
static Texture2D antiVirusSprite;
static Texture2D folderSprite;
static Texture2D ipadSprite;
static Texture2D pergunta_img;

static Music musicaFase;
static bool  audioAtivo = false;

static void DrawDialogPersonagem(int screenW, int screenH) 
{
    int boxX = 60;
    int marginBottom = 220;
    int boxY = screenH - marginBottom;
    int boxWidth = screenW - 120;
    int boxHeight = 130;
    int imgW = 1000;
    int imgH = pergunta_img_loaded ? pergunta_img.height - 130 : 220;
    int imgX = boxX;
    int imgY = boxY - imgH;
    const char *name = gSelectedCharacterName;
    
    Texture2D spr = sprJoao; 
    float scale = 0.6f;
    int carlosExtraOffset = 0, mamedeExtraOffset = 0;

    if(strcmp(name, "Dante") == 0) 
    {
        if (spriteStatus == SPRITE_VITORIA)  { spr = sprMateus2; scale = 0.8f; }
        else if (spriteStatus == SPRITE_DERROTA) { spr = sprMateus3; scale = 0.8f; }
        else { spr = sprMateus; scale = 1.3f; }
    } 
    else if(strcmp(name, "Alice") == 0) 
    {
        if (spriteStatus == SPRITE_VITORIA)  { spr = sprJoao2; scale = 0.95f; }
        else if (spriteStatus == SPRITE_DERROTA) { spr = sprJoao3; scale = 0.95f; }
        else { spr = sprJoao; scale = 0.6f;}
    } 
    else if(strcmp(name, "Jade") == 0) 
    {
        if (spriteStatus == SPRITE_VITORIA)  { spr = sprCarlos2; scale = 1.02f; carlosExtraOffset = -70;}
        else if (spriteStatus == SPRITE_DERROTA) { spr = sprCarlos3; scale = 1.0f;   carlosExtraOffset = -44;}
        else { spr = sprCarlos; scale = 0.56f; carlosExtraOffset = 0;}
    }
    else if(strcmp(name, "Levi") == 0) 
    {
        if (spriteStatus == SPRITE_VITORIA)  { spr = sprMamede2; scale = 1.0f; }
        else if (spriteStatus == SPRITE_DERROTA) { spr = sprMamede3; scale = 1.0f; }
        else { spr = sprMamede; scale = 1.0f;}
    } 
    else if(name[0]=='\0') {
        spr = sprJoao; scale = 0.6f;
    }

    float tw2 = spr.width * scale;
    float th  = spr.height * scale;
    Vector2 pos;
    pos.x = imgX - 330 + (imgW - tw2)/2.0f;
    pos.y = imgY - th + 210;
    pos.y += 36;
    if (strcmp(name, "Jade") == 0) pos.x += 24;
    if (strcmp(name, "Dante") == 0) pos.x += 30;
    if (strcmp(name, "Jade") == 0) pos.x += 100 + carlosExtraOffset;
    else if (strcmp(name, "Levi") == 0) pos.x += mamedeExtraOffset;
    DrawTextureEx(spr, pos, 0, scale, WHITE);
    
    int borderRadius = boxHeight/2;
    DrawRectangle(boxX, boxY, boxWidth - borderRadius, boxHeight, (Color){20,20,20,220});
    DrawCircle(boxX+boxWidth-borderRadius, boxY+borderRadius, borderRadius, (Color){20,20,20,220});

    if (writer.drawnChars > 0) {
        char txtbuf[512];
        strncpy(txtbuf, fala_exibida, writer.drawnChars);
        txtbuf[writer.drawnChars] = '\0';
        DrawText(txtbuf, boxX + 20, boxY + 30, 28, WHITE);
    }
}

static void AtualizaUbuntuBaseVars(void) 
{
    if (!ubuntuFundoLoaded) return;
    float scale = 0.656f;
    int corte = 140;
    float srcW = ubuntuFundo.width - 18;
    float srcH = ubuntuFundo.height - corte;
    float destWidth  = srcW * scale;
    float destHeight = srcH * scale;
    float destX = (GetScreenWidth()  - destWidth ) / 1.96f;
    float destY = (GetScreenHeight() - destHeight) / 6.0f;
    ubuntuBaseX = destX + destWidth / 2.0f;
    ubuntuBaseY = destY + destHeight;
    ubuntuBaseW = destWidth;
    ubuntuTopY  = destY;
    ubuntuAlturaVisivel = destHeight;
}
static void SpawnArquivo(Arquivo *a, int w, int h, bool antiVirus) 
{
    float minX = ubuntuBaseX - ubuntuBaseW/2.0f + 32.0f;
    float maxX = ubuntuBaseX + ubuntuBaseW/2.0f - 32.0f;
    if (maxX < minX) maxX = minX;
    a->radius = 32.0f;
    a->pos.x = GetRandomValue((int)minX, (int)maxX);
    a->pos.y = ubuntuBaseY;
    a->speed.x = GetRandomValue(-9, 9)/250.0f;
    float maxPicoAltura = ubuntuBaseY - (ubuntuAlturaVisivel * 4.0f / 5.0f);
    float alturaAteOPico = a->pos.y - maxPicoAltura;
    float grav = 0.0035f;
    a->speed.y = -sqrtf(2.0f * grav * alturaAteOPico);
    a->cut = false;
    a->active = true;
    a->animandoParaScore = false;
    a->animTimer = 0.0f;
    a->antiVirus = antiVirus;
}

void ResetArquivos(int w, int h, int total) 
{
    for (int i = 0; i < MAX_ARQUIVOS; i++) arquivos[i].active = false;
    arquivosSpawnados = 0;
    tempoDesdeUltimo = 0.0f;
    numArquivosOnda = total;
    AtualizaUbuntuBaseVars();
}

void ProximaOnda(int w, int h) 
{
    rodada++;
    int qtd = GetRandomValue(2, MAX_ARQUIVOS);
    ResetArquivos(w, h, qtd);
}

void Init_Desafio_04(void) 
{
    background = LoadTexture("src/sprites/pc_perfect.png");
    if (!ubuntuFundoLoaded) { ubuntuFundo = LoadTexture("src/sprites/ubuntuFundo.png"); ubuntuFundoLoaded = true; }
    if (!antiVirusLoaded) { antiVirusSprite = LoadTexture("src/sprites/antiVirus.png"); antiVirusLoaded = true; }
    if (!folderSpriteLoaded) { folderSprite = LoadTexture("src/sprites/os/folder.png"); folderSpriteLoaded = true; }
    if (!ipadLoaded) { ipadSprite = LoadTexture("src/sprites/ipad4.png"); ipadLoaded = true; }
    if (!sprJoao.id)     sprJoao    = LoadTexture("src/sprites/joaoSprite.png");
    if (!sprJoao2.id)    sprJoao2   = LoadTexture("src/sprites/joao2.png");
    if (!sprJoao3.id)    sprJoao3   = LoadTexture("src/sprites/joao3.png");
    if (!sprMateus.id)   sprMateus  = LoadTexture("src/sprites/mateusSprite.png");
    if (!sprMateus2.id)  sprMateus2 = LoadTexture("src/sprites/mateus2.png");
    if (!sprMateus3.id)  sprMateus3 = LoadTexture("src/sprites/mateus3.png");
    if (!sprCarlos.id)   sprCarlos  = LoadTexture("src/sprites/carlosSprite.png");
    if (!sprCarlos2.id)  sprCarlos2 = LoadTexture("src/sprites/carlos2.png");
    if (!sprCarlos3.id)  sprCarlos3 = LoadTexture("src/sprites/carlos3.png");
    if (!sprMamede.id)   sprMamede  = LoadTexture("src/sprites/mamedeSprite.png");
    if (!sprMamede2.id)  sprMamede2 = LoadTexture("src/sprites/mamede2.png");
    if (!sprMamede3.id)  sprMamede3 = LoadTexture("src/sprites/mamede3.png");
    if (!sprEnterButtonLoaded) { sprEnterButton = LoadTexture("src/sprites/enter_button.png"); sprEnterButtonLoaded = true; }
    if (!pergunta_img_loaded) { pergunta_img = LoadTexture("src/sprites/pergunta3.png"); pergunta_img_loaded = true; }

    musicaFase = LoadMusicStream("src/music/desafio-04.mp3"); 
    SetMusicVolume(musicaFase, 1.2f);
    PlayMusicStream(musicaFase);
    audioAtivo = true;

    AtualizaUbuntuBaseVars();
    strcpy(fala_exibida, FALA_INTRO);
    InitTypeWriter(&writer, fala_exibida, 27.5f);
    
    ipadScorePosX = ipadScorePosY = ipadScoreW = ipadScoreH = 0.0f;
    pontos = 0;
    rodada = 0;
    venceu_fase = false;
    jogoEncerrado = false;
    arquivosDesdeAntiVirus = 0;
    cronometro = FASE7_CHRONO_MAX;
    cronometro_elapsed = 0.0f;
    derrota_show_timer = 0.0f;
    vitoria_show_timer = 0.0f;
    podeAvancarDerrotaOuVitoria = false;
    faz_fadeout = false;
    fadeout_time = 0.0f;
    spriteStatus = SPRITE_NORMAL;
    preFalaInicial = true;
    fase_concluida = false;
}

void Update_Desafio_04(void) 
{
    float delta = GetFrameTime();
    if (audioAtivo && !preFalaInicial) UpdateMusicStream(musicaFase);

    if (preFalaInicial) 
    {
        UpdateTypeWriter(&writer, delta, IsKeyPressed(KEY_SPACE));
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            preFalaInicial = false;
            strcpy(fala_exibida, FALA_NORMAL_04);
            InitTypeWriter(&writer, fala_exibida, 27.5f);
            ResetArquivos(GetScreenWidth(), GetScreenHeight(), GetRandomValue(2, MAX_ARQUIVOS));
        }
        return;
    }

    if (faz_fadeout) { fadeout_time += delta; return; }

    if (jogoEncerrado) 
    {
        derrota_show_timer += delta;
        UpdateTypeWriter(&writer, delta, IsKeyPressed(KEY_SPACE));
        if (derrota_show_timer > RESPOSTA_MOSTRA_SEG)
            podeAvancarDerrotaOuVitoria = true;
        if (podeAvancarDerrotaOuVitoria &&
            (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
            faz_fadeout = true; 
            fadeout_time = 0.0f;

            SetD04Result(&playerStats, false, pontos);
            fase_concluida = true;
        }
        return;
    }

    if (venceu_fase) 
    {
        vitoria_show_timer += delta;
        UpdateTypeWriter(&writer, delta, IsKeyPressed(KEY_SPACE));
        if (vitoria_show_timer > RESPOSTA_MOSTRA_SEG)
            podeAvancarDerrotaOuVitoria = true;
        if (podeAvancarDerrotaOuVitoria &&
            (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
            faz_fadeout = true; 
            fadeout_time = 0.0f;

            SetD04Result(&playerStats, true, pontos);
            fase_concluida = true;
        }
        return;
    }

    cronometro_elapsed += delta;
    cronometro = FASE7_CHRONO_MAX - cronometro_elapsed;
    if (cronometro <= 0.0f && !jogoEncerrado && !venceu_fase) 
    {
        cronometro = 0.0f;
        venceu_fase = true;
        spriteStatus = SPRITE_VITORIA;
        strcpy(fala_exibida, FALA_VENCEU_04);
        InitTypeWriter(&writer, fala_exibida, 27.5f);
        vitoria_show_timer = 0.0f;
        podeAvancarDerrotaOuVitoria = false;
        return;
    }

    int w = GetScreenWidth();
    int h = GetScreenHeight();
    tempoDesdeUltimo += delta;
    if (arquivosSpawnados < numArquivosOnda && tempoDesdeUltimo >= tempoEntreArquivos * arquivosSpawnados) 
    {
        bool antiVirus = false;
        if (arquivosDesdeAntiVirus >= 5) {
            antiVirus = true;
            arquivosDesdeAntiVirus = 0;
        }
        SpawnArquivo(&arquivos[arquivosSpawnados], w, h, antiVirus);
        arquivosSpawnados++;
        if (!antiVirus) arquivosDesdeAntiVirus++;
    }

    int ativas = 0;
    for (int i = 0; i < numArquivosOnda; i++) {
        Arquivo *a = &arquivos[i];
        if (!a->active) continue;
        ativas++;
        if (a->animandoParaScore) {
            a->animTimer += delta;
            float t = a->animTimer / 0.5f;
            if (t > 1.0f) t = 1.0f;
            a->pos.x = a->animStartPos.x + (a->animEndPos.x - a->animStartPos.x) * t;
            a->pos.y = a->animStartPos.y + (a->animEndPos.y - a->animStartPos.y) * t;
            if (t >= 1.0f) {
                a->active = false;
                a->animandoParaScore = false;
                if (a->antiVirus) {
                    jogoEncerrado = true;
                    spriteStatus = SPRITE_DERROTA;
                    strcpy(fala_exibida, FALA_PERDEU_04);
                    InitTypeWriter(&writer, fala_exibida, 27.5f);
                    derrota_show_timer = 0.0f;
                    podeAvancarDerrotaOuVitoria = false;
                    return;
                } else {
                    pontos++;
                }
            }
            continue;
        }
        a->pos.x += a->speed.x;
        a->pos.y += a->speed.y;
        a->speed.y += 0.0035f;
        if (!a->cut && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            float dx = mouse.x - a->pos.x, dy = mouse.y - a->pos.y;
            if (sqrtf(dx*dx+dy*dy) < 32.0f) {
                a->cut = true;
                a->animandoParaScore = true;
                a->animStartPos = a->pos;
                if(ipadScorePosX != 0.0f || ipadScorePosY != 0.0f) {
                    a->animEndPos.x = ipadScorePosX;
                    a->animEndPos.y = ipadScorePosY;
                } else {
                    a->animEndPos.x = w-28-32;
                    a->animEndPos.y = 28+32;
                }
                a->animTimer = 0.0f;
            }
        }
        if (!a->cut && a->speed.y > 0 && a->pos.y >= ubuntuBaseY) {
            a->active = false;
        }
    }

    if (!jogoEncerrado && !venceu_fase && arquivosSpawnados == numArquivosOnda && ativas == 0 && cronometro > 0) 
    {
        ProximaOnda(w, h);
    }

    UpdateTypeWriter(&writer, delta, IsKeyPressed(KEY_SPACE));
}

void Draw_Desafio_04(void) 
{
    BeginDrawing();

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawTexturePro(
        background,
        (Rectangle){ 0, 0, (float)background.width, (float)background.height },
        (Rectangle){ 0, 0, (float)screenW, (float)screenH },
        (Vector2){0,0},
        0, WHITE
    );

    if (ubuntuFundoLoaded) 
    {
        float scale = 0.656f; int corte = 140;
        float srcW = ubuntuFundo.width - 18;
        float srcH = ubuntuFundo.height - corte;
        float destWidth  = srcW * scale;
        float destHeight = srcH * scale;
        float destX = (screenW  - destWidth ) / 1.96f;
        float destY = (screenH - destHeight) / 6.0f;
        DrawTexturePro(
            ubuntuFundo,
            (Rectangle){0, 0, srcW, srcH},
            (Rectangle){destX, destY, destWidth, destHeight},
            (Vector2){0,0}, 0, WHITE
        );
        ubuntuBaseX = destX + destWidth / 2.0f; ubuntuBaseY = destY + destHeight;
        ubuntuBaseW = destWidth; ubuntuTopY  = destY; ubuntuAlturaVisivel = destHeight;
    }

    if (preFalaInicial) 
    {
        DrawDialogPersonagem(screenW, screenH);
        if (sprEnterButtonLoaded) {
            float pulse = 0.07f * sinf(GetTime() * 3.0f);
            float btnScaleBase = 0.85f;
            float btnScale = btnScaleBase + pulse;
            float btnW = sprEnterButton.width * btnScale;
            float btnH = sprEnterButton.height * btnScale;
            float btnX = screenW/2 - btnW/2 + 630;
            float btnY = screenH/2 - 90;
            Color sombra = (Color){0,0,0,90};
            DrawRectangleRounded((Rectangle){btnX+8,btnY+8,btnW,btnH},0.25f,10,sombra);
            Color brilho = (pulse > 0.04f) ? (Color){255,255,255,75} : WHITE;
            DrawTextureEx(sprEnterButton, (Vector2){btnX,btnY}, 0.0f, btnScale, brilho);
        }
        EndDrawing();
        return;
    }

    if (!venceu_fase && !jogoEncerrado) {
        for (int i = 0; i < numArquivosOnda; i++) {
            Arquivo *a = &arquivos[i];
            if (a->active || a->animandoParaScore) {
                float destSize = a->radius*2.0f;
                if (a->antiVirus && antiVirusLoaded) {
                    DrawTexturePro(antiVirusSprite,
                        (Rectangle){0, 0, (float)antiVirusSprite.width, (float)antiVirusSprite.height},
                        (Rectangle){a->pos.x-destSize/2, a->pos.y-destSize/2, destSize, destSize},
                        (Vector2){0,0}, 0, WHITE);
                } else if (folderSpriteLoaded) {
                    DrawTexturePro(
                        folderSprite,
                        (Rectangle){0, 0, (float)folderSprite.width, (float)folderSprite.height},
                        (Rectangle){a->pos.x-destSize/2, a->pos.y-destSize/2, destSize, destSize},
                        (Vector2){0,0}, 0, WHITE);
                }
                if (a->cut && !a->animandoParaScore && a->antiVirus) {
                    DrawText("ANTI-VIRUS!", a->pos.x-55, a->pos.y-20, 22, PURPLE);
                }
            }
        }
    }

    float ipadScale = 0.7f, ipadOffsetX = 30.0f, ipadOffsetY = 100.0f, ipadW = 0, ipadH = 0, ipadX = 0, ipadY = 0;
    if (ipadLoaded) 
    {
        ipadW = ipadSprite.width * ipadScale;
        ipadH = ipadSprite.height * ipadScale;
        ipadX = screenW - ipadW - ipadOffsetX;
        ipadY = screenH - ipadH - ipadOffsetY;
        DrawTexturePro(ipadSprite,
            (Rectangle){0, 0, (float)ipadSprite.width, (float)ipadSprite.height},
            (Rectangle){ipadX, ipadY, ipadW, ipadH},
            (Vector2){0,0}, 0, WHITE);
        char pontosTxt[16];
        sprintf(pontosTxt, "%d", pontos);
        int fontSize = (int)(ipadH * 0.08f); if(fontSize < 10) fontSize = 10;
        int textW = MeasureText(pontosTxt, fontSize), textH = fontSize;
        float textX = ipadX + (ipadW / 2) - (textW / 2);
        float textY = ipadY + (ipadH / 2) - (textH / 2 + 68);
        Color verdeNeonEscuro = (Color){0, 200, 100, 255};
        DrawText(pontosTxt, (int)textX, (int)textY, fontSize, verdeNeonEscuro);
        ipadScorePosX = ipadX + (ipadW/2); ipadScorePosY = ipadY + (ipadH / 2) - 68;
        ipadScoreW = ipadW; ipadScoreH = ipadH;
    }

    DrawDialogPersonagem(screenW, screenH);

    if ((venceu_fase || jogoEncerrado) && !faz_fadeout) 
    {
        float timerFinal = venceu_fase ? vitoria_show_timer : derrota_show_timer;
        int w = screenW, h = screenH;
        int layers = 5, layerThick = 20;
        int pisca = ((int)(timerFinal * 4.1f))%2;
        for (int i = 0; i < layers; i++) {
            int thick = layerThick + i * layerThick;
            int alpha = (venceu_fase?(pisca?38-i*5:25-i*5):(38-i*5));
            if(alpha<6) alpha=6;
            Color base = venceu_fase ? (Color){40,195,70,alpha} : (Color){255,32,32,alpha};
            DrawRectangle(0,0,w,thick,base); DrawRectangle(0,h-thick,w,thick,base);
            DrawRectangle(0,0,thick,h,base); DrawRectangle(w-thick,0,thick,h,base);
        }
        if (timerFinal > RESPOSTA_MOSTRA_SEG && sprEnterButtonLoaded) {
            float pulse = 0.07f * sinf(GetTime() * 3.0f);
            float btnScaleBase = 0.85f;
            float btnScale = btnScaleBase + pulse;
            float btnW = sprEnterButton.width * btnScale;
            float btnH = sprEnterButton.height * btnScale;
            float btnX = screenW/2 - btnW/2 + 630;
            float btnY = screenH/2 - 90;
            Color sombra = (Color){0,0,0,90};
            DrawRectangleRounded((Rectangle){btnX+8,btnY+8,btnW,btnH},0.25f,10,sombra);
            Color brilho = (pulse > 0.04f) ? (Color){255,255,255,75} : WHITE;
            DrawTextureEx(sprEnterButton, (Vector2){btnX,btnY}, 0.0f, btnScale, brilho);
        }
        EndDrawing();
        return;
    }

    if (!jogoEncerrado && !venceu_fase && !faz_fadeout) DrawChronometer(cronometro, FASE7_CHRONO_MAX, screenW - 80, 80, 55);

    if (faz_fadeout) 
    {
        float perc = fadeout_time / FADEOUT_DURACAO;
        if (perc > 1.0f) perc = 1.0f;
        int alpha = (int)(255 * perc);
        DrawRectangle(0, 0, screenW, screenH, (Color){0,0,0, alpha});
    }

    EndDrawing();
}

bool Fase_Desafio_04_Concluida(void)
{
    return fase_concluida;
}

void Unload_Desafio_04(void) {
    UnloadTexture(background);
    if (ubuntuFundoLoaded) { UnloadTexture(ubuntuFundo); ubuntuFundoLoaded = false; }
    if (antiVirusLoaded) { UnloadTexture(antiVirusSprite); antiVirusLoaded = false; }
    if (folderSpriteLoaded) { UnloadTexture(folderSprite); folderSpriteLoaded = false; }
    if (ipadLoaded) { UnloadTexture(ipadSprite); ipadLoaded = false; }
    if (sprJoao.id)    UnloadTexture(sprJoao);
    if (sprJoao2.id)   UnloadTexture(sprJoao2);
    if (sprJoao3.id)   UnloadTexture(sprJoao3);
    if (sprMateus.id)  UnloadTexture(sprMateus);
    if (sprMateus2.id) UnloadTexture(sprMateus2);
    if (sprMateus3.id) UnloadTexture(sprMateus3);
    if (sprCarlos.id)  UnloadTexture(sprCarlos);
    if (sprCarlos2.id) UnloadTexture(sprCarlos2);
    if (sprCarlos3.id) UnloadTexture(sprCarlos3);
    if (sprMamede.id)  UnloadTexture(sprMamede);
    if (sprMamede2.id) UnloadTexture(sprMamede2);
    if (sprMamede3.id) UnloadTexture(sprMamede3);
    if (sprEnterButtonLoaded) { UnloadTexture(sprEnterButton); sprEnterButtonLoaded = false; }
    if (pergunta_img_loaded) { UnloadTexture(pergunta_img); pergunta_img_loaded = false; }
    
    if (audioAtivo) {
        StopMusicStream(musicaFase);
        UnloadMusicStream(musicaFase);
        audioAtivo = false;
    }
}