#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include <ws2tcpip.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>

using namespace std;
using namespace std::chrono;

#include "protocol.h"
#include "RingBuffer.h"
#include "main.h"

int main()
{
	TIMECAPS timecaps;
	timeGetDevCaps(&timecaps, sizeof(timecaps));
	timeBeginPeriod(timecaps.wPeriodMin);

	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

	cout << "WSAStartup #\n";

	g_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_ListenSock == INVALID_SOCKET)
	{
		const int socketErr = WSAGetLastError();
		cerr << "socketErr: " << socketErr << '\n';
		return 1;
	}

	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVER_PORT);

	if (bind(g_ListenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		const int bindErr = WSAGetLastError();
		cerr << "bindErr: " << bindErr << '\n';
		closesocket(g_ListenSock);
		WSACleanup();
		return 1;
	}

	cout << "Bind OK # Port: " << SERVER_PORT << '\n';

	if (listen(g_ListenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		const int listenErr = WSAGetLastError();
		cerr << "listenErr: " << listenErr << '\n';
		closesocket(g_ListenSock);
		WSACleanup();
		return 1;
	}

	cout << "Listen OK #\n";

	fd_set readSet;
	fd_set writeSet;

	g_tpNextGoal = steady_clock::now() + DURATION;

	for (;; ++g_logicFrameCount)
	{
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		FD_SET(g_ListenSock, &readSet);

		for (const auto& pair : g_Sessions)
		{
			const SOCKET& sock = pair.first;
			const Session* const session = pair.second;

			FD_SET(sock, &readSet);

			if (session->sendBuffer.GetUsedSize() > 0)
				FD_SET(sock, &writeSet);
		}

		steady_clock::time_point now = steady_clock::now();
		auto timeUntilNextUpdate = duration_cast<microseconds>(g_tpNextGoal - now);

		timeval tv;
		if (timeUntilNextUpdate.count() <= 0)
		{
			// 이미 업데이트 시간이 지났으면 대기 없이 즉시 리턴 (Polling)
			tv.tv_sec = 0;
			tv.tv_usec = 0;
		}
		else
		{
			// 남은 시간만큼만 대기
			tv.tv_sec = (long)timeUntilNextUpdate.count() / 1'000'000l;
			tv.tv_usec = (long)timeUntilNextUpdate.count() % 1'000'000l;
		}

		if (select(0, &readSet, &writeSet, nullptr, &tv) == SOCKET_ERROR)
		{
			const int selectErr = WSAGetLastError();
			cerr << "selectErr: " << selectErr << '\n';
			break;
		}

		if (FD_ISSET(g_ListenSock, &readSet))
			AcceptProc();

		for (auto it = g_Sessions.begin(); it != g_Sessions.end();)
		{
			SOCKET sock = it->first;
			Session* session = it->second;
			++it; // 다음 요소를 미리 가리켜 둠 (삭제 시 안전을 위해)

			if (FD_ISSET(sock, &readSet))
			{
				char tmpBuf[1 << 10];

				int recvLen = recv(sock, tmpBuf, sizeof(tmpBuf), 0);
				if (recvLen <= 0) // 연결 종료 또는 에러
				{
					Disconnect(sock);
					continue;
				}

				session->recvBuffer.Enqueue(tmpBuf, recvLen);

				while (true)
				{
					// A. 헤더 크기만큼 데이터가 쌓였는지 확인
					if (session->recvBuffer.GetUsedSize() < sizeof(Header))
						break;

					// B. 헤더를 살펴본다(Peek) - 아직 꺼내진 않음(Dequeue X)
					Header header;
					session->recvBuffer.Peek((char*)&header, sizeof(Header));

					// C. 패킷 전체가 완성되었는지 확인
					if (session->recvBuffer.GetUsedSize() < (int)(sizeof(header) + header.bySize))
						break; // 아직 데이터가 덜 옴 (다음 FD_READ 대기)

					// D. 완전한 패킷 처리: 여기까지 왔으면 패킷 1개가 온전하다는 뜻.
					// 임시 버퍼에 패킷 전체를 꺼냄
					char packetBuffer[100];
					session->recvBuffer.Dequeue(packetBuffer, sizeof(header) + header.bySize);

					switch (header.byType)
					{
					case PACKET_CS_MOVE_START:
					{
						CS_moveStart* packet_moveStart = (CS_moveStart*)packetBuffer;

						cout << "# PACKET_MOVESTART # SessionID: " << session->id << " / Direction: " << session->
							direction << " / X: " << session->x << " / Y: " << session->y << '\n';

						session->direction = packet_moveStart->body.direction;
						session->x = packet_moveStart->body.x;
						session->y = packet_moveStart->body.y;
						session->isMoving = true;

						SC_moveStart packet_SC_moveStart = {
							{PACKET_CODE, sizeof(packet_SC_moveStart.body), PACKET_SC_MOVE_START},
							{
								(uint32_t)session->id,
								(uint8_t)session->direction,
								(uint16_t)session->x,
								(uint16_t)session->y
							}
						};

						// 나를 제외한 모두에게 전송
						SendToAll(&packet_SC_moveStart, sizeof(packet_SC_moveStart), sock);
					}
					break;
					case PACKET_CS_MOVE_STOP:
					{
						CS_moveStop* packet_moveStop = (CS_moveStop*)packetBuffer;

						cout << "# PACKET_MOVESTOP # SessionID: " << session->id << " / Direction: " << session->
							direction << " / X: " << session->x << " / Y: " << session->y << '\n';

						session->direction = packet_moveStop->body.direction;
						session->x = packet_moveStop->body.x;
						session->y = packet_moveStop->body.y;
						session->isMoving = false;

						SC_moveStop packet_SC_moveStop = {
							{PACKET_CODE, sizeof(packet_SC_moveStop.body), PACKET_SC_MOVE_STOP},
							{
								(uint32_t)session->id,
								(uint8_t)session->direction,
								(uint16_t)session->x,
								(uint16_t)session->y
							}
						};

						// 나를 제외한 모두에게 전송
						SendToAll(&packet_SC_moveStop, sizeof(packet_SC_moveStop), sock);
					}
					break;
					case PACKET_CS_ATTACK1:
					{
						CS_attack1& packet_CS_attack1 = *(CS_attack1*)packetBuffer;
						session->direction = packet_CS_attack1.body.direction;

						// 클라가 보낸 좌표는 무시하되 로그만 남김
						int dx = abs(session->x - packet_CS_attack1.body.x);
						int dy = abs(session->y - packet_CS_attack1.body.y);
						if (dx > ERROR_RANGE || dy > ERROR_RANGE)
						{
							cout << "!! ATTACK1 Position Error !! Server Position: (" << session->x << ", " <<
								session->y <<
								") / Client Position: (" << packet_CS_attack1.body.x << ", " << packet_CS_attack1.
								body.y <<
								")\n";

							// 보정 패킷 전송
							SC_moveStop packet_SC_moveStop = {
								{PACKET_CODE, sizeof(packet_SC_moveStop.body), PACKET_SC_MOVE_STOP},
								{
									(uint32_t)session->id,
									(uint8_t)session->direction,
									(uint16_t)session->x,
									(uint16_t)session->y
								}
							};

							SendToAll(&packet_SC_moveStop, sizeof(packet_SC_moveStop), NULL);
						}

						cout << "# PACKET_ATTACK1 # SessionID: " << session->id << " / Direction: " << session->
							direction << " / X: " << packet_CS_attack1.body.x << " / Y: " << packet_CS_attack1.body.
							y << '\n';

						SC_attack1 packet_SC_attack1{
							{PACKET_CODE, sizeof(SC_attack1Body), PACKET_SC_ATTACK1},
							{
								(uint32_t)session->id,
								(uint8_t)session->direction,
								(uint16_t)session->x,
								(uint16_t)session->y
							}
						};

						SendToAll(&packet_SC_attack1, sizeof(packet_SC_attack1), sock);

						for (auto& [otherSock, otherSession] : g_Sessions)
						{
							if (sock == otherSock)
								continue;

							if (isCollide(session, otherSession, 1))
							{
								otherSession->hp -= ATTACK1_DAMAGE;
								if (otherSession->hp < 0) otherSession->hp = 0;

								SC_damage packet_SC_damage{
									{PACKET_CODE, sizeof(SC_damageBody), PACKET_SC_DAMAGE},
									{
										(uint32_t)session->id,
										(uint32_t)otherSession->id,
										(uint8_t)otherSession->hp
									}
								};

								SendToAll(&packet_SC_damage, sizeof(packet_SC_damage), NULL);
							}
						}
					}
					break;
					case PACKET_CS_ATTACK2:
					{
						CS_attack2& packet_CS_attack2 = *(CS_attack2*)packetBuffer;
						session->direction = packet_CS_attack2.body.direction;

						// 클라가 보낸 좌표는 무시하되 로그만 남김
						int dx = abs(session->x - packet_CS_attack2.body.x);
						int dy = abs(session->y - packet_CS_attack2.body.y);
						if (dx > ERROR_RANGE || dy > ERROR_RANGE)
						{
							cout << "!! ATTACK1 Position Error !! Server Position: (" << session->x << ", " <<
								session->y <<
								") / Client Position: (" << packet_CS_attack2.body.x << ", " << packet_CS_attack2.
								body.y <<
								")\n";

							// 보정 패킷 전송
							SC_moveStop packet_SC_moveStop = {
								{PACKET_CODE, sizeof(packet_SC_moveStop.body), PACKET_SC_MOVE_STOP},
								{
									(uint32_t)session->id,
									(uint8_t)session->direction,
									(uint16_t)session->x,
									(uint16_t)session->y
								}
							};

							SendToAll(&packet_SC_moveStop, sizeof(packet_SC_moveStop), NULL);
						}

						cout << "# PACKET_ATTACK2 # SessionID: " << session->id << " / Direction: " << session->
							direction << " / X: " << packet_CS_attack2.body.x << " / Y: " << packet_CS_attack2.body.
							y << '\n';

						SC_attack2 packet_SC_attack2{
							{PACKET_CODE, sizeof(SC_attack2Body), PACKET_SC_ATTACK2},
							{
								(uint32_t)session->id,
								(uint8_t)session->direction,
								(uint16_t)session->x,
								(uint16_t)session->y
							}
						};

						SendToAll(&packet_SC_attack2, sizeof(packet_SC_attack2), sock);

						for (auto& [otherSock, otherSession] : g_Sessions)
						{
							if (sock == otherSock)
								continue;

							if (isCollide(session, otherSession, 2))
							{
								otherSession->hp -= ATTACK2_DAMAGE;
								if (otherSession->hp < 0) otherSession->hp = 0;

								SC_damage packet_SC_damage{
									{PACKET_CODE, sizeof(SC_damageBody), PACKET_SC_DAMAGE},
									{
										(uint32_t)session->id,
										(uint32_t)otherSession->id,
										(uint8_t)otherSession->hp
									}
								};

								SendToAll(&packet_SC_damage, sizeof(packet_SC_damage), NULL);
							}
						}
					}
					break;
					case PACKET_CS_ATTACK3:
					{
						CS_attack3& packet_CS_attack3 = *(CS_attack3*)packetBuffer;
						session->direction = packet_CS_attack3.body.direction;

						// 클라가 보낸 좌표는 무시하되 로그만 남김
						int dx = abs(session->x - packet_CS_attack3.body.x);
						int dy = abs(session->y - packet_CS_attack3.body.y);
						if (dx > ERROR_RANGE || dy > ERROR_RANGE)
						{
							cout << "!! ATTACK1 Position Error !! Server Position: (" << session->x << ", " <<
								session->y <<
								") / Client Position: (" << packet_CS_attack3.body.x << ", " << packet_CS_attack3.
								body.y <<
								")\n";

							// 보정 패킷 전송
							SC_moveStop packet_SC_moveStop = {
								{PACKET_CODE, sizeof(packet_SC_moveStop.body), PACKET_SC_MOVE_STOP},
								{
									(uint32_t)session->id,
									(uint8_t)session->direction,
									(uint16_t)session->x,
									(uint16_t)session->y
								}
							};

							SendToAll(&packet_SC_moveStop, sizeof(packet_SC_moveStop), NULL);
						}

						cout << "# PACKET_ATTACK3 # SessionID: " << session->id << " / Direction: " << session->
							direction << " / X: " << packet_CS_attack3.body.x << " / Y: " << packet_CS_attack3.body.
							y << '\n';

						SC_attack3 packet_SC_attack3{
							{PACKET_CODE, sizeof(SC_attack1Body), PACKET_SC_ATTACK3},
							{
								(uint32_t)session->id,
								(uint8_t)session->direction,
								(uint16_t)session->x,
								(uint16_t)session->y
							}
						};

						SendToAll(&packet_SC_attack3, sizeof(packet_SC_attack3), sock);

						for (auto& [otherSock, otherSession] : g_Sessions)
						{
							if (sock == otherSock)
								continue;

							if (isCollide(session, otherSession, 3))
							{
								otherSession->hp -= ATTACK1_DAMAGE;
								if (otherSession->hp < 0) otherSession->hp = 0;

								SC_damage packet_SC_damage{
									{PACKET_CODE, sizeof(SC_damageBody), PACKET_SC_DAMAGE},
									{
										(uint32_t)session->id,
										(uint32_t)otherSession->id,
										(uint8_t)otherSession->hp
									}
								};

								SendToAll(&packet_SC_damage, sizeof(packet_SC_damage), NULL);
							}
						}
					}
					break;
					default:
						cerr << "Unknown Packet Type: " << (int)header.byType << '\n';
						break;
					}
				}
			}
		}

		for (auto it = g_Sessions.begin(); it != g_Sessions.end();)
		{
			SOCKET sock = it->first;
			Session* session = it->second;
			++it; // 다음 요소를 미리 가리켜 둠 (삭제 시 안전을 위해)

			if (FD_ISSET(sock, &writeSet))
			{
				char tmpBuf[1 << 10];
				int peekLen = session->sendBuffer.Peek(tmpBuf, sizeof(tmpBuf));

				if (peekLen > 0)
				{
					int sent = send(sock, tmpBuf, peekLen, 0);
					if (sent == SOCKET_ERROR)
					{
						int sendErr = WSAGetLastError();
						if (sendErr != WSAEWOULDBLOCK)
						{
							cout << "Send Error " << sendErr << ", Disconnecting # SessionID: " << session->id << '\n';
							Disconnect(sock);
						}
					}
					else if (sent > 0)
						session->sendBuffer.MoveFront(sent);
				}
			}
		}

		now = steady_clock::now();
		while (g_tpNextGoal <= now)
		{
			Update();
			g_tpNextGoal += DURATION;
		}
	}

	timeEndPeriod(timecaps.wPeriodMin);

	return 0;
}

BOOL AcceptProc()
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	SOCKET clientSock = accept(g_ListenSock, (SOCKADDR*)&clientAddr, &addrLen);

	if (clientSock == INVALID_SOCKET)
	{
		const int acceptErr = WSAGetLastError();
		cerr << "acceptErr: " << acceptErr << '\n';
		closesocket(clientSock);
		WSACleanup();
		return TRUE;
	}

	static random_device rd;
	static mt19937 gen(rd());
	uniform_int_distribution disX(RANGE_MOVE_LEFT, RANGE_MOVE_RIGHT);
	uniform_int_distribution disY(RANGE_MOVE_TOP, RANGE_MOVE_BOTTOM);
	uniform_int_distribution dir(0, 1);

	Session* newSession = new Session();
	newSession->id = g_userIdCounter++;
	newSession->direction = dir(gen) == 0 ? PACKET_MOVE_DIR_LL : PACKET_MOVE_DIR_RR;
	newSession->x = disX(gen);
	newSession->y = disY(gen);
	newSession->hp = MAX_HP;
	g_Sessions.insert({ clientSock, newSession });

	char ipAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(clientAddr.sin_addr), ipAddr, INET_ADDRSTRLEN);
	cout << "Connect # IP: " << ipAddr << " / SessionID: " << newSession->id << '\n';

	// 접속한 클라에게 본인 캐릭터 생성 패킷 전송
	SC_createMyChar packet_createMyChar(
		Header(PACKET_CODE, sizeof(createMyCharBody), PACKET_SC_CREATE_MY_CHARACTER),
		createMyCharBody(
			newSession->id,
			newSession->direction,
			newSession->x,
			newSession->y,
			newSession->hp)
	);

	SendPacket(clientSock, &packet_createMyChar, sizeof(packet_createMyChar));

	// 기존 유저들에게 새 유저(나)를 생성하라고 알림
	// 나(clientSock)는 제외하고 보냄
	SC_createOtherChar packet_createOtherChar(
		Header(PACKET_CODE, sizeof(createOtherCharBody), PACKET_SC_CREATE_OTHER_CHARACTER),
		createOtherCharBody(
			newSession->id,
			newSession->direction,
			newSession->x,
			newSession->y,
			newSession->hp
		));

	SendToAll(&packet_createOtherChar, sizeof(packet_createOtherChar), clientSock);

	cout << "Create Character # SessionID: " << newSession->id << "\tX: " << newSession->x << "\tY: " << newSession->y
		<< '\n';

	// 접속한 클라에게 기존 유저들 정보를 모두 보냄 (동기화)
	for (auto& pair : g_Sessions)
	{
		if (pair.first == clientSock) continue;

		Session* other = pair.second;
		SC_createOtherChar packet_createOtherChar(
			Header(PACKET_CODE, sizeof(createOtherCharBody), PACKET_SC_CREATE_OTHER_CHARACTER),
			createOtherCharBody(
				other->id,
				other->direction,
				other->x,
				other->y,
				other->hp)
		);

		SendPacket(clientSock, &packet_createOtherChar, sizeof(packet_createOtherChar));
	}

	return FALSE;
}

