#include <windows.h>
#include <cstdio>
#include "Console.h"

void cs_Initial()
{
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	// 커서 숨기기 (선택 사항이지만, 보통 버퍼 기반 출력 시 사용)
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsoleOutput, &cursorInfo);

	COORD bufferSize = { WIDTH, HEIGHT };
	SetConsoleScreenBufferSize(hConsoleOutput, bufferSize);

	SMALL_RECT windowSize = { 0, 0, WIDTH - 1, HEIGHT - 1 };
	SetConsoleWindowInfo(hConsoleOutput, TRUE, &windowSize);

	cs_Clear();
}

void cs_SetBuffer(int y, int x, WCHAR character, WORD attribute)
{
	if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT)
	{
		int index = y * WIDTH + x;
		consoleBuffer[index].Char.UnicodeChar = character;
		consoleBuffer[index].Attributes = attribute;
	}
}

void cs_SetString(int y, int x, const char* str, WORD attribute)
{
	int len = strlen(str);
	for (int i = 0; i < len; ++i)
	{
		// 윈도우 콘솔은 기본적으로 와이드 문자(WCHAR)를 사용하지만, 
		// 간단한 ASCII 문자열은 char 타입으로 받아서 WCHAR로 변환하여 사용합니다.
		cs_SetBuffer(y, x + i, (WCHAR)str[i], attribute);
	}
}

void cs_Clear()
{
	for (int i = 0; i < WIDTH * HEIGHT; ++i)
	{
		consoleBuffer[i].Char.UnicodeChar = ' ';
		consoleBuffer[i].Attributes = DEFAULT_COLOR;
	}
}

void cs_Render()
{
	COORD bufferSize = { WIDTH, HEIGHT };
	COORD bufferCoord = { 0, 0 };
	SMALL_RECT writeRegion = { 0, 0, WIDTH - 1, HEIGHT - 1 };

	WriteConsoleOutput(
		hConsoleOutput,
		consoleBuffer,
		bufferSize,
		bufferCoord,
		&writeRegion
	);
}