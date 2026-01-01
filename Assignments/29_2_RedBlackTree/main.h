#pragma once

constexpr int WIDTH = 1800;
constexpr int HEIGHT = 1000;

Node nil(0, BLACK, nullptr, nullptr, nullptr);
Node* g_root = &nil;

string g_inputBuffer = "";

void ProcessCommand(HWND hwnd);

void ShiftTree(Node* node, int shiftX);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void RunStressTest(HWND hwnd, int iterations);