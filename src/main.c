#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define GRAVITY 9.8f
#define JUMP_FORCE 10.0f
#define PLAYER_SPEED 20.0f

typedef enum{
  MENU,
  GAME,
  RANKING,
  EXIT
} GameState;

typedef struct{
  Vector2 position;
  Vector2 speed;
  bool isJumping;
  int score;
} Player;

typedef struct Obstacle{
  Vector2 position;
  Vector2 size;
  struct Obstacle *next;
} Obstacle;

typedef struct Enemy{
  Vector2 position;
  Vector2 initialPosition;
  Vector2 speed;
  Vector2 direction;
  float maxDistance;
  bool isAlive;
  struct Enemy *next;
} Enemy;

typedef struct RankingEntry{
  char nome[51];
  int score;
  struct RankingEntry *next;
} RankingEntry;

Player InitPlayer();
Obstacle *CreateObstacle(Vector2 position, Vector2 size, Obstacle *next);
Enemy *CreateEnemy(Vector2 position, float maxDistance, Enemy *next);
RankingEntry *LoadRanking();
void FreeObstacles(Obstacle *head);
void FreeEnemies(Enemy *head);
void FreeRanking(RankingEntry *head);
void UpdatePlayer(Player *player, float deltaTime);
void UpdateEnemy(Enemy *enemy, float deltaTime);
void CheckGroundCollision(Player *player, Obstacle *obstacles);
void DrawObstacles(Obstacle *head);
void DrawEnemies(Enemy *head);
void DrawHUD(Player *player);
void UpdateCameraPlayerBounds(Camera2D *camera, Player *player);
void ConstrainCameraToWorld(Camera2D *camera, float worldWidth, float worldHeight);

