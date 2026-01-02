#include <windows.h>
#include <string>
#include <vector>
#include <queue>

using namespace std;

#include "AStar.h"
#include "PathFinder.h"

PathFinder g_pf;
bool g_isAutoRun = false;
bool g_isLeftDown = false;
int g_prevGridX = -1;
int g_prevGridY = -1;

POINT g_mousePos = {0, 0};

HBITMAP g_hDoubleBufferImage = nullptr;

const COLORREF COL_BG = RGB(255, 255, 255);
const COLORREF COL_WALL = RGB(40, 40, 40);
const COLORREF COL_START = RGB(50, 205, 50);
const COLORREF COL_END = RGB(220, 20, 60);
const COLORREF COL_OPEN = RGB(135, 206, 250);
const COLORREF COL_CLOSED = RGB(255, 250, 205);
const COLORREF COL_PATH = RGB(220, 170, 220);
const COLORREF COL_GRID = RGB(220, 220, 220);
const COLORREF COL_TEXT = RGB(0, 0, 0);
const COLORREF COL_PARENT_LINE = RGB(0, 0, 0);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Draw(HDC hdc);
void DrawHelpText(HDC hdc);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
	WNDCLASSEX wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = "AStarClass";

	RegisterClassEx(&wcex);

	RECT rc = {0, 0, WIDTH, HEIGHT};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindow("AStarClass", "A* Visualizer",
	                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
	                         rc.right - rc.left, rc.bottom - rc.top,
	                         NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (true)
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (g_isAutoRun && g_pf.IsSearching())
		{
			g_pf.Step();
			InvalidateRect(hWnd, nullptr, FALSE);
		}

	return (int)msg.wParam;
}

void Draw(HDC hdc)
{
	HBRUSH hBrEmpty = CreateSolidBrush(COL_BG);
	HBRUSH hBrWall = CreateSolidBrush(COL_WALL);
	HBRUSH hBrStart = CreateSolidBrush(COL_START);
	HBRUSH hBrEnd = CreateSolidBrush(COL_END);
	HBRUSH hBrOpen = CreateSolidBrush(COL_OPEN);
	HBRUSH hBrClosed = CreateSolidBrush(COL_CLOSED);
	HBRUSH hBrPath = CreateSolidBrush(COL_PATH);
	HPEN hPenGrid = CreatePen(PS_SOLID, 1, COL_GRID);
	HPEN hPenParent = CreatePen(PS_SOLID, 1, COL_PARENT_LINE);

	HPEN hOldPen = (HPEN)SelectObject(hdc, hPenGrid);

	for (int y = 0; y < CELL_H; ++y)
		for (int x = 0; x < CELL_W; ++x)
		{
			node* n = g_pf.GetNode(x, y);
			HBRUSH hTargetBrush = hBrEmpty;

			if (n->type == WALL) hTargetBrush = hBrWall;
			else if (n->type == START) hTargetBrush = hBrStart;
			else if (n->type == END) hTargetBrush = hBrEnd;
			else if (n->isPath) hTargetBrush = hBrPath;
			else if (n->isClosed) hTargetBrush = hBrClosed;
			else if (n->isOpen) hTargetBrush = hBrOpen;

			SelectObject(hdc, hTargetBrush);
			Rectangle(hdc, x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE);

			if (n->parent)
			{
				SelectObject(hdc, hPenParent);

				int currentCenterX = x * CELL_SIZE + CELL_SIZE / 2;
				int currentCenterY = y * CELL_SIZE + CELL_SIZE / 2;
				int dx = n->parent->x - n->x;
				int dy = n->parent->y - n->y;
				int endX = currentCenterX + CELL_SIZE / 2 * dx;
				int endY = currentCenterY + CELL_SIZE / 2 * dy;

				MoveToEx(hdc, currentCenterX, currentCenterY, nullptr);
				LineTo(hdc, endX, endY);

				SelectObject(hdc, hPenGrid);
			}
		}

	SelectObject(hdc, hOldPen);
	DeleteObject(hBrEmpty);
	DeleteObject(hBrWall);
	DeleteObject(hBrStart);
	DeleteObject(hBrEnd);
	DeleteObject(hBrOpen);
	DeleteObject(hBrClosed);
	DeleteObject(hBrPath);
	DeleteObject(hPenGrid);

	DrawHelpText(hdc);
}

