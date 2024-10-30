#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define GRAVITY 9.8f
#define JUMP_FORCE 10.0f
#define PLAYER_SPEED 5.0f

typedef struct{
  Vector2 position;
  Vector2 speed;
  Vector2 direction;
  bool isJumping;
  int score;
} Player;

typedef struct{
  char nome[51];
  int score;
  struct RankingEntry *next;
} RankingEntry;

typedef struct{
  Vector2 position;
  Vector2 size;
  struct Obstacle *next;
} Obstacle;

typedef struct{
  Vector2 position;
  Vector2 initialPosition;
  Vector2 speed;
  Vector2 direction;
  float maxDistance;
  bool isAlive;
  struct Enemy *next;
} Enemy;

typedef struct{
  int volume;
  int difficulty;
  bool fullscreen;
} Settings;

Player InitPlayer();
Enemy *CreateEnemy(Vector2 position, float maxDistance, Enemy *next);
Obstacle *CreateObstacle(Vector2 position, Vector2 size, Obstacle *next);
void FreeEnemies(Enemy *head);
void FreeObstacles(Obstacle *head);
void UpdatePlayer(Player *player, float deltaTime);
void UpdateEnemy(Enemy *enemy, float deltaTime);
void UpdateObstacle(Obstacle *obstacle, float deltaTime);
bool CheckCollisionEnemy(Player *player, Enemy *enemy);
bool CheckCollisionObstacle(Player *player, Obstacle *Obstacle);
void AddToRanking(RankingEntry **head, const char *nome, int score);
void SaveRanking(RankingEntry *head);
RankingEntry *LoadRanking();
void FreeRanking(RankingEntry *head);
Settings *LoadSettings();
void SaveSettings(Settings *settings);
void UpdateCameraPlayerBounds(Camera2D *camera, Player *player);

int main(){
  Player player = InitPlayer();
  
  Camera2D camera;
  camera.target = (Vector2){ player.position.x + 25, player.position.y + 25 };
  camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  return 0;
}

Player InitPlayer(){
  Player player;
  player.position = (Vector2){ 100, SCREEN_HEIGHT - 50 };
  player.speed = (Vector2){ 0, 0 };
  player.direction = (Vector2){ 1, 0 };
  player.isJumping = false;
  player.score = 0;
  return player;
}

Enemy *CreateEnemy(Vector2 position, float maxDistance, Enemy *next){
  Enemy *enemy = (Enemy *)malloc(sizeof(Enemy));
  enemy->position = position;
  enemy->initialPosition = position;
  enemy->speed = (Vector2){ 100, 0 };
  enemy->direction = (Vector2){ -1, 0 };
  enemy->maxDistance = maxDistance;
  enemy->isAlive = true;
  enemy->next = next;
  return enemy;
}

Obstacle *CreateObstacle(Vector2 position, Vector2 size, Obstacle *next){
  Obstacle *obstacle = (Obstacle *)malloc(sizeof(Obstacle));
  obstacle->position = position;
  obstacle->size = size;
  obstacle->next = next;
  return obstacle;
}

void FreeObstacles(Obstacle *head){
  Obstacle *current = head;
  while (current != NULL) {
    Obstacle *next = current->next;
    free(current);
    current = next;
  }
}

void FreeEnemies(Enemy *head){
  Enemy *current = head;
  while (current != NULL) {
    Enemy *next = current->next;
    free(current);
    current = next;
  }
}

void UpdatePlayer(Player *player, float deltaTime){
  if (IsKeyDown(KEY_D)){
    player->position.x += PLAYER_SPEED * deltaTime;
    player->direction = (Vector2){1, 0};
  }
  if (IsKeyDown(KEY_A)){
    player->position.x -= PLAYER_SPEED * deltaTime;
    player->direction = (Vector2){-1, 0};
  }
  
  if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W)) && !player->isJumping){
    player->speed.y = -JUMP_FORCE;
    player->isJumping = true;
  }

  player->speed.y += GRAVITY * deltaTime;
  player->position.y += player->speed.y * deltaTime;

  if (player->position.y >= SCREEN_HEIGHT - 50){
    player->position.y = SCREEN_HEIGHT - 50;
    player->isJumping = false;
  }
}

void UpdateEnemy(Enemy *enemy, float deltaTime){
  enemy->position.x += enemy->speed.x * enemy->direction.x * deltaTime;

  if (enemy->position.x < enemy->initialPosition.x - enemy->maxDistance || 
    enemy->position.x > enemy->initialPosition.x + enemy->maxDistance) {
    enemy->direction.x *= -1;
  }
}

void UpdateObstacle(Obstacle *obstacle, float deltaTime){
  obstacle->position.x += obstacle->size.x * deltaTime;

  if (obstacle->position.x > 500 || obstacle->position.x < 100) {
    obstacle->size.x *= -1;
  }
}

bool CheckCollisionEnemy(Player *player, Enemy *enemy){
  Rectangle playerRec = { player->position.x, player->position.y, 50, 50 };
  Rectangle enemyRec = { enemy->position.x, enemy->position.y, 50, 50 };

  return CheckCollisionRecs(playerRec, enemyRec);
}

bool CheckCollisionObstacle(Player *player, Obstacle *obstacle){
  Rectangle playerRec = { player->position.x, player->position.y, 50, 50 };
  Rectangle obstacleRec = { obstacle->position.x, obstacle->position.y, 50, 50 };

  return CheckCollisionRecs(playerRec, obstacleRec);
}

void AddToRanking(RankingEntry **head, const char *nome, int score){
  RankingEntry *newEntry = (RankingEntry *)malloc(sizeof(RankingEntry));
  strcpy(newEntry->nome, nome);
  newEntry->score = score;
  newEntry->next = *head;
  *head = newEntry;
}

void SaveRanking(RankingEntry *head){
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

RankingEntry *LoadRanking(){
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

void FreeRanking(RankingEntry *head){
  RankingEntry *current = head;
  while (current != NULL) {
    RankingEntry *next = current->next;
    free(current);
    current = next;
  }
}

Settings *LoadSettings(){
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

void SaveSettings(Settings *settings){
  FILE *file = fopen("settings.txt", "w");
  if (file == NULL) {
    printf("Erro ao abrir o arquivo de configurações!\n");
    return;
  }

  fprintf(file, "%d %d %d\n", settings->volume, settings->difficulty, settings->fullscreen);
  fclose(file);
}

void UpdateCameraPlayerBounds(Camera2D *camera, Player *player){
  if (player->position.x > SCREEN_WIDTH/2.0f) {
    camera->target.x = player->position.x;
  }
  
  if (player->position.x < SCREEN_WIDTH/2.0f) {
    camera->target.x = player->position.x;
  }
}