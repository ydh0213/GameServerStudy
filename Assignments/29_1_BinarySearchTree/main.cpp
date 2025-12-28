#include <windows.h>
#include <ctime>
#include <algorithm>

#include "binary_search_tree.h"
#include "main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    srand((unsigned int)time(NULL)); // 난수 초기화

    const char CLASS_NAME[] = "BinaryTreeVisualizer";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "BST Visualizer - C++ Win32 API",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 배경 지우기
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));

        // 사용법 안내 텍스트
        TextOutA(hdc, 10, 10, "Instructions:", 13);
        TextOutA(hdc, 10, 30, "[I] Insert Random Number", 24);
        TextOutA(hdc, 10, 50, "[D] Delete Root Node", 20);
        TextOutA(hdc, 10, 70, "[C] Clear Tree", 14);

        // 트리 그리기 시작 (화면 중앙 상단에서 시작)
        if (g_root) {
            DrawTree(hdc, g_root, rect.right / 2, 120, rect.right / 4);
        }

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_KEYDOWN: {
        switch (wParam) {
        case 'I': // Insert
        case 'i':
            g_root = Insert(g_root, rand() % 100); // 0~99 난수
            InvalidateRect(hwnd, NULL, TRUE); // 화면 다시 그리기 요청
            break;
        case 'D': // Delete
        case 'd':
            if (g_root) {
                g_root = Delete(g_root, g_root->data);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case 'C': // Clear
        case 'c':
            DeleteTree(g_root);
            g_root = nullptr;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
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