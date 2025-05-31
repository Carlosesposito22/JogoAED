#include "shellBox.h"
#include "raylib.h"
#include "generalFunctions.h"
#include <math.h>
#include <string.h>
static bool fase_concluida = false;
static Texture2D cardboardBox;
static Texture2D unknown;
static Texture2D backgroundOutside;
static Texture2D closedCardboardBox;
static Texture2D openCardboardBox;
static Texture2D cdSprite;
static Texture2D cardHank;
static Texture2D sprEnterButton; // NOVO!
static Sound grabbingSound;
static Sound surpriseSound;
static Sound telefoneSound;
static Sound shellPhoneCallSound;
static Sound sinalDesligadoSound;
static float soundTimer = 0.0f;
static bool telefonePlayed = false;
static bool shellPhoneCallPlayed = false;
static bool sinalDesligadoPlayed = false;
static float unknownPosX;
static float unknownPosY;
static float cardboardPosX;
static float cardboardPosY;
static float velocityUnknown;
static float velocityCardboard;
static bool slowingDownUnknown = false;
static float fadeTimer = 0.0f;
static bool fadeOutActive = false;
static float fadeAlpha = 1.0f;
static float fadeInAlpha = 0.0f;
static bool boxAppeared = false;
static float floatOffset = 0.0f;
static float floatSpeed = 1.5f;
static bool boxOpened = false;
static float boxTimer = 0.0f;
static bool cdAnimationStarted = false;
static float cdAnimationTimer = 0.0f;
static float cdScale = 0.2f;
static float cdPosX;
static float cdPosY;
static bool cdMovingLeft = false;
static float cdMoveTimer = 0.0f;
static float fadeOutBoxAlpha = 1.0f;
static bool surprisePlayed = false;
// TEXTOBOX ANIMADA
static const char *typingText =
    "Você deve esta se perguntando, oque tem nessa caixa, certo? Dentro tem um cd contendo um malware reverse shell. Para sua tarefa \nfinal, você tem que infectar nosso PC com esse malware para controlá-lo remotamente. Vamos ver se você consegue cumprir isso...";
static int typingIndex = 0;
static float typingTimer = 0.0f;
static int typingTextOffset = 0;
static float textboxDelayTimer = 0.0f;
static bool textboxActive = false;
static bool startedTextboxDelay = false;
// --- FIM DA DIALOG/TELA CARD HANK ---
static bool dialogFadingOut = false; // Sinal: está sumindo (fade out)
static float dialogAlpha = 1.0f;
static bool cardHankActive = false;
static float cardHankPosX = 0.0f;
static float cardHankVel = 0.0f;
// Novos: para sobrepor/sincronizar melhor fade e hank
static bool hankStartedOnce = false;
static float hankStartAlpha = 0.78f; // inicia transição Hank quando alpha do dialog está < 0.87
#define HANK_TARGET_X (GetScreenWidth() - cardHank.width + 10)
#define HANK_MIN_VEL 7.0f
// --- NOVO: botão e fade
static bool showEnterButton = false;
static bool shellbox_fadeout = false;
static float shellbox_fadeout_time = 0.0f;
#define SHELLBOX_FADEOUT_DURACAO 0.8f

