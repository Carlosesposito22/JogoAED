#include "generalFunctions.h"
#include "interrogatorio.h"
#include "raylib.h"
#include "gemini.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "playerStats.h"

static void UpdateApresentacao(float dt);
static void UpdateFalaHank(float dt);
static void UpdateTransicaoPrePergunta(float dt);
static void UpdateFalaInterrogatorio(float dt);
static void DrawApresentacao(void);
static void DrawFalaHank(void);
static void DrawTransicaoPrePergunta(void);
static void DrawFalaInterrogatorio(void);
static void DrawFade(void);
static void AvaliarRespostaComIA(int indicePergunta, const char *pergunta, const char *respostaJogador);

static struct
{
    // Recursos
    Texture2D background, spriteNome, spriteBustup, spriteConfiante;
    Music     interrogationMusic;
    Sound     somSurpresa, somFalaDetetive2;
    Sound     falaAudio;
    const char *falaTexto;
    const char *falaTextoExtra;
    bool        usandoExtra; 

    // Controle geral
    float fadeWhiteAlpha;
    bool  fadeWhiteIn, fadeWhiteOut;

    // ---------- Apresentacao ----------
    Vector2 posNome, posBustup;
    Vector2 targetNome, targetBustup;
    float   speed, slowSpeed, tempoAposAnimacao;
    bool    bustupChegou, somSurpresaTocado;

    // ---------- Fala Hank ----------
    TypeWriter writer;
    bool mostrarConfiante, dialogoFinalizado;
    bool falaSomTocado;

    /* --- transição PRE-PERGUNTA --- */
    int   transPhase;        // 0=escurecer,1=balao,2=revelar
    float transTimer;        // cronômetro da fase
    float transDur;          // duração de cada efeito (1.0 s)
    float overlayProg;       // 0-1 esquerda→direita preto
    float revealProg;        // 0-1 esquerda→direita novo fundo
    Texture2D bgInterrog;    // novo fundo (sala de interrogatório)
    Sound     sfxHolofote;

    // ---------- Fala Interrogatorio ----------
    char  respostaBuf[384];
    int   respostaLen;
    int   perguntaAtual;
    bool  aguardandoInput;
    bool  respostaConfirmada;
    int   perguntaID;
    int   slot; 

} ctx;

Question perguntas[TOTAL_PERGUNTAS] = 
{
    { "Momentaneamente sua senha foi trocada para acesso temporário a uma área restrita do servidor, e acabou não sendo removida.\nVocê verifica que um dos seus familiares esta sendo investigado, o que você faria nessa situação?" },
    { "Em um cenário onde durante a reunião, alguém compartilha por engano a tela do computador exibindo dados sensíveis, que\npodem influenciar seu desempenho na empresa. Você comenta com colegas ou se mantém em silêncio e orienta o responsável?" },
    { "Imagine a seguinte situação, ao revisar uma de suas antigas demandas, você nota um arquivo que vaza diversas informações sensiveis de pessoas.\nVocê tenta solucionar o problema por si só, reporta ao seu superior e busca uma solução consjunta ou apaga sem contar para ninguém?" },
    { "Você encontra, sem querer, registros detalhados de performance e feedbacks confidenciais sobre seus colegas em uma pasta aberta no servidor.\nVocê lê para saber o que dizem sobre você, reporta o acesso indevido ou fecha a pasta sem analisar nada?" },
    { "Durante uma manutenção de rotina, você descobre mensagens internas trocadas entre diretores sobre uma possível demissão em massa.\nVocê compartilha com os colegas que podem ser impactados, relata ao setor responsável pelo vazamento ou ignora completamente a descoberta?" },
    { "Seu superior diz para ignorar uma falha pequena e focar em outro problema. Você aceita ou insiste em corrigir mesmo assim?" },
    { "Em uma investigação de rotina, descobre que há vídeos gravados por câmeras de segurança em locais sensíveis da empresa sem o conhecimento\ndos funcionários. Você assiste aos vídeos, alerta quem está gravando ou ignora para não se envolver?" },
    { "Você recebe um e-mail direcionado a outro funcionário, contendo anexos com documentos jurídicos confidenciais. Você lê o conteúdo para entender\ndo que se trata, avisa o remetente sobre o erro ou simplesmente apaga o e-mail?" },
    { "Durante uma entrevista de desligamento, um ex-funcionário pede para transferir arquivos pessoais de sua estação. Ao acessar a máquina, você \npercebe que há documentos da empresa nos mesmos diretórios. Você revisa o conteúdo, transfere tudo sem olhar, ou separa e reporta\nos documentos da empresa antes de transferir?" },
    { "Você sabe que outro candidato está tentando sabotar seu progresso. Usaria suas habilidades para retaliar ou manteria seu foco na vaga?" }
};