void SendPacket(SOCKET sock, void* packet, int packetSize)
{
	if (!g_Sessions.contains(sock))
		return;

	Session* session = g_Sessions[sock];
	RingBuffer& buf = session->sendBuffer;

	// 1. 링버퍼에 여유 공간이 있는지 확인
	if (buf.GetFreeSize() < packetSize)
		Disconnect(sock);

	buf.Enqueue((char*)packet, packetSize);
}

void SendToAll(void* packet, int packetSize, SOCKET excludeSock)
{
	for (auto& pair : g_Sessions)
	{
		if (pair.first == excludeSock) continue;

		SendPacket(pair.first, packet, packetSize);
	}
}

void Disconnect(SOCKET sock)
{
	auto it = g_Sessions.find(sock);
	if (it == g_Sessions.end()) return;

	Session* session = it->second;
	int deadId = session->id;

	SC_deleteChar packet_deleteChar(
		Header(PACKET_CODE, sizeof(deleteCharBody), PACKET_SC_DELETE_CHARACTER),
		deleteCharBody(deadId)
	);

	SendToAll(&packet_deleteChar, sizeof(packet_deleteChar), sock);

	closesocket(sock);
	delete session;
	g_Sessions.erase(it);

	cout << "Disconnect # SessionID: " << deadId << '\n';
}

