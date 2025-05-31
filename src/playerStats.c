#include "playerStats.h"
#include <stdbool.h>
#include <string.h>
#include "ranking.h"
#include <stdlib.h>
#include <stdio.h>
#include "gemini.h"

int  notasIA[MAX_PERGUNTAS]                  = {0};
char relatoriosIA[MAX_PERGUNTAS][512]        = {0};
char relatorioGeralIA[8192]                 = {0};
char gPlayerName[MAX_PLAYER_NAME]            = {0};
char gSelectedCharacterName[MAX_PLAYER_NAME] = "";

void InitPlayerStats(PlayerStats *stats)
{
    memset(stats, 0, sizeof(PlayerStats));
}

void SetD01Result(PlayerStats *stats, bool passed, int duration)
{
    stats->isPassed_D01 = passed;
    stats->duration_D01 = duration;
}

void SetD02Result(PlayerStats *stats, bool passed, int duration, int lives)
{
    stats->isPassed_D02 = passed;
    stats->duration_D02 = duration;
    stats->amountOfLives_D02 = lives;
}

void SetD03Result(PlayerStats *stats, bool passed, int duration)
{
    stats->isPassed_D03 = passed;
    stats->duration_D03 = duration;
}

void SetD04Result(PlayerStats *stats, bool passed, int icons)
{
    stats->isPassed_D04        = passed;
    stats->quantityOfIcons_D04 = icons;
}

static void SetPlayerAndCharacter(PlayerStats *stats, const char *playerName, const char *characterName)
{
    if (playerName == NULL || playerName[0] == '\0' || strspn(playerName, " ") == strlen(playerName))
        strncpy(stats->playerName, "Sem Nome", MAX_PLAYER_NAME - 1);
    else
        strncpy(stats->playerName, playerName, MAX_PLAYER_NAME - 1);
    stats->playerName[MAX_PLAYER_NAME - 1] = '\0';

    if (characterName) {
        strncpy(stats->characterName, characterName, MAX_PLAYER_NAME - 1);
        stats->characterName[MAX_PLAYER_NAME - 1] = '\0';
    }
}

static void GerarRelatorioGeralIA(char *relatorioGeral, size_t tamanho, bool aprovado)
{
    char prompt[8192];
    snprintf(prompt, sizeof(prompt),
        /* --- CONTEXTO DO PERSONAGEM --------------------------------------------- */
        "Voce e Hank, ex-agente de ciberseguranca e atual CEO da Firewall Corp. "
        "Veterano, sarcastico e autoconfiante, Hank fala como quem ja viu de tudo: "
        "elogia quando o candidato manda bem, critica sem rodeios quando pisa na bola, "
        "e sempre deixa claro se enxerga intencao de fazer o BEM ou o MAL com suas habilidades.\n\n"

        /* --- DIRETRIZES (SEM SOAR COMO ROBO) ------------------------------------ */
        "⚠️  Diretrizes (NAO seja excessivamente rigoroso):\n"
        "• Escreva como Hank falaria, em frases naturais.\n"
        "• O RELATORIO_GERAL deve ter **pelo menos 150 palavras**.\n"
        "• Se nao houver indicios claros de ma conduta, presuma boa fe.\n"
        "• Criticas severas so quando houver evidencia explicita de intencao maliciosa.\n"
        "• QUANDO o candidato for aprovado, Hank deve **elogiar com entusiasmo**: use verbos fortes "
        "(`mandou ver`, `arrasou`, `impecavel`, etc.), destaque varias qualidades concretas e deixe "
        "clarissimo que o participante esta CONTRATADO.\n"
        "• QUANDO o candidato for reprovado, Hank deve **apontar claramente os erros**: cite os deslizes "
        "especificos (ex. 'vacilou na gestao de senhas', 'foi precipitado', 'ignorou boas praticas'), "
        "explique por que esses problemas pesam na decisao e deixe evidente que ele NAO foi aprovado.\n\n"

        /* --- RESTRICOES DE FORMATO --------------------------------------------- */
        "🔄 Restricoes de formato:\n"
        "• O RELATORIO_GERAL deve caber em **UMA** unica linha (sem quebras).\n\n"

        /* --- INSTRUCAO DE SINTESE ---------------------------------------------- */
        "Com base nos relatórios individuais a seguir, gere um *RELATORIO_GERAL* coerente.\n"
        "IMPORTANTE: os QUATRO relatorios abaixo se referem ao MESMO candidato, analisado em momentos diferentes, "
        "nao a quatro pessoas distintas.\n"
        "Criterios de sintese:\n"
        "- Intencao Etica (Bem x Mal)\n"
        "- Responsabilidade Profissional\n"
        "- Gestao de Informacoes Sensiveis\n\n"

        /* --- LISTA DE RELATORIOS ------------------------------------------------ */
        "Relatorios individuais (mesmo candidato):\n"
    );

    for (int i = 0; i < 4; i++) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "Relatório %d: %s\n", i + 1, relatoriosIA[i]);
        strcat(prompt, buffer);
    }

    strcat(prompt,
        "\nFormato da resposta:\n"
        "RELATORIO_GERAL=<relatório consolidado e objetivo>"
    );

    char retorno[8192] = {0};
    ObterRespostaGemini(prompt, retorno);

    const char *TAG = "RELATORIO_GERAL=";
    const char *src = strstr(retorno, TAG);

    if (src) src += strlen(TAG);
    else if (retorno[0]) src = retorno;
    else src = "Relatório geral indisponível — resposta vazia da IA.";

    strncpy(relatorioGeral, src, tamanho - 1);
    relatorioGeral[tamanho - 1] = '\0';
}


static int SomarNotasIA(void) 
{
    int soma = 0;
    for (int i = 0; i < 4; i++) soma += notasIA[i];
    return soma;
}

static float CalculatePlayerScore(PlayerStats *ps)
{
    if (!ps) return 0.0f;

    float notaGeral = 0.0;

    notaGeral += ps->aiOverallScore * 0.5f;

    if (ps->isPassed_D01) notaGeral += 15.0f;
    if (ps->isPassed_D02) notaGeral += 15.0f;
    if (ps->isPassed_D03) notaGeral += 10.0f;
    if (ps->isPassed_D04) notaGeral += 10.0f;

    float bonus = 0.0;

    if (ps->isPassed_D02) bonus += (0.54 * (float)ps->amountOfLives_D02);
    if (ps->isPassed_D04) bonus += (0.2 * (float)ps->quantityOfIcons_D04);

    notaGeral += bonus;

    int desafiosAprovados =
        (ps->isPassed_D01 ? 1 : 0) +
        (ps->isPassed_D02 ? 1 : 0) +
        (ps->isPassed_D03 ? 1 : 0) +
        (ps->isPassed_D04 ? 1 : 0);

    ps->isPassouSelecao = (ps->aiOverallScore >= 60.0f && desafiosAprovados >= 3);

    return notaGeral;
}

void SetPlayerGeneralStats(PlayerStats *ps)
{
    if (!ps) return;

    SetPlayerAndCharacter(ps, gPlayerName, gSelectedCharacterName);
    ps->aiOverallScore = SomarNotasIA();
    ps->notalGeral = CalculatePlayerScore(ps);
    GerarRelatorioGeralIA(ps->relatorioGeral, sizeof(ps->relatorioGeral), ps->isPassouSelecao);
}
