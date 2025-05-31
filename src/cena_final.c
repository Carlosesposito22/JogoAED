#include "cena_final.h"
#include "raylib.h"
#include "generalFunctions.h"
#include "playerStats.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

// --- UMA SEMANA DEPOIS ---
static bool introActive = true;
static float introTimer = 0.0f;
static float introAlpha = 0.0f;
static const float INTRO_FADE = 1.5f;
static const float INTRO_HOLD = 2.5f;
static bool faseFinalPrincipal = false;

// ESSA É A TELA DO RESULTADO DO PROCESSO SELETIVO
static bool fase_concluida = false;
// Texturas
static Texture2D carinhaRevelationSilhouette;
static Texture2D carinhaRevelation;
static Texture2D meninaRevelationSilhouette;
static Texture2D meninaRevelation;
static Texture2D calvoRevelationSilhouette;
static Texture2D calvoRevelation;
static Texture2D deBoneRevelationSilhouette;
static Texture2D deBoneRevelation;
static Texture2D cybertechShield;
static bool textAnimActive = false;
static float textAnimProgress = 0.0f;
static const float TEXT_ANIM_DURATION = 0.65f;
static bool textPersistent = true;
static float textScale = 10.0f;
static const float maxTextScale = 10.0f;
static Sound medal;

static bool resultadoAnimActive = false;
static float resultadoAnimProgress = 0.0f;
static const float RESULTADO_ANIM_DURATION = 0.7f;

// Timer para controlar o aparecimento do escudo
static float shieldAppearTimer = 0.0f;
static bool shieldStartTimer = false;
// --- ANIMAÇÃO DO SHIELD ENTRANDO ---
static bool shieldAnimActive = false;
static float shieldAnimProgress = 0.0f;
static const float SHIELD_ANIM_DURATION = 0.65f;
static const float SHIELD_DELAY_AFTER_REVELATION = 0.15f;
// --- ANIMAÇÃO DE SAÍDA DO SHIELD ---
static bool shieldExitAnimActive = false;
static float shieldExitAnimProgress = 0.0f;
static const float SHIELD_EXIT_ANIM_DURATION = 0.7f;
static float shieldExitTimer = 0.0f; // conta os 4s até o início da saída
static bool shieldShouldExit = false;
// --- ANIMAÇÃO DE SAÍDA DO TEXTO ---
static bool textExitAnimActive = false;
static float textExitAnimProgress = 0.0f;
// --- ANIMAÇÃO DE SAÍDA DOS SPRITES ---
static bool spritesExitAnimActive = false;
static float spritesExitAnimProgress = 0.0f;
// Áudio
static Music writtenInTheStars;
static Sound surpriseSound;
static Sound congrats;
// Variáveis de controle
static float parallaxTimer = 0.0f;
static const float parallaxDuration = 13.0f;
static bool movimentoFinalizado = false;
static float parallaxProgress = 0.0f;
static bool surprisePlayed = false;
// Posições finais
static const int initialPos = 700;
static const int finalPosMenina = 800;
static const int finalPosCalvo = 900;
static const int finalPosDeBone = 1000;
// Variáveis para animação
static bool spriteTrocaRealizada = false;
static bool deBoneTrocaRealizada = false;
static bool meninaTrocaRealizada = false;
static bool calvoTrocaRealizada = false;
// Variáveis para efeito de flash
static float flashAlpha = 0.0f;
static bool flashAtivo = false;
static const float flashDuration = 0.6f;
static float flashTimer = 0.0f;

static Texture2D caderneta;

static Font fonteEscritaAMao;

static Font resultadoFont;

// Variáveis da animação da caderneta
static bool cadernetaAnimActive = false;
static float cadernetaAnimProgress = 0.0f;
static const float CADERNETA_ANIM_DURATION = 0.7f;

static Sound booSound;
static bool booPlayed = false;

static Sound deuBomSound;
static bool deuBomPlayed = false;

static Sound paperSound;
static bool paperSoundPlayed = false;

static bool podeFinalizar = false;
static Texture2D sprEnterButton; 

static void CarregaFonteManuscrito(void)
{
    static int latin1[224];
    for (int i = 0; i < 224; i++) latin1[i] = 32 + i;
    fonteEscritaAMao = LoadFontEx("src/fonts/Manuscrito.ttf", 26, latin1, 224);
}

