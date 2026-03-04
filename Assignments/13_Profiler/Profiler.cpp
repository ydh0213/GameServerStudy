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

void ProfileEnd(const WCHAR* szName) {}

void ProfileDataOutText(const WCHAR* szFileName) {}