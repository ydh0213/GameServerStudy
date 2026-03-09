#include <iostream>
#include <thread>
#include <vector>

#include "my_new.h"

double* g_AbsoluteLeak = nullptr;

void WorkerThread()
{
	// 로그에 남지 않는 정상적인 할당 및 해제
	int* p = new int;
	delete p;

	// 해제 안 함: Leak Error
	double* leakPtr = new double[5];
}

int main()
{
	std::cout << "메모리 프로파일러 테스트 시작 (멀티스레드)...\n";

	g_AbsoluteLeak = new double[100];
	std::vector<std::thread> threads;

	for (int i = 0; i < 10; ++i)
		threads.emplace_back(WorkerThread);

	int* a = new int[10];
	delete a; // delete[] 를 안 썼으므로 ARRAY 에러!

	int* b = nullptr;
	delete b; // 할당한 적 없으므로 NOALLOC 에러!

	for (auto& th : threads)
		th.join();

	DumpMemoryLeaks();

	std::cout << "테스트 종료. txt 파일을 확인하세요!\n";

	return 0;
}
