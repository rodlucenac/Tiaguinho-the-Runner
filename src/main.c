#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define GRAVITY 9.8f
#define JUMP_FORCE 10.0f
#define PLAYER_SPEED 5.0f

typedef struct {
  Vector2 position;
  Vector2 speed;
  Vector2 direction;
  bool isJumping;
  int score;
} Player;

typedef struct RankingEntry {
  char nome[51];
  int score;
  struct RankingEntry *next;
} RankingEntry;

typedef struct Obstacle {
  Vector2 position;
  Vector2 size;
  bool isMoving;
  Vector2 speed;
  Vector2 direction;
  struct Obstacle *next;
} Obstacle;

typedef struct Enemy {
  Vector2 position;
  Vector2 speed;
  Vector2 direction;
  bool isAlive;
  struct Enemy *next;
} Enemy;

typedef struct {
    int volume;
    int difficulty;
    bool fullscreen;
} Settings;

Player InitPlayer();
Enemy CreateEnemy(Vector2 position, Enemy *next);
Obstacle CreateObstacle(Vector2 position, Vector2 size, bool isMoving, Vector2 speed, Vector2 direction, Obstacle *next);
void FreeEnemies(Enemy *head);
void FreeObstacles(Obstacle *head);
void AddToRanking(RankingEntry **head, const char *nome, int score);
void SaveRanking(RankingEntry *head);
RankingEntry *LoadRanking();
void FreeRanking(RankingEntry *head);
Settings *LoadSettings();
void SaveSettings(Settings *settings);

int main() {
  return 0;
}

Player InitPlayer() {
  Player player;
  player.position = (Vector2){ 100, SCREEN_HEIGHT - 50 };
  player.speed = (Vector2){ 0, 0 };
  player.direction = (Vector2){ 1, 0 };
  player.isJumping = false;
  player.score = 0;
  return player;
}

Enemy *CreateEnemy(Vector2 position, Enemy *next) {
  Enemy *enemy = (Enemy *)malloc(sizeof(Enemy));
  enemy->position = position;
  enemy->speed = (Vector2){ 0, 0 };
  enemy->direction = (Vector2){ -1, 0 };
  enemy->isAlive = true;
  enemy->next = next;
  return enemy;
}

Obstacle *CreateObstacle(Vector2 position, Vector2 size, bool isMoving, Vector2 speed, Vector2 direction, Obstacle *next) {
  Obstacle *obstacle = (Obstacle *)malloc(sizeof(Obstacle));
  obstacle->position = position;
  obstacle->size = size;
  obstacle->speed = speed;
  obstacle->direction = direction;
  obstacle->isMoving = isMoving;
  obstacle->next = next;
  return obstacle;
}

void FreeObstacles(Obstacle *head) {
  Obstacle *current = head;
  while (current != NULL) {
    Obstacle *next = current->next;
    free(current);
    current = next;
  }
}

void FreeEnemies(Enemy *head) {
  Enemy *current = head;
  while (current != NULL) {
    Enemy *next = current->next;
    free(current);
    current = next;
  }
}

void AddToRanking(RankingEntry **head, const char *nome, int score) {
  RankingEntry *newEntry = (RankingEntry *)malloc(sizeof(RankingEntry));
  strcpy(newEntry->nome, nome);
  newEntry->score = score;
  newEntry->next = *head;
  *head = newEntry;
}

void SaveRanking(RankingEntry *head) {
  FILE *file = fopen("ranking.txt", "w");
  if (file == NULL) {
    printf("Erro ao abrir o arquivo de ranking!\n");
    return;
  }

  RankingEntry *current = head;
  while (current != NULL) {
    fprintf(file, "%s %d\n", current->nome, current->score);
    current = current->next;
  }

  fclose(file);
}

RankingEntry *LoadRanking() {
  FILE *file = fopen("ranking.txt", "r");
  if (file == NULL) {
    printf("Erro ao abrir o arquivo de ranking! O ranking será inicializado vazio.\n");
    return NULL;
  }

  RankingEntry *head = NULL;
  char nome[51];
  int score;

  while (fscanf(file, "%50s %d", nome, &score) == 2) {
    AddToRanking(&head, nome, score);
  }

  fclose(file);
  return head;
}

void FreeRanking(RankingEntry *head) {
  RankingEntry *current = head;
  while (current != NULL) {
    RankingEntry *next = current->next;
    free(current);
    current = next;
  }
}

Settings *LoadSettings() {
  FILE *file = fopen("settings.txt", "r");
  if (file == NULL) {
    printf("Erro ao abrir o arquivo de configurações! Usando configurações padrão.\n");
    Settings *defaultSettings = (Settings *)malloc(sizeof(Settings));
    defaultSettings->volume = 50;
    defaultSettings->difficulty = 2;
    defaultSettings->fullscreen = false;
    return defaultSettings;
  }

  Settings *settings = (Settings *)malloc(sizeof(Settings));
  fscanf(file, "%d %d %d", &settings->volume, &settings->difficulty, &settings->fullscreen);
  fclose(file);
  return settings;
}

void SaveSettings(Settings *settings) {
  FILE *file = fopen("settings.txt", "w");
  if (file == NULL) {
    printf("Erro ao abrir o arquivo de configurações!\n");
    return;
  }

  fprintf(file, "%d %d %d\n", settings->volume, settings->difficulty, settings->fullscreen);
  fclose(file);
}