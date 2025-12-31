#pragma once

constexpr int WIDTH = 1800;
constexpr int HEIGHT = 1000;

Node* g_root = nullptr;

std::string g_inputBuffer = ""; // 사용자가 현재 치고 있는 명령어를 저장

void ProcessCommand(HWND hwnd);

void ShiftTree(Node* node, int shiftX);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