static void DrawTextWrapped(Font font,
                            const char *text,
                            Rectangle rec,
                            float fontSize,
                            float spacing,
                            Color tint)
{
    const float lineHeight = (fontSize + spacing) * 1.55f;
    float y = rec.y;

    char line[1024] = {0};
    int  lineLen    = 0;

    for (const char *p = text; *p; )
    {
        // 1) Trata quebra de linha explícita
        if (*p == '\n') {
            line[lineLen] = '\0';
            DrawTextEx(font, line, (Vector2){rec.x, y}, fontSize, spacing, tint);
            lineLen = 0;
            y += lineHeight;
            if (y + lineHeight > rec.y + rec.height) return;
            ++p;
            continue;
        }

        // 2) Copia a próxima palavra
        char word[256];
        int  wLen = 0;
        while (*p && *p != ' ' && *p != '\n') {
            word[wLen++] = *p++;
            if (wLen >= 255) break;
        }
        word[wLen] = '\0';

        // 3) Avança o ponteiro se havia espaço depois da palavra
        bool hadSpace = (*p == ' ');
        if (hadSpace) ++p;

        // 4) Constrói uma linha “teste” com a palavra adicionada
        char testLine[1024];
        strcpy(testLine, line);
        if (lineLen) strcat(testLine, " ");
        strcat(testLine, word);

        float testWidth = MeasureTextEx(font, testLine, fontSize, spacing).x;

        if (testWidth > rec.width && lineLen > 0) {
            // A palavra não cabe: desenha a linha atual e inicia outra
            line[lineLen] = '\0';
            DrawTextEx(font, line, (Vector2){rec.x, y}, fontSize, spacing, tint);
            y += lineHeight;
            if (y + lineHeight > rec.y + rec.height) return;

            // reinicia a linha com a palavra que estourou
            strcpy(line, word);
            lineLen = wLen;
        } else {
            // Cabe: concatena (com espaço se não for a primeira)
            if (lineLen) line[lineLen++] = ' ';
            strcpy(line + lineLen, word);
            lineLen += wLen;
        }
    }

    // Desenha a última linha restante
    if (lineLen && y + lineHeight <= rec.y + rec.height) {
        line[lineLen] = '\0';
        DrawTextEx(font, line, (Vector2){rec.x, y}, fontSize, spacing, tint);
    }
}

void DrawResultadoSelecao(void) {
    int screenWidth = GetScreenWidth();
    const char *resultadoTexto = playerStats.isPassouSelecao ? "APROVADO" : "REPROVADO";
    Color resultadoCor = playerStats.isPassouSelecao ? GREEN : RED;

    if (!playerStats.isPassouSelecao && !booPlayed) {
        PlaySound(booSound);
        booPlayed = true;
    }
    else if(playerStats.isPassouSelecao && !booPlayed){
        PlaySound(deuBomSound);
        booPlayed = true;
    }

    // Tamanho grande para o texto
    float fontSize = 400.0f;

    // Medir o texto usando a fonte especificada
    Vector2 textSize = MeasureTextEx(resultadoFont, resultadoTexto, fontSize, 5);
    
    // Centralizar horizontalmente e alinhar no topo
    int textX = (screenWidth - textSize.x) / 2;
    int textY = textSize.y / 4;  // Ajuste para manter parte do texto visível na tela

    // Desenhar o texto com a fonte resultadoFont
    DrawTextEx(resultadoFont, resultadoTexto, (Vector2){textX, textY}, fontSize, 5, resultadoCor);
}