int main(){
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tiaguinho the Runner");
  InitAudioDevice();
  SetTargetFPS(60);

  Texture2D background = LoadTexture("resources/texture/background.png");
  Texture2D ground = LoadTexture("resources/texture/ground.png");

  GameState state = MENU;
  int selectedOption = 0;
  Player player = InitPlayer();
  Obstacle *obstacles = CreateObstacle((Vector2){200, 200}, (Vector2){150, 30}, NULL);
  obstacles = CreateObstacle((Vector2){400, 100}, (Vector2){150, 30}, obstacles);
  obstacles = CreateObstacle((Vector2){700, 100}, (Vector2){150, 30}, obstacles);
  Enemy *enemies = CreateEnemy((Vector2){500, SCREEN_HEIGHT - 70}, 100, NULL);
  RankingEntry *ranking = LoadRanking();

  while (!WindowShouldClose() && state != EXIT){
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (state){
      case MENU:{
        DrawTexture(background, 0, 0, WHITE);
        DrawTexturePro(
          ground,
          (Rectangle){0, 0, ground.width, ground.height},
          (Rectangle){0, SCREEN_HEIGHT - ground.height, SCREEN_WIDTH, ground.height},
          (Vector2){0, 0},
          0.0f,
          WHITE
        );

        const char *gameName = "TIAGUINHO THE RUNNER";
        DrawText(
          gameName,
          SCREEN_WIDTH / 2 - MeasureText(gameName, 40) / 2,
          50,
          40,
          DARKBLUE
        );

        const char *options[] ={"Start Game", "View Ranking", "Exit"};
        for (int i = 0; i < 3; i++){
          Color color = (i == selectedOption) ? RED : DARKGRAY;
          DrawText(
            options[i],
            SCREEN_WIDTH / 2 - MeasureText(options[i], 30) / 2,
            200 + i * 50,
            30,
            color
          );
        }

        if (IsKeyPressed(KEY_DOWN)) selectedOption = (selectedOption + 1) % 3;
        if (IsKeyPressed(KEY_UP)) selectedOption = (selectedOption + 2) % 3;
        if (IsKeyPressed(KEY_ENTER)){
          if (selectedOption == 0) state = GAME;
          else if (selectedOption == 1) state = RANKING;
          else if (selectedOption == 2) state = EXIT;
        }
      } break;

      case GAME:{
        Camera2D camera = {0};
        camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
        camera.zoom = 1.0f;
        camera.rotation = 0.0f;

        float deltaTime = GetFrameTime();

        if (IsKeyDown(KEY_D)){
          if (player.position.x < SCREEN_WIDTH / 2){
            player.position.x += PLAYER_SPEED * deltaTime;
          } else{
            camera.target.x = player.position.x + 25;
          }
        }

        if (IsKeyDown(KEY_A)){
          player.position.x -= PLAYER_SPEED * deltaTime;
          if (player.position.x < 0){
            player.position.x = 0;
          }
        }

        UpdatePlayer(&player, deltaTime);

        if (player.position.y >= SCREEN_HEIGHT - ground.height - 50){
          player.position.y = SCREEN_HEIGHT - ground.height - 50;
          player.isJumping = false;
        }

        if (player.position.x > SCREEN_WIDTH / 2){
          camera.target.x = player.position.x + 25;
        } else{
          camera.target.x = SCREEN_WIDTH / 2;
        }
        camera.target.y = SCREEN_HEIGHT / 2;

        BeginMode2D(camera);

        DrawTexture(background, 0, 0, WHITE);
        DrawTexturePro(
          ground,
          (Rectangle){0, 0, ground.width, ground.height},
          (Rectangle){0, SCREEN_HEIGHT - ground.height, SCREEN_WIDTH, ground.height},
          (Vector2){0, 0},
          0.0f,
          WHITE
        );

        static Texture2D platformTexture ={0};
        if (!platformTexture.id) platformTexture = LoadTexture("resources/texture/platform.png");

        Obstacle *currentObstacle = obstacles;
        while (currentObstacle != NULL){
          DrawTexturePro(
            platformTexture,
            (Rectangle){0, 0, platformTexture.width, platformTexture.height},
            (Rectangle){
              currentObstacle->position.x, 
              currentObstacle->position.y, 
              currentObstacle->size.x, 
              currentObstacle->size.y
            },
            (Vector2){0, 0},
            0.0f,
            WHITE
          );
          currentObstacle = currentObstacle->next;
        }

        DrawRectangleV(player.position, (Vector2){50, 50}, RED);

        EndMode2D();

        CheckGroundCollision(&player, obstacles);

        if (IsKeyPressed(KEY_BACKSPACE)){
          state = MENU;
          UnloadTexture(platformTexture);
        }
      } break;

      case RANKING:{
        int yOffset = 100;
        RankingEntry *current = ranking;
        DrawText("Ranking:", SCREEN_WIDTH / 2 - 100, 50, 30, DARKGRAY);
        while (current != NULL){
          DrawText(
            TextFormat("%s - %d", current->nome, current->score),
            SCREEN_WIDTH / 2 - 150,
            yOffset,
            20,
            DARKGRAY
          );
          yOffset += 30;
          current = current->next;
        }

        DrawText("Press BACKSPACE to return", SCREEN_WIDTH / 2 - 150, yOffset + 50, 20, GRAY);
        if (IsKeyPressed(KEY_BACKSPACE)) state = MENU;
      } break;

      case EXIT:
        break;
    }

    EndDrawing();
  }

  UnloadTexture(background);
  UnloadTexture(ground);
  FreeObstacles(obstacles);
  FreeEnemies(enemies);
  FreeRanking(ranking);
  CloseWindow();

  return 0;
}

Player InitPlayer(){
  Player player;
  player.position = (Vector2){100, SCREEN_HEIGHT - 50};
  player.speed = (Vector2){0, 0};
  player.isJumping = false;
  player.score = 0;
  return player;
}

Obstacle *CreateObstacle(Vector2 position, Vector2 size, Obstacle *next){
  Obstacle *obstacle = (Obstacle *)malloc(sizeof(Obstacle));
  obstacle->position = position;
  obstacle->size = size;
  obstacle->next = next;
  return obstacle;
}

Enemy *CreateEnemy(Vector2 position, float maxDistance, Enemy *next){
  Enemy *enemy = (Enemy *)malloc(sizeof(Enemy));
  enemy->position = position;
  enemy->initialPosition = position;
  enemy->speed = (Vector2){100, 0};
  enemy->direction = (Vector2){-1, 0};
  enemy->maxDistance = maxDistance;
  enemy->isAlive = true;
  enemy->next = next;
  return enemy;
}

