#pragma once

Node* g_root = nullptr;

std::string g_inputBuffer = ""; // 사용자가 현재 치고 있는 명령어를 저장

void ProcessCommand(HWND hwnd);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
