#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

using namespace std;

const char* SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 8888;

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

	int connectedCount = 0;

	cout << "연결 폭격을 시작합니다...\n";

	while (true)
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

		if (sock == INVALID_SOCKET)
		{
			cerr << "소켓 생성 실패! Error: " << WSAGetLastError() << '\n';
			continue;
		}

		if (connect(sock, (sockaddr*)&serverAddr, sizeof serverAddr) == SOCKET_ERROR)
		{
			cerr << "\n[!] 연결 실패! 큐가 꽉 찼거나 타임아웃 발생. Error: " << WSAGetLastError() << '\n';
			cout << "총 성공한 연결 횟수 (큐에 들어간 개수): " << connectedCount << "\n";
			break;
		}

		connectedCount++;

		// 진행 상황 표시 (100번마다)
		if (connectedCount % 100 == 0)
			cout << connectedCount << "번째 연결 큐에 진입...\n";
	}

	Sleep(INFINITE); // 연결 폭격이 끝난 후 프로그램이 종료되지 않도록 대기
	WSACleanup();
	return 0;
}
