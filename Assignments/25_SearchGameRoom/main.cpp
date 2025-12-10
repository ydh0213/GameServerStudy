#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <iostream>

constexpr int BUF_SIZE = 128;
const uint8_t sendBuf[] = { 0xff, 0xee, 0xdd, 0xaa, 0x00, 0x99, 0x77, 0x55, 0x33, 0x11 };
const bool bEnable = TRUE;
const int WAIT_TIME = 200;

using namespace std;

int main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cerr << "윈속 초기화 실패\n";
		return 1;
	}

	SOCKET sock = INVALID_SOCKET;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "소켓 생성 실패\n";
		WSACleanup();
		return 1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)) == SOCKET_ERROR)
	{
		int setsockoptErr = WSAGetLastError();
		cerr << "소켓 옵션 브로드캐스팅 수정 실패. error code: " << setsockoptErr << "\n";
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	for (int serverPort = 10000; serverPort <= 10100; ++serverPort)
	{
		SOCKADDR_IN serverAddr;
		ZeroMemory(&serverAddr, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(serverPort);
		serverAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

		// if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		// {
		// 	int connErr = WSAGetLastError();
		// 	cerr << "서버 접속 실패. error code: " << connErr << "\n";
		// 	closesocket(sock);
		// 	WSACleanup();
		// 	return 1;
		// }

		int sendRetVal = sendto(sock, (const char*)sendBuf, sizeof(sendBuf), 0, (SOCKADDR*)&serverAddr,
			sizeof(serverAddr));

		if (sendRetVal == SOCKET_ERROR)
		{
			int sendErr = WSAGetLastError();
			cerr << "send 실패. error code: " << sendErr << "\n";
			closesocket(sock);
			WSACleanup();
			return 1;
		}

		cout << "UDP " << sendRetVal << " Byte 전송 완료, Port: " << serverPort << "\n";

		SOCKADDR_IN peerAddr;
		int addrLen = sizeof(peerAddr);
		wchar_t recvBuf[BUF_SIZE];
		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&WAIT_TIME, sizeof(WAIT_TIME)) == SOCKET_ERROR)
		{
			int setsockoptErr = WSAGetLastError();
			cerr << "소켓 옵션 recv 타임아웃 수정 실패. error code: " << setsockoptErr << "\n";
			closesocket(sock);
			WSACleanup();
			return 1;
		}

		int recvVal = recvfrom(sock, (char*)recvBuf, BUF_SIZE, 0, (SOCKADDR*)&peerAddr, &addrLen);
		// cout << "recvVal: " << recvVal << "\n";
		if (recvVal == SOCKET_ERROR)
		{
			int recvErr = WSAGetLastError();

			if (recvErr == WSAETIMEDOUT)
				continue;

			cerr << "recv 실패. error code: " << recvErr << "\n";
			closesocket(sock);
			WSACleanup();
			return 1;
		}

		wchar_t peerAddrStr[32];
		DWORD peerAddrStrLen;
		if (WSAAddressToStringW((SOCKADDR*)&peerAddr, sizeof(peerAddr), NULL, peerAddrStr, &peerAddrStrLen) == SOCKET_ERROR)
		{
			int addToStrErr = WSAGetLastError();
			cerr << "WSAAddressToStringA 실패. error code: " << addToStrErr << "\n";
			closesocket(sock);
			WSACleanup();
			return 1;
		}

		wcout << "UDP 수신완료. 수신 데이터: " << recvBuf << ", IP: " << peerAddrStr << ", Port: " << serverPort << "\n";
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}
