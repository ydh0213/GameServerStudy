#include "Profiler.h"
#include <iostream>
#include <fstream>
#include <iomanip>

PROFILE_SAMPLE g_ProfileSamples[MAX_PROFILES];
LARGE_INTEGER g_Frequency;

void ProfileInit()
{
	QueryPerformanceFrequency(&g_Frequency);
	ProfileReset();
}

void ProfileReset()
{
	for (int i = 0; i < MAX_PROFILES; ++i)
	{
		g_ProfileSamples[i].lFlag = 0;
		g_ProfileSamples[i].szName[0] = L'\0';
		g_ProfileSamples[i].lStartTime.QuadPart = 0;
		g_ProfileSamples[i].iTotalTime = 0;

		g_ProfileSamples[i].iMin[0] = LLONG_MAX;
		g_ProfileSamples[i].iMin[1] = LLONG_MAX;
		g_ProfileSamples[i].iMax[0] = 0;
		g_ProfileSamples[i].iMax[1] = 0;
		g_ProfileSamples[i].iCall = 0;
	}
}

PROFILE_SAMPLE* GetProfileSample(const WCHAR* szName)
{
	int emptyIndex = -1;
	for (int i = 0; i < MAX_PROFILES; ++i)
	{
		if (g_ProfileSamples[i].lFlag == 1 && wcscmp(g_ProfileSamples[i].szName, szName) == 0)
			return &g_ProfileSamples[i]; // 기존 샘플 반환

		if (g_ProfileSamples[i].lFlag == 0 && emptyIndex == -1)
			emptyIndex = i; // 첫 번째 빈 자리 기록
	}

	// 빈 자리가 있으면 새로 등록
	if (emptyIndex != -1)
	{
		g_ProfileSamples[emptyIndex].lFlag = 1;
		wcscpy_s(g_ProfileSamples[emptyIndex].szName, szName);
		return &g_ProfileSamples[emptyIndex];
	}

	return nullptr; // 배열이 꽉 참
}

void ProfileBegin(const WCHAR* szName)
{
	PROFILE_SAMPLE* pSample = GetProfileSample(szName);

	if (pSample)
		QueryPerformanceCounter(&pSample->lStartTime);
}

void ProfileEnd(const WCHAR* szName)
{
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	PROFILE_SAMPLE* pSample = GetProfileSample(szName);

	// 시작 시간이 기록되지 않았으면 무시
	if (!pSample || pSample->lStartTime.QuadPart == 0)
		return;

	__int64 elapsedTicks = endTime.QuadPart - pSample->lStartTime.QuadPart;
	__int64 microSec = (elapsedTicks * 1'000'000ll) / g_Frequency.QuadPart;

	// 데이터 갱신
	pSample->iTotalTime += microSec;
	pSample->iCall++;

	// 최소값 갱신
	if (pSample->iMin[0] > microSec)
	{
		pSample->iMin[1] = pSample->iMin[0];
		pSample->iMin[0] = microSec;
	}
	else if (pSample->iMin[1] > microSec)
		pSample->iMin[1] = microSec;

	// 최대값 갱신
	if (pSample->iMax[0] < microSec)
	{
		pSample->iMax[1] = pSample->iMax[0];
		pSample->iMax[0] = microSec;
	}
	else if (pSample->iMax[1] < microSec)
		pSample->iMax[1] = microSec;
}

void ProfileDataOutText(const WCHAR* szFileName)
{
	std::wofstream fout(szFileName);
	if (!fout || !fout.is_open())
	{
		std::wcerr << L"파일을 열 수 없습니다: " << szFileName << std::endl;
		return;
	}

	fout << L"---------------------------------------------------------------------------------------------------\n";
	fout << std::setw(25) << L"Name" << L" | "
		<< std::setw(17) << L"Average" << L" | "
		<< std::setw(17) << L"Min" << L" | "
		<< std::setw(17) << L"Max" << L" | "
		<< std::setw(10) << L"Call" << L" |\n";
	fout << L"---------------------------------------------------------------------------------------------------\n";

	for (int i = 0; i < MAX_PROFILES; ++i)
		if (g_ProfileSamples[i].lFlag == 1)
		{
			double average = 0.0;
			
			if (g_ProfileSamples[i].iCall)
			{
				double totalTime = g_ProfileSamples[i].iTotalTime;

				if (g_ProfileSamples[i].iCall > 2)
				{
					totalTime -= g_ProfileSamples[i].iMax[0] - g_ProfileSamples[i].iMin[0];

					if (g_ProfileSamples[i].iCall > 4)
						totalTime -= g_ProfileSamples[i].iMax[1] - g_ProfileSamples[i].iMin[1];
				}
				
				average = totalTime / g_ProfileSamples[i].iCall;
			}
			
			fout << std::setw(25) << g_ProfileSamples[i].szName << L" | "
				<< std::fixed << std::setprecision(4)
				<< std::setw(14) << average << L" ㎲ | "
				<< std::setw(14) << (double)g_ProfileSamples[i].iMin[0] << L" ㎲ | "
				<< std::setw(14) << (double)g_ProfileSamples[i].iMax[0] << L" ㎲ | "
				<< std::setw(10) << g_ProfileSamples[i].iCall << L" |\n";
		}

	fout << L"---------------------------------------------------------------------------------------------------\n";
	fout.close();
}

void ProfileDataPrint()
{
	std::wcout << L"---------------------------------------------------------------------------------------------------\n";
	std::wcout << std::setw(25) << L"Name" << L" | "
		<< std::setw(17) << L"Average" << L" | "
		<< std::setw(17) << L"Min" << L" | "
		<< std::setw(17) << L"Max" << L" | "
		<< std::setw(10) << L"Call" << L" |\n";
	std::wcout << L"---------------------------------------------------------------------------------------------------\n";

	for (int i = 0; i < MAX_PROFILES; ++i)
		if (g_ProfileSamples[i].lFlag == 1)
		{
			double average = 0.0;

			if (g_ProfileSamples[i].iCall)
			{
				double totalTime = g_ProfileSamples[i].iTotalTime;

				if (g_ProfileSamples[i].iCall > 2)
				{
					totalTime -= g_ProfileSamples[i].iMax[0] - g_ProfileSamples[i].iMin[0];

					if (g_ProfileSamples[i].iCall > 4)
						totalTime -= g_ProfileSamples[i].iMax[1] - g_ProfileSamples[i].iMin[1];
				}

				average = totalTime / g_ProfileSamples[i].iCall;
			}

			std::wcout << std::setw(25) << g_ProfileSamples[i].szName << L" | "
				<< std::fixed << std::setprecision(4)
				<< std::setw(14) << average << L" ㎲ | "
				<< std::setw(14) << (double)g_ProfileSamples[i].iMin[0] << L" ㎲ | "
				<< std::setw(14) << (double)g_ProfileSamples[i].iMax[0] << L" ㎲ | "
				<< std::setw(10) << g_ProfileSamples[i].iCall << L" |\n";
		}

	std::wcout << L"---------------------------------------------------------------------------------------------------\n";
}