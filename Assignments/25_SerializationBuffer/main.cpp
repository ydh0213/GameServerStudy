#include <iostream>

#include "CPacket.h"

using namespace std;

int main()
{
	cout << "--- [클라이언트] 데이터 직렬화 ---\n";

	// 1. 패킷 객체 생성 (1024 Byte 크기의 버퍼 할당)
	CPacket packet(1024);

	// 2. 전송할 데이터 준비
	int characterId = 777;
	short xPos = 150;
	short yPos = 200;
	float hp = 95.5f;

	cout << "보낼 데이터: ID " << characterId << ", 좌표 (" << xPos << ", " << yPos << "), HP " << hp << "\n";

	// 3. 패킷에 데이터 직렬화
	packet << characterId << xPos << yPos << hp;

	cout << "-> 직렬화 완료! 현재 버퍼 사용량: " << packet.GetDataSize() << " Bytes\n\n";

	/* ========================================================================
	  [네트워크 전송 시뮬레이션]
	  실제 환경에서는 여기서 send() 함수를 이용해 네트워크로 데이터를 보냅니다.
	  ex) send(socket, sendPacket.GetBufferPtr(), sendPacket.GetDataSize(), 0);
	========================================================================
	*/

	cout << "--- [서버] 데이터 역직렬화 ---\n";

	// 4. 수신받을 변수 준비
	int recvCharacterId;
	short recvXPos;
	short recvYPos;
	float recvHp;

	// 5. 패킷에서 데이터 역직렬화
	packet >> recvCharacterId >> recvXPos >> recvYPos >> recvHp;

	// 6. 결과 확인
	cout << "받은 데이터: ID " << recvCharacterId << ", 좌표 (" << recvXPos << ", " << recvYPos << "), HP " << recvHp << "\n";

	return 0;
}
