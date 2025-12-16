#pragma comment(lib, "winmm.lib")
#include <Windows.h>

#include <cstdio>

#include <chrono>
#include <format>
#include <fstream>
#include <vector>
#include <sstream>

#include "game.h"
#include "Console.h"

using namespace std;
using namespace std::chrono;


Scene g_gameScene = Scene::TITLE;

Player g_player;
int g_playerHp = 5;
PlayerBullet g_playerBullets[PLAYER_BULLET_MAX];

map<int, EnemyType> g_enemyTypes;
map<string, MovePattern> g_movePattern;
Enemy g_enemy[ENEMY_MAX];
EnemyBullet g_enemyBullet[ENEMY_BULLET_MAX];

vector<string> g_stageName;
int g_selectStageCursor = 0;

steady_clock::time_point g_lastTp = TP_START;
steady_clock::time_point g_tpNextGoal = TP_START + DURATION; // 다음 목표 시각
int g_frameCount_Logic;
int g_frameCount_Render;
int g_frame_Logic;
int g_frame_Render;
long long g_tickCount;
string g_steadyClockLog;
string g_frameLogicLog;
string g_frameRenderLog;
char szScreenBuffer[HEIGHT][WIDTH];


void UpdateTitle()
{
	// 입력처리
	short keyStateSpace = GetAsyncKeyState(VK_SPACE);

	if (KeyPressed(keyStateSpace))
		g_gameScene = Scene::SELECT_STAGE;

	// Title씬은 로직 없음
	++g_frameCount_Logic;

	// 시간 계산
	time_point now = steady_clock::now();
	CalcTime(now);

	if (g_tpNextGoal < now) // 목표 예정 시간을 초과했으므로 렌더링 포기
	{
		g_tpNextGoal += DURATION;
		return;
	}

	// 렌더링
	Buffer_Clear();

	Sprite_Draw(0, 0, '*');
	Sprite_Draw(0, WIDTH - 2, '*');
	Sprite_Draw(HEIGHT - 4, 0, '*');
	Sprite_Draw(HEIGHT - 4, WIDTH - 2, '*');
	Sprite_String(6, 15, "made by Daehyun Yoon");
	Sprite_String(8, 28, "2025.09");

	if (duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000 < 650)
		Sprite_String(12, 10, "Press Space Bar to play game.");

	CommonRender();
}


void UpdateSelectStage()
{
	// 입력처리
	short keyStateEscape = GetAsyncKeyState(VK_ESCAPE);
	short keyStateEnter = GetAsyncKeyState(VK_RETURN);
	short keyStateUp = GetAsyncKeyState(VK_UP);
	short keyStateDown = GetAsyncKeyState(VK_DOWN);

	if (KeyPressed(keyStateEnter))
		g_gameScene = Scene::LOAD;

	if (KeyPressed(keyStateEscape))
		g_gameScene = Scene::OVER;

	if (KeyPressed(keyStateUp) && g_selectStageCursor > 0)
		--g_selectStageCursor;

	if (KeyPressed(keyStateDown) && g_selectStageCursor + 1 < g_stageName.size())
		++g_selectStageCursor;

	// 로직
	if (g_stageName.empty())
	{
		ifstream stageInfoFile(STAGEINFO_FILENAME);

		if (!stageInfoFile.is_open())
			throw runtime_error("파일 열기 실패: " + STAGEINFO_FILENAME);

		string fileName;
		while (getline(stageInfoFile, fileName))
			g_stageName.emplace_back(fileName);

		stageInfoFile.close();
	}

	++g_frameCount_Logic;

	// 시간 계산
	time_point now = steady_clock::now();
	CalcTime(now);

	if (g_tpNextGoal < now) // 목표 예정 시간을 초과했으므로 렌더링 포기
	{
		g_tpNextGoal += DURATION;
		return;
	}

	// 렌더링
	Buffer_Clear();

	for (int i = 0; i < g_stageName.size(); ++i)
		Sprite_String(4 + 2 * i, 4, g_stageName[i]);

	Sprite_String(4 + 2 * g_stageName.size(), 4, "Press Enter to select stage.");

	if (duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000 < 650)
		Sprite_Draw(4 + 2 * g_selectStageCursor, 2, '>');

	Sprite_String(HEIGHT - 1, WIDTH - 25, "Press ESC to end game.");

	CommonRender();
}


