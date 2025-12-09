#include <windows.h>
#include <windowsx.h>
#include <cstdio>

#include "framework.h"
#include "packetProtocol.h"
#include "23_NetDrawClient.h"
#include "RingBuffer.h"

#pragma comment(lib, "ws2_32.lib")


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		MessageBox(NULL, L"WSAStartup 실패!", L"에러", MB_OK);
		return 0;
	}

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MY23NETDRAWCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		WSACleanup();
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY23NETDRAWCLIENT));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WSACleanup();

	return (int)msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY23NETDRAWCLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY23NETDRAWCLIENT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

void DrawLine(HWND hWnd, int sx, int sy, int ex, int ey)
{
	HDC hdc = GetDC(hWnd);
	SelectObject(hdc, g_hPen);
	MoveToEx(hdc, sx, sy, nullptr);
	LineTo(hdc, ex, ey);

	SelectObject(g_hMemDC, g_hPen);
	MoveToEx(g_hMemDC, sx, sy, nullptr);
	LineTo(g_hMemDC, ex, ey);
	ReleaseDC(hWnd, hdc);
}

void ConnectToServer(HWND hWnd)
{
	if (g_hSocket != INVALID_SOCKET) closesocket(g_hSocket);

	g_hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_hSocket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		wchar_t szBuf[100];
		wsprintf(szBuf, L"소켓 생성 실패! Error: %d", err);
		MessageBox(hWnd, szBuf, L"에러", MB_OK);
		return;
	}

	WSAAsyncSelect(g_hSocket, hWnd, WM_SOCKET, FD_CONNECT | FD_READ | FD_CLOSE);

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(g_szIP);

	int result = connect(g_hSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			wchar_t szErrMsg[100];
			swprintf_s(szErrMsg, L"접속 시도 실패 (Error: %d)", error);
			MessageBox(hWnd, szErrMsg, L"알림", MB_OK | MB_ICONERROR);

			closesocket(g_hSocket);
			g_hSocket = INVALID_SOCKET;
		}
	}
}

void FlushSendBuffer()
{
	if (g_hSocket == INVALID_SOCKET) return;
	if (g_pSendRB == nullptr) return;

	// 보낼 데이터가 있는 동안 계속 시도
	while (g_pSendRB->GetUsedSize() > 0)
	{
		int sent = send(g_hSocket, g_pSendRB->GetFrontBufferPtr(), g_pSendRB->DirectDequeueSize(), 0);

		if (sent == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// 커널 버퍼가 꽉 참. 전송 중단
				// 데이터는 링버퍼에 그대로 두고, 나중에 FD_WRITE가 오면 다시 시도함.
				return;
			}

			// 기타 에러 (연결 끊김 등)
			return;
		}

		g_pSendRB->MoveFront(sent);
	}
}

