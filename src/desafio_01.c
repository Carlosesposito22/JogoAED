#include "desafio_01.h"
#include <string.h>
#include <math.h>
#define NUM_COLUNAS 6
#define MAX_LINHAS 40
#define BALL_SIZE 28
#define PADDLE_W 32
#define PADDLE_H 120
#define BALL_SPEED 620
#define BALL_MAX_SPEED 760
#define FADEOUT_DURACAO 0.8f
#define ANTIVIRUS_SPEED_FACTOR 0.8f
#define ANTIVIRUS_SPAWN_CHANCE 9
#define TOPO_BLOCOS    48
#define LIMITE_INFERIOR (GetScreenHeight() - 270)
#define MAX_BOLAS 10
#define MAX_ANTIVIRUS 4
#define SC 0.8f
#define BLOCOS_RIGHT_MARGIN 30
#define BLOCOS_RIGHT_MARGIN_EXTRA 34
#define AREA_PONG_X 454
#define AREA_PONG_Y TOPO_BLOCOS
#define AREA_PONG_W (GetScreenWidth() - AREA_PONG_X)
#define AREA_PONG_H (LIMITE_INFERIOR - TOPO_BLOCOS)
#define PONG_AREA_W  ((int)(AREA_PONG_W * SC)+92)
#define PONG_AREA_H  ((int)(AREA_PONG_H * SC)+10)
#define PONG_AREA_X  ((GetScreenWidth()  - PONG_AREA_W)/2+6)
#define PONG_AREA_Y  ((GetScreenHeight() - PONG_AREA_H)/2-160)
#define FALA_NORMAL_03 "Quebre uma barreira de blocos vermelhos usando a bola e vença o PongWall!\n\nMova a barra com o mouse. Clique para lançar."
#define FALA_ACERTO_03 "Escolha certeira! Hank deve ter anotado meu cuidado — estou um passo à frente na vaga."
#define FALA_ERRO_03   "Putz… decisão errada! O sistema de monitoramento disparou e Hank vai notar esse deslize."
#define FALA_JOAO_ACERTO "Ufa, rebati bem! Venha o próximo desafio!"
#define FALA_JOAO_ERRO   "Droga, perdi! O alarme tocou e o Hank vai questionar minha atenção."
#define FALA_CARLOS_ACERTO "Isso foi tranquilo — Pong na veia. Mais pontos pra mim!"
#define FALA_CARLOS_ERRO   "Como deixei passar essa? Um erro bobo bem na frente do Hank."
#define FALA_MAMEDE_ACERTO "Perfeito. Cada etapa concluída me deixa mais perto da contratação."
#define FALA_MAMEDE_ERRO   "Ah, vacilei justo agora! Preciso compensar nos próximos testes ou perco a chance."
#define EXPLO_FRAME_DUR 0.10f
#define EXPLO_TOTAL_FRAMES 10
#define FALA_INTRO_03 "Mostre habilidade no pong: destrua blocos vermelhos, fuja dos antivírus e impressione Hank!"
#define FASE3_CHRONO_MAX 120.0f

static bool preFalaInicial = true;
static float cronometro = 0.0f;
static float cronometro_elapsed = 0.0f;
static bool cronometro_iniciado = false;
typedef struct {
    float x, y, w, h;
} ExploFrame;
static const ExploFrame explosionFrames[EXPLO_TOTAL_FRAMES] = {
    {-69,   -63,  33,  40},
    {-203,  -59,  48,  50},
    {-336,  -46,  70,  76},
    {-477,  -43,  85,  95},
    {-610,  -30,  91, 109},
    {-37,  -179, 103, 113},
    {-180, -186,  99, 105},
    {-331, -187,  96, 101},
    {-467, -177,  98, 113},
    {-618, -180,  87, 109}
};
typedef struct {
    Rectangle rect;
    bool ativa;
    bool amarelo;
    bool exploding;
    float explosionTimer;
    int explosionFrame;
    bool destroyed;
} BlocoVermelho;
typedef struct {
    Vector2 pos;
    Vector2 vel;
    bool lancada;
    bool ativa;
    float angle;
    float angularSpeed;
    float pulseOffset;
} Bola;
typedef struct {
    Rectangle rect;
} Paddle;
typedef struct {
    Vector2 pos;
    Vector2 vel;
    bool ativa;
    Rectangle rect;
} AntiVirus;
static AntiVirus antiviruses[MAX_ANTIVIRUS];
static Texture2D sprAntiVirus;
static TypeWriter writer;
static char fala_exibida[512];
static bool podeAvancar = false;
static float respostaShowTimer = 0.0f;
static const float RESPOSTA_MOSTRA_SEG = 5.0f;
static bool fadeout_ativo = false;
static float fadeout_time = 0.0f;
static bool fase_concluida = false;
static Texture2D fundo;
static Texture2D pergunta_img;
static Texture2D sprJoao, sprJoao2, sprJoao3;
static Texture2D sprMateus, sprMateus2, sprMateus3;
static Texture2D sprCarlos, sprCarlos2, sprCarlos3;
static Texture2D sprMamede, sprMamede2, sprMamede3;
static Texture2D sprEnterButton;
static Texture2D texParede;
static Texture2D sprVirus;
static Texture2D sprBaseJogo;
static Texture2D bgJogo;
static Texture2D exploTex;
static BlocoVermelho blocos[NUM_COLUNAS][MAX_LINHAS];
static int linhas_ativas = 0;
static int blocos_ativos = 0;
static Bola bolas[MAX_BOLAS];
static int num_bolas = 2;
static Paddle paddle;
static bool pong_vitoria = false;
static bool pong_derrota = false;
static bool pong_mostrabotao = false;

