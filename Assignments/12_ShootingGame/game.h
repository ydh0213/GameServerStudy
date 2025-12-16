#pragma once

#include <chrono>
#include <vector>
#include <map>
#include <string>

#include "Console.h"

using namespace std;
using namespace std::chrono;

struct Player
{
	int y, x;
};

struct PlayerBullet
{
	int y, x;
};

struct EnemyType
{
	char shape;
	int hp;
	int speed;
	string movePattern;
};

struct MovePattern
{
	vector<pair<int, int>> coord;
};

struct Enemy
{
	char shape;
	int y, x;
	int hp;
	int speed;
	string movePattern;
};

struct EnemyBullet
{
	int y, x;
};


enum class Scene
{
	TITLE, SELECT_STAGE, LOAD, GAME, OVER, EXIT
};

const int PLAYER_BULLET_MAX = 50;
const string ENEMYINFO_FILENAME = "EnemyInfo.txt";
const int ENEMY_MAX = 80;
const int ENEMY_BULLET_MAX = 100;
const string STAGEINFO_FILENAME = "StageInfo.txt";
const int FPS = 25; // 기준 frames per second
const milliseconds DURATION = milliseconds(1000 / FPS); // 프레임 간격 시간
const time_point TP_START = steady_clock::now();

extern Scene g_gameScene;
extern Player g_player;
extern int g_playerHp;
extern PlayerBullet g_playerBullets[PLAYER_BULLET_MAX];
extern map<int, EnemyType> g_enemyTypes;
extern map<string, MovePattern> g_movePattern;
extern Enemy g_enemy[ENEMY_MAX];
extern EnemyBullet g_enemyBullet[ENEMY_BULLET_MAX];
extern vector<string> g_stageName;
extern int g_selectStageCursor;

extern steady_clock::time_point g_lastTp;
extern steady_clock::time_point g_tpNextGoal; // 다음 목표 시각
extern int g_frameCount_Logic;
extern int g_frameCount_Render;
extern int g_frame_Logic;
extern int g_frame_Render;
extern long long g_tickCount;
extern string g_steadyClockLog;
extern string g_frameLogicLog;
extern string g_frameRenderLog;
extern char szScreenBuffer[HEIGHT][WIDTH];


void Buffer_Flip();

void Buffer_Clear();

void Sprite_Draw(int y, int x, char ch);

void Sprite_String(int y, int x, string str);

void UpdateTitle();

void UpdateSelectStage();

void LoadStage();

void UpdateGame();

void UpdateGameOver();

bool KeyPressed(short keyState);

void CalcTime(const steady_clock::time_point& now);

void CommonRender();
