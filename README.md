https://github.com/user-attachments/assets/b0eaf4c6-e71d-4837-95b8-e3179a23b9eb

---

## 🌟 Sobre o Jogo

O jogador assume o papel de um candidato tentando ingressar na **Cybertech**, um braço do **FBI** especializado em cibersegurança. Para isso, deve enfrentar desafios práticos e éticos sob avaliação do agente **Hank Miccuci**. A cada fase, decisões técnicas e morais são postas à prova, e o desempenho final determinará o sucesso no processo seletivo.

### 🎯 Objetivo

- **Superar os desafios cibernéticos**: Gerenciar recursos, manter sistemas críticos ativos e lidar com situações que testam a ética profissional.
- **Desafios técnicos complexos:** Desenvolver um firewall, implementar um proxy, criar um keylogger e realizar ataques de brute force simulados.
- **Fase final**: Na fase final, o jogador precisará utilizar suas habilidades para hackear um outro computador, provando seu domínio completo sobre técnicas de invasão e controle remoto.
- **Convencer o Hank**: Durante os interrogatórios, responder perguntas que avaliam a ética e a intenção das decisões tomadas ao longo do jogo.

---

## 🪟 Instalação em Windows

1. Baixe o instalador clicando no link: 👉 [**Instalador para Windows**](https://github.com/Carlosesposito22/ThoroughTheFirewall/releases/download/Game/Instalador.Jogo.TTF.exe)

2. Abra o instalador e siga as instruções para concluir a instalação.

3. Após a instalação, execute o jogo diretamente do atalho criado na área de trabalho ou no menu Iniciar.

---

## ⚙️ Requisitos para clonar o projeto

- **Windows**
- [Raylib](https://www.raylib.com/) baixada manualmente
- Acesso ao terminal `w64devkit` (incluso na pasta da Raylib/w64devkit)

---

## 🧭 Passo a Passo para Rodar

### 1. Baixe a Raylib

Acesse o site oficial da Raylib:

🔗 [https://www.raylib.com/](https://www.raylib.com/)

- Baixe a versão para **Windows**
- Extraia os arquivos e navegue até a pasta `/c/raylib/w64devkit`

---

### 2. Crie um atalho do terminal

- Dentro da pasta `w64devkit`, localize o arquivo `w64devkit.exe`
- Crie um **atalho** desse executável na sua **área de trabalho**
- Este terminal é onde você vai compilar e executar o projeto

---

### 3. Entre no terminal especial e rode o jogo

- Clique no atalho criado para abrir o terminal `w64devkit`
- No diretório do jogo, execute o comando:

```bash
make run
```
---

### ⚠️ Aviso Importante ⚠️

O executável do jogo deve estar no mesmo diretório que o arquivo **libcurl-x64.dll** para funcionar corretamente.  

- Mas isso já é feito automaticamente ao utilizar o comando **make run**.
