@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
color 0a
title Keylogger
cls

:: ASCII Art Inicial
echo Keylogger
echo.
echo ██████  ███████  ██████  ██████   ██████ ███████ ██████ .
echo ██   ██ ██      ██    ██ ██   ██ ██      ██      ██   ██ .
echo ██████  █████   ██    ██ ██████  ██      █████   ██████  .
echo ██   ██ ██      ██    ██ ██   ██ ██      ██   ██ ██   ██.
echo ██████  ██       ██████  ██   ██  ██████ ███████ ██   ██ .
echo.

:: Solicita e valida o IP antes de prosseguir
:AskIP
set /p ip=Machine's IP: 
if not "!ip!"=="192.168.54.233" (
    echo [ERRO] Couldn't establish connection. Try again with a valid IP Address.
    goto AskIP
)

cls
echo [INFO] Conectando ao dispositivo...
ping 127.0.0.1 -n 2 >nul
echo [INFO] Conexao estabelecida.
echo.

:: Lista ordenada de palavras com sentido lógico
set palavras[0]=ssh
set palavras[1]=root@localhost
set palavras[2]=[KEY_ENTER]
set palavras[3]=password:
set palavras[4]=admin123
set palavras[5]=[KEY_ENTER]
set palavras[6]=cd
set palavras[7]=/etc/credentials
set palavras[8]=[KEY_ENTER]
set palavras[9]=ls
set palavras[10]=-l
set palavras[11]=[KEY_ENTER]
set palavras[12]=nano
set palavras[13]=api_config.txt
set palavras[14]=[KEY_ENTER]
set palavras[15]=API_KEY:
set palavras[16]=[KEY_SPACE]
set palavras[17]=XI5hp7VKyWhPGvzINNKoqNQkre1qZLeOVkQfR2T8lJBvhSnsYuRCD1DrbR750w8Fy3ksZZQufiJ6M7fKP5kwTYg9ltFyDzOfWHBzpGy4YlfVmdTMBtHRbEAaZoUtlLBF
set palavras[18]=[KEY_ENTER]
set palavras[19]=chmod
set palavras[20]=600
set palavras[21]=api_config.txt
set palavras[22]=[KEY_ENTER]

:: Variável para controle da posição da lista
set pos=0

:: Loop para exibir todas as palavras da lista
:Simulacao
cls
echo [INFO] Keylogger running...
echo.

:loop
if %pos% GEQ 23 goto SolicitarAPI
echo !palavras[%pos%]!
set /a pos+=1
ping 127.0.0.1 -n 2 >nul
goto loop

:: Solicita a API Key após exibir todas as palavras
:SolicitarAPI
cls
echo [INFO] Unexpected error: Connection Lost.
echo.

:AskAPI
set /p inputKey=Insira qual a API KEY descoberta: 
if "%inputKey%"=="XI5hp7VKyWhPGvzINNKoqNQkre1qZLeOVkQfR2T8lJBvhSnsYuRCD1DrbR750w8Fy3ksZZQufiJ6M7fKP5kwTYg9ltFyDzOfWHBzpGy4YlfVmdTMBtHRbEAaZoUtlLBF" (
    echo [SUCESSO] API Key correta. Fechando terminal.
    echo OK > "%~dp0dadosKeylogger.txt"
    ping 127.0.0.1 -n 2 >nul
    exit
) else (
    echo [ERRO] API Key incorreta. Tente novamente.
    goto AskAPI
)
