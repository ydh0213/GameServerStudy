#include <iostream>
#include <Windows.h>
#include <process.h>

using namespace std;

volatile bool g_flag[2] = {false, false};
volatile int g_turn = -1;
volatile long g_count = 0;
int g_sharedData = 0;

unsigned __stdcall Worker0(void* arg)
{
	SetThreadDescription(GetCurrentThread(), L"Worker Thread 0");
	int loopCount = *(int*)arg;
	volatile bool flag[2] = {g_flag[0], g_flag[1]};
	volatile int turn = g_turn;

	for (int i = 0; i < loopCount; ++i)
	{
		g_flag[0] = true;
		g_turn = 0;

		for (;;)
		{
			if (!g_flag[1])
			{
				flag[0] = g_flag[0];
				flag[1] = g_flag[1];
				turn = g_turn;
				break;
			}

			if (g_turn != 0)
			{
				flag[0] = g_flag[0];
				flag[1] = g_flag[1];
				turn = g_turn;
				break;
			}
		}

		// 기대값: 0 → 1
		int count = InterlockedIncrement(&g_count);
		if (count != 1)
			DebugBreak();

		// Critical Section Begin

		++g_sharedData;

		// Critical Section End

		// 기대값: 1 → 0
		count = InterlockedDecrement(&g_count);
		if (count != 0)
			DebugBreak();

		g_flag[0] = false;
	}

	return 0u;
}

unsigned __stdcall Worker1(void* arg)
{
	SetThreadDescription(GetCurrentThread(), L"Worker Thread 1");
	int loopCount = *(int*)arg;
	volatile bool flag[2] = {g_flag[0], g_flag[1]};
	volatile int turn = g_turn;

	for (int i = 0; i < loopCount; ++i)
	{
		g_flag[1] = true;
		g_turn = 1;

		for (;;)
		{
			if (!g_flag[0])
			{
				flag[0] = g_flag[0];
				flag[1] = g_flag[1];
				turn = g_turn;
				break;
			}

			if (g_turn != 1)
			{
				flag[0] = g_flag[0];
				flag[1] = g_flag[1];
				turn = g_turn;
				break;
			}
		}

		// 기대값: 0 → 1
		int count = InterlockedIncrement(&g_count);
		if (count != 1)
			DebugBreak();

		// Critical Section Begin

		++g_sharedData;

		// Critical Section End

		// 기대값: 1 → 0
		count = InterlockedDecrement(&g_count);
		if (count != 0)
			DebugBreak();

		g_flag[1] = false;
	}

	return 0u;
}

int main()
{
	const int LOOP_COUNT = 100'000'000;
	int loopCount = LOOP_COUNT;
	HANDLE hThreads[2];

	hThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, Worker0, &loopCount, 0, nullptr);
	if (hThreads[0] == 0)
	{
		cout << "스레드 " << 0 << " 생성 실패!\n";
		return 1;
	}

	hThreads[1] = (HANDLE)_beginthreadex(nullptr, 0, Worker1, &loopCount, 0, nullptr);
	if (hThreads[1] == 0)
	{
		cout << "스레드 " << 1 << " 생성 실패!\n";
		return 1;
	}

	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);

	cout << "최종값: " << g_sharedData << "\n기대값: " << 2 * LOOP_COUNT << "\n";

	return 0;
}
