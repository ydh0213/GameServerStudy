#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <ranges>

using namespace std;

#include "red_black_tree.h"
#include "main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const char CLASS_NAME[] = "Red-Black Tree Visualizer";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0, CLASS_NAME, CLASS_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		NULL, NULL, hInstance, NULL
	);

	if (hwnd == NULL) return 0;

	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

void ProcessCommand(HWND hwnd)
{
	if (g_inputBuffer.empty()) return;

	stringstream ss(g_inputBuffer);
	char cmd;
	int value = 1000;
	string errorMsg;

	ss >> cmd;

	if (toupper(cmd) == 'I')
	{
		if (ss >> value)
		{
			Insert(value);

			if (!ValidateTree(g_root, errorMsg))
				MessageBoxA(hwnd, errorMsg.c_str(), "Insert Failed!", MB_OK | MB_ICONERROR);
		}
	}
	else if (toupper(cmd) == 'D')
	{
		if (ss >> value)
		{
			Delete(value);

			if (!ValidateTree(g_root, errorMsg))
				MessageBoxA(hwnd, errorMsg.c_str(), "Delete Failed!", MB_OK | MB_ICONERROR);
		}
	}
	else if (toupper(cmd) == 'C')
	{
		DeleteTree(g_root);
		g_root = &nil;
	}
	else if (toupper(cmd == 'T'))
	{
		ss >> value;
		RunStressTest(hwnd, value);
		g_inputBuffer = "";
		return;
	}

	g_inputBuffer = "";
	InvalidateRect(hwnd, nullptr, TRUE);
}

void ShiftTree(Node* node, int shiftX)
{
	if (!node) return;

	node->x += shiftX;
	ShiftTree(node->left, shiftX);
	ShiftTree(node->right, shiftX);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			RECT rect;
			GetClientRect(hwnd, &rect);

			SetBkMode(hdc, TRANSPARENT);

			string displayText1 = "Commands: [I 34] Insert 34    [D 58] Delete 58";
			TextOut(hdc, 10, 10, displayText1.c_str(), displayText1.size());

			string displayText2 = "[C] Clear             [T 100] Stress Test X100";
			TextOut(hdc, 90, 30, displayText2.c_str(), displayText2.size());

			string displayCmd = "Input: " + g_inputBuffer + "_";

			SetTextColor(hdc, RGB(0, 0, 255)); // 파란색
			TextOut(hdc, 10, 50, displayCmd.c_str(), displayCmd.length());
			SetTextColor(hdc, RGB(0, 0, 0)); // 검은색 복구

			if (g_root)
			{
				int startOrder = 0;
				CalculatePositions(g_root, 0, startOrder);

				int centerX = rect.right / 2;
				int currentRootX = g_root->x;
				int shift = centerX - currentRootX;

				ShiftTree(g_root, shift);

				DrawTree(hdc, g_root);
			}

			EndPaint(hwnd, &ps);
			break;
		}

	case WM_CHAR:
		{
			char ch = (char)wParam;

			if (ch == VK_RETURN)
				ProcessCommand(hwnd);
			else if (ch == VK_BACK)
			{
				if (!g_inputBuffer.empty())
					g_inputBuffer.pop_back();
			}
			else if (ch == VK_ESCAPE)
				g_inputBuffer = "";
			else if (ch == VK_SPACE || isalnum(ch))
				g_inputBuffer += ch;

			InvalidateRect(hwnd, nullptr, TRUE);
			break;
		}

	case WM_DESTROY:
		DeleteTree(g_root);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

void RunStressTest(HWND hwnd, int iterations)
{
	DeleteTree(g_root);
	g_root = &nil;

	vector<int> shadowData, log;
	string errorMsg;
	char buffer[1 << 10];

	for (int i = 0; i < iterations; ++i)
	{
		int action = rand() % 11;
		int val = 0;

		if (shadowData.empty() || i < 20 || action < 6)
		{
			int tryCount = 0;
			do
			{
				val = rand() % 1000;
				++tryCount;
			}
			while (ranges::find(shadowData, val) != shadowData.end() && tryCount < 10);

			if (ranges::find(shadowData, val) != shadowData.end())
				continue;

			Insert(val);
			shadowData.emplace_back(val);
			log.emplace_back(val);
		}
		else
		{
			int index = rand() % shadowData.size();
			val = shadowData[index];

			Delete(val);

			shadowData[index] = shadowData.back();
			shadowData.pop_back();
			log.emplace_back(-val);
		}

		if (!ValidateTree(g_root, errorMsg))
		{
			string rec = "";
			for (int n : log)
				rec += " " + to_string(n);

			sprintf_s(buffer, "Stress Test Failed!\n\nIteration: %d\nAction: %s\nValue: %d\n\nReason:\n%s\nLog:%s",
			          i + 1, (action == 0 ? "Insert" : "Delete"), val, errorMsg.c_str(), rec.c_str());

			MessageBoxA(hwnd, buffer, "Validation Error", MB_OK | MB_ICONERROR);

			InvalidateRect(hwnd, NULL, TRUE);
			return;
		}
	}

	sprintf_s(buffer, "Success! %d operations verified.\nFinal Tree Size: %d", iterations, shadowData.size());
	MessageBoxA(hwnd, buffer, "Test Complete", MB_OK | MB_ICONINFORMATION);

	InvalidateRect(hwnd, NULL, TRUE);
}