void Init_ShellBox(void)
{
    cardboardBox = LoadTexture("src/sprites/cardboard_box.png");
    unknown = LoadTexture("src/sprites/unknown.png");
    backgroundOutside = LoadTexture("src/sprites/background_outside.png");
    closedCardboardBox = LoadTexture("src/sprites/closed_cardboardBox.png");
    openCardboardBox = LoadTexture("src/sprites/open_cardboardBox.png");
    cdSprite = LoadTexture("src/sprites/cd.png");
    cardHank = LoadTexture("src/sprites/cardHank.png");
    sprEnterButton = LoadTexture("src/sprites/enter_button.png"); // Botão! (adicione sua imagem)
    grabbingSound = LoadSound("src/music/grabbing.mp3");
    surpriseSound = LoadSound("src/music/surprise.mp3");
    telefoneSound = LoadSound("src/music/telefone.mp3");
    shellPhoneCallSound = LoadSound("src/music/shellPhoneCall.wav");
    sinalDesligadoSound = LoadSound("src/music/som_telefone_sinal_desligado_ou_ocupado_caio_audio.mp3");
    SetSoundVolume(telefoneSound, 1.0f);
    SetSoundVolume(shellPhoneCallSound, 1.0f);
    SetSoundVolume(sinalDesligadoSound, 1.0f);
    SetSoundVolume(surpriseSound, 1.0f);
    PlaySound(surpriseSound);
    unknownPosX = GetScreenWidth();
    unknownPosY = 100;
    cardboardPosX = -cardboardBox.width;
    cardboardPosY = 200;
    velocityUnknown = 20.0f;
    velocityCardboard = 20.0f;
    fadeTimer = 0.0f;
    fadeOutActive = false;
    fadeAlpha = 1.0f;
    fadeInAlpha = 0.0f;
    boxOpened = false;
    boxTimer = 0.0f;
    cdAnimationStarted = false;
    cdAnimationTimer = 0.0f;
    cdScale = 0.2f;
    cdMovingLeft = false;
    cdMoveTimer = 0.0f;
    fadeOutBoxAlpha = 1.0f;
    surprisePlayed = false;
    fase_concluida = false;
    textboxDelayTimer = 0.0f;
    startedTextboxDelay = false;
    typingIndex = 0;
    typingTimer = 0.0f;
    typingTextOffset = 0;
    dialogFadingOut = false;
    dialogAlpha = 1.0f;
    cardHankActive = false;
    cardHankPosX = (float)GetScreenWidth();
    cardHankVel = 1100.0f;
    hankStartedOnce = false;
    // --- Novos
    showEnterButton = false;
    shellbox_fadeout = false;
    shellbox_fadeout_time = 0.0f;
}

