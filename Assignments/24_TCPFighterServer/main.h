#pragma once

constexpr int SERVER_PORT = 5000;

constexpr int WIDTH = 640;
constexpr int HEIGHT = 480;

// 화면 이동영역
constexpr int RANGE_MOVE_TOP = 50;
constexpr int RANGE_MOVE_LEFT = 10;
constexpr int RANGE_MOVE_RIGHT = 630;
constexpr int RANGE_MOVE_BOTTOM = 470;

constexpr int SPEED_MOVE_X = 3; // 이동 속도 (픽셀/프레임)
constexpr int SPEED_MOVE_Y = 2;
const int g_dX[] = { -SPEED_MOVE_X, -SPEED_MOVE_X, 0, SPEED_MOVE_X, SPEED_MOVE_X, SPEED_MOVE_X, 0, -SPEED_MOVE_X };
const int g_dY[] = { 0, -SPEED_MOVE_Y, -SPEED_MOVE_Y, -SPEED_MOVE_Y, 0, SPEED_MOVE_Y, SPEED_MOVE_Y, SPEED_MOVE_Y };
const string g_dir[] = { "LL", "LU", "UU", "RU", "RR", "RD", "DD", "LD" };

// 공격 범위
constexpr int ATTACK1_RANGE_X = 80;
constexpr int ATTACK2_RANGE_X = 90;
constexpr int ATTACK3_RANGE_X = 100;
constexpr int ATTACK1_RANGE_Y = 10;
constexpr int ATTACK2_RANGE_Y = 10;
constexpr int ATTACK3_RANGE_Y = 20;

// 데미지
constexpr int ATTACK1_DAMAGE = 3;
constexpr int ATTACK2_DAMAGE = 6;
constexpr int ATTACK3_DAMAGE = 10;

constexpr int MAX_HP = 100;

constexpr int ERROR_RANGE = 50; // 위치 오차 허용 범위

constexpr int FPS = 50;
const milliseconds DURATION = milliseconds(1000 / FPS); // 프레임 간격 시간
const time_point TP_START = steady_clock::now();

steady_clock::time_point g_lastTp;
steady_clock::time_point g_tpNextGoal = TP_START + DURATION; // 다음 목표 시각
int g_logicFrame;
int g_logicFrameCount;

SOCKET g_ListenSock;

struct Session
{
	int id;
	int direction;
	int x;
	int y;
	int hp;
	bool isMoving = false;
	RingBuffer recvBuffer;
	RingBuffer sendBuffer;

	Session() : recvBuffer(1 << 10), sendBuffer(1 << 10)
	{
	}
};

map<SOCKET, Session*> g_Sessions;
int g_userIdCounter = 0;

BOOL AcceptProc();

void SendPacket(SOCKET sock, void* packet, int packetSize);

void SendToAll(void* packet, int packetSize, SOCKET excludeSock);

void Disconnect(SOCKET sock);

void Update();

bool isCollide(Session* attacker, Session* defender, int type);
