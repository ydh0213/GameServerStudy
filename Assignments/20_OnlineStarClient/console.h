#pragma once

constexpr int DEFAULT_COLOR = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
constexpr int WIDTH = 80;
constexpr int HEIGHT = 23;

extern HANDLE hConsoleOutput; // 콘솔 출력 핸들

// 콘솔 버퍼 (화면에 출력할 내용이 담기는 메모리)
extern CHAR_INFO consoleBuffer[WIDTH * HEIGHT];

/**
 * @brief 콘솔 초기화 및 버퍼 설정
 */
void cs_Initial();

/**
 * @brief 버퍼에 문자 및 속성을 설정
 * @param y 설정할 행 (0 ~ HEIGHT-1)
 * @param x 설정할 열 (0 ~ WIDTH-1)
 * @param character 설정할 문자
 * @param attribute 설정할 색상/속성 (WINAPI 색상 상수 사용)
 */
void cs_SetBuffer(int y, int x, WCHAR character, WORD attribute = DEFAULT_COLOR);

/**
 * @brief 버퍼에 문자열을 설정 (편의 함수)
 */
void cs_SetString(int y, int x, const char* str, WORD attribute = DEFAULT_COLOR);

/*
 * 버퍼 지우기
 */
void cs_Clear();

/**
 * @brief 버퍼 내용을 콘솔 화면에 출력 (실제 렌더링 함수)
 */
void cs_Render();