static const char *INTRO_FALA1 =
        "Impressionante... Não é todo dia que alguém ultrapassa meu firewall tão rápido. Sou Hank, agente do FBI "
        "e responsável pelo processo seletivo. Para provar que é o melhor, complete cinco desafios práticos que "
        "envolvem áreas da cibersegurança. [ENTER]";
        
static const char *INTRO_FALA2 =
        "Mas fique ligado, pois, além de avaliar sua técnica, vou observar principalmente como você se comporta, "
        "para ver se realmente merece fazer parte do FBI. Então, vamos deixar de conversa e partir para o primeiro desafio! [ENTER]";

static void (*stageUpdates[ETAPA_TOTAL])(float) = { UpdateApresentacao, UpdateFalaHank, UpdateTransicaoPrePergunta, UpdateFalaInterrogatorio };
static void (*stageDraws  [ETAPA_TOTAL])(void ) = { DrawApresentacao,  DrawFalaHank, DrawTransicaoPrePergunta, DrawFalaInterrogatorio  };
int perguntasSelecionadas[MAX_PERGUNTAS];
static bool semPergunta = false; 
static bool fase_concluida = false;

Node *atual = NULL;
Node *tail = NULL;
Node *head = NULL;

void AddNode(InterrogatorioStage st)
{
    Node *novo = malloc(sizeof(Node));
    if (novo) {
        novo->stage = st;
        novo->prev = novo->next = NULL;
        if (head == NULL) head = tail = novo;
        else {
            tail->next = novo;
            novo->prev = tail;
            tail = novo;
        }
    }
}

void NextStage(void)
{
    if (atual != NULL && atual->next != NULL) atual = atual->next;
}

void ClearStages(void)
{
    Node *p = head;
    while (p) {
        Node *next = p->next;
        free(p);
        p = next;
    }
    head = tail = atual = NULL;
}