// 패킷을 만들어 송신 링버퍼에 넣는 함수
void EnqueuePacket(int sx, int sy, int ex, int ey)
{
	if (g_pSendRB == nullptr) return;

	st_HEADER header;
	header.len = sizeof(st_DRAW_PACKET);

	st_DRAW_PACKET packet;
	packet.startX = sx;
	packet.startY = sy;
	packet.endX = ex;
	packet.endY = ey;

	g_pSendRB->Enqueue((char*)&header, sizeof(st_HEADER));
	g_pSendRB->Enqueue((char*)&packet, sizeof(st_DRAW_PACKET));

	FlushSendBuffer();
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		g_hPen = CreatePen(PS_SOLID, 0, RGB(0, 0, 0));

		HDC hdc = GetDC(hWnd);
		GetClientRect(hWnd, &g_MemDCRect);
		g_hBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
		g_hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);

		g_hOldBitmap = (HBITMAP)SelectObject(g_hMemDC, g_hBitmap);
		PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);

		g_pRecvRB = new RingBuffer(1 << 12);
		g_pSendRB = new RingBuffer(1 << 12);

		ConnectToServer(hWnd);

		// PostMessage(hWnd, WM_SHOW_CONNECT_DIALOG, 0, 0);
	}
	break;
	// case WM_SHOW_CONNECT_DIALOG:
	// 	{
	// 		INT_PTR result = DialogBox(hInst, MAKEINTRESOURCE(IDD_CONNECT), hWnd, ConnectDlg);
	//
	// 		if (result == IDOK)
	// 			ConnectToServer(hWnd);
	// 		else
	// 			DestroyWindow(hWnd);
	// 	}
	// 	break;
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket((SOCKET)wParam);
			return 0;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_CONNECT:
		{
			MessageBox(hWnd, L"서버 접속 성공", L"알림", MB_OK);

			// 연결 되자마자 혹시 큐에 쌓인게 있으면 보냄
			FlushSendBuffer();
		}
		break;
		case FD_READ:
		{
			// 1. 소켓에서 데이터를 임시 버퍼로 읽음
			char szTmpBuf[1 << 12];
			int recvLen = recv((SOCKET)wParam, szTmpBuf, sizeof(szTmpBuf), 0);

			if (recvLen > 0)
			{
				// 2. 읽은 데이터를 링버퍼에 넣음 (Enqueue)
				// 링버퍼가 내부적으로 Rear를 이동시키고 버퍼 끝에 도달하면 앞으로(Circular) 복사해줌
				g_pRecvRB->Enqueue(szTmpBuf, recvLen);

				// 3. 링버퍼에서 패킷 조립 (While Loop)
				while (true)
				{
					// A. 헤더 크기만큼 데이터가 쌓였는지 확인
					if (g_pRecvRB->GetUsedSize() < sizeof(st_HEADER))
						break;

					// B. 헤더를 살펴본다(Peek) - 아직 꺼내진 않음(Dequeue X)
					st_HEADER header;
					g_pRecvRB->Peek((char*)&header, sizeof(st_HEADER));

					// C. 패킷 전체 크기 계산 (헤더 + 바디)
					int packetSize = sizeof(st_HEADER) + header.len;

					// D. 패킷 전체가 완성되었는지 확인
					if (g_pRecvRB->GetUsedSize() < packetSize)
						break; // 아직 데이터가 덜 옴 (다음 FD_READ 대기)

					// E. 완전한 패킷 처리: 여기까지 왔으면 패킷 1개가 온전하다는 뜻.
					// 임시 버퍼에 패킷 전체를 꺼냄
					char packetBuffer[30]; // Header(2) + Body(16) = 18. 넉넉하게 30
					g_pRecvRB->Dequeue(packetBuffer, packetSize);

					st_DRAW_PACKET* pBody = (st_DRAW_PACKET*)(packetBuffer + sizeof(st_HEADER));

					DrawLine(hWnd, pBody->startX, pBody->startY, pBody->endX, pBody->endY);
				}
			}
		}
		break;
		case FD_WRITE:
			FlushSendBuffer();
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wParam);
			g_hSocket = INVALID_SOCKET;
			MessageBox(hWnd, L"서버 연결 종료", L"알림", MB_OK);
			break;
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		g_bClick = true;
		g_iOldX = GET_X_LPARAM(lParam);
		g_iOldY = GET_Y_LPARAM(lParam);
	}
	break;
	case WM_LBUTTONUP:
		g_bClick = false;
		break;
	case WM_MOUSEMOVE:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (g_bClick)
		{
			if (g_hSocket != INVALID_SOCKET)
				EnqueuePacket(g_iOldX, g_iOldY, xPos, yPos);
		}

		g_iOldX = xPos;
		g_iOldY = yPos;
	}
	break;
	// case WM_RBUTTONDOWN:
	// 	ConnectToServer(hWnd);
	// 	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
	{
		DeleteObject(g_hPen);

		if (g_hMemDC)
		{
			SelectObject(g_hMemDC, g_hOldBitmap);
			DeleteDC(g_hMemDC);
		}

		if (g_hBitmap) DeleteObject(g_hBitmap);

		if (g_hSocket != INVALID_SOCKET) closesocket(g_hSocket);

		if (g_pRecvRB)
		{
			delete g_pRecvRB;
			g_pRecvRB = nullptr;
		}

		if (g_pSendRB)
		{
			delete g_pSendRB;
			g_pSendRB = nullptr;
		}

		PostQuitMessage(0);
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
