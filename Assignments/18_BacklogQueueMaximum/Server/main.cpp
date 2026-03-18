#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <vector>

using namespace std;

constexpr int SERVER_PORT = 8888;

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "WSAStartup 실패! Error: " << WSAGetLastError() << '\n';
		return 1;
	}

	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(SERVER_PORT);

	bind(listenSock, (sockaddr*)&serverAddr, sizeof serverAddr);

	// SOMAXCONN으로 하면 backlog queue 크기는 200개 정도가 됨
	// if (listen(sock, SOMAXCONN) == SOCKET_ERROR)

	if (listen(listenSock, SOMAXCONN_HINT(65535)) == SOCKET_ERROR)
	{
		cerr << "listen 실패! Error: " << WSAGetLastError() << '\n';
		closesocket(listenSock);
		WSACleanup();
		return 1;
	}

	cout << "서버가 listen 중...\n";
	cout << "클라에서 접속 폭격이 끝나면 '1'을 누르세요: ";

	string input;
	cin >> input;

	if (input == "1") {
		cout << "\n큐에서 연결을 수락하기 시작합니다.\n";
		int acceptCount = 0;

		vector<SOCKET> acceptedSockets;

		while (true) {
			fd_set readFds;
			FD_ZERO(&readFds);
			FD_SET(listenSock, &readFds);

			timeval timeout = {}; // 대기 시간 0초 (즉시 확인 후 반환)

			int selectResult = select(0, &readFds, nullptr, nullptr, &timeout);

			if (selectResult > 0) {
				SOCKET clientSocket = accept(listenSock, nullptr, nullptr);

				if (clientSocket != INVALID_SOCKET) {
					++acceptCount;
					acceptedSockets.emplace_back(clientSocket);
				}
			}
			else if (selectResult == 0) {
				cout << "큐가 비었습니다!\n";
				break;
			}
			else {
				cout << "select() 에러 발생! Error: " << WSAGetLastError() << "\n";
				break;
			}
		}

		cout << "\n======================================\n";
		cout << "총 수락 연결수: " << acceptCount << "\n";
		cout << "연결을 전부 해제하려면 '2'를 누르세요: ";

		cin >> input;
		if (input == "2")
			for (SOCKET sock : acceptedSockets)
				closesocket(sock);
	}

	closesocket(listenSock);
	WSACleanup();

	system("pause"); // 프로그램 종료 전 대기
	return 0;
}