void Init_Interrogatorio(int perguntaIndex, const char *audio, const char *texto)
{
    // Carregar recursos --------------------------------------------------
    ctx.background        = LoadTexture("src/sprites/background_outside.png");
    ctx.spriteNome        = LoadTexture("src/sprites/detetive-hank-text.png");
    ctx.spriteBustup      = LoadTexture("src/sprites/Gumshoe_OA (1).png");
    ctx.spriteConfiante   = LoadTexture("src/sprites/detective_confident.png");
    ctx.interrogationMusic= LoadMusicStream("src/music/interrogationThemeA.mp3");
    ctx.somSurpresa       = LoadSound("src/music/surprise.mp3");
    ctx.somFalaDetetive2  = LoadSound("src/music/detectiveSpeaking2.mp3");
    ctx.falaAudio         = LoadSound(audio);
    SetMusicVolume(ctx.interrogationMusic, 0.7f);
    SetSoundVolume(ctx.falaAudio, 4.0f);
    PlayMusicStream(ctx.interrogationMusic);

    // Estado geral -------------------------------------------------------
    ctx.fadeWhiteAlpha = 0.0f;
    ctx.fadeWhiteIn = ctx.fadeWhiteOut = false;
    semPergunta = (perguntaIndex < 0);
    fase_concluida = false;
    
    if (semPergunta) {
        ctx.falaTexto      = INTRO_FALA1;
        ctx.falaAudio      = LoadSound("src/music/fala_apresentacao_1.mp3");
        SetSoundVolume(ctx.falaAudio, 4.0f);
        ctx.falaTextoExtra = INTRO_FALA2;
    } else { 
        ctx.falaTexto      = texto;
        ctx.falaTextoExtra = NULL;  
    }
    ctx.usandoExtra   = false;
    ctx.falaSomTocado = false;

    // Inicia a Lista Duplamente Encadeada ---------------------------------
    ClearStages();

    if (semPergunta) {
        AddNode(APRESENTACAO);
        AddNode(FALA_HANK);
    } else {
        AddNode(APRESENTACAO);
        AddNode(FALA_HANK);
        AddNode(TRANSICAO_PRE_PERGUNTA);
        AddNode(PERGUNTA_INTERROGATORIO);
    }
    atual = head;

    // ETAPA 1 ------------------------------------------------------------
    ctx.posNome   = (Vector2){ GetScreenWidth(), GetScreenHeight() - ctx.spriteNome.height - 50 };
    ctx.posBustup = (Vector2){ -ctx.spriteBustup.width, GetScreenHeight() - ctx.spriteBustup.height };
    ctx.targetNome   = (Vector2){ GetScreenWidth() - ctx.spriteNome.width, ctx.posNome.y };
    ctx.targetBustup = (Vector2){ 0, ctx.posBustup.y };
    ctx.speed = 2500.f; ctx.slowSpeed = 10.f;
    ctx.tempoAposAnimacao = 0.f;
    ctx.bustupChegou = ctx.somSurpresaTocado = false;

    // ETAPA 2 ------------------------------------------------------------
    InitTypeWriter(&ctx.writer, ctx.falaTexto, 15.0f);
    ctx.mostrarConfiante = ctx.dialogoFinalizado = false;

    // ETAPA 3 ------------------------------------------------------------
    ctx.bgInterrog = LoadTexture("src/sprites/russia.png");
    ctx.transPhase   = 0;
    ctx.transTimer   = 0.0f;
    ctx.transDur     = 1.0f;      // 1 segundo para escurecer ou revelar
    ctx.overlayProg  = 0.0f;
    ctx.revealProg   = 0.0f;
    ctx.sfxHolofote = LoadSound("src/music/among2.mp3");  // coloque o arquivo na pasta
    SetSoundVolume(ctx.sfxHolofote, 2.5f);

    // ETAPA 4 ------------------------------------------------------------
    ctx.respostaLen   = 0;
    ctx.respostaBuf[0]= '\0';
    ctx.aguardandoInput = false;
    ctx.perguntaID = semPergunta ? -1 : perguntasSelecionadas[perguntaIndex];
    ctx.slot       = perguntaIndex;
}

void Update_Interrogatorio(void)
{
    UpdateMusicStream(ctx.interrogationMusic);
    float dt = GetFrameTime();
    stageUpdates[atual->stage](dt);
}

void Draw_Interrogatorio(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

    Rectangle src = {0,0,(float)ctx.background.width,(float)ctx.background.height};
    Rectangle dst = {0,0,(float)GetScreenWidth(),(float)GetScreenHeight()};
    DrawTexturePro(ctx.background, src, dst, (Vector2){0,0}, 0, WHITE);

    stageDraws[atual->stage]();
    DrawFade();

    EndDrawing();
}

static void UpdateApresentacao(float dt)
{
    if (!ctx.somSurpresaTocado) { PlaySound(ctx.somSurpresa); ctx.somSurpresaTocado = true; }

    if (ctx.posNome.x > ctx.targetNome.x) {
        ctx.posNome.x -= ctx.speed * dt;
        if (ctx.posNome.x < ctx.targetNome.x) ctx.posNome.x = ctx.targetNome.x;
    }
    if (!ctx.bustupChegou) {
        ctx.posBustup.x += ctx.speed * dt;
        if (ctx.posBustup.x >= ctx.targetBustup.x) {
            ctx.posBustup.x = ctx.targetBustup.x; ctx.bustupChegou = true;
        }
    }
    if (ctx.bustupChegou && ctx.posNome.x == ctx.targetNome.x) {
        ctx.posBustup.x += ctx.slowSpeed * dt;
        ctx.tempoAposAnimacao += dt;
        if (ctx.tempoAposAnimacao > 2.f && !ctx.fadeWhiteIn && !ctx.fadeWhiteOut) ctx.fadeWhiteIn = true;
    }

    if (ctx.fadeWhiteIn) {
        ctx.fadeWhiteAlpha += dt * 1.5f;
        if (ctx.fadeWhiteAlpha >= 1.f) {
            ctx.fadeWhiteAlpha = 1.f; ctx.fadeWhiteIn = false; ctx.fadeWhiteOut = true;
            NextStage();
            ctx.mostrarConfiante = true;
        }
    }
}

