#include <windows.h>
#include <string>
#include <sstream>

#include "binary_search_tree.h"
#include "main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const char CLASS_NAME[] = "Binary Search Tree Visualizer";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0, CLASS_NAME, "BST Visualizer",
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

	std::stringstream ss(g_inputBuffer);
	char cmd;
	int value;

	ss >> cmd; // 첫 글자 읽기 (I 또는 D)

	// 뒤에 숫자가 있는지 확인
	if (ss >> value)
	{
		if (cmd == 'I' || cmd == 'i')
			g_root = Insert(g_root, value);
		else if (cmd == 'D' || cmd == 'd')
		{
			// 값이 있으면 그 값을 삭제, 없으면 루트 삭제(옵션)
			// 여기서는 "D 58" 처럼 특정 값 삭제로 구현
			g_root = Delete(g_root, value);
		}
	}
	else
	{
		// 숫자가 없을 때 (예: 그냥 "C"만 쳤을 때 - 전체 삭제)
		if (cmd == 'C' || cmd == 'c')
		{
			DeleteTree(g_root);
			g_root = nullptr;
		}
	}

	g_inputBuffer = ""; // 버퍼 비우기
	InvalidateRect(hwnd, NULL, TRUE); // 화면 갱신
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

			// 1. 안내 문구 및 현재 입력 중인 명령어 표시
			SetBkMode(hdc, TRANSPARENT);

			// 사용법 안내
			TextOut(hdc, 10, 10, "Commands: [I 34] Insert 34 / [D 58] Delete 58 / [C] Clear", 57);

			// 현재 입력 중인 텍스트 (강조)
			std::string displayCmd = "Current Input: " + g_inputBuffer;
			// 커서 깜빡임 효과 (_)
			displayCmd += "_";

			SetTextColor(hdc, RGB(0, 0, 255)); // 파란색 글씨
			TextOut(hdc, 10, 30, displayCmd.c_str(), (int)displayCmd.length());
			SetTextColor(hdc, RGB(0, 0, 0)); // 검은색 복구

			// 2. 트리 그리기
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

	// [핵심] 키보드 문자 입력 처리
	case WM_CHAR:
		{
			// wParam: 입력된 키의 문자 코드 (ASCII)
			char ch = (char)wParam;

			if (ch == VK_RETURN)
			{
				// 엔터 키 (Enter)
				ProcessCommand(hwnd);
			}
			else if (ch == VK_BACK)
			{
				// 백스페이스 (지우기)
				if (!g_inputBuffer.empty())
					g_inputBuffer.pop_back();
			}
			else if (ch == VK_ESCAPE)
			{
				// ESC 키 (취소)
				g_inputBuffer = "";
			}
			else
			{
				// 일반 문자 (숫자, 알파벳, 공백 등)
				// 제어 문자가 아닌 경우에만 추가
				if (ch >= 32 && ch <= 126)
					g_inputBuffer += ch;
			}

			// 입력할 때마다 화면을 다시 그려서 글자가 보이게 함
			InvalidateRect(hwnd, NULL, TRUE);
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
