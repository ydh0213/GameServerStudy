#pragma comment(lib, "winmm.lib")
#include <Windows.h>

#include <cstdio>

#include <chrono>
#include <thread>
#include <format>

#include "Console.h"
#include "game.h"

using namespace std;
using namespace std::chrono;


int main()
{
	cs_Initial();

	TIMECAPS timecaps;
	timeGetDevCaps(&timecaps, sizeof(timecaps));
	timeBeginPeriod(timecaps.wPeriodMin);

	for (; g_gameScene != Scene::EXIT; ++g_tickCount)
	{
		switch (g_gameScene)
		{
		case Scene::TITLE:
			UpdateTitle();
			break;
		case Scene::SELECT_STAGE:
			UpdateSelectStage();
			break;
		case Scene::LOAD:
			LoadStage();
			break;
		case Scene::GAME:
			UpdateGame();
			break;
		case Scene::OVER:
			UpdateGameOver();
			break;
		}
	}

	timeEndPeriod(timecaps.wPeriodMin);
}


bool KeyPressed(short keyState)
{
	return keyState & 0x8000;
}


void CalcTime(const steady_clock::time_point& now)
{
	if (now - g_lastTp >= seconds(1))
	{
		g_frame_Logic = g_frameCount_Logic;
		g_frameCount_Logic = 0;

		g_frame_Render = g_frameCount_Render;
		g_frameCount_Render = 0;

		g_lastTp = now;
	}

	long long duration = duration_cast<milliseconds>(now - TP_START).count();
	g_steadyClockLog = format("Steady Clock: {0}.{1}", duration / 1000, duration % 1000);
	g_frameLogicLog = format(" Logic Frame: {}", g_frame_Logic);
	g_frameRenderLog = format("Render Frame: {}", g_frame_Render);
}


void CommonRender()
{
	Sprite_String(HEIGHT - 3, 0, g_steadyClockLog);
	Sprite_String(HEIGHT - 2, 0, g_frameLogicLog);
	Sprite_String(HEIGHT - 1, 0, g_frameRenderLog);

	Buffer_Flip();

	++g_frameCount_Render;

	this_thread::sleep_until(g_tpNextGoal);

	g_tpNextGoal += DURATION;
}


void UpdateGameOver()
{
	// 입력처리
	short keyStateEnter = GetAsyncKeyState(VK_RETURN);
	short keyStateSpace = GetAsyncKeyState(VK_SPACE);

	if (KeyPressed(keyStateEnter))
		g_gameScene = Scene::EXIT;

	if (KeyPressed(keyStateSpace))
		g_gameScene = Scene::SELECT_STAGE;

	// GameOver씬은 로직 없음
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

	Sprite_String(10, 16, "G A M E   O V E R");
	Sprite_String(12, 10, "Press Space Bar to replay.");
	Sprite_String(14, 10, "Press Enter to exit.");

	CommonRender();
}


void Buffer_Flip()
{
	for (int i = 0; i < HEIGHT; i++)
	{
		cs_MoveCursor(i, 0);
		printf(szScreenBuffer[i]);
	}
}


void Buffer_Clear()
{
	memset(szScreenBuffer, ' ', HEIGHT * WIDTH);

	for (int i = 0; i < HEIGHT; i++)
		szScreenBuffer[i][WIDTH - 1] = '\0';
}


void Sprite_Draw(int y, int x, char ch)
{
	if (x < 0 || y < 0 || x >= WIDTH - 1 || y >= HEIGHT)
		return;

	szScreenBuffer[y][x] = ch;
}


void Sprite_String(int y, int x, string str)
{
	for (int i = 0; i < str.length(); ++i)
		Sprite_Draw(y, x + i, str[i]);
}