static void UpdateFalaHank(float dt)
{
    if (ctx.fadeWhiteOut) {
        ctx.fadeWhiteAlpha -= dt * 1.5f;
        if (ctx.fadeWhiteAlpha <= 0.f) { ctx.fadeWhiteAlpha = 0.f; ctx.fadeWhiteOut = false; }
    }

    UpdateTypeWriter(&ctx.writer, dt, false);

    if (!ctx.falaSomTocado) {
        PlaySound(ctx.falaAudio);
        ctx.falaSomTocado = true;
    }

    if(ctx.writer.done && IsKeyPressed(KEY_ENTER)) {

        if (semPergunta)
        {
            if (!ctx.usandoExtra && ctx.falaTextoExtra)
            {
                ctx.usandoExtra = true;
                ctx.falaTexto   = ctx.falaTextoExtra;
                InitTypeWriter(&ctx.writer, ctx.falaTexto, 17.0f);
                ctx.falaAudio = LoadSound("src/music/fala_apresentacao_2.mp3");
                SetSoundVolume(ctx.falaAudio, 4.0f);
                ctx.falaSomTocado = false;
                return;
            }

            fase_concluida = true;
            return;
        }

        NextStage();
        PauseMusicStream(ctx.interrogationMusic);
        PlaySound(ctx.sfxHolofote);
        ctx.transPhase  = 0;
        ctx.transTimer  = 0.0f;
        ctx.overlayProg = 0.0f;
        ctx.revealProg  = 0.0f;
        return;
    }

    if (ctx.dialogoFinalizado) {
        if (semPergunta) {
            fase_concluida = true;
            return;
        } else {
            PlaySound(ctx.somFalaDetetive2);
            NextStage();
            ctx.respostaLen = 0;
            ctx.respostaBuf[0] = '\0';
            ctx.aguardandoInput = true;
            return;
        }
    }
}

static void UpdateFalaInterrogatorio(float dt)
{
    if (!ctx.aguardandoInput) return;

    int ch;
    while ((ch = GetCharPressed()) > 0) {
        if (ctx.respostaLen < (int)sizeof(ctx.respostaBuf)-1 && ch >= 32 && ch <= 126) {
            ctx.respostaBuf[ctx.respostaLen++] = (char)ch;
            ctx.respostaBuf[ctx.respostaLen]   = '\0';
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && ctx.respostaLen > 0) {
        ctx.respostaBuf[--ctx.respostaLen] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER)) {
        ctx.respostaConfirmada = true;
        ctx.aguardandoInput = false;

        AvaliarRespostaComIA(
            ctx.slot,
            perguntas[ctx.perguntaID].pergunta,
            ctx.respostaBuf
        );

        fase_concluida = true;
    }
}

static void DrawApresentacao(void)
{
    DrawTexture(ctx.spriteNome,   ctx.posNome.x,   ctx.posNome.y,   WHITE);
    DrawTexture(ctx.spriteBustup, ctx.posBustup.x, ctx.posBustup.y, WHITE);
}

