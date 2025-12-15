#pragma comment(lib, "ws2_32")

#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <map>
#include <algorithm>
#include <format>
#include <random>

using namespace std;
using namespace std::chrono;

#include "console.h"
#include "main.h"
#include "packetProtocol.h"

int main()
{
	cs_Initial();

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	g_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_ListenSock == INVALID_SOCKET)
	{
		errorVal = WSAGetLastError();
		cs_SetString(0, 0, format("소켓 생성 실패 error code: {}", errorVal).c_str());
		return 1;
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(g_ListenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		cs_SetString(0, 0, format("bind 실패 error code: {}", errorVal).c_str());
		closesocket(g_ListenSock);
		WSACleanup();
		return 1;
	}

	if (listen(g_ListenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		cs_SetString(0, 0, format("listen 실패 error code: {}", errorVal).c_str());
		closesocket(g_ListenSock);
		WSACleanup();
		return 1;
	}

	fd_set ReadSet;

	for (;;)
	{
		cs_Clear();

		// ============================================================
		// [ 네트워크 ]
		// ============================================================

		FD_ZERO(&ReadSet);
		FD_SET(g_ListenSock, &ReadSet);

		for (const auto& pair : g_Sessions)
			FD_SET(pair.first, &ReadSet);

		if (select(0, &ReadSet, nullptr, nullptr, nullptr) == SOCKET_ERROR)
		{
			errorVal = WSAGetLastError();
			cs_SetString(0, 0, format("select 실패 error code: {}", errorVal).c_str());
			break;
		}

		if (FD_ISSET(g_ListenSock, &ReadSet))
			AcceptProc();

		// 데이터 수신 처리 (Client Sockets)
		for (auto it = g_Sessions.begin(); it != g_Sessions.end();)
		{
			SOCKET sock = it->first;
			Session* session = it->second;
			++it; // 다음 요소를 미리 가리켜 둠 (삭제 시 안전을 위해)

			if (FD_ISSET(sock, &ReadSet))
			{
				// 버퍼 오버런 방지
				int remain = sizeof(session->recvBuf) - session->recvBytes;
				if (remain <= 0)
				{
					Disconnect(sock); // 비정상 상태
					continue;
				}

				int ret = recv(sock, session->recvBuf + session->recvBytes, remain, 0);
				if (ret <= 0) // 연결 종료 또는 에러
				{
					Disconnect(sock);
					continue;
				}

				session->recvBytes += ret;

				while (session->recvBytes >= PACKET_SIZE)
				{
					ProcessPacket(session, session->recvBuf);

					session->recvBytes -= PACKET_SIZE;
					memmove(session->recvBuf, session->recvBuf + PACKET_SIZE, session->recvBytes);
				}
			}
		}

		// ============================================================
		// [ 렌더링 ]
		// ============================================================

		int userCount = 0;
		for (auto& pair : g_Sessions)
		{
			Session* s = pair.second;
			cs_SetString(s->y, s->x, "*");
			++userCount;
		}

		cs_SetString(HEIGHT - 1, 0, format("Connected Clients: {}", userCount).c_str());

		cs_Render();
	}

	WSACleanup();
	return 0;
}

BOOL AcceptProc()
{
	SOCKADDR_IN clientAddr;
	int len = sizeof(clientAddr);
	SOCKET clientSock = accept(g_ListenSock, (SOCKADDR*)&clientAddr, &len);

	if (clientSock == INVALID_SOCKET)
	{
		errorVal = WSAGetLastError();
		cs_SetString(0, 0, format("AcceptProc() error code: {}", errorVal).c_str());
		closesocket(clientSock);
		WSACleanup();
		return TRUE;
	}

	static random_device rd;
	static mt19937 gen(rd());
	uniform_int_distribution disX(0, WIDTH - 1);
	uniform_int_distribution disY(0, HEIGHT - 1);

	Session* newSession = new Session();
	newSession->sock = clientSock;
	newSession->id = g_userIdCounter++;
	newSession->x = disX(gen);
	newSession->y = disY(gen);
	g_Sessions.insert({ clientSock, newSession });

	// 접속한 클라에게 ID 부여 패킷 전송
	st_set_id setIdPk;
	setIdPk.Type = 0;
	setIdPk.ID = newSession->id;
	SendPacket(clientSock, &setIdPk);

	// 접속한 클라에게 본인 캐릭터 생성 패킷 전송
	st_create_star createMyPk;
	createMyPk.Type = 1;
	createMyPk.ID = newSession->id;
	createMyPk.X = newSession->x;
	createMyPk.Y = newSession->y;
	SendPacket(clientSock, &createMyPk);

	// 기존 유저들에게 새 유저(나)를 생성하라고 알림
	// 나(clientSock)는 제외하고 보냄
	SendToAll(&createMyPk, clientSock);

	// 접속한 클라에게 기존 유저들 정보를 모두 보냄 (동기화)
	for (auto& pair : g_Sessions)
	{
		if (pair.first == clientSock) continue;

		Session* other = pair.second;
		st_create_star createOtherPk;
		createOtherPk.Type = 1;
		createOtherPk.ID = other->id;
		createOtherPk.X = other->x;
		createOtherPk.Y = other->y;
		SendPacket(clientSock, &createOtherPk);
	}

	return FALSE;
}

void SendPacket(SOCKET sock, void* packet)
{
	int retval = send(sock, (char*)packet, PACKET_SIZE, 0);
	if (retval == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		cs_SetString(0, 0, format("AcceptProc() error code: {}", errorVal).c_str());
	}
}

void SendToAll(void* packet, SOCKET excludeSock)
{
	for (auto& pair : g_Sessions)
	{
		if (pair.first == excludeSock) continue;

		SendPacket(pair.first, packet);
	}
}

void Disconnect(SOCKET sock)
{
	auto it = g_Sessions.find(sock);
	if (it == g_Sessions.end()) return;

	Session* session = it->second;
	int deadId = session->id;

	st_delete_star deletePk;
	deletePk.Type = 2;
	deletePk.ID = deadId;
	SendToAll(&deletePk, sock);

	closesocket(sock);
	delete session;
	g_Sessions.erase(it);
}

void ProcessPacket(Session* session, char* packet)
{
	int type = *(int*)packet;
	switch (type)
	{
	case 3:
	{
		st_move_star* movePk = (st_move_star*)packet;
		session->x = movePk->X;
		session->y = movePk->Y;

		SendToAll(movePk, session->sock);
		break;
	}
	}
}