static Music musicaFase;
static bool  audioAtivo = false;

// --- ESCOLHA DE SPRITE IGUAL DESAFIO 01 ---
typedef struct {
    Texture2D* sprite;
    float scale;
    int carlosExtraOffset;
    int mamedeExtraOffset;
} CharacterSpriteResult;
CharacterSpriteResult GetCharacterSprite(const char* name, bool ganhou, bool perdeu) {
    CharacterSpriteResult res = {&sprJoao, 0.6f, 0, 0}; // Se vazio: sprJoao!
    if (!name) name = "";
    if (strcmp(name, "Dante") == 0) {
        if (ganhou)      { res.sprite = &sprMateus2; res.scale = 0.8f; }
        else if(perdeu)  { res.sprite = &sprMateus3; res.scale = 0.8f; }
        else             { res.sprite = &sprMateus;  res.scale = 1.3f; }
    }
    else if (strcmp(name, "Alice") == 0) {
        if (ganhou)      { res.sprite = &sprJoao2; res.scale = 0.95f; }
        else if(perdeu)  { res.sprite = &sprJoao3; res.scale = 0.95f; }
        else             { res.sprite = &sprJoao;  res.scale = 0.6f; }
    }
    else if (strcmp(name, "Jade") == 0) {
        if (ganhou)      { res.sprite = &sprCarlos2; res.scale = 1.02f; res.carlosExtraOffset = -70; }
        else if(perdeu)  { res.sprite = &sprCarlos3; res.scale = 1.0f;  res.carlosExtraOffset = -44; }
        else             { res.sprite = &sprCarlos;  res.scale = 0.56f; res.carlosExtraOffset = 0; }
    }
    else if (strcmp(name, "Levi") == 0) {
        if (ganhou)      { res.sprite = &sprMamede2; res.scale = 1.0f; }
        else if(perdeu)  { res.sprite = &sprMamede3; res.scale = 1.0f; }
        else             { res.sprite = &sprMamede;  res.scale = 1.0f; }
    }
    // Se vazio, já é sprJoao por default
    return res;
}
// --------------------------------------------------------
const char* FalaPorResultado(const char* name, bool acerto) {
    if (!name || !name[0]) name = "Alice";
    if (strcmp(name, "Alice") == 0)      return acerto ? FALA_JOAO_ACERTO   : FALA_JOAO_ERRO;
    if (strcmp(name, "Jade") == 0)       return acerto ? FALA_CARLOS_ACERTO : FALA_CARLOS_ERRO;
    if (strcmp(name, "Levi") == 0)       return acerto ? FALA_MAMEDE_ACERTO : FALA_MAMEDE_ERRO;
    return acerto ? FALA_ACERTO_03 : FALA_ERRO_03;
}

static inline int CountActiveBolas(void) {
    int c = 0; for (int i = 0; i < MAX_BOLAS;     ++i) if (bolas[i].ativa)       ++c;
    return c;
}
static inline int CountActiveAntivirus(void) {
    int c = 0; for (int i = 0; i < MAX_ANTIVIRUS; ++i) if (antiviruses[i].ativa) ++c;
    return c;
}

static inline void ClampBallSpeed(Bola *b)
{
    float vmax = BALL_MAX_SPEED * SC;
    float v = sqrtf(b->vel.x * b->vel.x + b->vel.y * b->vel.y);
    if (v > vmax) {
        float f = vmax / v;
        b->vel.x *= f;
        b->vel.y *= f;
    }
}