static void DrawFalaHank(void)
{
    // Sprite confiante
    float scale = 1.3f;

    Rectangle src;
    if (!ctx.usandoExtra)
        src = (Rectangle){2087, 0, 631, 722};
    else
        src = (Rectangle){3029, 3357, 631, 725};

    float w = src.width*scale, h = src.height*scale;
    Vector2 pos = { (GetScreenWidth()-w)/2.f, GetScreenHeight()-h };
    DrawTexturePro(ctx.spriteConfiante, src, (Rectangle){pos.x,pos.y,w,h}, (Vector2){0,0},0, WHITE);

    // Caixa de diálogo
    const float boxH = 160.f;
    const float boxOffsetY = 60.f;
    float boxY = GetScreenHeight() - boxH - boxOffsetY;

    DrawRectangle(50, boxY, GetScreenWidth() - 100, boxH, (Color){0, 0, 0, 220});
    DrawRectangleLines(50, boxY, GetScreenWidth() - 100, boxH, WHITE);

    if (!ctx.dialogoFinalizado) {
        char buf[512] = {0};

        size_t len = ctx.writer.done ? strlen(ctx.falaTexto)
                                     : (size_t)ctx.writer.drawnChars;
        strncpy(buf, ctx.falaTexto, len);

        const int fs = 24;
        const int maxW = GetScreenWidth() - 140;
        const int x0 = 70;
        const int lineH = fs + 6;
        int y = boxY + 10;

        const char* line = buf;
        while (*line) {
            const char* p = line;
            int lastSp = -1, c = 0, w = 0;
            while (*p && w < maxW) {
                if (*p == ' ') lastSp = c;
                char tmp[256] = {0};
                strncpy(tmp, line, c + 1);
                w = MeasureText(tmp, fs);
                ++p; ++c;
            }
            if (w >= maxW && lastSp >= 0) c = lastSp;

            char out[256] = {0};
            strncpy(out, line, c);
            DrawText(out, x0, y, fs, WHITE);

            line += c;
            while (*line == ' ') ++line;
            y += lineH;
            if (y + lineH > boxY + boxH - 10) break;
        }
    }
}

static void DrawTransicaoPrePergunta(void)
{
    if (ctx.transPhase == 0) {
        int coverW = (int)(GetScreenWidth() * ctx.overlayProg);
        DrawRectangle(0, 0, coverW, GetScreenHeight(), BLACK);
    } else {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
    }

    float baseScale  = 1.3f;
    float closeScale = 1.7f;

    float scale;
    if      (ctx.transPhase == 0) scale = baseScale  + (closeScale - baseScale) * ctx.overlayProg;
    else if (ctx.transPhase == 1) scale = closeScale;
    else                          scale = closeScale + (baseScale - closeScale) * ctx.revealProg;

    Rectangle srcH = { 2087, 0, 631, 722 };
    float w = srcH.width  * scale;
    float h = srcH.height * scale;

    /* --- calcula deslocamento vertical para dar close no ROSTO --- */
    float shiftPct = (scale - baseScale) / (closeScale - baseScale);
    float yOffset  = shiftPct * h * 0.35f;
    Vector2 pos = { (GetScreenWidth() - w) / 2.0f, GetScreenHeight() - h + yOffset };

    Color spot = (Color){255, 255, 255, 50};
    Vector2 apex  = { pos.x + w*0.5f, pos.y + h*0.10f };
    Vector2 mid   = { pos.x + w*0.5f, pos.y + h*0.50f }; 
    DrawTriangle((Vector2){ 0, 0}, mid, apex, spot);
    DrawTriangle((Vector2){ GetScreenWidth(), 0}, apex, mid, spot);

    if (ctx.transPhase == 1) {
        const char *txt = "Agora... Conte-me mais sobre você";
        int fs    = 28;
        int tw    = MeasureText(txt, fs);
        int boxW  = tw + 40;
        int boxH  = fs + 24;
        int x     = (GetScreenWidth() - boxW)/2;
        int y     = pos.y - boxH - 20;

        Rectangle outer = { x - 6, y - 6, boxW + 12, boxH + 12 };
        Rectangle inner = { x - 3, y - 3, boxW +  6, boxH +  6 };

        Color cLt = (Color){195,195,195,255}; 
        Color cDk = (Color){ 90, 90, 90,255};
        DrawRectangleGradientEx(outer, cLt, cLt, cDk, cDk);
        DrawRectangleGradientEx(inner, cDk, cDk, cLt, cLt);

        const int rr = 3;
        DrawCircle(outer.x + rr + 2,                 outer.y + rr + 2,                 rr, cDk);
        DrawCircle(outer.x + outer.width  - rr - 2,  outer.y + rr + 2,                 rr, cDk);
        DrawCircle(outer.x + rr + 2,                 outer.y + outer.height - rr - 2,  rr, cDk);
        DrawCircle(outer.x + outer.width  - rr - 2,  outer.y + outer.height - rr - 2,  rr, cDk);

        DrawRectangle(x, y, boxW, boxH, (Color){20,20,20,220});
        DrawRectangleLinesEx((Rectangle){x,y,boxW,boxH}, 1, (Color){220,220,220,255});

        DrawTriangle(
            (Vector2){pos.x + w/2 - 10, y + boxH},
            (Vector2){pos.x + w/2 + 10, y + boxH},
            (Vector2){pos.x + w/2,      y + boxH + 15},
            (Color){220,220,220,255}
        );
        DrawText(txt, x + 20, y + 12, fs, WHITE);
    }

    if (ctx.transPhase == 2) {
        int revealW = (int)(GetScreenWidth() * ctx.revealProg);
        Rectangle srcBg = {
            0, 0,
            (float)(ctx.background.width  * ctx.revealProg),
            (float) ctx.background.height
        };
        Rectangle dstBg = { 0, 0, (float)revealW, (float)GetScreenHeight() };

        DrawTexturePro(ctx.background, srcBg, dstBg, (Vector2){0,0}, 0, WHITE);
    }

    DrawTexturePro(ctx.spriteConfiante, srcH, (Rectangle){pos.x,pos.y, w ,h}, (Vector2){0,0}, 0, WHITE);
}

