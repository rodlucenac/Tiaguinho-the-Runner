#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define GRAVITY 40.0f
#define JUMP_FORCE 80.0f
#define PLAYER_SPEED 120.0f
#define MAX_SPEED 300.0f
#define SPEED_INCREMENT 1.0f

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
void AddToRanking(RankingEntry **head, const char *nome, int score);
void SaveRanking(RankingEntry *head);
void FreeObstacles(Obstacle *head);
void FreeEnemies(Enemy *head);
void FreeRanking(RankingEntry *head);
void UpdatePlayer(Player *player, float deltaTime, float currentSpeed);
void CheckGroundCollision(Player *player, Obstacle *obstacles);
void ResetGame(Player *player, Obstacle **obstacles, Enemy **enemies, float *lastGeneratedX, float *lastGeneratedBackgroundX);

int main(){
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tiaguinho the Runner");
  InitAudioDevice();
  SetTargetFPS(60);

  Texture2D background = LoadTexture("resources/texture/background.png");
  Texture2D ground = LoadTexture("resources/texture/ground.png");

  GameState state = MENU;
  int selectedOption = 0;
  Player player;
  Obstacle *obstacles = NULL;
  Enemy *enemies = NULL;
  RankingEntry *ranking = LoadRanking();
  char playerName[51] = "";
  bool nameEntered = false;

  static float lastGeneratedX = SCREEN_WIDTH;
  static float lastGeneratedBackgroundX = SCREEN_WIDTH;

  while (!nameEntered){
    BeginDrawing();
    ClearBackground(RAYWHITE);
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
    DrawText("Enter your name: ", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 20, 20, DARKBLUE);
    DrawText(playerName, SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 20, 20, DARKBLUE);

    int key = GetCharPressed();
    if (key >= 32 && key <= 125 && strlen(playerName) < 50){
      strncat(playerName, (char[]){(char)key, '\0'}, 1);
    }
    if (IsKeyPressed(KEY_BACKSPACE) && strlen(playerName) > 0){
      playerName[strlen(playerName) - 1] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER) && strlen(playerName) > 0){
      nameEntered = true;
    }
    EndDrawing();
  }

  while (!WindowShouldClose() && state != EXIT){
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (state){
      case MENU:{

        bool resetGame = true;
        if (resetGame){
          ResetGame(&player, &obstacles, &enemies, &lastGeneratedX, &lastGeneratedBackgroundX);
          resetGame = false;
        }
        
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

        const char *options[] = {"Start Game", "View Ranking", "Exit"};
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
          if (selectedOption == 0){
            state = GAME;
          } else if (selectedOption == 1){
            state = RANKING;
          } else if (selectedOption == 2){
            state = EXIT;
          }
        }
      } break;

      case GAME:{
        Camera2D camera = {0};
        camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
        camera.zoom = 1.0f;
        camera.rotation = 0.0f;

        static Texture2D platformTexture = {0};
        if (!platformTexture.id) platformTexture = LoadTexture("resources/texture/platform.png");

        static Texture2D enemyTexture = {0};
        if (!enemyTexture.id) enemyTexture = LoadTexture("resources/texture/enemy.png");

        static Texture2D playerRightTexture = {0};
        if (!playerRightTexture.id) playerRightTexture = LoadTexture("resources/texture/player_right.png");

        static Texture2D playerLeftTexture = {0};
        if (!playerLeftTexture.id) playerLeftTexture = LoadTexture("resources/texture/player_left.png");

        float deltaTime = GetFrameTime();
        static float startTime = 0.0f;

        if (startTime == 0.0f){
          startTime = GetTime();
        }

        float elapsedTime = GetTime() - startTime;

        float currentSpeed = PLAYER_SPEED + (SPEED_INCREMENT * elapsedTime);
        if (currentSpeed > MAX_SPEED){
          currentSpeed = MAX_SPEED;
        }

        Texture2D currentPlayerTexture = playerRightTexture;
        if (IsKeyDown(KEY_A)){
          currentPlayerTexture = playerLeftTexture;
        }

        UpdatePlayer(&player, deltaTime, currentSpeed);

        if (player.position.y >= SCREEN_HEIGHT - ground.height - 135){
          player.position.y = SCREEN_HEIGHT - ground.height - 135;
          player.isJumping = false;
        }

        if (player.position.x > SCREEN_WIDTH / 2){
          camera.target.x = player.position.x + 25;
        } else{
          camera.target.x = SCREEN_WIDTH / 2;
        }
        camera.target.y = SCREEN_HEIGHT / 2;

        if (player.position.x + SCREEN_WIDTH > lastGeneratedX){
          obstacles = CreateObstacle(
            (Vector2){lastGeneratedX + 200, 250},
            (Vector2){150, 30},
            obstacles
          );
          obstacles = CreateObstacle(
            (Vector2){lastGeneratedX + 500, 150},
            (Vector2){150, 30},
            obstacles
          );

          enemies = CreateEnemy(
            (Vector2){lastGeneratedX + 400, SCREEN_HEIGHT - ground.height - 50},
            100,
            enemies
          );

          lastGeneratedX += SCREEN_WIDTH;
        }

        if (player.position.x + SCREEN_WIDTH > lastGeneratedBackgroundX){
          lastGeneratedBackgroundX += SCREEN_WIDTH;
        }

        Enemy *currentEnemy = enemies;
        while (currentEnemy != NULL){
          currentEnemy->position.x += currentEnemy->speed.x * currentEnemy->direction.x * deltaTime;

          if (currentEnemy->position.x < currentEnemy->initialPosition.x - currentEnemy->maxDistance ||
            currentEnemy->position.x > currentEnemy->initialPosition.x + currentEnemy->maxDistance){
            currentEnemy->direction.x *= -1;
          }

          Rectangle playerRec = {player.position.x, player.position.y, 100, 150};
          Rectangle enemyRec = {currentEnemy->position.x, currentEnemy->position.y, 50, 50};

          if (CheckCollisionRecs(playerRec, enemyRec) && currentEnemy->isAlive){
            AddToRanking(&ranking, playerName, (int)elapsedTime);
            SaveRanking(ranking);
            state = MENU;
            startTime = 0.0f;
            break;
          }

          currentEnemy = currentEnemy->next;
        }

        BeginMode2D(camera);

        for (float x = 0; x <= lastGeneratedBackgroundX; x += background.width){
          DrawTexture(background, x, 0, WHITE);
        }
        for (float x = 0; x <= lastGeneratedX; x += ground.width){
          DrawTexturePro(
            ground,
            (Rectangle){0, 0, ground.width, ground.height},
            (Rectangle){x, SCREEN_HEIGHT - ground.height, ground.width, ground.height},
            (Vector2){0, 0},
            0.0f,
            WHITE
          );
        }

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

        currentEnemy = enemies;
        while (currentEnemy != NULL){
          if (currentEnemy->isAlive){
            DrawTexturePro(
              enemyTexture,
              (Rectangle){0, 0, enemyTexture.width, enemyTexture.height},
              (Rectangle){
                currentEnemy->position.x,
                currentEnemy->position.y,
                50, 50
              },
              (Vector2){0, 0},
              0.0f,
              WHITE
            );
          }
          currentEnemy = currentEnemy->next;
        }

        DrawTexturePro(
          currentPlayerTexture,
          (Rectangle){0, 0, currentPlayerTexture.width, currentPlayerTexture.height},
          (Rectangle){
            player.position.x,
            player.position.y,
            100, 150
          },
          (Vector2){0, 0},
          0.0f,
          WHITE
        );

        EndMode2D();

        if (IsKeyPressed(KEY_BACKSPACE)){
          state = MENU;
          SaveRanking(ranking);
          startTime = 0.0f;
        }
      } break;


      case RANKING:{
        int yOffset = 100;
        RankingEntry *current = ranking;

        DrawTexture(background, 0, 0, WHITE);
        DrawTexturePro(
          ground,
          (Rectangle){0, 0, ground.width, ground.height},
          (Rectangle){0, SCREEN_HEIGHT - ground.height, SCREEN_WIDTH, ground.height},
          (Vector2){0, 0},
          0.0f,
          WHITE
        );

        const char *gameName = "RANKING";
        DrawText(
          gameName,
          SCREEN_WIDTH / 2 - MeasureText(gameName, 40) / 2,
          50,
          40,
          DARKBLUE
        );

        while (current != NULL){
          char rankingText[100];
          snprintf(rankingText, sizeof(rankingText), "%s - %d seconds", current->nome, current->score);

          DrawText(
            rankingText,
            SCREEN_WIDTH / 2 - MeasureText(rankingText, 20) / 2,
            yOffset,
            20,
            DARKBLUE
          );
          yOffset += 30;
          current = current->next;
        }

        const char *backText = "Press BACKSPACE to return";
        DrawText(
          backText,
          SCREEN_WIDTH / 2 - MeasureText(backText, 20) / 2,
          yOffset + 50,
          20,
          DARKGRAY
        );

        if (IsKeyPressed(KEY_BACKSPACE)){
          state = MENU;
        }
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

void AddToRanking(RankingEntry **head, const char *nome, int score){
  RankingEntry *newEntry = (RankingEntry *)malloc(sizeof(RankingEntry));
  strcpy(newEntry->nome, nome);
  newEntry->score = score;
  newEntry->next = NULL;

  if (*head == NULL || (*head)->score < score){
    newEntry->next = *head;
    *head = newEntry;
  } else{
    RankingEntry *current = *head;
    while (current->next != NULL && current->next->score >= score){
      current = current->next;
    }
    newEntry->next = current->next;
    current->next = newEntry;
  }
}

void SaveRanking(RankingEntry *head){
  FILE *file = fopen("ranking.txt", "a+");
  if (file == NULL){
    printf("Erro ao salvar o ranking!\n");
    return;
  }
  RankingEntry *current = head;
  while (current != NULL){
    fprintf(file, "%s %d\n", current->nome, current->score);
    current = current->next;
  }
  fclose(file);
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

void UpdatePlayer(Player *player, float deltaTime,float currentSpeed){
  if (IsKeyDown(KEY_D)){
    player->position.x += currentSpeed * deltaTime;
  }
  if (IsKeyDown(KEY_A)){
    player->position.x -= currentSpeed * deltaTime;
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
    Rectangle playerRect = {player->position.x, player->position.y, 100, 150};
    Rectangle obstacleRect = {obstacles->position.x, obstacles->position.y, obstacles->size.x, obstacles->size.y};

    if (CheckCollisionRecs(playerRect, obstacleRect)){
      player->position.y = obstacles->position.y - 135;
      player->isJumping = false;
      player->speed.y = 0;
    }
    obstacles = obstacles->next;
  }
}

void ResetGame(Player *player, Obstacle **obstacles, Enemy **enemies, float *lastGeneratedX, float *lastGeneratedBackgroundX){
  FreeObstacles(*obstacles);
  FreeEnemies(*enemies);

  *player = InitPlayer();

  *obstacles = CreateObstacle((Vector2){200, 200}, (Vector2){150, 30}, NULL);
  *obstacles = CreateObstacle((Vector2){400, 100}, (Vector2){150, 30}, *obstacles);
  *obstacles = CreateObstacle((Vector2){700, 100}, (Vector2){150, 30}, *obstacles);

  *enemies = CreateEnemy((Vector2){500, SCREEN_HEIGHT - 200}, 100, NULL);

  *lastGeneratedX = SCREEN_WIDTH;
  *lastGeneratedBackgroundX = SCREEN_WIDTH;
}