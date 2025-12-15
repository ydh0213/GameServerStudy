#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm.lib")
#define NOMINMAX

#include <windows.h>
#include <cstdio>
#include <iostream>
#include <chrono>
#include <thread>
#include <format>
#include <string>
#include <list>
#include <limits>

#include "main.h"
#include "console.h"
#include "packetProtocol.h"

using namespace std;
using namespace std::chrono;

const int PACKET_SIZE = 16;
const int FPS = 50;
const milliseconds DURATION = milliseconds(1000 / FPS); // 프레임 간격 시간
const time_point TP_START = steady_clock::now();

HANDLE hConsoleOutput;
CHAR_INFO consoleBuffer[WIDTH * HEIGHT];

struct Star
{
	int id;
	int y;
	int x;
};

SOCKET sock = INVALID_SOCKET;
int errorVal;

char g_recvBuf[160];
int g_recvBuf_used = 0; // 버퍼에 현재 쌓여있는 데이터 크기

steady_clock::time_point g_lastTp;
steady_clock::time_point g_tpNextGoal; // 다음 목표 시각
int g_frameCount_Logic;
int g_frameCount_Render;
int g_frame_Logic;
int g_frame_Render;
char szScreenBuffer[HEIGHT][WIDTH];

list<unique_ptr<Star>> g_stars;
Star* g_myStar = nullptr;
int g_myId = -1;

void RecvPacket();

void ProcessPacket(char* buf);

bool KeyPressed(short keyState);

void CalcTime(const steady_clock::time_point& now);

void UpdateGame();