void StartTextAnimation(void)
{
    textAnimActive = true;
    textAnimProgress = 0.0f;
}
void UpdateTextAnimation(void)
{
    if (textAnimActive || textPersistent)
    {
        textAnimProgress += GetFrameTime() / TEXT_ANIM_DURATION;
        if (textAnimProgress >= 1.0f)
        {
            textAnimProgress = 1.0f;
            textPersistent = true;
        }
    }
}
void DrawTextWithAnimationFadeOut(const char *text, float fadeOutAlpha)
{
    int screenHeight = GetScreenHeight();
    if (textScale > maxTextScale)
        textScale = maxTextScale;
    float scale = textScale;
    float alongamento = 4.0f;
    float expoente = 7.0f;
    float t = textAnimProgress;
    float ease = 1.0f - powf(1.0f - fminf(t / alongamento, 1.0f), expoente);
    if (textPersistent)
        ease += GetFrameTime() * 0.02f;
    static float slowMoveOffset = 0.0f;
    slowMoveOffset += GetFrameTime() * 10.0f;
    int posY = -50 * scale + (int)((screenHeight * 0.8) * ease) + (int)slowMoveOffset;
    unsigned char alpha = (unsigned char)(150 * fadeOutAlpha);
    DrawTextEx(GetFontDefault(), TextToUpper(text), (Vector2){20, posY}, 50 * scale, 2, (Color){255, 255, 255, alpha});
}
void Init_FinalJogo(void)
{
    fase_concluida = false;
    carinhaRevelationSilhouette = LoadTexture("src/sprites/revelation/carinhaRevelationSilhouette.png");
    carinhaRevelation = LoadTexture("src/sprites/revelation/carinhaRevelation.png");
    meninaRevelationSilhouette = LoadTexture("src/sprites/revelation/meninaRevelationSilhouette.png");
    meninaRevelation = LoadTexture("src/sprites/revelation/meninaRevelation.png");
    calvoRevelationSilhouette = LoadTexture("src/sprites/revelation/calvoRevelationSilhouette.png");
    calvoRevelation = LoadTexture("src/sprites/revelation/calvoRevelation.png");
    deBoneRevelationSilhouette = LoadTexture("src/sprites/revelation/deBoneRevelationSilhouette.png");
    deBoneRevelation = LoadTexture("src/sprites/revelation/deBoneRevelation.png");
    cybertechShield = LoadTexture("src/sprites/revelation/cybertechShield.png");
    caderneta = LoadTexture("src/sprites/revelation/caderneta.png");
    congrats = LoadSound("src/music/congrats.wav");
    medal = LoadSound("src/music/medal.mp3");
    writtenInTheStars = LoadMusicStream("src/music/writtenInTheStars.mp3");
    surpriseSound = LoadSound("src/music/surprise.mp3");
    paperSound = LoadSound("src/music/paperSound.mp3");
    CarregaFonteManuscrito();
    resultadoFont = LoadFont("src/fonts/Schluber.ttf");
    booSound = LoadSound("src/sprites/revelation/boo.mp3");
    deuBomSound = LoadSound("src/sprites/revelation/deubom.mp3");
    sprEnterButton = LoadTexture("src/sprites/enter_button.png");
    booPlayed = false;
    deuBomPlayed = false;
    PlayMusicStream(writtenInTheStars);
    SetMusicVolume(writtenInTheStars, 0.5f);
    PlaySound(congrats);
    parallaxTimer = 0.0f;
    parallaxProgress = 0.0f;
    movimentoFinalizado = false;
    spriteTrocaRealizada = false;
    deBoneTrocaRealizada = false;
    meninaTrocaRealizada = false;
    calvoTrocaRealizada = false;
    surprisePlayed = false;
    shieldAppearTimer = 0.0f;
    shieldStartTimer = false;
    shieldAnimActive = false;
    shieldAnimProgress = 0.0f;
    shieldExitAnimActive = false;
    shieldExitAnimProgress = 0.0f;
    shieldExitTimer = 0.0f;
    shieldShouldExit = false;
    textExitAnimActive = false;
    textExitAnimProgress = 0.0f;
    spritesExitAnimActive = false;
    spritesExitAnimProgress = 0.0f;

    // --- UMA SEMANA DEPOIS ---
    introActive = true;
    introTimer = 0.0f;
    introAlpha = 0.0f;
    faseFinalPrincipal = false;
}
void Update_FinalJogo(void)
{
    float delta = GetFrameTime();

    // --- UMA SEMANA DEPOIS ---
    if (introActive) {
        introTimer += delta;
        if (introTimer <= INTRO_FADE)
            introAlpha = introTimer / INTRO_FADE;
        else if (introTimer <= INTRO_FADE + INTRO_HOLD)
            introAlpha = 1.0f;
        else if (introTimer <= INTRO_FADE*2 + INTRO_HOLD)
            introAlpha = 1.0f - (introTimer - INTRO_FADE - INTRO_HOLD) / INTRO_FADE;
        else {
            introActive = false;
            introAlpha  = 0.0f;
            faseFinalPrincipal = true;
        }
        return;
    }
    if (!faseFinalPrincipal)
        return;

    UpdateTextAnimation();
    if (podeFinalizar && IsKeyPressed(KEY_ENTER)) {
        fase_concluida = true;
    }
    UpdateMusicStream(writtenInTheStars);
    if (!movimentoFinalizado)
    {
        parallaxTimer += GetFrameTime();
        parallaxProgress = parallaxTimer / parallaxDuration;
        if (parallaxProgress >= 1.0f)
        {
            parallaxProgress = 1.0f;
            movimentoFinalizado = true;
            flashAtivo = true;
            flashTimer = 0.0f;
            surprisePlayed = false;
            spriteTrocaRealizada = false;
            deBoneTrocaRealizada = false;
            meninaTrocaRealizada = false;
            calvoTrocaRealizada = false;
            const char *personagemVisivel = playerStats.isPassouSelecao
                                                ? gSelectedCharacterName
                                                : (strcmp(gSelectedCharacterName, "Levi") == 0 ? "Alice" : "Levi");
            if (!playerStats.isPassouSelecao)
            {
                if (strcmp(personagemVisivel, "Alice") == 0)
                    meninaTrocaRealizada = true;
                else
                    calvoTrocaRealizada = true;
            }
            else
            {
                if (strcmp(gSelectedCharacterName, "Dante") == 0)
                    spriteTrocaRealizada = true;
                else if (strcmp(gSelectedCharacterName, "Jade") == 0)
                    deBoneTrocaRealizada = true;
                else if (strcmp(gSelectedCharacterName, "Alice") == 0)
                    meninaTrocaRealizada = true;
                else if (strcmp(gSelectedCharacterName, "Levi") == 0)
                    calvoTrocaRealizada = true;
            }
            shieldStartTimer = true;
            shieldAppearTimer = 0.0f;
        }
    }
    if (flashAtivo)
    {
        if (!surprisePlayed)
        {
            PlaySound(surpriseSound);
            flashAlpha = 1.0f;
            surprisePlayed = true;
        }
        flashTimer += GetFrameTime();
        if (flashTimer >= flashDuration / 2)
        {
            flashAlpha = 1.0f - (flashTimer - flashDuration / 2) * 2.0f / flashDuration;
        }
        if (flashTimer >= flashDuration)
        {
            flashAtivo = false;
            flashAlpha = 0.0f;
        }
    }
    if (flashAtivo && !textAnimActive)
    {
        StartTextAnimation();
    }
    if (shieldStartTimer && !shieldAnimActive)
    {
        shieldAppearTimer += GetFrameTime();
        if (shieldAppearTimer >= SHIELD_DELAY_AFTER_REVELATION)
        {
            shieldAnimActive = true;
            shieldAnimProgress = 0.0f;
            shieldStartTimer = false;
        }
    }
    if (shieldAnimActive)
    {
        shieldAnimProgress += GetFrameTime() / SHIELD_ANIM_DURATION;
    }
    if (shieldAnimActive && !shieldShouldExit && shieldAnimProgress >= 1.0f)
    {
        shieldShouldExit = true;
        shieldExitTimer = 0.0f;
    }
    if (shieldShouldExit && !shieldExitAnimActive)
    {
        shieldExitTimer += GetFrameTime();
        if (shieldExitTimer >= 10.0f)
        {
            shieldExitAnimActive = true;
            shieldExitAnimProgress = 0.0f;
            shieldShouldExit = false;
            textExitAnimActive = true;
            textExitAnimProgress = 0.0f;
            spritesExitAnimActive = true;
            spritesExitAnimProgress = 0.0f;
        }
    }
    if (shieldExitAnimActive)
    {
        shieldExitAnimProgress += GetFrameTime() / SHIELD_EXIT_ANIM_DURATION;
        if (shieldExitAnimProgress >= 1.0f)
            shieldExitAnimProgress = 1.0f;
    }
    if (textExitAnimActive)
    {
        textExitAnimProgress += GetFrameTime() / SHIELD_EXIT_ANIM_DURATION;
        if (textExitAnimProgress >= 1.0f)
        {
            textExitAnimProgress = 1.0f;
            textExitAnimActive = false;
        }
    }
    if (spritesExitAnimActive)
    {
        spritesExitAnimProgress += GetFrameTime() / SHIELD_EXIT_ANIM_DURATION;
        if (spritesExitAnimProgress >= 1.0f)
        {
            spritesExitAnimProgress = 1.0f;
            spritesExitAnimActive = false;
        }
    }
    if (shieldExitAnimActive && shieldExitAnimProgress >= 1.0f && !cadernetaAnimActive)
    {
        cadernetaAnimActive = true;
        cadernetaAnimProgress = 0.0f;
        if (!paperSoundPlayed)
        {
            PlaySound(paperSound);
            paperSoundPlayed = true;
        }
    }
    if (cadernetaAnimActive)
    {
        cadernetaAnimProgress += GetFrameTime() / CADERNETA_ANIM_DURATION;
        if (cadernetaAnimProgress >= 1.0f)
            cadernetaAnimProgress = 1.0f;
    }
    if (cadernetaAnimActive && !resultadoAnimActive)
    {
        resultadoAnimActive = true;
        resultadoAnimProgress = 0.0f;
    }
    if (resultadoAnimActive)
    {
        resultadoAnimProgress += GetFrameTime() / RESULTADO_ANIM_DURATION;
        if (resultadoAnimProgress >= 1.0f)
        {
            resultadoAnimProgress = 1.0f;
        }
    }

    if (!podeFinalizar &&
        cadernetaAnimActive && cadernetaAnimProgress >= 1.0f &&
        resultadoAnimActive  && resultadoAnimProgress  >= 1.0f) {
        podeFinalizar = true;
    }
}
void Draw_FinalJogo(void)
{
    BeginDrawing();

    // --- UMA SEMANA DEPOIS ---
    if (introActive || introAlpha > 0.0f) {
        ClearBackground(BLACK);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, introAlpha));
        const char *msg = "Uma semana depois";
        int fSize = 48;
        int txtW = MeasureText(msg, fSize);
        DrawText(msg, (GetScreenWidth() - txtW) / 2, GetScreenHeight() / 2 - fSize / 2, fSize, Fade(WHITE, introAlpha));
        EndDrawing();
        return;
    }

    ClearBackground(BLACK);
    int screenHeight = GetScreenHeight();
    int screenWidth = GetScreenWidth();
    const char *personagemVisivel = playerStats.isPassouSelecao
                ? gSelectedCharacterName
                : (strcmp(gSelectedCharacterName, "Levi") == 0 ? "Alice" : "Levi");
    float textAlpha = 1.0f;
    if (textExitAnimActive || shieldExitAnimActive)
    {
        float t = (shieldExitAnimActive ? shieldExitAnimProgress : 0.0f);
        if (textExitAnimActive)
            t = textExitAnimProgress;
        textAlpha = 1.0f - powf(fminf(t, 1.0f), 2.0f);
        if (textAlpha < 0.0f)
            textAlpha = 0.0f;
    }
    if (textAnimActive)
        DrawTextWithAnimationFadeOut(personagemVisivel, textAlpha);
    // Fatores de escala
    const float scaleCarinha = 0.4f;
    const float scaleMenina = 0.6f;
    const float scaleCalvo = 0.8f;
    const float scaleDeBone = 1.0f;
    // --- ANIMAÇÃO DE SAÍDA SPRITES ---
    float spritesExitF = 0.0f;
    if (spritesExitAnimActive || shieldExitAnimActive)
    {
        float t = (spritesExitAnimActive ? spritesExitAnimProgress : 0.0f);
        if (shieldExitAnimActive)
            t = shieldExitAnimProgress;
        spritesExitF = powf(fminf(t, 1.0f), 2.5f); // ease-in
    }
    int xMeninaBase = initialPos + (finalPosMenina - initialPos) * parallaxProgress;
    int xCalvoBase = initialPos + (finalPosCalvo - initialPos) * parallaxProgress;
    int xDeBoneBase = initialPos + (finalPosDeBone - initialPos) * parallaxProgress;
    int xCarinhaBase = initialPos;
    int outMenina = screenWidth + 50;
    int outCalvo = screenWidth + 50;
    int outDeBone = screenWidth + 50;
    int outCarinha = screenWidth + 50;
    int xCarinha = xCarinhaBase + (outCarinha - xCarinhaBase) * spritesExitF;
    int xMenina = xMeninaBase + (outMenina - xMeninaBase) * spritesExitF;
    int xCalvo = xCalvoBase + (outCalvo - xCalvoBase) * spritesExitF;
    int xDeBone = xDeBoneBase + (outDeBone - xDeBoneBase) * spritesExitF;

    int yCarinha = screenHeight - carinhaRevelationSilhouette.height * scaleCarinha;
    int yMenina = screenHeight - meninaRevelationSilhouette.height * scaleMenina;
    int yCalvo = screenHeight - calvoRevelationSilhouette.height * scaleCalvo;
    int yDeBone = screenHeight - deBoneRevelationSilhouette.height * scaleDeBone;
    if (strcmp(personagemVisivel, "Dante") == 0 && spriteTrocaRealizada)
    {
        DrawTexturePro(carinhaRevelation,
                       (Rectangle){0, 0, carinhaRevelation.width, carinhaRevelation.height},
                       (Rectangle){xCarinha, yCarinha,
                                   carinhaRevelation.width * scaleCarinha,
                                   carinhaRevelation.height * scaleCarinha},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawTexturePro(carinhaRevelationSilhouette,
                       (Rectangle){0, 0, carinhaRevelationSilhouette.width, carinhaRevelationSilhouette.height},
                       (Rectangle){xCarinha, yCarinha,
                                   carinhaRevelationSilhouette.width * scaleCarinha,
                                   carinhaRevelationSilhouette.height * scaleCarinha},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
    if (strcmp(personagemVisivel, "Alice") == 0 && meninaTrocaRealizada)
    {
        DrawTexturePro(meninaRevelation,
                       (Rectangle){0, 0, meninaRevelation.width, meninaRevelation.height},
                       (Rectangle){xMenina, yMenina,
                                   meninaRevelation.width * scaleMenina,
                                   meninaRevelation.height * scaleMenina},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawTexturePro(meninaRevelationSilhouette,
                       (Rectangle){0, 0, meninaRevelationSilhouette.width, meninaRevelationSilhouette.height},
                       (Rectangle){xMenina, yMenina,
                                   meninaRevelationSilhouette.width * scaleMenina,
                                   meninaRevelationSilhouette.height * scaleMenina},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
    if (strcmp(personagemVisivel, "Levi") == 0 && calvoTrocaRealizada)
    {
        DrawTexturePro(calvoRevelation, (Rectangle){0, 0, calvoRevelation.width, calvoRevelation.height}, (Rectangle){xCalvo, yCalvo, calvoRevelation.width * scaleCalvo, calvoRevelation.height * scaleCalvo},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawTexturePro(calvoRevelationSilhouette,
                       (Rectangle){0, 0, calvoRevelationSilhouette.width, calvoRevelationSilhouette.height},
                       (Rectangle){xCalvo, yCalvo,
                                   calvoRevelationSilhouette.width * scaleCalvo,
                                   calvoRevelationSilhouette.height * scaleCalvo},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
    if (strcmp(personagemVisivel, "Jade") == 0 && deBoneTrocaRealizada)
    {
        DrawTexturePro(deBoneRevelation,
                       (Rectangle){0, 0, deBoneRevelation.width, deBoneRevelation.height},
                       (Rectangle){xDeBone, yDeBone,
                                   deBoneRevelation.width * scaleDeBone,
                                   deBoneRevelation.height * scaleDeBone},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawTexturePro(deBoneRevelationSilhouette,
                       (Rectangle){0, 0, deBoneRevelationSilhouette.width, deBoneRevelationSilhouette.height},
                       (Rectangle){xDeBone, yDeBone,
                                   deBoneRevelationSilhouette.width * scaleDeBone,
                                   deBoneRevelationSilhouette.height * scaleDeBone},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
    if (flashAlpha > 0.0f)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight,
                      (Color){255, 255, 255, (unsigned char)(flashAlpha * 255)});
    }
    float scale = 0.35f;
    int shieldFinalX = 20;
    int shieldFinalY = 20;
    int shieldStartX = -cybertechShield.width * scale;
    PlaySound(medal);
    if (shieldAnimActive && !shieldExitAnimActive)
    {
        float alongamento = 4.0f;
        float expoente = 7.0f;
        float t = shieldAnimProgress;
        float ease = 1.0f - powf(1.0f - fminf(t / alongamento, 1.0f), expoente);
        int shieldX = (int)(shieldStartX + (shieldFinalX - shieldStartX) * ease);
        DrawTexturePro(
            cybertechShield,
            (Rectangle){0, 0, cybertechShield.width, cybertechShield.height},
            (Rectangle){shieldX, shieldFinalY, cybertechShield.width * scale, cybertechShield.height * scale},
            (Vector2){0, 0}, 0.0f, WHITE);
    }
    else if (shieldExitAnimActive)
    {
        float t = shieldExitAnimProgress;
        float ease = powf(fminf(t, 1.0f), 2.5f);
        int shieldOutX = (int)(shieldFinalX + (shieldStartX - shieldFinalX) * ease);
        DrawTexturePro(
            cybertechShield,
            (Rectangle){0, 0, cybertechShield.width, cybertechShield.height},
            (Rectangle){shieldOutX, shieldFinalY, cybertechShield.width * scale, cybertechShield.height * scale},
            (Vector2){0, 0}, 0.0f, WHITE);
    }

    if (cadernetaAnimActive)
    {
        float t = cadernetaAnimProgress;
        float ease = 1.0f - powf(1.0f - t, 2.0f); // Ease-out para suavidade
        int yFinal = screenHeight / 2 - caderneta.height / 2;
        int yStart = screenHeight;
        int yCaderneta = yStart - (int)((yStart - yFinal) * ease);
        DrawResultadoSelecao();
        DrawTexture(caderneta, screenWidth / 2 - caderneta.width / 2, yCaderneta, WHITE);

        const int marginL = 420;   // esquerda
        const int marginT = 150;   // topo
        const int marginR = 480;   // direita
        const int marginB = 120;   // base

        Rectangle folhaRec = {
            screenWidth / 2 - caderneta.width / 2 + marginL,   // X
            yCaderneta              + marginT,                 // Y
            caderneta.width  - marginL - marginR,              // Largura disponível
            caderneta.height - marginT - marginB               // Altura disponível
        };

        // Escreve o relatório dentro da folha
        DrawTextWrapped(fonteEscritaAMao,
                        playerStats.relatorioGeral,
                        folhaRec,
                        26.0f,   // tamanho da fonte
                        1.0f,    // espaçamento entre chars
                        BLACK
        );
    }

    if (podeFinalizar)          // só depois de caderneta + resultado prontos
    {
        float pulse = 0.07f * sinf(GetTime() * 2.0f);   // mesmo efeito pulsante
        float btnScaleBase = 0.7f;
        float btnScale     = btnScaleBase + pulse;

        float btnW = sprEnterButton.width  * btnScale;
        float btnH = sprEnterButton.height * btnScale;

        float btnX = 40.0f;                               // canto esquerdo
        float btnY = GetScreenHeight() - btnH - 40.0f;    // canto inferior

        Color sombra = (Color){0, 0, 0, 90};
        DrawRectangleRounded((Rectangle){btnX + 6, btnY + 6, btnW, btnH},
                            0.25f, 10, sombra);

        Color brilho = (pulse > 0.04f)
                    ? (Color){255, 255, 255, 75}
                    : WHITE;
        DrawTextureEx(sprEnterButton, (Vector2){btnX, btnY},
                    0.0f, btnScale, brilho);
    }

    EndDrawing();
}
bool Fase_FinalJogo_Concluida(void)
{
    return fase_concluida;
}
void Unload_FinalJogo(void)
{
    UnloadTexture(carinhaRevelationSilhouette);
    UnloadTexture(carinhaRevelation);
    UnloadTexture(meninaRevelationSilhouette);
    UnloadTexture(meninaRevelation);
    UnloadTexture(calvoRevelationSilhouette);
    UnloadTexture(calvoRevelation);
    UnloadTexture(deBoneRevelationSilhouette);
    UnloadTexture(deBoneRevelation);
    UnloadTexture(cybertechShield);
    StopMusicStream(writtenInTheStars);
    UnloadMusicStream(writtenInTheStars);
    UnloadTexture(sprEnterButton);
    UnloadSound(surpriseSound);
    UnloadSound(congrats);
    UnloadSound(medal);
    UnloadSound(booSound);
}