void Update()
{
	for (auto& [sock, session] : g_Sessions)
	{
		if (session->isMoving)
		{
			int nextX = session->x + g_dX[session->direction];
			int nextY = session->y + g_dY[session->direction];

			if (nextX < RANGE_MOVE_LEFT) nextX = RANGE_MOVE_LEFT;
			if (nextX > RANGE_MOVE_RIGHT) nextX = RANGE_MOVE_RIGHT;
			if (nextY < RANGE_MOVE_TOP) nextY = RANGE_MOVE_TOP;
			if (nextY > RANGE_MOVE_BOTTOM) nextY = RANGE_MOVE_BOTTOM;

			session->x = nextX;
			session->y = nextY;

			cout << "# gameRun: " << g_dir[session->direction] << " # SessionID: " << session->id << " / X: " << session
				->x << " / Y: " << session->y << '\n';
		}
	}
}

bool isCollide(Session* attacker, Session* defender, int type)
{
	int dx = abs(attacker->x - defender->x);
	switch (type)
	{
	case 1:
		if (dx > ATTACK1_RANGE_X) return false;
		break;
	case 2:
		if (dx > ATTACK2_RANGE_X) return false;
		break;
	case 3:
		if (dx > ATTACK3_RANGE_X) return false;
		break;
	default:
		return false;
	}

	// attacker가 왼쪽에 있는데 오른쪽 공격이 아니면 유효한 공격 아님
	if (attacker->x < defender->x && attacker->direction != PACKET_MOVE_DIR_RR)
		return false;

	// attacker가 오른쪽에 있는데 왼쪽 공격이 아니면 유효한 공격 아님
	if (attacker->x > defender->x && attacker->direction != PACKET_MOVE_DIR_LL)
		return false;

	int dy = abs(attacker->y - defender->y);
	switch (type)
	{
	case 1:
		if (dy > ATTACK1_RANGE_Y) return false;
		break;
	case 2:
		if (dy > ATTACK2_RANGE_Y) return false;
		break;
	case 3:
		if (dy > ATTACK3_RANGE_Y) return false;
		break;
	default:
		return false;
	}

	cout << "collide! AttackerID: " << attacker->id << " / DefenderID: " << defender->id << '\n';

	return true;
}