void DrawHelpText(HDC hdc)
{
	int oldBkMode = SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, COL_TEXT);

	// string text =
	// 	"[Controls]\n"
	// 	"Key '1' : Set START (at Mouse)\n"
	// 	"Key '2' : Set END (at Mouse)\n"
	// 	"L-Click : Toggle WALL\n"
	// 	"Space   : Step-by-Step Search\n"
	// 	"Enter   : Auto Search (On/Off)\n"
	// 	"Key 'R' : Reset Search (Keep Walls)\n"
	// 	"Key 'C' : Clear All";

	HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
	                         DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	RECT rcText = {10, 10, 400, 300};

	DrawText(
		hdc,
		"1 : 시작점 설정\n2 : 도착점 설정\n좌클릭 : 벽 설정 (Toggle)\nR : 탐색 경로 지우기 (Reset)\nC : 전체 지우기 (Clear)\nSpace : 한 단계 진행\nEnter : 자동 진행",
		-1, &rcText, DT_LEFT | DT_TOP);

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
	SetBkMode(hdc, oldBkMode);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		g_pf.ResetSearch();
		break;

	case WM_SIZE:
		{
			HDC hdc = GetDC(hWnd);
			if (g_hDoubleBufferImage) DeleteObject(g_hDoubleBufferImage);

			RECT rc;
			GetClientRect(hWnd, &rc);

			g_hDoubleBufferImage = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
			ReleaseDC(hWnd, hdc);
		}
		break;

	case WM_MOUSEMOVE:
		{
			g_mousePos.x = LOWORD(lParam);
			g_mousePos.y = HIWORD(lParam);

			if (g_isLeftDown)
			{
				int x = g_mousePos.x / CELL_SIZE;
				int y = g_mousePos.y / CELL_SIZE;

				if (x != g_prevGridX || y != g_prevGridY)
				{
					g_pf.ToggleWall(x, y);
					g_prevGridX = x;
					g_prevGridY = y;
					InvalidateRect(hWnd, nullptr, FALSE);
				}
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			g_isLeftDown = true;
			int x = LOWORD(lParam) / CELL_SIZE;
			int y = HIWORD(lParam) / CELL_SIZE;

			g_pf.ToggleWall(x, y);

			g_prevGridX = x;
			g_prevGridY = y;

			InvalidateRect(hWnd, nullptr, FALSE);
		}
		break;

	case WM_LBUTTONUP:
		g_isLeftDown = false;
		g_prevGridX = -1;
		g_prevGridY = -1;
		break;

	case WM_KEYDOWN:
		{
			int mx = g_mousePos.x / CELL_SIZE;
			int my = g_mousePos.y / CELL_SIZE;

			switch (wParam)
			{
			case '1':
				g_pf.SetStart(mx, my);
				g_pf.ResetSearch();
				InvalidateRect(hWnd, nullptr, FALSE);
				break;

			case '2':
				g_pf.SetEnd(mx, my);
				g_pf.ResetSearch();
				InvalidateRect(hWnd, nullptr, FALSE);
				break;

			case VK_SPACE:
				g_isAutoRun = false;
				g_pf.Step();
				InvalidateRect(hWnd, nullptr, FALSE);
				break;

			case VK_RETURN:
				g_isAutoRun = !g_isAutoRun;
				break;

			case 'R':
				g_isAutoRun = false;
				g_pf.ResetSearch();
				InvalidateRect(hWnd, nullptr, FALSE);
				break;

			case 'C':
				g_isAutoRun = false;
				g_pf.ClearWalls();
				InvalidateRect(hWnd, nullptr, FALSE);
				break;
			}
		}
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			HDC hMemDC = CreateCompatibleDC(hdc);
			HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, g_hDoubleBufferImage);

			RECT rc = {0, 0, WIDTH, HEIGHT};
			FillRect(hMemDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

			Draw(hMemDC);

			BitBlt(hdc, 0, 0, WIDTH, HEIGHT, hMemDC, 0, 0, SRCCOPY);

			SelectObject(hMemDC, hOldBmp);
			DeleteDC(hMemDC);

			EndPaint(hWnd, &ps);
		}
		break;

	case WM_DESTROY:
		if (g_hDoubleBufferImage) DeleteObject(g_hDoubleBufferImage);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
