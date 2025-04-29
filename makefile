# ---------------- Variables --------------- #
# Nom du fichier executable final
EXEC = program

# Compilateur et ses arguments
CXX = g++
CXXFLAGS = -Wall `pkg-config --cflags gtk+-2.0`
LDLIBS = `pkg-config --libs gtk+-2.0`

# Répertoires type
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

# Fichiers
SRCS = $(wildcard $(SRC_DIR)/*.cpp) 
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/$(EXEC)

# ------------------ Règles ------------------ #
# Résoudre l'exécutable cible (règle par défaut !)
all: $(TARGET)
	@echo "Création de l'executable terminée."

# Créer les dossiers
setup:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)
	@echo "Création des dossiers terminée."

# Compiler chaque fichier source .cpp en un fichier objet .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compilation terminée."

# Lier les fichiers objets .o ensemble en un executable avec le compilateur (résolue par "all" !)
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDLIBS)
	@echo "Liaisons terminées."

# Exécuter le fichier binaire
run: $(TARGET)
	./$(TARGET)

# Supprimer les fichiers compilés
clean:
	rm -r -f $(OBJ_DIR)/* $(BIN_DIR)/*
	@echo "Nettoyage terminé."

# Recompiler
rebuild: clean all

# Éviter de confondre les règles avec des noms de fichiers
.PHONY: all clean rebuild setup