void LoadEnemyInfo()
{
	ifstream enemyInfoFile(ENEMYINFO_FILENAME);

	if (!enemyInfoFile.is_open())
		throw runtime_error("파일 열기 실패: " + ENEMYINFO_FILENAME);

	int i = 0;
	string strLine;

	while (getline(enemyInfoFile, strLine))
		if (i++ > 0) // 첫 줄은 주석, 둘째 줄부터 읽을 data
		{
			stringstream ss(strLine);
			int num;
			char shape;
			int hp;
			int speed;
			string movePattern;

			ss >> num >> shape >> hp >> speed >> movePattern;

			g_enemyTypes[num] = {shape, hp, speed, movePattern};
		}
}


void LoadMovePattern()
{
	for (const auto& [key, value] : g_enemyTypes)
	{
		const auto& [shape, hp, speed, movePattern] = value;

		if (!g_movePattern.contains(movePattern))
		{
			ifstream movePatternFile(movePattern);

			if (!movePatternFile.is_open())
				throw runtime_error("파일 열기 실패: " + movePattern);

			vector<pair<int, int>> coord;

			string strLine;
			while (getline(movePatternFile, strLine))
			{
				stringstream ss(strLine);
				int dy, dx;

				ss >> dy >> dx;
				coord.emplace_back(dy, dx);
			}

			g_movePattern[movePattern] = {coord};
		}
	}
}


void LoadStage()
{
	for (int i = 0; i < ENEMY_MAX; ++i)
		g_enemy[i].y = -1;

	for (int i = 0; i < ENEMY_BULLET_MAX; ++i)
		g_enemyBullet[i].y = HEIGHT - 3;

	g_player.y = 13;
	g_player.x = WIDTH / 2;
	g_playerHp = 5;

	ifstream stageFile(g_stageName[g_selectStageCursor]);

	if (!stageFile.is_open())
		throw runtime_error("파일 열기 실패: " + g_stageName[g_selectStageCursor]);

	if (g_enemyTypes.empty())
		LoadEnemyInfo();

	if (g_movePattern.empty())
		LoadMovePattern();

	int i = 0;
	vector<string> mapBuffer(HEIGHT);
	while (getline(stageFile, mapBuffer[i]))
	{
		for (int j = 0; j < mapBuffer[i].length(); ++j)
			if (mapBuffer[i][j] != ' ')
				for (int k = 0; k < ENEMY_MAX; ++k)
					if (g_enemy[k].y < 0)
					{
						const auto& [shape, hp, speed, movePattern] = g_enemyTypes[mapBuffer[i][j] - '0'];
						g_enemy[k].shape = shape;
						g_enemy[k].y = i;
						g_enemy[k].x = j;
						g_enemy[k].hp = hp;
						g_enemy[k].speed = speed;
						g_enemy[k].movePattern = movePattern;
						break;
					}

		++i;
	}

	stageFile.close();

	++g_frameCount_Logic;

	g_gameScene = Scene::GAME;

	// 시간 계산
	time_point now = steady_clock::now();
	CalcTime(now);

	if (g_tpNextGoal < now) // 목표 예정 시간을 초과했으므로 렌더링 포기
	{
		g_tpNextGoal += DURATION;
		return;
	}

	// 렌더링
	Buffer_Clear();

	CommonRender();
}


bool enemyAllDead()
{
	for (int i = 0; i < ENEMY_MAX; ++i)
		if (g_enemy[i].y >= 0)
			return false;

	return true;
}


