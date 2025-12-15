#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

const char* SERVER_DOMAIN = "procademyserver.iptime.org";
const char* SERVER_PORT = "10010";
const int BUF_SIZE = 1000;

// 프로토콜 헤더
#pragma pack(push, 1)
struct st_PACKET_HEADER
{
	DWORD dwPacketCode;
	WCHAR szName[32];
	WCHAR szFileName[128];
	int iFileSize;
};
#pragma pack(pop)

// 소켓 함수 오류 출력 후 종료
void err_quit(const wchar_t* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, nullptr);
	MessageBox(nullptr, (LPCWSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const wchar_t* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, nullptr);
	wcout << L"[" << msg << L"] " << (wchar_t*)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

BOOL SendAll(SOCKET sock, const char* data, int totalBytes)
{
	int bytesSent = 0;
	while (bytesSent < totalBytes)
	{
		int ret = send(sock, data + bytesSent, totalBytes - bytesSent, 0);

		if (ret == SOCKET_ERROR)
		{
			err_display(L"SendAll() error");
			return FALSE;
		}

		if (ret == 0)
		{
			err_display(L"SendAll() connection closed unexpectedly");
			return FALSE;
		}

		bytesSent += ret;
	}
	return TRUE;
}

int main()
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		err_quit(L"WSAStartup()");

	// ----------------------------------------------------
	// 1. getaddrinfo 사용 준비
	// ----------------------------------------------------

	addrinfo hints;
	addrinfo* result = nullptr;
	addrinfo* ptr = nullptr;

	// 소켓 변수
	SOCKET sock = INVALID_SOCKET;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// ----------------------------------------------------
	// 2. getaddrinfo 호출 (DNS 조회 및 주소 정보 획득)
	// ----------------------------------------------------
	int retval = getaddrinfo(SERVER_DOMAIN, SERVER_PORT, &hints, &result);
	if (retval != 0)
	{
		wchar_t errorMsg[256];
		swprintf_s(errorMsg, L"getaddrinfo() failed with error: %d", retval);
		MessageBox(nullptr, errorMsg, L"getaddrinfo Error", MB_ICONERROR);
		WSACleanup();
		return 1;
	}

	cout << "getaddrinfo() 성공. 서버 접속 시도..." << endl;

	// ----------------------------------------------------
	// 3. 결과 목록을 순회하며 접속 시도
	// ----------------------------------------------------
	// getaddrinfo는 여러 개의 주소를 반환할 수 있음 (링크드 리스트 형태)
	// 첫 번째 성공하는 주소로 접속합니다.
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		// socket()
		sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (sock == INVALID_SOCKET)
		{
			err_display(L"socket() error, next address trying...");
			continue;
		}

		// connect()
		retval = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (retval == SOCKET_ERROR)
		{
			err_display(L"connect() error");
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}

		cout << "connect() 성공!" << endl;
		break;
	}

	// ----------------------------------------------------
	// 4. getaddrinfo 결과 메모리 해제
	// ----------------------------------------------------
	freeaddrinfo(result);

	// ----------------------------------------------------
	// 5. 최종 접속 성공 여부 확인
	// ----------------------------------------------------
	if (sock == INVALID_SOCKET)
		err_quit(L"서버에 접속할 수 없습니다.");

	// 파일 열기
	string filePath = "C:\\Users\\ydh02\\Downloads\\pachirisu.png";
	ifstream file(filePath, ios::binary);

	if (!file.is_open())
	{
		cerr << "오류: 파일을 열 수 없습니다. 경로: " << filePath << endl;
		return 1;
	}

	cout << "성공: 파일을 성공적으로 열었습니다!" << endl;

	// 파일 크기 확인
	file.seekg(0, ios::end);
	streampos fileSize = file.tellg();
	file.seekg(0, ios::beg);

	cout << "파일 크기: " << fileSize << " 바이트" << endl;

	st_PACKET_HEADER header;
	header.dwPacketCode = 0x11223344;
	wcscpy_s(header.szName, _countof(header.szName), L"DaehyunYoon");
	wcscpy_s(header.szFileName, _countof(header.szFileName), L"pachirisu.png");
	header.iFileSize = static_cast<long>(fileSize);

	// 헤더 전송
	if (!SendAll(sock, (char*)&header, sizeof(header)))
		err_quit(L"헤더 전송 실패");

	// 서버로 파일 데이터 1000B 단위로 전송
	char buffer[BUF_SIZE];
	while (file.read(buffer, BUF_SIZE))
		if (!SendAll(sock, buffer, BUF_SIZE))
		{
			err_display(L"send()");
			break;
		}

	if (file.gcount() > 0)
	{
		if (!SendAll(sock, buffer, (int)file.gcount()))
			err_display(L"파일 마지막 조각 전송 실패");
	}

	if (!file.eof())
		cerr << "오류: 파일 읽기 중 오류 발생 (EOF 아님)" << endl;

	// 파일전송 성공시 서버에서 클라이언트로 0xdddddddd 를 보내줌
	DWORD recvPacketCode;
	int received = 0;
	char* p = (char*)&recvPacketCode;
	bool recvError = false;

	while (received < sizeof(recvPacketCode))
	{
		int ret = recv(sock, p + received, sizeof(recvPacketCode) - received, 0);

		if (ret == SOCKET_ERROR)
		{
			err_quit(L"recv() error");
			recvError = true;
			break; // 오류 발생 시 즉시 탈출
		}

		if (ret == 0)
		{
			err_display(L"서버가 연결을 종료했습니다.\n");
			recvError = true;
			break; // 연결 종료 시 즉시 탈출
		}

		received += ret;
	}

	if (!recvError && received == sizeof(recvPacketCode))
	{
		recvPacketCode = ntohl(recvPacketCode);
		cout << "서버로부터 받은 패킷 코드: 0x" << hex << recvPacketCode << endl;
	}
	else
		cerr << "오류: 서버로부터 응답을 제대로 받지 못했습니다." << endl;

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();

	return 0;
}
