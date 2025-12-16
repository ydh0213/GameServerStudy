// Windows 의 콘솔 화면에서 커서 제어
#ifndef __CONSOLE__
#define __CONSOLE__


constexpr int WIDTH = 121; // 콘솔 가로 마지막은 NULL
constexpr int HEIGHT = 25; // 콘솔 세로


// 콘솔 제어를 위한 준비 작업
void cs_Initial();


// 콘솔 화면의 커서를 y, x 좌표로 이동
void cs_MoveCursor(int y, int x);


// 콘솔 화면을 초기화
void cs_ClearScreen();


#endif