void UpdateGame()
{
	// 입력처리
	short keyStateEscape = GetAsyncKeyState(VK_ESCAPE);
	short keyStateUp = GetAsyncKeyState(VK_UP);
	short keyStateDown = GetAsyncKeyState(VK_DOWN);
	short keyStateLeft = GetAsyncKeyState(VK_LEFT);
	short keyStateRight = GetAsyncKeyState(VK_RIGHT);
	short keyStateSpace = GetAsyncKeyState(VK_SPACE);

	if (KeyPressed(keyStateEscape))
		g_gameScene = Scene::OVER;

	if (KeyPressed(keyStateUp) && g_player.y > 0) --g_player.y;

	if (KeyPressed(keyStateDown) && g_player.y + 1 < HEIGHT - 3) ++g_player.y;

	if (KeyPressed(keyStateLeft) && g_player.x > 0) --g_player.x;

	if (KeyPressed(keyStateRight) && g_player.x + 1 < WIDTH - 1) ++g_player.x;

	if (KeyPressed(keyStateSpace)) // player가 bullet 발사
		for (int i = 0; i < PLAYER_BULLET_MAX; ++i)
			if (g_playerBullets[i].y < 0)
			{
				g_playerBullets[i].y = g_player.y;
				g_playerBullets[i].x = g_player.x;
				break;
			}

	// Todo: 로직
	for (int i = 0; i < PLAYER_BULLET_MAX; ++i)
	{
		for (int j = 0; j < ENEMY_MAX; ++j)
			if (g_playerBullets[i].y == g_enemy[j].y && g_playerBullets[i].x == g_enemy[j].x) // player bullet과 enemy 충돌
			{
				g_playerBullets[i].y = -1;

				if (--g_enemy[j].hp == 0) // enemy 사망
				{
					g_enemy[j].y = -1;

					if (enemyAllDead())
						g_gameScene = Scene::OVER;
				}
			}

		if (g_playerBullets[i].y >= 0)
			--g_playerBullets[i].y;
	}

	for (int i = 0; i < ENEMY_BULLET_MAX; ++i)
	{
		if (g_enemyBullet[i].y == g_player.y && g_enemyBullet[i].x == g_player.x) // enemy bullet과 player 충돌
		{
			g_enemyBullet[i].y = HEIGHT - 3;

			if (--g_playerHp == 0) // player 사망
				g_gameScene = Scene::OVER;
		}

		if (g_enemyBullet[i].y < HEIGHT - 3)
			++g_enemyBullet[i].y;
	}

	for (int i = 0; i < ENEMY_MAX; ++i) // enemy가 움직임
		if (g_enemy[i].y >= 0)
		{
			auto coords = g_movePattern[g_enemy[i].movePattern].coord;
			int ratio = FPS / g_enemy[i].speed;

			if (g_tickCount % ratio == 0)
			{
				auto coord = coords[g_tickCount / ratio % coords.size()];
				g_enemy[i].y += coord.first;
				g_enemy[i].x += coord.second;

				for (int j = 0; j < ENEMY_BULLET_MAX; ++j) // enemy가 bullet 발사
				{
					if (g_enemyBullet[j].y == HEIGHT - 3)
					{
						g_enemyBullet[j].y = g_enemy[i].y;
						g_enemyBullet[j].x = g_enemy[i].x;
						break;
					}
				}
			}
		}

	++g_frameCount_Logic;

	// 시간 계산
	time_point now = steady_clock::now();
	CalcTime(now);

	if (g_tpNextGoal < now) // 목표 예정 시간을 초과했으므로 렌더링 포기
	{
		g_tpNextGoal += DURATION;
		return;
	}

	// 렌더링
	Buffer_Clear();


	for (int i = 0; i < ENEMY_MAX; ++i)
		if (g_enemy[i].y >= 0)
			Sprite_Draw(g_enemy[i].y, g_enemy[i].x, g_enemy[i].shape);

	for (int i = 0; i < ENEMY_BULLET_MAX; ++i)
		if (g_enemyBullet[i].y < HEIGHT - 3)
			Sprite_Draw(g_enemyBullet[i].y, g_enemyBullet[i].x, 'x');

	for (int i = 0; i < PLAYER_BULLET_MAX; ++i)
		if (g_playerBullets[i].y >= 0)
			Sprite_Draw(g_playerBullets[i].y, g_playerBullets[i].x, 'O');

	Sprite_Draw(g_player.y, g_player.x, '@');
	Sprite_String(HEIGHT - 1, WIDTH - 25, "Press ESC to end game.");

	CommonRender();
}
