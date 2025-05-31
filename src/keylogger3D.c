#include "keylogger3D.h"
#include "raylib.h"
#include "generalFunctions.h"
#include "playerStats.h"
#include <string.h>
#include <math.h>
static Model modelo3D;
static Texture2D pergunta_img;
static Texture2D telefone_sprite;
static Texture2D hankFalaSprite;
static Sound somFase1;
static Sound somTelefone;
static Sound somRadio;
static Camera3D camera;
static Model portaModel;
static Texture2D portaTexture;
static float cameraYaw = 0.0f;
static const float maxYaw = PI / 4.0f;
static const float minYaw = -PI / 4.0f;
static bool somFase1Tocado = false;
static bool somRadioTocado = false;
static bool interromperTelefone = false;
static bool telefoneVisivel = false;
static bool animandoTelefone = false;
static bool telefoneSubindo = false;
static bool animacaoFeita = false;
static float animacaoTelefoneY = 0.0f;
static float tempoDesdeInicio = 0.0f;
static float cooldownTelefone = -7.0f;
static Rectangle telefoneBounds = {0};
static float delayTexto = 0.0f;
static float hangUpCooldown = -1.0f;
static TypeWriter fase1Writer;
static bool typeStarted = false;
static float fadeAlphaFase1 = 2.0f;
static const float FADEIN_DURATION = 2.0f;
static bool fase1FadeInDone = false;
static bool showComputerButton = false;
static bool telefoneAtendido = false;
static Sound somPersonagem;         // áudio do personagem
static TypeWriter personagemWriter; // typewriter do personagem
static bool personagemTypeStarted = false;
static bool personagemAudioTocado = false;
static bool unknownDone = false;
static float timeAfterUnknown = -1.0f;
static float postUnknownTimer = -1.0f;
static Sound somChamadaAcabada;
static bool somChamadaTocado = false;
static float gapTimer = -1.0f;
static bool gapSoundPlayed = false;
#define RING_GAP 0.60f
static bool fase_concluida = false;
static const char *characterName = "";
static bool dicaVisivel = false;
static float dicaTimer = 0.0f;
static bool dicaAnimando = false;
static float posicaoDicaX = -300.0f;
static const float velocidadeDica = 300.0f;
static Sound steam_som;
static bool steam_tocando = false;
const char *GetCurrentTextFaseKeylogger(TypeWriter *writer)
{
    return writer->text;
}
void Init_Keylogger3D()
{
    modelo3D = LoadModel("src/models/old-computer.obj");
    pergunta_img = LoadTexture("src/sprites/pergunta3.png");
    telefone_sprite = LoadTexture("src/sprites/tel_hank.png"); // TELEFONE DO HANK
    hankFalaSprite  = LoadTexture("src/sprites/hankFala.png");
    somFase1 = LoadSound("src/music/fase1-mateus.wav");
    somTelefone = LoadSound("src/music/telefone.mp3");
    somRadio = LoadSound("src/music/audio4.mp3"); // <-- ALTERADO
    somPersonagem = LoadSound("");
    somChamadaAcabada = LoadSound("src/music/som_telefone_sinal_desligado_ou_ocupado_caio_audio.mp3");
    steam_som  = LoadSound("src/music/steam-achievement.mp3");
    characterName = gSelectedCharacterName;
    portaModel = LoadModel("src/models/DOOR.obj");
    portaTexture = LoadTexture("src/models/Garage_Metalness.png");
    portaModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = portaTexture;
    SetSoundVolume(somFase1, 1.0f);
    SetSoundVolume(somTelefone, 1.0f);
    SetSoundVolume(somRadio, 0.9f);
    SetSoundVolume(somPersonagem, 1.0f);
    SetMasterVolume(1.0f);
    SetSoundVolume(somChamadaAcabada, 2.0f);
    // Reset de estado
    somFase1Tocado = false;
    somRadioTocado = false;
    interromperTelefone = false;
    telefoneVisivel = false;
    animandoTelefone = false;
    telefoneSubindo = false;
    animacaoFeita = false;
    animacaoTelefoneY = 0.0f;
    tempoDesdeInicio = 0.0f;
    cooldownTelefone = -7.0f;
    delayTexto = 0.0f;
    typeStarted = false;
    hangUpCooldown = -1.0f;
    fase_concluida = false;
    telefoneAtendido = false;
    // Câmera
    camera.position = (Vector3){0.0f, 1.6f, 0.0f};
    camera.target = (Vector3){0.0f, 1.6f, -1.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    DisableCursor();
    // Dica
    dicaVisivel = false;
    dicaTimer = 0.0f;
    dicaAnimando = false;
    posicaoDicaX = -300.0f;
}
void Update_Keylogger3D(void)
{
    float delta = GetFrameTime();
    tempoDesdeInicio += delta;
    if (!fase1FadeInDone)
    {
        fadeAlphaFase1 = UpdateFade(delta, FADEIN_DURATION, true);
        if (fadeAlphaFase1 <= 0.0f)
        {
            fadeAlphaFase1 = 0.0f;
            fase1FadeInDone = true;
        }
    }
    if (hangUpCooldown >= 0.0f)
    {
        hangUpCooldown += delta;
        if (hangUpCooldown >= 5.0f)
        {
            hangUpCooldown = -1.0f;
            cooldownTelefone = 0.0f;
        }
    }
    // Inicia fala digitada após delay
    if (delayTexto > 0.0f)
    {
        delayTexto -= delta;
        if (delayTexto <= 0.0f && !somRadioTocado)
        {
            PlaySound(somRadio);
            somRadioTocado = true;
            const char *fala =
                "Um funcionário da empresa está responsável por integrar uma API em um serviço nosso. "
                "Ele está ciente de que os participantes do\nprocesso têm de interceptar "
                "as informações de seu computador. Para isso, use um keylogger - um vírus que intercepta "
                "teclas digitadas\nde um computador remoto, pegando o IP da máquina para realizar o ataque.";
            InitTypeWriter(&fase1Writer, fala, 21.0f); // <-- ALTERADO
            typeStarted = true;
        }
    }
    if (typeStarted)
        UpdateTypeWriter(&fase1Writer, delta, IsKeyPressed(KEY_SPACE));
    if (typeStarted && !unknownDone && fase1Writer.drawnChars >= strlen(GetCurrentTextFaseKeylogger(&fase1Writer)))
    {
        unknownDone = true;
        timeAfterUnknown = 0.0f; // inicia a contagem de 2 s
    }
    if (unknownDone && !personagemAudioTocado)
    {
        if (!somChamadaTocado)
        {
            PlaySound(somChamadaAcabada);
            somChamadaTocado = true;
        }
        timeAfterUnknown += delta;
        if (timeAfterUnknown >= 5.0f)
        {
            PlaySound(somPersonagem);
            personagemAudioTocado = true;
            const char *falaP = "Tá certo. Pra pegar o IP é só ir na internet e puxar um vazamento de dados que tenha a Cybertech...";
            InitTypeWriter(&personagemWriter, falaP, 18.5f);
            personagemTypeStarted = true;
        }
    }
    if (typeStarted && !unknownDone &&
        fase1Writer.drawnChars >= (int)strlen(GetCurrentTextFaseKeylogger(&fase1Writer)))
    {
        unknownDone = true;
        postUnknownTimer = 0.0f;
        gapTimer = 0.0f;
        gapSoundPlayed = false;
    }
    if (unknownDone && postUnknownTimer >= 0.0f)
    {
        postUnknownTimer += delta;
        if (postUnknownTimer >= 1.0f && typeStarted)
        {
            typeStarted = false;
            unknownDone = false;
            char *mutableText = (char *)fase1Writer.text;
            mutableText[0] = '\0';
            fase1Writer.drawnChars = 0;
        }
    }
    if (personagemTypeStarted &&
        personagemWriter.drawnChars >= (int)strlen(GetCurrentTextFaseKeylogger(&personagemWriter)))
    {
        showComputerButton = true;
    }
    if (personagemTypeStarted)
        UpdateTypeWriter(&personagemWriter, delta, IsKeyPressed(KEY_SPACE));
    // TELEFONE - lógica idêntica fase Hank/Proxy
    if (!interromperTelefone && !telefoneAtendido)
    {
        if (IsSoundPlaying(somTelefone)) {
            cooldownTelefone = 0.0f;
        }
        else {
            cooldownTelefone += delta;
            if (cooldownTelefone >= RING_GAP)
            {
                PlaySound(somTelefone);
                telefoneVisivel = true;
                if (!animacaoFeita)
                {
                    animandoTelefone = true;
                    telefoneSubindo = true;
                    animacaoTelefoneY = 1.0f;
                }
                cooldownTelefone = 0.0f;
            }
        }
    }
    else
    {
        // NADA aqui: recusa/desliga é só por tecla agora, trata abaixo!
    }
    // ---- ANIMAÇÃO DESCENDO TANTO PARA ATENDER (A) QUANTO PARA RECUSAR (D) ----
    if (telefoneVisivel && !telefoneAtendido && IsKeyPressed(KEY_A)) // atender
    {
        interromperTelefone = true;
        telefoneAtendido = true;
        if (!typeStarted)
            delayTexto = 2.3f;
        StopSound(somTelefone);
        animandoTelefone = true;
        telefoneSubindo  = false;
        animacaoTelefoneY = 0.0f;
    }
    else if (telefoneVisivel && !telefoneAtendido && IsKeyPressed(KEY_D)) // recusa
    {
        StopSound(somTelefone);
        // ANIMAÇÃO DESCENDO (igual ao atender)
        animandoTelefone = true;
        telefoneSubindo  = false;
        animacaoTelefoneY = 0.0f;
        interromperTelefone = false;
        // hangUpCooldown e cooldownTelefone são acertados depois via lógica da animacao
        // não seta telefoneVisivel = false agora, vai sumir só no fim da animacao!
        // também não altera typeStarted direto aqui
    }
    if (animandoTelefone)
    {
        float speed = 2.5f * delta;
        animacaoTelefoneY += (telefoneSubindo ? -speed : speed);
        if ((telefoneSubindo && animacaoTelefoneY <= 0.0f) || (!telefoneSubindo && animacaoTelefoneY >= 1.0f))
        {
            animacaoTelefoneY = telefoneSubindo ? 0.0f : 1.0f;
            animandoTelefone = false;
            animacaoFeita = true;
            // Só torna invisível após a animação completa
            if (!telefoneSubindo)
            {
                telefoneVisivel = false;
                // se foi recusar/desligar, reseta timers
                if (!telefoneAtendido)
                {
                    hangUpCooldown = 0.0f;
                    cooldownTelefone = -1.0f;
                    // typeStarted = false; // só se quiser sumir o balão também imediatamente
                }
            }
        }
    }
    if (tempoDesdeInicio >= 4.0f && !somFase1Tocado)
    {
        PlaySound(somFase1);
        somFase1Tocado = true;
    }
    if (tempoDesdeInicio >= 2.0f && !dicaVisivel)
    {
        dicaVisivel = true;
        dicaAnimando = true;
    }
    if (dicaVisivel)
    {
        dicaTimer += delta;
        if (dicaAnimando && dicaTimer < 1.0f)
        {
            posicaoDicaX += velocidadeDica * delta;
            if (posicaoDicaX >= 20.0f)
            {
                posicaoDicaX = 20.0f;
                dicaAnimando = false;
            }
        }
        if (dicaTimer >= 5.0f && dicaTimer < 7.0f)
        {
            dicaAnimando = true;
            posicaoDicaX -= velocidadeDica * delta;
            if (posicaoDicaX <= -420.0f)
            {
                posicaoDicaX = -422.0f;
                dicaVisivel = false;
            }
        }
    }
    float mouseDeltaX = GetMouseDelta().x;
    cameraYaw += mouseDeltaX * 0.002f;
    if (cameraYaw > maxYaw)
        cameraYaw = maxYaw;
    if (cameraYaw < minYaw)
        cameraYaw = minYaw;
    float distance = 1.0f;
    camera.target.x = camera.position.x + sinf(cameraYaw) * distance;
    camera.target.z = camera.position.z - cosf(cameraYaw) * distance;
    camera.target.y = camera.position.y;
}
static void DrawDialogueBox(const char *speaker,
                            const TypeWriter *writer,
                            int fontTitle,
                            int fontBody)
{
    int boxX = 60;
    int marginBottom = 220;
    int boxY = GetScreenHeight() - marginBottom;
    int boxWidth = GetScreenWidth() - 120;
    int boxHeight = 130;
    int borderRadius = boxHeight / 2;
    int imgW = 1000;
    int imgH = pergunta_img.height - 130;
    int imgX = boxX;
    int imgY = boxY - imgH;
    DrawTexturePro(pergunta_img, (Rectangle){0, 0, pergunta_img.width, pergunta_img.height},
                   (Rectangle){imgX, imgY, imgW, imgH}, (Vector2){0, 0}, 0.0f, WHITE);
    DrawText(speaker, imgX + 10, imgY + imgH - 30, fontTitle, WHITE);
    DrawRectangle(boxX, boxY, boxWidth - borderRadius, boxHeight,
                  (Color){20, 20, 20, 220});
    DrawCircle(boxX + boxWidth - borderRadius, boxY + borderRadius,
               borderRadius, (Color){20, 20, 20, 220});
    if (writer->drawnChars > 0)
    {
        char tmp[writer->drawnChars + 1];
        strncpy(tmp, GetCurrentTextFaseKeylogger((TypeWriter *)writer), writer->drawnChars);
        tmp[writer->drawnChars] = '\0';
        DrawText(tmp, boxX + 20, boxY + 30, fontBody, WHITE);
    }
}
void Draw_Keylogger3D()
{
    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode3D(camera);
    DrawModel(modelo3D, (Vector3){0.0f, -0.5f, -2.0f}, 0.05f, WHITE);
    Vector3 portaPos = {28.0f, -1.0f, -12.0f};
    Vector3 portaRotAxis = {0.0f, 1.0f, 0.0f};
    float portaRotAngle = 45.0f;
    Vector3 portaScale = {0.05f, 0.05f, 0.05f};
    DrawModelEx(portaModel, portaPos, portaRotAxis, portaRotAngle, portaScale, WHITE);
    EndMode3D();
    bool drawUnknownNow = (interromperTelefone && typeStarted && !personagemTypeStarted);
    if (drawUnknownNow)
    {
        int imgHeight = hankFalaSprite.height;
        int x = 40;
        int y = GetScreenHeight() - 220 - imgHeight + 200;
        float hankScale = 0.6f;
        DrawTextureEx(hankFalaSprite, (Vector2){x, y}, 0.0f, hankScale, WHITE);
        DrawDialogueBox("Agente Hank", &fase1Writer, 24, 26);
    }
    if (personagemTypeStarted)
    {
        DrawDialogueBox(characterName, &personagemWriter, 24, 26);
    }
    // TELEFONE: Hank/proxy style
    if (telefoneVisivel)
    {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        int spriteW = 389;
        int spriteH = 803;
        float scale = 1.25f;
        int destW = spriteW * scale;
        int destH = spriteH * scale * 0.75f;
        int destX = screenWidth - destW - 180;
        int destYFinal = screenHeight - destH;
        int deslocamentoY = (int)(animacaoTelefoneY * (screenHeight + destH));
        int destY = telefoneSubindo ? destYFinal + deslocamentoY : destYFinal + deslocamentoY;
        if (!animandoTelefone)
            destY = destYFinal;
        telefoneBounds = (Rectangle){destX, destY, destW, destH};
        DrawTexturePro(
            telefone_sprite,
            (Rectangle){0, 0, spriteW, spriteH * 0.75f},
            telefoneBounds,
            (Vector2){0, 0},
            0.0f,
            WHITE);
    }
    if (showComputerButton)
    {
        int fontSize = 20;
        const char *text = "Usar Computador [SPACE]";
        int textWidth = MeasureText(text, fontSize);
        int padding = 20;
        Rectangle btnBounds = {
            GetScreenWidth() / 2 - (textWidth / 2) - padding,
            GetScreenHeight() / 2 + 100,
            textWidth + (2 * padding),
            fontSize + 20};
        DrawRectangleRec(btnBounds, GREEN);
        DrawText(text, btnBounds.x + (btnBounds.width - textWidth) / 2,
                 btnBounds.y + (btnBounds.height - fontSize) / 2, fontSize, BLACK);
        if (IsKeyPressed(KEY_SPACE))
        {
            StopSound(somFase1);
            StopSound(somTelefone);
            StopSound(somRadio);
            fase_concluida = true;
        }
    }
    if (fadeAlphaFase1 > 0.0f)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      (Color){0, 0, 0, (unsigned char)(fadeAlphaFase1 * 255)});
    }
    if (dicaVisivel)
    {
        DrawDica(posicaoDicaX, 20, "Dica: atenda o telefonema de Hank");
        if (!steam_tocando)
        {
            PlaySound(steam_som);
            steam_tocando = true;
        }
    }
    EndDrawing();
}
bool Fase_Keylogger3D_Concluida(void)
{
    return fase_concluida;
}
void Unload_Keylogger3D(void)
{
    UnloadModel(modelo3D);
    UnloadTexture(pergunta_img);
    UnloadTexture(telefone_sprite);
    UnloadTexture(hankFalaSprite);
    UnloadSound(somFase1);
    UnloadSound(somTelefone);
    UnloadSound(somRadio);
    UnloadSound(somPersonagem);
    UnloadSound(somChamadaAcabada);
    UnloadSound(steam_som);
    EnableCursor();
}