void Update_ShellBox(void)
{
    fadeTimer += GetFrameTime();
    if (fadeTimer >= 5.0f && !fadeOutActive)
    {
        fadeOutActive = true;
        fadeAlpha = 1.0f;
    }
    if (fadeOutActive)
    {
        fadeAlpha -= 0.01f;
        if (fadeAlpha <= 0.0f)
        {
            fadeAlpha = 0.0f;
            boxAppeared = true;
            if (!startedTextboxDelay)
            {
                startedTextboxDelay = true;
                textboxDelayTimer = 0.0f;
            }
        }
    }
    if (startedTextboxDelay && !textboxActive)
    {
        textboxDelayTimer += GetFrameTime();
        if (textboxDelayTimer >= 0.1f)
        {
            textboxActive = true;
            typingIndex = 0;
            typingTimer = 0.0f;
            typingTextOffset = 0;
        }
    }
    if (textboxActive && typingIndex < (int)strlen(typingText))
    {
        typingTimer += GetFrameTime();
        if (typingTimer >= 0.056f)
        {
            typingIndex++;
            typingTimer = 0.0f;
        }
    }
    if (!dialogFadingOut && !IsSoundPlaying(sinalDesligadoSound) && sinalDesligadoPlayed)
    {
        if (textboxActive)
            dialogFadingOut = true;
    }
    if (dialogFadingOut)
    {
        dialogAlpha -= GetFrameTime() * 4.1f;
        if (dialogAlpha < 0.0f)
            dialogAlpha = 0.0f;
        if (!hankStartedOnce && dialogAlpha < hankStartAlpha)
        {
            cardHankActive = true;
            hankStartedOnce = true;
        }
        if (dialogAlpha == 0.0f)
            textboxActive = false;
    }
    // ---- animação do hank...
    if (cardHankActive)
    {
        float targetX = HANK_TARGET_X;
        float dx = cardHankPosX - targetX;
        if (fabsf(dx) > 2.0f)
        {
            float force = fmaxf(0.22f, fabsf(dx / 12.0f));
            cardHankVel *= 0.940f;
            if (cardHankVel < HANK_MIN_VEL)
                cardHankVel = HANK_MIN_VEL;
            cardHankPosX -= cardHankVel * GetFrameTime() * force;
            if (cardHankPosX < targetX)
                cardHankPosX = targetX;
        }
        // Quando Hank estiver na posição final, mostramos o botão (uma só vez)
        if (!showEnterButton && fabsf(cardHankPosX - targetX) <= 2.0f)
        {
            showEnterButton = true;
        }
    }
    // ---------------- BOTÃO ENTER + FADE --------------
    if (showEnterButton && !shellbox_fadeout)
    {
        // 2x menor e 200px pra esquerda
        float btnScaleBase = 0.7f;
        float pulse = 0.07f * sinf(GetTime() * 5.0f);
        float btnScale = btnScaleBase + pulse;
        float btnW = sprEnterButton.width * btnScale;
        float btnH = sprEnterButton.height * btnScale;

        // Centro menos 200 px (mais à esquerda)
        float btnX = GetScreenWidth() / 2 - btnW / 2 - 200;
        float btnY = GetScreenHeight() / 2 - btnH / 2;

        Vector2 mouse = GetMousePosition();
        Rectangle btnRect = {btnX, btnY, btnW, btnH};
        bool hover = CheckCollisionPointRec(mouse, btnRect);

        // Clique/click
        if ((hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
        {
            shellbox_fadeout = true;
            shellbox_fadeout_time = 0.0f;
        }
    }
    // --- Fade de saída ao clicar enter
    if (shellbox_fadeout)
    {
        shellbox_fadeout_time += GetFrameTime();
        if (shellbox_fadeout_time >= SHELLBOX_FADEOUT_DURACAO)
        {
            fase_concluida = true;
        }
    }
    // ------------------- fim botão/enter -------------------
    // -- trecho da animação/caixa permanece igual --
    if (!slowingDownUnknown)
    {
        unknownPosX -= velocityUnknown;
        if (unknownPosX <= GetScreenWidth() - (unknown.width * 1.1f) + 50)
        {
            slowingDownUnknown = true;
            velocityUnknown = 8.0f;
        }
    }
    else
    {
        unknownPosX -= velocityUnknown * GetFrameTime();
        if (velocityUnknown < 5.0f)
        {
            velocityUnknown = 5.0f;
        }
    }
    float distanceToBordaCardboard = cardboardPosX;
    if (distanceToBordaCardboard < 0)
    {
        velocityCardboard = fmax(5.0f, 10.0f + (10.0f * (-distanceToBordaCardboard / GetScreenWidth())));
        cardboardPosX += velocityCardboard;
        if (cardboardPosX > 0)
            cardboardPosX = 0;
    }
    if (boxAppeared)
    {
        fadeInAlpha += 0.01f;
        if (fadeInAlpha > 1.0f)
            fadeInAlpha = 1.0f;
        floatOffset = sin(GetTime() * floatSpeed) * 5.0f;
        boxTimer += GetFrameTime();
        if (boxTimer >= 4.0f && !boxOpened)
        {
            boxOpened = true;
            PlaySound(grabbingSound);
        }
    }
    soundTimer += GetFrameTime();
    if (!telefonePlayed && soundTimer >= 1.5f)
    {
        PlaySound(telefoneSound);
        telefonePlayed = true;
    }
    if (!shellPhoneCallPlayed && telefonePlayed && !IsSoundPlaying(telefoneSound))
    {
        PlaySound(shellPhoneCallSound);
        shellPhoneCallPlayed = true;
    }
    if (!sinalDesligadoPlayed && shellPhoneCallPlayed && !IsSoundPlaying(shellPhoneCallSound))
    {
        PlaySound(sinalDesligadoSound);
        sinalDesligadoPlayed = true;
    }
    if (boxAppeared && boxOpened)
    {
        cdAnimationTimer += GetFrameTime();
        if (!cdAnimationStarted && cdAnimationTimer >= 1.5f)
        {
            cdAnimationStarted = true;
        }
        if (cdAnimationStarted)
        {
            float targetScale = 1.0f;
            float targetPosX = (GetScreenWidth() - cdSprite.width * targetScale) / 2;
            float targetPosY = (GetScreenHeight() - cdSprite.height * targetScale) / 2;
            cdScale += (targetScale - cdScale) * 0.05f;
            cdPosX += (targetPosX - cdPosX) * 0.05f;
            cdPosY += (targetPosY - cdPosY) * 0.05f;
            fadeOutBoxAlpha -= 0.02f;
            if (fadeOutBoxAlpha < 0.0f)
                fadeOutBoxAlpha = 0.0f;
            cdMoveTimer += GetFrameTime();
            if (cdMoveTimer >= 2.0f && !cdMovingLeft)
            {
                cdMovingLeft = true;
            }
            if (cdMovingLeft)
            {
                float targetPosX = -cdSprite.width * cdScale;
                cdPosX += (targetPosX - cdPosX) * 0.05f;
            }
        }
        else
        {
            float boxScale = 0.5f;
            cdPosX = (GetScreenWidth() - openCardboardBox.width * boxScale) / 2 +
                     (openCardboardBox.width * boxScale / 2) - (cdSprite.width * cdScale / 2);
            cdPosY = (GetScreenHeight() - openCardboardBox.height * boxScale) / 2 + floatOffset + 30.0f -
                     (cdSprite.height * cdScale);
        }
    }
}
void Draw_ShellBox(void)
{
    BeginDrawing();
    ClearBackground(BLACK);
    Color fadeColor = (Color){255, 255, 255, (unsigned char)(fadeAlpha * 255)};
    DrawTexturePro(backgroundOutside,
                   (Rectangle){0, 0, (float)backgroundOutside.width, (float)backgroundOutside.height},
                   (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   (Vector2){0, 0}, 0.0f, fadeColor);
    DrawTextureEx(cardboardBox, (Vector2){cardboardPosX, cardboardPosY}, 0.0f, 1.0f, fadeColor);
    DrawTextureEx(unknown, (Vector2){unknownPosX, unknownPosY}, 0.0f, 1.1f, fadeColor);
    if (boxAppeared)
    {
        Color fadeInColor = (Color){255, 255, 255, (unsigned char)(fadeInAlpha * 255)};
        float boxScale = 0.5f;
        float posX = (GetScreenWidth() - closedCardboardBox.width * boxScale) / 2;
        float posY = (GetScreenHeight() - closedCardboardBox.height * boxScale) / 2 + floatOffset + 30.0f;
        if (!boxOpened)
            DrawTextureEx(closedCardboardBox, (Vector2){posX, posY}, 0.0f, boxScale, fadeInColor);
        else
        {
            Color fadeOutColor = (Color){255, 255, 255, (unsigned char)(fadeOutBoxAlpha * 255)};
            DrawTextureEx(openCardboardBox, (Vector2){posX, posY}, 0.0f, boxScale, fadeOutColor);
            DrawTextureEx(cdSprite, (Vector2){cdPosX, cdPosY}, 0.0f, cdScale, WHITE);
        }
    }
    // Aparecer a dialog box com fade
    if (textboxActive || dialogFadingOut)
    {
        int boxWidth = GetScreenWidth() * 0.75;
        int boxHeight = 86;
        int boxPosX = (GetScreenWidth() - boxWidth) / 2;
        int marginBottom = 180;
        int boxPosY = GetScreenHeight() - marginBottom - boxHeight;
        int fontSize = 22;
        int horizontalMargin = 27;
        int verticalMargin = 16;
        int maxLineWidth = boxWidth - 2 * horizontalMargin;
        int maxLines = 2;
        Color boxCol0 = (Color){0, 0, 0, (unsigned char)(100 * dialogAlpha)};
        Color boxCol1 = (Color){32, 42, 56, (unsigned char)(215 * dialogAlpha)};
        Color boxCol2 = (Color){220, 220, 255, (unsigned char)(130 * dialogAlpha)};
        DrawRectangleRounded((Rectangle){boxPosX + 6, boxPosY + 8, boxWidth, boxHeight}, 0.19f, 16, boxCol0);
        DrawRectangleRounded((Rectangle){boxPosX, boxPosY, boxWidth, boxHeight}, 0.19f, 16, boxCol1);
        DrawRectangleRoundedLines((Rectangle){boxPosX, boxPosY, boxWidth, boxHeight}, 0.19f, 16, boxCol2);
        Vector2 textPos = {boxPosX + horizontalMargin, boxPosY + verticalMargin};
        char displayText[600];
        int displayLen = 0;
        int currLineWidth = 0, currLineCount = 1;
        int lastSpaceDisplayIdx = -1;
        int thisIdx;
        for (thisIdx = typingTextOffset; thisIdx < typingIndex && displayLen < sizeof(displayText) - 2; thisIdx++)
        {
            char c = typingText[thisIdx];
            displayText[displayLen++] = c;
            if (c == '\n')
            {
                currLineCount++;
                currLineWidth = 0;
                lastSpaceDisplayIdx = -1;
                if (currLineCount > maxLines)
                    break;
                continue;
            }
            char onechar[2] = {c, 0};
            int charW = MeasureText(onechar, fontSize);
            currLineWidth += charW;
            if (c == ' ')
            {

                lastSpaceDisplayIdx = displayLen - 1;
            }
            if (currLineWidth > maxLineWidth)
            {
                if (lastSpaceDisplayIdx >= 0)
                {
                    displayText[lastSpaceDisplayIdx] = '\n';
                    currLineWidth = 0;
                    for (int i = lastSpaceDisplayIdx + 1; i < displayLen; i++)
                    {
                        char onech[2] = {displayText[i], 0};
                        currLineWidth += MeasureText(onech, fontSize);
                    }
                    currLineCount++;
                    lastSpaceDisplayIdx = -1;
                }
                else
                {
                    displayText[displayLen++] = '\n';
                    currLineCount++;
                    currLineWidth = 0;
                }
            }
            if (currLineCount > maxLines)
                break;
        }
        displayText[displayLen] = '\0';
        if (currLineCount > maxLines || (thisIdx < typingIndex && typingIndex - typingTextOffset > 8))
        {
            char *firstNL = strchr(displayText, '\n');
            if (firstNL != NULL)
            {
                typingTextOffset += (firstNL - displayText) + 1;
            }
            else
            {
                typingTextOffset++;
            }
            if (typingTextOffset > typingIndex - 1)
                typingTextOffset = typingIndex - 1;
        }
        Color txtShadow = (Color){0, 0, 0, (unsigned char)(96 * dialogAlpha)};
        Color txtColor = (Color){255, 255, 255, (unsigned char)(255 * dialogAlpha)};
        DrawTextEx(GetFontDefault(), displayText, (Vector2){textPos.x + 1.5f, textPos.y + 2.5f}, fontSize, 1, txtShadow);
        DrawTextEx(GetFontDefault(), displayText, textPos, fontSize, 1, txtColor);
        float arrowAlpha = 100 + 80 * sinf(GetTime() * 4);
        if (typingIndex >= (int)strlen(typingText))
        {
            int arrowW = 34;
            int arrowH = 30;
            DrawTextEx(GetFontDefault(), ">>",
                       (Vector2){
                           boxPosX + boxWidth - arrowW,
                           boxPosY + boxHeight - arrowH},
                       fontSize, 1, (Color){180, 200, 230, (unsigned char)(arrowAlpha * dialogAlpha)});
        }
    }
    // Hank chegando (aparece junto do fade)
    if (cardHankActive)
    {
        int centerY = GetScreenHeight() / 2 - cardHank.height / 2;
        DrawTexture(cardHank, (int)cardHankPosX, centerY, WHITE);
    }
    // ------ BOTÃO ENTER ------
    if (showEnterButton && !shellbox_fadeout)
    {
        // 2x menor e 200 px para a esquerda
        float btnScaleBase = 0.7f;
        float pulse = 0.07f * sinf(GetTime() * 5.0f);
        float btnScale = btnScaleBase + pulse;
        float btnW = sprEnterButton.width * btnScale;
        float btnH = sprEnterButton.height * btnScale;

        float btnX = GetScreenWidth() / 2 - btnW / 2 - 200;
        float btnY = GetScreenHeight() / 2 - btnH / 2;

        Color sombra = (Color){0, 0, 0, 90};
        DrawRectangleRounded((Rectangle){btnX + 8, btnY + 8, btnW, btnH}, 0.25f, 10, sombra);
        Color brilho = (pulse > 0.04f) ? (Color){255, 255, 255, 75} : WHITE;
        DrawTextureEx(sprEnterButton, (Vector2){btnX, btnY}, 0.0f, btnScale, brilho);
    }
    // ------ FADE OUT final ------
    if (shellbox_fadeout)
    {
        float perc = shellbox_fadeout_time / SHELLBOX_FADEOUT_DURACAO;
        if (perc > 1.0f)
            perc = 1.0f;
        int alpha = (int)(255 * perc);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, alpha});
    }
    EndDrawing();
}
bool Fase_ShellBox_Concluida(void)
{
    return fase_concluida;
}
void Unload_ShellBox(void)
{
    UnloadSound(grabbingSound);
    StopSound(surpriseSound);
    UnloadSound(surpriseSound);
    UnloadSound(telefoneSound);
    UnloadSound(shellPhoneCallSound);
    UnloadSound(sinalDesligadoSound);
    UnloadTexture(cardboardBox);
    UnloadTexture(unknown);
    UnloadTexture(backgroundOutside);
    UnloadTexture(closedCardboardBox);
    UnloadTexture(openCardboardBox);
    UnloadTexture(cdSprite);
    UnloadTexture(cardHank);
    UnloadTexture(sprEnterButton);
}