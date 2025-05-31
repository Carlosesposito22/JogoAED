@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
color 0a
title Terminal de Teste - FBI
cls

echo.
echo  88""Yb 888888  dP"Yb  88""Yb  dP""b8 888888 88""Yb      
echo  88__dP 88__   dP   Yb 88__dP dP   `" 88__   88__dP     
echo  88""Yb 88""   Yb   dP 88"Yb  Yb      88""   88"Yb     
echo  88oodP 88      YbodP  88  Yb  YboodP 888888 88  Yb    
echo.

:: Solicita e valida URL do site antes de prosseguir
:AskURL
set /p siteURL=Cole o link do site: 
if /i not "!siteURL!"=="https://cybertechinc.vercel.app/" (
    echo [ERRO] URL invalida. Tente novamente.
    goto AskURL
)

:: Solicita path de users.txt até ser válido
:AskUsers
set /p usersFile=Cole o arquivo users.txt aqui: 
set "usersFile=!usersFile:"=! "
if not exist "!usersFile!" (
    echo [ERRO] Arquivo "!usersFile!" nao encontrado.
    goto AskUsers
)

:: Solicita path de passwords.txt até ser válido
:AskPasswords
set /p passFile=Cole o arquivo passwords.txt aqui: 
set "passFile=!passFile:"=! "
if not exist "!passFile!" (
    echo [ERRO] Arquivo "!passFile!" nao encontrado.
    goto AskPasswords
)

:: Inicializa contadores e barra
set tentativa=0
set "bar="

cls
echo [INFO] Iniciando brute-force...
echo.

:: Loop de brute-force com contagem, barra visual e delay
for /f "usebackq delims=" %%u in ("!usersFile!") do (
    for /f "usebackq delims=" %%p in ("!passFile!") do (
        set /a tentativa+=1
        set "bar=!bar!#"
        cls
        echo [INFO] [!bar!]
        echo [INFO] Tentativa !tentativa!: %%u / %%p

        :: Pequeno delay para visualizar cada "frame"
        ping 127.0.0.1 -n 1 >nul

        :: Verifica credenciais corretas
        if /i "%%u"=="superuser" if "%%p"=="passwd#Su123" (
            echo.
            echo credentials found:
            echo Username: superuser
            echo Senha: passwd#Su123
            ping 127.0.0.1 -n 61 >nul
            echo [INFO] Atualizando...
            ping 127.0.0.1 -n 6 >nul
        )
    )
)

:AskSecretKey
cls
echo [INFO] Secret Base64 Encrypted ID Key:
set /p secretKey=Digite a chave secreta encontrada no site: 
if /i "!secretKey!"=="dGVzdCBwYXNzZWQ=" (
    echo [INFO] Decrypted key: 'test passed'
    echo OK > dadosBruteForce.txt
    ping 127.0.0.1 -n 3 >nul
    exit
) else (
    echo [ERRO] Chave incorreta. Tente novamente.
    goto AskSecretKey
)

:End
endlocal
exit /b
