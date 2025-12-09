#pragma once

#include "resource.h"
#include "RingBuffer.h"

#define MAX_LOADSTRING 100

constexpr int WM_SOCKET = WM_USER + 1;
constexpr int WM_SHOW_CONNECT_DIALOG = WM_USER + 2;
constexpr int SERVER_PORT = 25000;

HINSTANCE hInst; // current instance
WCHAR szTitle[MAX_LOADSTRING]; // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

void DrawLine(HWND hWnd, int sx, int sy, int ex, int ey);
void ConnectToServer(HWND hWnd);
void FlushSendBuffer();
void EnqueuePacket(int sx, int sy, int ex, int ey);

BOOL g_bClick;
HPEN g_hPen;
int g_iOldX, g_iOldY;
HBITMAP g_hBitmap, g_hOldBitmap;
HDC g_hMemDC;
RECT g_MemDCRect;

SOCKET g_hSocket = INVALID_SOCKET;
char g_szIP[32] = "127.0.0.1";

RingBuffer* g_pRecvRB = nullptr;
RingBuffer* g_pSendRB = nullptr;
