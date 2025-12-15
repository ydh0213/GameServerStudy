#include <windows.h>
#include <cstdio>
#include "Console.h"

void cs_Initial()
{
	// 1. 콘솔 화면 (standard output) 핸들을 구합니다.
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	// 2. 커서 숨기기 (선택 사항이지만, 보통 버퍼 기반 출력 시 사용)
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsoleOutput, &cursorInfo);

	// 3. 스크린 버퍼 크기 설정 (화면 크기와 동일하게 설정)
	COORD bufferSize = { WIDTH, HEIGHT };
	SetConsoleScreenBufferSize(hConsoleOutput, bufferSize);

	// 4. 콘솔 창 크기 설정
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
	COORD bufferCoord = { 0, 0 }; // 버퍼의 시작점 (항상 0, 0)
	SMALL_RECT writeRegion = { 0, 0, WIDTH - 1, HEIGHT - 1 }; // 화면 전체 영역

	// WriteConsoleOutput 함수를 호출하여 메모리 버퍼의 내용을 화면에 한 번에 출력
	WriteConsoleOutput(
		hConsoleOutput, // 콘솔 출력 핸들
		consoleBuffer, // 출력할 데이터 (CHAR_INFO 배열)
		bufferSize, // 버퍼의 크기 (WIDTH, HEIGHT)
		bufferCoord, // 버퍼의 좌상단 시작점 (0, 0)
		&writeRegion // 실제 출력할 콘솔 화면 영역
	);
}