static void UpdateTransicaoPrePergunta(float dt)
{
    ctx.transTimer += dt;

    switch (ctx.transPhase)
    {
        /* ── 0. escurecendo da esquerda p/ direita ─────────────────── */
        case 0:
            ctx.overlayProg = fminf(ctx.transTimer / ctx.transDur, 1.0f);
            if (ctx.overlayProg >= 1.0f) {
                ctx.transPhase = 1;
                ctx.transTimer = 0.0f;          // zera para contagem de 3 s
            }
            break;

        /* ── 1. balão “Agora…” fica 3 s na tela ────────────────────── */
        case 1:
            if (ctx.transTimer >= 3.0f) {       // ← 3 segundos
                ctx.transPhase  = 2;
                ctx.transTimer  = 0.0f;
                ctx.revealProg  = 0.0f;
            }
            break;

        /* ── 2. revela novo fundo da esquerda p/ direita ───────────── */
        case 2:
            ctx.revealProg = fminf(ctx.transTimer / ctx.transDur, 1.0f);
            if (ctx.revealProg >= 1.0f) {
                NextStage();
                ctx.aguardandoInput = true;      // permite digitar
                ctx.respostaLen     = 0;         // zera buffer
                ctx.respostaBuf[0]  = '\0';
                ctx.interrogationMusic = LoadMusicStream("src/music/suspense.mp3");
                PlayMusicStream(ctx.interrogationMusic);
                return;
            }
            break;
    }
}

static void DrawFalaInterrogatorio(void)
{
    float scale = 1.3f;
    Rectangle src = { 3029, 3357, 631, 725 };
    float w = src.width*scale, h = src.height*scale;
    Vector2 pos = { (GetScreenWidth()-w)/2.f, GetScreenHeight()-h };
    DrawTexturePro(ctx.spriteConfiante, src,
                   (Rectangle){pos.x,pos.y,w,h}, (Vector2){0,0},0, WHITE);
    // Caixa da pergunta
    const float boxH = 160.f;
    const float boxOffsetY = 60.f;
    float boxY = GetScreenHeight() - boxH - boxOffsetY;
    DrawRectangle(50, boxY, GetScreenWidth() - 100, boxH, (Color){0,0,0,220});
    DrawRectangleLines(50, boxY, GetScreenWidth() - 100, boxH, WHITE);
    // Pergunta atual
    DrawText(perguntas[ctx.perguntaID].pergunta, 70, boxY + 10, 24, WHITE);
    // Campo de input (sub‑caixa)
    const float inputH = 40.f;
    int yInput = boxY + boxH - inputH - 15;
    DrawRectangle(70, yInput, GetScreenWidth()-140, inputH, (Color){20,20,20,255});
    DrawRectangleLines(70, yInput, GetScreenWidth()-140, inputH, WHITE);

    // --- AJUSTE DO BUFFER VISÍVEL ---
    const int fs = 24;
    const int inputMaxWidth = GetScreenWidth() - 160; // margem extra
    const char *toDraw = ctx.respostaBuf;
    int textLen = strlen(ctx.respostaBuf);

    // Após digitar além do espaço, mostra só o fim!
    while (textLen > 0 && MeasureText(toDraw, fs) > inputMaxWidth) {
        toDraw++;
        textLen--;
    }

    DrawText(toDraw, 80, yInput+8, fs, GREEN);

    // Cursor piscante ao final do texto visível
    if (ctx.aguardandoInput && (GetTime()*2.0 - floor(GetTime()*2.0)) > 0.5) {
        DrawText("|", 80 + MeasureText(toDraw, fs), yInput+8, fs, GREEN);
    }
}

