#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>

using namespace std;

const char* SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 8888;
constexpr int BATCH_NUMBER = 14'000;
constexpr int WAIT_MS = 125'000;

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "WSAStartup 실패! Error: " << WSAGetLastError() << '\n';
		return 1;
	}

	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0)
	{
		cout << "IP 주소 변환에 실패했습니다.\n";
		WSACleanup();
		return -1;
	}

	int totalConnectedCount = 0;
	bool isQueueFull = false; // 백로그 큐가 꽉 찼는지

	cout << "연결 폭격을 시작합니다 (14,000개 배치 및 125초 대기 모드)...\n";

	while (!isQueueFull)
	{
		vector<SOCKET> sockets;
		cout << "\n[배치 시작] 새로운 14,000개 연결을 시도합니다...\n";

		for (int i = 0; i < BATCH_NUMBER; ++i)
		{
			SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
			if (sock == INVALID_SOCKET)
			{
				cerr << "소켓 생성 실패! Error: " << WSAGetLastError() << '\n';
				continue;
			}

			if (connect(sock, (sockaddr*)&serverAddr, sizeof serverAddr) == SOCKET_ERROR)
			{
				// 에러 발생: 큐가 꽉 찼거나(10061/10060) 버퍼가 부족함(10055)
				cerr << "\n[!] 연결 실패! 큐가 꽉 찼습니다. Error: " << WSAGetLastError() << '\n';
				closesocket(sock);
				isQueueFull = true;
				break;
			}

			sockets.emplace_back(sock);

			if (++totalConnectedCount % 1000 == 0)
				cout << totalConnectedCount << "번째 연결 큐에 진입...\n";
		}

		// 이번 배치에서 연결했던 모든 소켓을 closesocket() 하여 FIN 패킷 전송 (서버는 CLOSE_WAIT 상태로 전환)
		cout << "\n[정리 작업] 이번 배치에서 연결된 " << sockets.size() << "개의 소켓을 닫습니다...\n";
		for (SOCKET sock : sockets)
			closesocket(sock);

		sockets.clear();

		if (isQueueFull)
			break;

		// closesocket() 후 FINE_WAIT_2 상태로 진입하여 기본값 120초 이상 지나서 포트가 완전히 해제되길 기다림
		cout << "[휴식] 포트 고갈(10055) 방지를 위해 125초간 대기합니다...\n";
		Sleep(WAIT_MS);
	}

	cout << "\n======================================\n";
	cout << "테스트 종료! 총 성공한 연결 횟수: " << totalConnectedCount << "\n";
	cout << "======================================\n";

	Sleep(INFINITE); // 연결 폭격이 끝난 후 프로그램이 종료되지 않도록 대기
	WSACleanup();
	return 0;
}
