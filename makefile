TARGET = Through-The-Firewall

# Estrutura do projeto
SRC_DIR = src
INCLUDE_DIR = include
LIB_DIR = lib_raylib
BUILD_DIR = bin
RELEASE_DIR = release
RELEASE_SRC = $(RELEASE_DIR)/src
CURL_DLL = $(BUILD_DIR)/libcurl-x64.dll

# Diretórios que precisam ir para a release/src
ASSET_DIRS = fonts models music sprites tasks
BAT_FILES = bruteForce_terminal.bat first_terminal.bat keylogger_terminal.bat proxy_terminal.bat shell.bat
TXT_FILES = shellPICA.txt ranking.txt
EXTRA_DIRS = reverseShellFiles hackingFiles

# Saídas
EXE_EXT = .exe
BIN_TARGET = $(BUILD_DIR)/$(TARGET)$(EXE_EXT)
REL_TARGET = $(RELEASE_DIR)/$(TARGET)$(EXE_EXT)
ICON_RC  = resource.rc
ICON_OBJ = resource.o

# Compilador e opções
CC = gcc
CFLAGS = -Wall -std=c99 -I$(INCLUDE_DIR) -Icurl/include -mwindows
LIBS = -L$(LIB_DIR) -Lcurl/lib -lraylib -lcurl -lopengl32 -lgdi32 -lwinmm -mwindows
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Compilar para pasta bin
$(BIN_TARGET): $(SOURCES) $(ICON_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) $(ICON_OBJ) -o $@ $(LIBS)

# Compila ícone no Windows quando existir resource.rc
$(ICON_OBJ): $(ICON_RC)
	windres $< -o $@

# Executar o jogo
run: $(BIN_TARGET)
	./$<

# Compilar e rodar em sequência
makerun: $(BIN_TARGET)
	./$<

# ---------- “Release” ----------
release: $(BIN_TARGET)
	@echo "Criando release..."
	@mkdir -p $(RELEASE_SRC)
	@cp $(BIN_TARGET) $(REL_TARGET)

	@echo ">> Copiando libcurl-x64.dll:"
	@if [ -f $(CURL_DLL) ]; then \
		cp $(CURL_DLL) $(RELEASE_DIR)/ ; \
		echo "   - $(CURL_DLL) copiado para release/"; \
	else \
		echo "   ! $(CURL_DLL) não encontrado - verifique a build."; \
	fi

	@echo ">> Copiando assets:"
	@for d in $(ASSET_DIRS); do \
		if [ -d $(SRC_DIR)/$$d ]; then \
			echo "   - $$d"; \
			cp -r $(SRC_DIR)/$$d $(RELEASE_SRC)/ ; \
		else \
			echo "   ! Diretório $(SRC_DIR)/$$d não encontrado - ignorado."; \
		fi ; \
	done

	@echo ">> Copiando scripts .bat e .txt:"
	@for f in $(BAT_FILES) $(TXT_FILES); do \
		if [ -f $$f ]; then \
			cp $$f $(RELEASE_DIR)/ ; \
			echo "   - $$f"; \
		else \
			echo "   ! $$f não encontrado - ignorado."; \
		fi ; \
	done

	@echo ">> Copiando diretorios extras:"
	@for d in $(EXTRA_DIRS); do \
		if [ -d $$d ]; then \
			cp -r $$d $(RELEASE_DIR)/ ; \
			echo "   - $$d"; \
		else \
			echo "   ! $$d não encontrado - ignorado."; \
		fi ; \
	done
	@echo "Release gerada em: $(RELEASE_DIR)"

# Limpar binário
clean:
	cmd /C "del /Q $(BUILD_DIR)\$(TARGET)$(EXE_EXT) 2>nul || echo Nada para limpar"