void Init_Desafio_01(void)
{
    fundo         = LoadTexture("src/sprites/pc_perfect.png");
    sprJoao       = LoadTexture("src/sprites/joaoSprite.png");
    sprJoao2      = LoadTexture("src/sprites/joao2.png");
    sprJoao3      = LoadTexture("src/sprites/joao3.png");
    sprMateus     = LoadTexture("src/sprites/mateusSprite.png");
    sprMateus2    = LoadTexture("src/sprites/mateus2.png");
    sprMateus3    = LoadTexture("src/sprites/mateus3.png");
    sprCarlos     = LoadTexture("src/sprites/carlosSprite.png");
    sprCarlos2    = LoadTexture("src/sprites/carlos2.png");
    sprCarlos3    = LoadTexture("src/sprites/carlos3.png");
    sprMamede     = LoadTexture("src/sprites/mamedeSprite.png");
    sprMamede2    = LoadTexture("src/sprites/mamede2.png");
    sprMamede3    = LoadTexture("src/sprites/mamede3.png");
    sprEnterButton= LoadTexture("src/sprites/enter_button.png");
    pergunta_img  = LoadTexture("src/sprites/pergunta3.png");
    texParede     = LoadTexture("src/sprites/paredepong2.png");
    sprVirus      = LoadTexture("src/sprites/virus.png");
    sprBaseJogo   = LoadTexture("src/sprites/baseJogo.png");
    bgJogo        = LoadTexture("src/sprites/bgJogo.png");
    sprAntiVirus  = LoadTexture("src/sprites/antiVirus.png");
    exploTex      = LoadTexture("src/sprites/explo.png");
    musicaFase = LoadMusicStream("src/music/desafio-01.mp3"); 
    SetMusicVolume(musicaFase, 1.2f);
    PlayMusicStream(musicaFase);
    audioAtivo = true;

    SetTextureFilter(texParede, TEXTURE_FILTER_POINT);
    preFalaInicial = true;
    strcpy(fala_exibida, FALA_INTRO_03);
    InitTypeWriter(&writer, fala_exibida, 26.0f); 
    cronometro = FASE3_CHRONO_MAX;
    cronometro_elapsed = 0.0f;
    cronometro_iniciado = false;
    podeAvancar         = false;
    respostaShowTimer   = 0.0f;
    fadeout_ativo       = false;
    fadeout_time        = 0.0f;
    fase_concluida      = false;

    paddle.rect.x = PONG_AREA_X+20;
    paddle.rect.width  = PADDLE_W * SC;
    paddle.rect.height = PADDLE_H * SC;
    paddle.rect.y = PONG_AREA_Y + ((PONG_AREA_H/2 - paddle.rect.height/2));
    memset(bolas, 0, sizeof(bolas));
    memset(antiviruses, 0, sizeof(antiviruses));
    num_bolas = 1;
    bolas[0].pos     = (Vector2){
        paddle.rect.x + paddle.rect.width + 6*SC,
        paddle.rect.y + paddle.rect.height/2 - BALL_SIZE*SC/2
    };
    bolas[0].vel     = (Vector2){ 0, 0 };
    bolas[0].lancada = false;
    bolas[0].ativa   = true;
    bolas[0].angle = 0.0f;
    bolas[0].angularSpeed = 40.0f;
    bolas[0].pulseOffset = (float)GetRandomValue(0, 6283)/1000.0f;
    for (int i=1; i<MAX_BOLAS; i++) {
        bolas[i].ativa = false;
        bolas[i].lancada = true;
        bolas[i].angle = 0.0f;
        bolas[i].angularSpeed = 40.0f;
        bolas[i].pulseOffset = (float)GetRandomValue(0, 6283)/1000.0f;
    }
    float bloco_base_w = 24.0f * SC;
    float bloco_base_h = bloco_base_w;
    float bloco_w = bloco_base_w * 1.5f;
    float bloco_h = bloco_base_h * 1.5f;
    float altura_util = (float)(PONG_AREA_H);
    int max_linhas_possivel = (int)(altura_util / bloco_h);
    if (max_linhas_possivel < 1) max_linhas_possivel = 1;
    linhas_ativas = max_linhas_possivel;
    float largura_total_nova = NUM_COLUNAS * bloco_w;
    float blocos_right_x = PONG_AREA_X + PONG_AREA_W - (BLOCOS_RIGHT_MARGIN + BLOCOS_RIGHT_MARGIN_EXTRA);
    float esquerda = blocos_right_x - largura_total_nova;
    float topo = PONG_AREA_Y;
    for(int cx=0; cx<NUM_COLUNAS; ++cx) {
        for(int ly=0; ly<linhas_ativas; ++ly) {
            float x = esquerda + cx * bloco_w;
            float y = topo + ly * bloco_h;
            blocos[cx][ly].rect.x = (int)x;
            blocos[cx][ly].rect.y = (int)y;
            if(cx == NUM_COLUNAS-1)
                blocos[cx][ly].rect.width = (int)(esquerda + largura_total_nova - x + 0.5f);
            else
                blocos[cx][ly].rect.width = (int)(bloco_w + 0.5f);
            if(ly == linhas_ativas-1)
                blocos[cx][ly].rect.height = (int)(topo + altura_util - y + 0.5f);
            else
                blocos[cx][ly].rect.height = (int)(bloco_h + 0.5f);
            blocos[cx][ly].ativa = true;
            blocos[cx][ly].amarelo = false;
            blocos[cx][ly].exploding = false;
            blocos[cx][ly].explosionTimer = 0.0f;
            blocos[cx][ly].explosionFrame = 0;
            blocos[cx][ly].destroyed = false;
        }
    }
    for(int cx=0; cx<NUM_COLUNAS; ++cx) {
        for(int ly=linhas_ativas; ly<MAX_LINHAS; ++ly) {
            blocos[cx][ly].ativa = false;
            blocos[cx][ly].amarelo = false;
            blocos[cx][ly].exploding = false;
            blocos[cx][ly].explosionTimer = 0.0f;
            blocos[cx][ly].explosionFrame = 0;
            blocos[cx][ly].destroyed = false;
        }
    }
    int totalBlocos = NUM_COLUNAS * linhas_ativas;
    int totalAmarelos = (int)(totalBlocos * 0.3f);
    int countAmarelo = 0;
    while (countAmarelo < totalAmarelos) {
        int col = GetRandomValue(0, NUM_COLUNAS-1);
        int lin = GetRandomValue(0, linhas_ativas-1);
        if (!blocos[col][lin].amarelo) {
            blocos[col][lin].amarelo = true;
            countAmarelo++;
        }
    }
    blocos_ativos = totalBlocos;
    pong_vitoria = false;
    pong_derrota = false;
    pong_mostrabotao = false;
}


