#include <iostream>
#include <windows.h>
#include <process.h>

using namespace std;

// #define LOCK

volatile long g_lock = 0;
int g_sharedData = 0;

void Lock(volatile long* lock)
{
	while (true)
		if (InterlockedExchange(lock, 1) == 0)
			return;
}

void Unlock(volatile long* lock)
{
	InterlockedExchange(lock, 0);
}

unsigned __stdcall Worker(void* arg)
{
	int loopCount = *(int*)arg;

	for (int i = 0; i < loopCount; ++i)
	{
		Lock(&g_lock);
		// Critical Section Begin

		++g_sharedData;

		// Critical Section End
		Unlock(&g_lock);
	}

	return 0u;
}

unsigned __stdcall WorkerNoLock(void* arg)
{
	int loopCount = *(int*)arg;

	for (int i = 0; i < loopCount; ++i)
		++g_sharedData;

	return 0u;
}

int main()
{
	const int THREAD_COUNT = 17, LOOP_COUNT = 10'000'000;
	HANDLE hThreads[THREAD_COUNT];
	int loopCount = LOOP_COUNT;

#ifdef LOCK
	unsigned (__stdcall *SelectedWorker)(void*) = Worker;
	const char* ModeName = "Lock ON (SpinLock)";
#else
	unsigned(__stdcall * SelectedWorker)(void*) = WorkerNoLock;
	const char* ModeName = "Lock OFF (Data Race)";
#endif

	cout << ModeName << "\n";
	cout << "스레드 " << THREAD_COUNT << "개 생성 시작\n";

	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, SelectedWorker, &loopCount, 0, nullptr);

		if (hThreads[i] == 0)
		{
			cout << "스레드 " << i << " 생성 실패!\n";
			break;
		}
	}

	WaitForMultipleObjects(THREAD_COUNT, hThreads, TRUE, INFINITE);

	for (int i = 0; i < THREAD_COUNT; ++i)
		CloseHandle(hThreads[i]);

	cout << "최종값: " << g_sharedData << "\n기대값: " << THREAD_COUNT * LOOP_COUNT << "\n";

	return 0;
}
