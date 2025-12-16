#pragma comment(lib, "Winmm.lib")

#include <iostream>
#include <cstdio>
#include <windows.h>

using namespace std;

void* Alloc_overflow_check(int size);

bool Free_overflow_check(void* p);

void* Alloc_underflow_check(int size);

bool Free_underflow_check(void* p);

int main()
{
	// 주석들 중 하나라도 풀면 NOACESS 페이지 또는 decommit된 페이지에 write 시도를 하게 되어 에러 발생

	int* t = (int*)Alloc_overflow_check(0x1400);

	printf("0x%x: size %d\n", t - 1, *(t - 1));

	// *(t + 0x1400) = 0xff;

	printf("result: %d\n", Free_overflow_check(t));

	// printf("0x%x: size %d\n", t - 1, *(t - 1));



	int* p = (int*)Alloc_underflow_check(0x1400);

	printf("0x%x: size %d\n", p - 1, *(p - 1));

	// *(p - 0xf) = 0x22;

	printf("result: %d\n", Free_underflow_check(p));

	// printf("0x%x: size %d\n", p - 1, *(p - 1));

	return 0;
}

void* Alloc_overflow_check(int size)
{
	// 할당한 메모리 주소 앞 4B 영역에 size값을 넣을 거라서 실제 필요한 메모리는 size + 4 만큼이다
	int totSz = size + 4;
	int pageCnt = (totSz + USN_PAGE_SIZE - 1) / USN_PAGE_SIZE;

	// 필요 Page 수 + 1 만큼 Page를 할당
	void* p = VirtualAlloc(nullptr, USN_PAGE_SIZE * (pageCnt + 1), MEM_COMMIT, PAGE_READWRITE);

	// 마지막 Page는 접근 불가로 속성 변경
	DWORD oldProt;
	if (!VirtualProtect((void*)((int)p + USN_PAGE_SIZE * pageCnt), USN_PAGE_SIZE, PAGE_NOACCESS, &oldProt))
		printf("VirtualProtect() fail, error: %lu\n", GetLastError());

	*(int*)((int)p + USN_PAGE_SIZE * pageCnt - totSz) = size;

	return (void*)((int)p + USN_PAGE_SIZE * pageCnt - size);
}

bool Free_overflow_check(void* pt)
{
	// pt 앞 4B에 size가 있다
	int size = *((int*)pt - 1);
	void* p = (void*)((unsigned int)pt & ~0xffff);
	int totSz = size + 4;
	int pageCnt = (totSz + USN_PAGE_SIZE - 1) / USN_PAGE_SIZE;

	return VirtualFree(p, USN_PAGE_SIZE * (pageCnt + 1), MEM_DECOMMIT);
}

void* Alloc_underflow_check(int size)
{
	// 할당한 메모리 주소 앞 4B 영역에 size값을 넣을 거라서 실제 필요한 메모리는 size + 4 만큼이다
	int totSz = size + 4;
	int pageCnt = (totSz + USN_PAGE_SIZE - 1) / USN_PAGE_SIZE;

	// 필요 Page 수 + 1 만큼 Page를 할당
	void* p = VirtualAlloc(nullptr, USN_PAGE_SIZE * (pageCnt + 1), MEM_COMMIT, PAGE_READWRITE);

	// 첫 Page는 접근 불가로 속성 변경
	DWORD oldProt;
	if (!VirtualProtect(p, USN_PAGE_SIZE, PAGE_NOACCESS, &oldProt))
		printf("VirtualProtect() fail, error: %lu\n", GetLastError());

	*(int*)((int)p + USN_PAGE_SIZE) = size;

	return (void*)((int)p + USN_PAGE_SIZE + 4);
}

bool Free_underflow_check(void* pt)
{
	// pt 앞 4B에 size가 있다
	int* p = (int*)pt;
	--p;
	int size = *p;
	int totSz = size + 4;
	p -= 0x400; // 1 page 앞으로
	int pageCnt = (totSz + USN_PAGE_SIZE - 1) / USN_PAGE_SIZE;

	return VirtualFree(p, USN_PAGE_SIZE * (pageCnt + 1), MEM_DECOMMIT);
}