static void DrawFade(void)
{
    if (ctx.fadeWhiteAlpha>0.f)
        DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),
                      (Color){255,255,255,(unsigned char)(ctx.fadeWhiteAlpha*255)});
}

bool Fase_Interrogatorio_Concluida(void) { return fase_concluida; }

static void AvaliarRespostaComIA(int indicePergunta, const char *pergunta, const char *respostaJogador)
{
    char prompt[2048];
    snprintf(prompt, sizeof(prompt),
        "Você é o agente Hank, encarregado de decidir se um candidato a uma unidade cibernética especial do FBI usará seus amplos poderes "
        "de invasão e defesa de sistemas **para o bem ou para o mal**. Seu foco é avaliar a ética do candidato.\n\n"

        "⚠️  Diretrizes importantes para a sua avaliação (NÃO seja excessivamente rigoroso):\n"
        "• As respostas podem ser breves; **não penalize** pela falta de detalhes.\n"
        "• Se a resposta não indicar claramente má conduta, **presuma boa fé** e dê pelo menos metade da pontuação da categoria.\n"
        "• Reserve notas muito baixas (≤ 2 em qualquer categoria) apenas quando houver indícios explícitos de uso mal-intencionado dos poderes.\n"
        "• **Se a resposta estiver fora de contexto, incoerente ou não responder à pergunta**, dê no máximo **6/25** no total.\n\n"

        "Avalie a resposta do candidato nos seguintes critérios (total máximo = 25):\n"
        "- Intenção Ética (Bem x Mal): Demonstra uso das habilidades para benefício legítimo? (0-12)\n"
        "- Responsabilidade Profissional: Reconhece riscos e age com prudência? (0-7)\n"
        "- Gestão de Informações Sensíveis: Respeita privacidade e sigilo? (0-6)\n\n"

        "Pergunta: \"%s\"\n"
        "Resposta: \"%s\"\n\n"

        "Formato da resposta:\n"
        "NOTA=<inteiro de 0 a 25>\n"
        "RELATORIO=<breve análise da resposta>",
        pergunta, respostaJogador
    );

    char retorno[1024] = {0};
    ObterRespostaGemini(prompt, retorno);

    int nota = -1;
    char relatorio[512] = {0};

    if (sscanf(retorno, "NOTA=%d", &nota) == 1) {
        const char *pRel = strstr(retorno, "RELATORIO=");
        if (pRel) strncpy(relatorio,
                          pRel + strlen("RELATORIO="),
                          sizeof(relatorio)-1);
    }

    if (indicePergunta >= 0 && indicePergunta < MAX_PERGUNTAS) {
        notasIA[indicePergunta] = nota;
        strncpy(relatoriosIA[indicePergunta], relatorio, sizeof(relatoriosIA[indicePergunta]) - 1);
    }
}

void SelecionarPerguntasAleatorias(void)
{
    int indices[TOTAL_PERGUNTAS];
    for (int i = 0; i < TOTAL_PERGUNTAS; i++) {
        indices[i] = i;
    }

    for (int i = TOTAL_PERGUNTAS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    for (int i = 0; i < MAX_PERGUNTAS; i++) {
        perguntasSelecionadas[i] = indices[i];
    }
}

void Unload_Interrogatorio(void)
{
    UnloadTexture(ctx.background);
    UnloadTexture(ctx.spriteNome);
    UnloadTexture(ctx.spriteBustup);
    UnloadTexture(ctx.spriteConfiante);
    UnloadTexture(ctx.bgInterrog);
    UnloadMusicStream(ctx.interrogationMusic);
    UnloadSound(ctx.somSurpresa);
    UnloadSound(ctx.falaAudio);
    UnloadSound(ctx.somFalaDetetive2);
    UnloadSound(ctx.sfxHolofote);
}