RankingEntry *LoadRanking(){
  FILE *file = fopen("ranking.txt", "r");
  if (file == NULL){
    printf("Erro ao abrir o arquivo de ranking! O ranking será inicializado vazio.\n");
    return NULL;
  }
  RankingEntry *head = NULL;
  char nome[51];
  int score;

  while (fscanf(file, "%50s %d", nome, &score) == 2){
    RankingEntry *newEntry = (RankingEntry *)malloc(sizeof(RankingEntry));
    strcpy(newEntry->nome, nome);
    newEntry->score = score;
    newEntry->next = NULL;

    if (head == NULL || head->score < score){
      newEntry->next = head;
      head = newEntry;
    } else{
      RankingEntry *current = head;
      while (current->next != NULL && current->next->score >= score){
        current = current->next;
      }
      newEntry->next = current->next;
      current->next = newEntry;
    }
  }
  fclose(file);
  return head;
}

void FreeObstacles(Obstacle *head){
  while (head != NULL){
    Obstacle *next = head->next;
    free(head);
    head = next;
  }
}

void FreeEnemies(Enemy *head){
  while (head != NULL){
    Enemy *next = head->next;
    free(head);
    head = next;
  }
}

void FreeRanking(RankingEntry *head){
  while (head != NULL){
    RankingEntry *next = head->next;
    free(head);
    head = next;
  }
}

void UpdatePlayer(Player *player, float deltaTime){
  if (IsKeyDown(KEY_D)){
    player->position.x += PLAYER_SPEED * deltaTime;
  }
  if (IsKeyDown(KEY_A)){
    player->position.x -= PLAYER_SPEED * deltaTime;
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

void CheckGroundCollision(Player *player, Obstacle *obstacles){
  while (obstacles != NULL){
    Rectangle playerRect = {player->position.x, player->position.y, 50, 50};
    Rectangle obstacleRect = {obstacles->position.x, obstacles->position.y, obstacles->size.x, obstacles->size.y};

    if (CheckCollisionRecs(playerRect, obstacleRect)){
      player->position.y = obstacles->position.y - 50;
      player->isJumping = false;
      player->speed.y = 0;
    }
    obstacles = obstacles->next;
  }
}

void UpdateEnemy(Enemy *enemy, float deltaTime){
  while (enemy != NULL){
    enemy->position.x += enemy->speed.x * enemy->direction.x * deltaTime;

    if (enemy->position.x < enemy->initialPosition.x - enemy->maxDistance || 
      enemy->position.x > enemy->initialPosition.x + enemy->maxDistance){
      enemy->direction.x *= -1;
    }
    enemy = enemy->next;
  }
}

void DrawObstacles(Obstacle *head){
  while (head != NULL){
    DrawRectangleV(head->position, head->size, BROWN);
    head = head->next;
  }
}

void DrawEnemies(Enemy *head){
  while (head != NULL){
    if (head->isAlive){
      DrawRectangleV(head->position, (Vector2){50, 50}, DARKPURPLE);
    }
    head = head->next;
  }
}

void DrawHUD(Player *player){
  DrawText(TextFormat("Score: %d", player->score), 10, 10, 20, BLACK);
}

void UpdateCameraPlayerBounds(Camera2D *camera, Player *player){
  camera->target = player->position;
}

void ConstrainCameraToWorld(Camera2D *camera, float worldWidth, float worldHeight){
  if (camera->target.x < SCREEN_WIDTH / 2) camera->target.x = SCREEN_WIDTH / 2;
  if (camera->target.y < SCREEN_HEIGHT / 2) camera->target.y = SCREEN_HEIGHT / 2;

  if (camera->target.x > worldWidth - SCREEN_WIDTH / 2) camera->target.x = worldWidth - SCREEN_WIDTH / 2;
  if (camera->target.y > worldHeight - SCREEN_HEIGHT / 2) camera->target.y = worldHeight - SCREEN_HEIGHT / 2;
}