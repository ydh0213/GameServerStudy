#pragma once

constexpr BYTE PACKET_CODE = 0x89;

constexpr BYTE PACKET_MOVE_DIR_LL = 0;
constexpr BYTE PACKET_MOVE_DIR_LU = 1;
constexpr BYTE PACKET_MOVE_DIR_UU = 2;
constexpr BYTE PACKET_MOVE_DIR_RU = 3;
constexpr BYTE PACKET_MOVE_DIR_RR = 4;
constexpr BYTE PACKET_MOVE_DIR_RD = 5;
constexpr BYTE PACKET_MOVE_DIR_DD = 6;
constexpr BYTE PACKET_MOVE_DIR_LD = 7;

constexpr BYTE PACKET_SC_CREATE_MY_CHARACTER = 0;
constexpr BYTE PACKET_SC_CREATE_OTHER_CHARACTER = 1;
constexpr BYTE PACKET_SC_DELETE_CHARACTER = 2;
constexpr BYTE PACKET_CS_MOVE_START = 10;
constexpr BYTE PACKET_SC_MOVE_START = 11;
constexpr BYTE PACKET_CS_MOVE_STOP = 12;
constexpr BYTE PACKET_SC_MOVE_STOP = 13;

constexpr BYTE PACKET_CS_ATTACK1 = 20;
constexpr BYTE PACKET_SC_ATTACK1 = 21;
constexpr BYTE PACKET_CS_ATTACK2 = 22;
constexpr BYTE PACKET_SC_ATTACK2 = 23;
constexpr BYTE PACKET_CS_ATTACK3 = 24;
constexpr BYTE PACKET_SC_ATTACK3 = 25;
constexpr BYTE PACKET_SC_DAMAGE = 30;

#pragma pack(push, 1)

struct Header
{
	BYTE byCode; // 패킷코드 0x89 고정
	BYTE bySize;
	BYTE byType;
};

//////////////// 캐릭터 생성/삭제 패킷 ////////////////

struct createMyCharBody
{
	uint32_t id;
	uint8_t direction; // LL / RR
	uint16_t x;
	uint16_t y;
	uint8_t hp; // 100 
};

struct SC_createMyChar
{
	Header header;
	createMyCharBody body;
};

struct createOtherCharBody
{
	uint32_t id;
	uint8_t direction; // LL / RR
	uint16_t x;
	uint16_t y;
	uint8_t hp; // 100 
};

struct SC_createOtherChar
{
	Header header;
	createOtherCharBody body;
};

struct deleteCharBody
{
	uint32_t id;
};

struct SC_deleteChar
{
	Header header;
	deleteCharBody body;
};

//////////////// 캐릭터 이동 패킷 ////////////////

struct CS_moveStartBody
{
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct CS_moveStart
{
	Header header;
	CS_moveStartBody body;
};

struct SC_moveStartBody
{
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct SC_moveStart
{
	Header header;
	SC_moveStartBody body;
};

struct CS_moveStopBody
{
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct CS_moveStop
{
	Header header;
	CS_moveStopBody body;
};

struct SC_moveStopBody
{
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct SC_moveStop
{
	Header header;
	SC_moveStopBody body;
};

//////////////// 캐릭터 공격 패킷 ////////////////

struct CS_attack1Body
{
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct CS_attack1
{
	Header header;
	CS_attack1Body body;
};

struct SC_attack1Body
{
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct SC_attack1
{
	Header header;
	SC_attack1Body body;
};

struct CS_attack2Body
{
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct CS_attack2
{
	Header header;
	CS_attack2Body body;
};

struct SC_attack2Body
{
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct SC_attack2
{
	Header header;
	SC_attack2Body body;
};

struct CS_attack3Body
{
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct CS_attack3
{
	Header header;
	CS_attack1Body body;
};

struct SC_attack3Body
{
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct SC_attack3
{
	Header header;
	SC_attack1Body body;
};

//////////////// 캐릭터 데미지 패킷 ////////////////

struct SC_damageBody
{
	uint32_t attackId;
	uint32_t damageId;
	uint8_t remainingHp;
};

struct SC_damage
{
	Header header;
	SC_damageBody body;
};

#pragma pack(pop)