int main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cerr << "윈속 초기화 실패\n";
		return 1;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "소켓 생성 실패\n";
		WSACleanup();
		return 1;
	}

	char serverIpStr[20];
	cout << "접속할 IP 주소를 입력하세요: ";
	cin >> serverIpStr;

	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cin.clear();

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(serverIpStr);

	if (connect(sock, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		cerr << "서버 접속 실패. error code: " << errorVal << "\n";
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	cs_Initial();

	g_tpNextGoal = steady_clock::now() + DURATION;
	g_lastTp = steady_clock::now();

	TIMECAPS timecaps;
	timeGetDevCaps(&timecaps, sizeof(timecaps));
	timeBeginPeriod(timecaps.wPeriodMin);

	unsigned long nonBlocking = 1;
	if (ioctlsocket(sock, FIONBIO, &nonBlocking) == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		cerr << "소켓 논블로킹 모드 설정 실패. error code: " << errorVal << "\n";
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	for (;;)
	{
		UpdateGame();
	}

	timeEndPeriod(timecaps.wPeriodMin);

	return 0;
}

void RecvPacket()
{
	int remainSize = sizeof(g_recvBuf) - g_recvBuf_used;
	if (remainSize < 0) return; // 심각한 오류 (버퍼 오버런 방지)

	int recv_ret_val = recv(sock, g_recvBuf + g_recvBuf_used, remainSize, 0);

	if (recv_ret_val > 0)
	{
		g_recvBuf_used += recv_ret_val;

		while (g_recvBuf_used >= PACKET_SIZE)
		{
			// 버퍼의 맨 앞에서 PACKET_SIZE 만큼을 패킷으로 처리
			ProcessPacket(g_recvBuf);

			// 처리된 패킷만큼 버퍼의 데이터를 앞으로 당김 (잔여 데이터 처리)
			g_recvBuf_used -= PACKET_SIZE;
			memmove(g_recvBuf, g_recvBuf + PACKET_SIZE, g_recvBuf_used);
		}
	}
	else if (recv_ret_val == 0)
	{
		// 연결 종료 (상대방이 소켓을 닫음)
		cerr << "서버 연결 종료\n";
		// 프로그램 종료 또는 재접속 로직 필요
	}
	else // recv_ret_val == SOCKET_ERROR
	{
		errorVal = WSAGetLastError();
		if (errorVal != WSAEWOULDBLOCK)
		{
			// WSAEWOULDBLOCK이 아니면 심각한 오류 발생
			cerr << "패킷 수신 실패. error code: " << errorVal << "\n";
			// 프로그램 종료 로직 필요
		}
		// WSAEWOULDBLOCK이면 데이터가 없는 것 뿐이므로 정상 처리
	}
}

void ProcessPacket(char* buf)
{
	int packetType = *reinterpret_cast<int*>(buf);

	switch (packetType)
	{
	case 0:
	{
		st_set_id* setIdPk = reinterpret_cast<st_set_id*>(buf);
		g_myId = setIdPk->ID;
		break;
	}
	case 1:
	{
		st_create_star* createStarPk = reinterpret_cast<st_create_star*>(buf);
		unique_ptr<Star> newStarPtr = make_unique<Star>();
		newStarPtr->id = createStarPk->ID;
		newStarPtr->y = createStarPk->Y;
		newStarPtr->x = createStarPk->X;

		Star* newStarRawPtr = newStarPtr.get();
		g_stars.push_back(move(newStarPtr));

		if (g_myId != -1 && g_myId == newStarRawPtr->id)
			g_myStar = newStarRawPtr;

		break;
	}
	case 2:
	{
		st_delete_star* deleteStarPk = reinterpret_cast<st_delete_star*>(buf);
		int deleteId = deleteStarPk->ID;

		for (auto it = g_stars.begin(); it != g_stars.end(); )
		{
			if ((*it)->id == deleteId)
			{
				if (g_myStar == it->get())
					g_myStar = nullptr;

				it = g_stars.erase(it);
				return;
			}

			++it;
		}

		break;
	}
	case 3:
	{
		st_move_star* moveStarPk = reinterpret_cast<st_move_star*>(buf);
		int moveId = moveStarPk->ID;
		int newX = moveStarPk->X;
		int newY = moveStarPk->Y;

		for (const auto& starPtr : g_stars)
			if (starPtr->id == moveId)
			{
				starPtr->x = newX;
				starPtr->y = newY;
				break;
			}

		break;
	}
	default:
		cerr << format("알 수 없는 패킷 수신. Type: {}\n", packetType);
	}
}

void UpdateGame()
{
	RecvPacket();

	// 입력 처리
	bool myStarMoved = false;
	short keyStateUp = GetAsyncKeyState(VK_UP);
	short keyStateDown = GetAsyncKeyState(VK_DOWN);
	short keyStateLeft = GetAsyncKeyState(VK_LEFT);
	short keyStateRight = GetAsyncKeyState(VK_RIGHT);

	if (g_myStar != nullptr)
	{
		if (KeyPressed(keyStateUp) && g_myStar->y > 0)
		{
			--g_myStar->y;
			myStarMoved = true;
		}

		if (KeyPressed(keyStateDown) && g_myStar->y + 1 < HEIGHT)
		{
			++g_myStar->y;
			myStarMoved = true;
		}

		if (KeyPressed(keyStateLeft) && g_myStar->x > 0)
		{
			--g_myStar->x;
			myStarMoved = true;
		}

		if (KeyPressed(keyStateRight) && g_myStar->x + 1 < WIDTH - 1)
		{
			++g_myStar->x;
			myStarMoved = true;
		}
	}

	// 로직
	if (myStarMoved && g_myStar != nullptr)
	{
		st_move_star moveStarPk{ 3, g_myStar->id, g_myStar->x, g_myStar->y };
		int send_ret_val = send(sock, (const char*)&moveStarPk, sizeof(moveStarPk), 0);
		if (send_ret_val == SOCKET_ERROR)
		{
			errorVal = WSAGetLastError();
			cerr << "별 이동 패킷 전송 실패. error code: " << errorVal << "\n";
		}
	}

	// 시간 계산
	time_point now = steady_clock::now();
	CalcTime(now);

	if (g_tpNextGoal < now) // 목표 예정 시간을 초과했으므로 렌더링 포기
	{
		g_tpNextGoal += DURATION;
		return;
	}

	// 렌더링
	cs_Clear();

	for (const auto& star : g_stars)
		cs_SetString(star->y, star->x, "*");

	cs_Render();

	this_thread::sleep_until(g_tpNextGoal);
}

bool KeyPressed(short keyState)
{
	return keyState & 0x8000;
}

void CalcTime(const steady_clock::time_point& now)
{
	if (now - g_lastTp >= seconds(1))
	{
		g_frame_Logic = g_frameCount_Logic;
		g_frameCount_Logic = 0;

		g_frame_Render = g_frameCount_Render;
		g_frameCount_Render = 0;

		g_lastTp = now;
	}

	long long duration = duration_cast<milliseconds>(now - TP_START).count();
}
