#pragma once

constexpr int SERVER_PORT = 3000;
constexpr int PACKET_SIZE = 16;

const int FPS = 50;
const milliseconds DURATION = milliseconds(1000 / FPS); // 프레임 간격 시간
const time_point TP_START = steady_clock::now();

HANDLE hConsoleOutput;
CHAR_INFO consoleBuffer[WIDTH * HEIGHT];

struct Session
{
	SOCKET sock;
	int id;
	int x = 0;
	int y = 0;
	char recvBuf[1024];
	int recvBytes = 0;
};

SOCKET g_ListenSock;
map<SOCKET, Session*> g_Sessions;
int g_userIdCounter = 0;
int errorVal;

steady_clock::time_point g_lastTp;
steady_clock::time_point g_tpNextGoal; // 다음 목표 시각
int g_frameCount_Logic;
int g_frameCount_Render;
int g_frame_Logic;
int g_frame_Render;
char szScreenBuffer[HEIGHT][WIDTH];

// 새로운 접속 처리 (Listen Socket)
BOOL AcceptProc();

void SendPacket(SOCKET sock, void* packet);

void SendToAll(void* packet, SOCKET excludeSock = INVALID_SOCKET);

void Disconnect(SOCKET sock);

void ProcessPacket(Session* session, char* packet);