void Update_Desafio_01(void)
{
    float dt = GetFrameTime();
    Vector2 mouse = GetMousePosition();

    if (audioAtivo) UpdateMusicStream(musicaFase);

    if (preFalaInicial)
    {
        UpdateTypeWriter(&writer, dt, IsKeyPressed(KEY_SPACE));
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
        {
            preFalaInicial = false;
            strcpy(fala_exibida, FALA_NORMAL_03);
            InitTypeWriter(&writer, fala_exibida, 25.0f);
            cronometro_elapsed = 0.0f;
        }
        return;
    }
    float miny = PONG_AREA_Y;
    float maxy = PONG_AREA_Y + PONG_AREA_H - paddle.rect.height;
    paddle.rect.y = mouse.y - paddle.rect.height/2;
    if (paddle.rect.y < miny) paddle.rect.y = miny;
    if (paddle.rect.y > maxy) paddle.rect.y = maxy;
    if (fadeout_ativo) {
        fadeout_time += dt;
        if (fadeout_time >= FADEOUT_DURACAO) {
            bool sucesso = pong_vitoria && !pong_derrota;
            SetD01Result(&playerStats, sucesso, 0);
            fase_concluida = true;
        }
        return;
    }
    if (pong_vitoria || pong_derrota) {
        respostaShowTimer += dt;
        if (respostaShowTimer > RESPOSTA_MOSTRA_SEG) pong_mostrabotao = true;
        if (pong_mostrabotao) {
            Vector2 mpos = GetMousePosition();
            float btnScale = 0.9f + 0.07f*sinf(GetTime()*3.0f);
            float btnW = sprEnterButton.width * btnScale;
            float btnH = sprEnterButton.height * btnScale;
            float btnX = GetScreenWidth()/2 + 120 - btnW/2;
            float btnY = GetScreenHeight()/2 - 130;
            Rectangle btnB = {btnX, btnY, btnW, btnH};
            if((CheckCollisionPointRec(mpos, btnB) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_ENTER)) {
                fadeout_ativo = true;
                fadeout_time = 0.0f;
            }
        }
        UpdateTypeWriter(&writer, dt, IsKeyPressed(KEY_SPACE));
        if(!podeAvancar && writer.done) podeAvancar = true;
        return;
    }
    if (cronometro_iniciado) {
        cronometro_elapsed += dt;
        cronometro = FASE3_CHRONO_MAX - cronometro_elapsed;
        if(cronometro <= 0.0f && !pong_derrota && !pong_vitoria) {
            cronometro = 0.0f;
            pong_derrota = true;
            const char* name = gSelectedCharacterName;
            strcpy(fala_exibida, FalaPorResultado(name, false));
            InitTypeWriter(&writer, fala_exibida, 25.0f);
            respostaShowTimer = 0.0f;
            return;
        }
    }
    for(int cx=0; cx<NUM_COLUNAS; ++cx) {
        for(int ly=0; ly<linhas_ativas; ++ly) {
            BlocoVermelho* blk = &blocos[cx][ly];
            if(blk->exploding && blk->explosionFrame < EXPLO_TOTAL_FRAMES) {
                blk->explosionTimer += dt;
                if(blk->explosionTimer >= EXPLO_FRAME_DUR) {
                    blk->explosionTimer -= EXPLO_FRAME_DUR;
                    blk->explosionFrame++;
                    if(blk->explosionFrame >= EXPLO_TOTAL_FRAMES) {
                        blk->exploding = false;
                        blk->explosionFrame = EXPLO_TOTAL_FRAMES;
                    }
                }
            }
        }
    }
    for (int i = 0; i < num_bolas; ++i)
    {
        if (!bolas[i].ativa) continue;
        if (!bolas[i].lancada)
        {
            bolas[i].pos.x = paddle.rect.x + paddle.rect.width + 6*SC;
            bolas[i].pos.y = paddle.rect.y + paddle.rect.height/2 - BALL_SIZE*SC/2;
            if (i==0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                float ang = (float)(GetRandomValue(-20,20)*PI/180.0f);
                float vel = BALL_SPEED * SC;
                bolas[i].vel.x = -fabsf(vel * cosf(ang));
                bolas[i].vel.y = BALL_SPEED * SC * sinf(ang);
                bolas[i].lancada = true;
                if (!cronometro_iniciado) {
                    cronometro_iniciado = true;
                    cronometro_elapsed = 0.0f;
                }
            }
        }
    }
    for (int b=0; b<num_bolas; b++)
    {
        if (!bolas[b].ativa) continue;
        if (!bolas[b].lancada) continue;
        bolas[b].pos.x += bolas[b].vel.x * dt;
        bolas[b].pos.y += bolas[b].vel.y * dt;
        float minSpeed = 40.0f*SC, decaimento = 0.96f;
        bolas[b].angle += bolas[b].angularSpeed * dt;
        while (bolas[b].angle > 360.0f) bolas[b].angle -= 360.0f;
        while (bolas[b].angle < 0.0f)   bolas[b].angle += 360.0f;
        if (bolas[b].angularSpeed > minSpeed)
            bolas[b].angularSpeed *= decaimento;
        if (bolas[b].angularSpeed < minSpeed)
            bolas[b].angularSpeed = minSpeed;
        if (bolas[b].pos.y < PONG_AREA_Y) {
            bolas[b].pos.y = PONG_AREA_Y;
            bolas[b].vel.y *= -1.0f;
            bolas[b].angularSpeed += 130.0f;
            if (bolas[b].angularSpeed > 360.0f) bolas[b].angularSpeed = 360.0f;
        }
        if (bolas[b].pos.y + BALL_SIZE*SC > PONG_AREA_Y + PONG_AREA_H) {
            bolas[b].pos.y = PONG_AREA_Y + PONG_AREA_H - BALL_SIZE*SC;
            bolas[b].vel.y *= -1.0f;
            bolas[b].angularSpeed += 130.0f;
            if (bolas[b].angularSpeed > 360.0f) bolas[b].angularSpeed = 360.0f;
        }
        if (bolas[b].pos.x < PONG_AREA_X) {
            int vivos = 0;
            for (int k=0; k<num_bolas; k++)
                if (k != b && bolas[k].ativa) vivos++;
            if (vivos == 0) {
                bolas[b].ativa = false;
                if (b == num_bolas - 1) {
                    while (num_bolas > 0 && !bolas[num_bolas - 1].ativa)
                        --num_bolas;
                }
                pong_derrota = true;
                const char* name = gSelectedCharacterName;
                strcpy(fala_exibida, FalaPorResultado(name, false));
                InitTypeWriter(&writer, fala_exibida, 25.5f);
                respostaShowTimer = 0.0f;
                return;
            } else {
                bolas[b].ativa = false;
                if (b == num_bolas - 1) {
                    while (num_bolas > 0 && !bolas[num_bolas - 1].ativa)
                        --num_bolas;
                }
                continue;
            }
        }
        if (bolas[b].pos.x > PONG_AREA_X + PONG_AREA_W) {
            if (!pong_vitoria) {
                pong_vitoria = true;
                const char* name = gSelectedCharacterName;
                strcpy(fala_exibida, FalaPorResultado(name, true));
                InitTypeWriter(&writer, fala_exibida, 25.0f);
                respostaShowTimer = 0.0f;
                return;
            }
        }
        Rectangle bolaR = {bolas[b].pos.x, bolas[b].pos.y, BALL_SIZE*SC, BALL_SIZE*SC};
        if (bolas[b].vel.x < 0 && CheckCollisionRecs(bolaR, paddle.rect)) {
            bolas[b].pos.x = paddle.rect.x + paddle.rect.width + 1.5f*SC;
            bolas[b].vel.x *= -1.0f;
            float interY = (bolas[b].pos.y + BALL_SIZE*SC/2) - (paddle.rect.y + paddle.rect.height/2);
            bolas[b].vel.y += interY * 2.35f;
            float v = sqrtf(bolas[b].vel.x*bolas[b].vel.x + bolas[b].vel.y*bolas[b].vel.y);
            if (v > BALL_MAX_SPEED*SC) {
                bolas[b].vel.x *= BALL_MAX_SPEED * SC / v;
                bolas[b].vel.y *= BALL_MAX_SPEED * SC / v;
            }
            bolas[b].angularSpeed += 130.0f;
            if (bolas[b].angularSpeed > 360.0f) bolas[b].angularSpeed = 360.0f;
        }
        for(int cx=0; cx<NUM_COLUNAS; ++cx) {
            for(int ly=0; ly<linhas_ativas; ++ly) {
                BlocoVermelho* blk = &blocos[cx][ly];
                if (blk->ativa && !blk->destroyed && !blk->exploding && CheckCollisionRecs(bolaR, blk->rect)) {
                    float old_vx = bolas[b].vel.x, old_vy = bolas[b].vel.y;
                    Rectangle r = blk->rect;
                    bool colidiuPeloX = (bolas[b].pos.x+BALL_SIZE*SC-3 < r.x || bolas[b].pos.x > r.x+r.width-3);
                    if (blk->amarelo && CountActiveBolas() < MAX_BOLAS) {
                        int added = 0;
                        for (int n=0; n<MAX_BOLAS && num_bolas < MAX_BOLAS && added < 1; n++) {
                            if (!bolas[n].ativa) {
                                bolas[n].ativa = true;
                                bolas[n].lancada = true;
                                bolas[n].pos = bolas[b].pos;
                                bolas[n].angle = bolas[b].angle;
                                bolas[n].angularSpeed = bolas[b].angularSpeed;
                                bolas[n].pulseOffset = (float)GetRandomValue(0, 6283)/1000.0f;
                                float a = (float)(GetRandomValue(-60,60)*PI/180.0f);
                                float base_angle = atan2f(bolas[b].vel.y, bolas[b].vel.x);
                                float new_angle = base_angle + a;
                                bolas[n].vel.x = -fabsf(BALL_SPEED * SC * cosf(new_angle));
                                bolas[n].vel.y = BALL_SPEED * SC * sinf(new_angle);
                                ClampBallSpeed(&bolas[n]);
                                ++added;
                                if (n >= num_bolas) num_bolas = n+1;
                            }
                        }
                    }
                    blk->destroyed = true;
                    blk->exploding = true;
                    blk->explosionTimer = 0.0f;
                    blk->explosionFrame = 0;
                    blk->ativa = false;
                    blocos_ativos--;
                    ClampBallSpeed(&bolas[b]);
                    if (GetRandomValue(1, 100) <= ANTIVIRUS_SPAWN_CHANCE && CountActiveAntivirus() < MAX_ANTIVIRUS) {
                        for(int a=0; a<MAX_ANTIVIRUS; ++a) {
                            if (!antiviruses[a].ativa) {
                                antiviruses[a].ativa = true;
                                antiviruses[a].pos.x = r.x + r.width/2.0f - (BALL_SIZE*SC)/2.0f;
                                antiviruses[a].pos.y = r.y + r.height/2.0f - (BALL_SIZE*SC)/2.0f;
                                antiviruses[a].rect.x = antiviruses[a].pos.x;
                                antiviruses[a].rect.y = antiviruses[a].pos.y;
                                antiviruses[a].rect.width = BALL_SIZE*SC;
                                antiviruses[a].rect.height= BALL_SIZE*SC;
                                float boost = ANTIVIRUS_SPEED_FACTOR;
                                float vx, vy;
                                if (colidiuPeloX) {
                                    vx = old_vx * boost;
                                    vy = -old_vy * boost;
                                } else {
                                    vx = -old_vx * boost;
                                    vy = old_vy * boost;
                                }
                                antiviruses[a].vel.x = -fabsf(vx);
                                antiviruses[a].vel.y = vy;
                                if (fabsf(antiviruses[a].vel.x) < 40.0f)
                                    antiviruses[a].vel.x = -40.0f;
                                if (fabsf(antiviruses[a].vel.y) < 10.0f)
                                    antiviruses[a].vel.y = 0;
                                break;
                            }
                        }
                    }
                    if(colidiuPeloX)
                        bolas[b].vel.x *= -1.0f;
                    else
                        bolas[b].vel.y *= -1.0f;
                    bolas[b].angularSpeed += 130.0f;
                    ClampBallSpeed(&bolas[b]);
                    if (bolas[b].angularSpeed > 360.0f) bolas[b].angularSpeed = 360.0f;
                    goto cond2;
                }
            }
        }
        cond2: ;
    }
    for (int a=0; a<MAX_ANTIVIRUS; a++){
        if (!antiviruses[a].ativa) continue;
        antiviruses[a].pos.x += antiviruses[a].vel.x * dt;
        antiviruses[a].pos.y += antiviruses[a].vel.y * dt;
        antiviruses[a].rect.x = antiviruses[a].pos.x;
        antiviruses[a].rect.y = antiviruses[a].pos.y;
        if (antiviruses[a].pos.y < PONG_AREA_Y) {
            antiviruses[a].pos.y = PONG_AREA_Y;
            antiviruses[a].vel.y *= -1.0f;
        }
        if (antiviruses[a].pos.y + antiviruses[a].rect.height > PONG_AREA_Y + PONG_AREA_H) {
            antiviruses[a].pos.y = PONG_AREA_Y + PONG_AREA_H - antiviruses[a].rect.height;
            antiviruses[a].vel.y *= -1.0f;
        }
        if (antiviruses[a].pos.x + antiviruses[a].rect.width < PONG_AREA_X) {
            antiviruses[a].ativa = false;
        }
        if (CheckCollisionRecs(antiviruses[a].rect, paddle.rect)) {
            antiviruses[a].ativa = false;
            pong_derrota = true;
            const char* name = gSelectedCharacterName;
            strcpy(fala_exibida, FalaPorResultado(name, false));
            InitTypeWriter(&writer, fala_exibida, 25.5f);
            respostaShowTimer = 0.0f;
            return;
        }
    }
    bool tem_bola = false;
    for (int k=0; k<num_bolas; ++k)
        if (bolas[k].ativa) tem_bola=true;
    if (!tem_bola){
        pong_derrota = true;
        const char* name = gSelectedCharacterName;
        strcpy(fala_exibida, FalaPorResultado(name, false));
        InitTypeWriter(&writer, fala_exibida, 25.5f);
        respostaShowTimer = 0.0f;
        return;
    }
    UpdateTypeWriter(&writer, dt, IsKeyPressed(KEY_SPACE));
    if(!podeAvancar && writer.done) podeAvancar = true;
}
void Draw_Desafio_01(void)
{
    static float blinkTimer = 0.0f;
    float delta = GetFrameTime();
    blinkTimer += delta;
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawTextureEx(fundo, (Vector2){0,0}, 0.0f, (float)GetScreenWidth() / fundo.width, RAYWHITE);
    DrawTexturePro(bgJogo,
        (Rectangle){ 0, 0, bgJogo.width, bgJogo.height },
        (Rectangle){ PONG_AREA_X, PONG_AREA_Y, PONG_AREA_W, PONG_AREA_H },
        (Vector2){ 0, 0 }, 0.0f, WHITE);
    if (preFalaInicial)
    {
        int boxX = 60;
        int marginBottom = 220;
        int boxY = GetScreenHeight() - marginBottom;
        int boxWidth = GetScreenWidth() - 120;
        int boxHeight = 130;
        int imgW = 1000;
        int imgH = pergunta_img.height - 130;
        int imgX = boxX, imgY = boxY - imgH;
        DrawTexturePro(pergunta_img, (Rectangle){0,0,pergunta_img.width, pergunta_img.height},
            (Rectangle){imgX, imgY, imgW, imgH}, (Vector2){0,0}, 0, WHITE);
        // --- Exibir nome personagem igual desafio 01 ---
        const char* nome_personagem = (gSelectedCharacterName[0] == '\0') ? "???" : gSelectedCharacterName;
        DrawText(nome_personagem, imgX + 10, imgY + imgH - 30, 30, WHITE);
        // --- Sprite e posição igual desafio 01 ---
        CharacterSpriteResult ch = GetCharacterSprite(gSelectedCharacterName, false, false);
        float tw2 = ch.sprite->width * ch.scale, th = ch.sprite->height * ch.scale;
        Vector2 pos;
        pos.x = imgX - 330 + (imgW - tw2)/2.0f;
        pos.y = imgY - th + 210;
        if (strcmp(gSelectedCharacterName,"Jade") == 0)
            pos.x += 100 + ch.carlosExtraOffset;
        else if (strcmp(gSelectedCharacterName,"Levi") == 0)
            pos.x += ch.mamedeExtraOffset;
        DrawTextureEx(*ch.sprite, pos, 0, ch.scale, WHITE);
        int borderRadius = boxHeight/2;
        DrawRectangle(boxX, boxY, boxWidth-borderRadius, boxHeight, (Color){20,20,20,220});
        DrawCircle(boxX+boxWidth-borderRadius, boxY+borderRadius, borderRadius, (Color){20,20,20,220});
        if (writer.drawnChars > 0) {
            char txtbuf[512];
            strncpy(txtbuf, fala_exibida, writer.drawnChars);
            txtbuf[writer.drawnChars] = '\0';
            DrawText(txtbuf, boxX+20, boxY+30, 28, WHITE);
        }
        if (sprEnterButton.id) {
            float pulse = 0.07f * sinf(GetTime() * 3.0f);
            float btnScaleBase = 0.85f;
            float btnScale = btnScaleBase + pulse;
            float btnW = sprEnterButton.width * btnScale;
            float btnH = sprEnterButton.height * btnScale;
            float btnX = GetScreenWidth()/2 - btnW/2 + 120;
            float btnY = GetScreenHeight()/2 - 150;
            Color sombra = (Color){0,0,0,90};
            DrawRectangleRounded((Rectangle){btnX+8,btnY+8,btnW,btnH},0.25f,10,sombra);
            Color brilho = (pulse > 0.04f) ? (Color){255,255,255,75} : WHITE;
            DrawTextureEx(sprEnterButton, (Vector2){btnX,btnY}, 0.0f, btnScale, brilho);
        }

        EndDrawing();
        return;
    }
    BeginScissorMode(PONG_AREA_X, PONG_AREA_Y, PONG_AREA_W, PONG_AREA_H);
    for(int cx=0; cx<NUM_COLUNAS; ++cx) {
        for(int ly=0; ly<linhas_ativas; ++ly) {
            BlocoVermelho* blk = &blocos[cx][ly];
            if (blk->ativa && !blk->destroyed) {
                Rectangle dest = blk->rect;
                Rectangle source;
                if (blk->amarelo) {
                    source.x = 77;
                    source.y = 373;
                    source.width = 50;
                    source.height = 50;
                } else {
                    source.x = 77;
                    source.y = 149;
                    source.width = 51;
                    source.height = 50;
                }
                DrawTexturePro(texParede, source, dest, (Vector2){0,0}, 0, RAYWHITE);
                if (blk->amarelo)
                    DrawRectangleLinesEx(dest, 2, (Color){165,150,32,76});
                else
                    DrawRectangleLinesEx(dest, 1, (Color){130,12,12,38});
            } else if (blk->exploding && blk->explosionFrame < EXPLO_TOTAL_FRAMES) {
                const ExploFrame* exp = &explosionFrames[blk->explosionFrame];
                Rectangle src;
                src.x = -exp->x;
                src.y = -exp->y;
                src.width = exp->w;
                src.height = exp->h;
                Rectangle dst;
                dst.width = exp->w * SC;
                dst.height = exp->h * SC;
                dst.x = blk->rect.x + blk->rect.width/2 - dst.width/2;
                dst.y = blk->rect.y + blk->rect.height/2 - dst.height/2;
                DrawTexturePro(exploTex, src, dst, (Vector2){0,0}, 0, WHITE);
            }
        }
    }
    if (sprBaseJogo.id > 0) {
        Rectangle source = {0, 0, (float)sprBaseJogo.width, (float)sprBaseJogo.height};
        Rectangle dest = paddle.rect;
        DrawTexturePro(sprBaseJogo, source, dest, (Vector2){0,0}, 0, WHITE);
    }
    for (int j=0; j<num_bolas; j++)
        if (bolas[j].ativa){
            float t = GetTime();
            float pulse = sinf(t * 1.2f + bolas[j].pulseOffset) * 0.05f;
            float scale = 1.0f + pulse;
            Rectangle dest = {
                bolas[j].pos.x + BALL_SIZE*SC/2,
                bolas[j].pos.y + BALL_SIZE*SC/2,
                BALL_SIZE*SC * scale, BALL_SIZE*SC * scale };
            Rectangle src = {0, 0, (float)sprVirus.width, (float)sprVirus.height};
            DrawTexturePro(sprVirus, src, dest, (Vector2){BALL_SIZE*SC/2, BALL_SIZE*SC/2}, bolas[j].angle, WHITE);
        }
    for (int a=0; a<MAX_ANTIVIRUS; a++) {
        if (antiviruses[a].ativa) {
            Rectangle dest = antiviruses[a].rect;
            Rectangle src = {0, 0, (float)sprAntiVirus.width, (float)sprAntiVirus.height};
            if (sprAntiVirus.id > 0)
                DrawTexturePro(sprAntiVirus, src, dest, (Vector2){0,0}, 0, WHITE);
            else
                DrawRectangleRec(dest, RED);
        }
    }
    EndScissorMode();
    int boxX = 60;
    int marginBottom = 220;
    int boxY = GetScreenHeight() - marginBottom;
    int boxWidth = GetScreenWidth() - 120;
    int boxHeight = 130;
    int imgW = 1000;
    int imgH = pergunta_img.height - 130;
    int imgX = boxX;
    int imgY = boxY - imgH;
    DrawTexturePro(pergunta_img, (Rectangle){0,0,pergunta_img.width, pergunta_img.height},
            (Rectangle){imgX, imgY, imgW, imgH}, (Vector2){0,0}, 0.0f, WHITE);
    if(pong_vitoria || pong_derrota)
    {
        // --- Exibir nome igual desafio 01 ---
        const char* nome_personagem = (gSelectedCharacterName[0] == '\0') ? "???" : gSelectedCharacterName;
        DrawText(nome_personagem, imgX + 10, imgY + imgH - 30, 30, WHITE);
        // --- Sprite igual desafio 01 ---
        CharacterSpriteResult ch = GetCharacterSprite(gSelectedCharacterName, pong_vitoria, pong_derrota);
        float tw2 = ch.sprite->width * ch.scale;
        float th  = ch.sprite->height * ch.scale;
        Vector2 pos;
        pos.x = imgX - 330 + (imgW - tw2)/2.0f;
        pos.y = imgY - th + 210;
        if (strcmp(gSelectedCharacterName, "Jade") == 0)
            pos.x += 100 + ch.carlosExtraOffset;
        else if (strcmp(gSelectedCharacterName, "Levi") == 0)
            pos.x += ch.mamedeExtraOffset;
        DrawTextureEx(*ch.sprite, pos, 0, ch.scale, WHITE);
    }
    int borderRadius = boxHeight / 2;
    DrawRectangle(boxX, boxY, boxWidth - borderRadius, boxHeight, (Color){20, 20, 20, 220});
    DrawCircle(boxX + boxWidth - borderRadius, boxY + borderRadius, borderRadius, (Color){20, 20, 20, 220});
    if (writer.drawnChars > 0) {
        char txtbuf[512];
        strncpy(txtbuf, fala_exibida, writer.drawnChars);
        txtbuf[writer.drawnChars] = '\0';
        DrawText(txtbuf, boxX + 20, boxY + 30, 28, WHITE);
    }
    if (pong_derrota) {
        int w = GetScreenWidth(), h = GetScreenHeight();
        int layers = 5, layerThick = 20;
        for (int i = 0; i < layers; i++) {
            int thick = layerThick + i*layerThick;
            int alpha = 38 - i*5;
            if (alpha < 6) alpha = 6;
            Color blurRed = (Color){255, 32, 32, alpha};
            DrawRectangle(0, 0, w, thick, blurRed);
            DrawRectangle(0, h-thick, w, thick, blurRed);
            DrawRectangle(0, 0, thick, h, blurRed);
            DrawRectangle(w-thick, 0, thick, h, blurRed);
        }
    }
    if (pong_mostrabotao) {
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
    if (!pong_vitoria && !pong_derrota && !preFalaInicial && cronometro_iniciado) {
        DrawChronometer(cronometro, FASE3_CHRONO_MAX, GetScreenWidth()-80, 80, 55);
    }
    if(fadeout_ativo) {
        float perc = fadeout_time / FADEOUT_DURACAO;
        if (perc > 1.0f) perc = 1.0f;
        int alpha = (int)(255 * perc);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0,0,0, alpha});
    }

    EndDrawing();
}
bool Fase_Desafio_01_Concluida(void) { return fase_concluida; }
void Unload_Desafio_01(void)
{
    UnloadTexture(fundo);
    UnloadTexture(pergunta_img);
    UnloadTexture(sprJoao);    UnloadTexture(sprJoao2);    UnloadTexture(sprJoao3);
    UnloadTexture(sprMateus);  UnloadTexture(sprMateus2);  UnloadTexture(sprMateus3);
    UnloadTexture(sprCarlos);  UnloadTexture(sprCarlos2);  UnloadTexture(sprCarlos3);
    UnloadTexture(sprMamede);  UnloadTexture(sprMamede2);  UnloadTexture(sprMamede3);
    UnloadTexture(sprEnterButton);
    UnloadTexture(texParede);
    UnloadTexture(sprVirus);
    UnloadTexture(sprBaseJogo);
    UnloadTexture(bgJogo);
    UnloadTexture(sprAntiVirus);
    UnloadTexture(exploTex);

    if (audioAtivo) {
        StopMusicStream(musicaFase);
        UnloadMusicStream(musicaFase);
        audioAtivo = false;
    }
}