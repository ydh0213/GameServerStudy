#include <windows.h>
#include <stdio.h>
#include "Console.h"


HANDLE hConsole;


// 콘솔 제어를 위한 준비 작업
void cs_Initial()
{
	// 1. 콘솔 화면 (standard output) 핸들을 구합니다.
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// 2. 커서 정보 설정 (커서 숨기기)
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	// 3. 스크린 버퍼 크기 설정
	COORD bufferSize = {WIDTH, HEIGHT};
	SetConsoleScreenBufferSize(hConsole, bufferSize);

	// 4. 콘솔 창 크기 설정
	SMALL_RECT windowSize = {0, 0, WIDTH - 1, HEIGHT - 1};
	SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
}


// 콘솔 화면의 커서를 y, x 좌표로 이동
void cs_MoveCursor(int y, int x)
{
	SetConsoleCursorPosition(hConsole, {static_cast<SHORT>(x), static_cast<SHORT>(y)});
}


// 콘솔 화면을 초기화
void cs_ClearScreen()
{
	FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', HEIGHT * WIDTH, {0, 0}, nullptr);
}
