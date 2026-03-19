#include <iostream>
#include <vector>
#include <string>
#include "CPacket.h"

using namespace std;

struct Item
{
	int id;
	string name;
};

struct Page
{
	short id;
	vector<Item> items;
};

struct Inventory
{
	vector<Page> pages;
};

int main()
{
	// ========================================================================
	// 시뮬레이션 1. 캐릭터 정보 패킷 생성 및 전송
	// ========================================================================
	cout << "--- [클라이언트] 데이터 직렬화 ---\n";

	// 패킷 객체 생성 (100 Bytes 크기의 버퍼 할당)
	CPacket packet1(100);

	// 전송할 데이터 준비
	int characterId = 777;
	short xPos = 150;
	short yPos = 200;
	float hp = 95.5f;

	cout << "보낼 데이터: ID " << characterId << ", 좌표 (" << xPos << ", " << yPos << "), HP " << hp << "\n";

	// 패킷에 데이터 직렬화
	packet1 << characterId << xPos << yPos << hp;

	cout << "-> 직렬화 완료! 현재 버퍼 사용량: " << packet1.GetDataSize() << " Bytes\n\n";

	/* ========================================================================
	  [네트워크 전송 시뮬레이션]
	  실제 환경에서는 여기서 send() 함수를 이용해 네트워크로 데이터를 보냅니다.
	  ex) send(socket, sendPacket.GetBufferPtr(), sendPacket.GetDataSize(), 0);
	========================================================================
	*/

	cout << "--- [서버] 데이터 역직렬화 ---\n";

	// 수신받을 변수 준비
	int recvCharacterId;
	short recvXPos;
	short recvYPos;
	float recvHp;

	// 패킷에서 데이터 역직렬화
	packet1 >> recvCharacterId >> recvXPos >> recvYPos >> recvHp;

	cout << "받은 데이터: ID " << recvCharacterId << ", 좌표 (" << recvXPos << ", " << recvYPos << "), HP " << recvHp << "\n\n";


	// ========================================================================
	// 시뮬레이션 2. 인벤토리, 아이템 정보 패킷 생성 및 전송
	// ========================================================================
	cout << "\n--- [클라이언트] 인벤토리 데이터 세팅 ---\n";
	Inventory myInventory;

	Page page1;
	page1.id = 1;
	page1.items.emplace_back(101, "강철 검");
	page1.items.emplace_back(102, "나무 방패 (손상됨)");

	Page page2;
	page2.id = 2;
	page2.items.emplace_back(201, "체력 물약");

	myInventory.pages.emplace_back(page1);
	myInventory.pages.emplace_back(page2);

	// 패킷 객체 생성 (1024 Bytes 크기의 버퍼 할당)
	CPacket packet2(1024);

	short totalPages = (short)myInventory.pages.size();
	packet2 << totalPages;

	for (const Page& page : myInventory.pages)
	{
		packet2 << page.id;

		short itemCount = (short)page.items.size();
		packet2 << itemCount;

		for (const Item& item : page.items)
		{
			packet2 << item.id;

			short nameLength = (short)item.name.length();
			packet2 << nameLength;
			packet2.PutData((char*)item.name.c_str(), nameLength);
		}
	}

	cout << "-> 직렬화 완료! 현재 버퍼 사용량: " << packet2.GetDataSize() << " Bytes\n\n";

	/* ========================================================================
	  [네트워크 전송 시뮬레이션]
	  실제 환경에서는 여기서 send() 함수를 이용해 네트워크로 데이터를 보냅니다.
	========================================================================
	*/

	cout << "--- [서버] 데이터 역직렬화 ---\n";

	// 수신받을 변수 준비
	Inventory recvInventory;

	short recvTotalPages = 0;
	packet2 >> recvTotalPages;

	// 방어 코드: 페이지 수가 비정상적으로 크면 해킹/오류로 간주
	if (recvTotalPages < 0 || recvTotalPages > 50)
	{
		cout << "[ERROR] 비정상적인 페이지 개수입니다!\n";
		return -1;
	}

	for (short i = 0; i < recvTotalPages; ++i)
	{
		Page newPage;
		packet2 >> newPage.id;

		short recvItemCount = 0;
		packet2 >> recvItemCount;

		// 방어 코드: 한 페이지 내의 아이템 수가 너무 크면 차단
		if (recvItemCount < 0 || recvItemCount > 100)
		{
			cout << "[ERROR] 비정상적인 아이템 개수입니다!\n";
			return -1;
		}

		for (short j = 0; j < recvItemCount; ++j)
		{
			Item newItem;
			packet2 >> newItem.id;

			short nameLength = 0;
			packet2 >> nameLength;

			// 방어 코드: 문자열 길이가 비정상적이면 차단
			if (nameLength > 0 && nameLength < 256)
			{
				char tmpBuf[256] = {};
				packet2.GetData(tmpBuf, nameLength);
				newItem.name = tmpBuf;
			}
			else if (nameLength != 0)
			{
				cout << "[ERROR] 비정상적인 아이템 이름 길이입니다!\n";
				return -1;
			}

			newPage.items.emplace_back(newItem);
		}

		recvInventory.pages.emplace_back(newPage);
	}

	// 복원된 데이터 출력
	for (const Page& page : recvInventory.pages)
	{
		cout << "[페이지 ID: " << page.id << "]\n";

		for (const Item& item : page.items)
			cout << "  - 아이템 ID: " << item.id << ", 이름: " << item.name << "\n";
	}

	return 0;
}
