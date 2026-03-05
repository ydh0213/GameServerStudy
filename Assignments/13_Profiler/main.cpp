#include <iostream>
#include <windows.h>
#include <locale>
#include "Profiler.h"

// 1. RAII 클래스 방식을 테스트하는 함수
void TestClassProfile()
{
	// 스코프가 끝날 때 알아서 PRO_END가 호출됨
	Profile p(L"Test_Class_RAII");

	// 빠른 함수
	for (int i = 0; i < 100; i++);
}

// 2. 명시적 매크로 방식을 테스트하는 함수
void TestMacroProfile()
{
	PRO_BEGIN(L"Test_Macro_Explicit");

	// 중간 빠른 함수
	for (int i = 0; i < 10'000; i++);

	PRO_END(L"Test_Macro_Explicit");
}

// 3. 섞어서 사용 및 중첩 측정을 테스트하는 함수
void TestNestedProfile()
{
	// 함수 전체 측정
	Profile p(L"Test_Nested_Total");

	// 중간 느린 함수
	for (int i = 0; i < 100'000; i++);

	// 특정 구간만 별도로 측정
	PRO_BEGIN(L"Test_Nested_Inner");

	// 느린 함수
	for (int i = 0; i < 100'000; i++)
		for (int j = 0; j < 1'000; j++);

	PRO_END(L"Test_Nested_Inner");
}

int main()
{
	_wsetlocale(LC_ALL, L"korean");
	std::locale::global(std::locale("korean"));

	ProfileInit();

	std::wcout << L"프로파일링을 시작합니다..." << std::endl;

	for (int i = 0; i < 10; ++i)
	{
		TestClassProfile();
		TestMacroProfile();
		TestNestedProfile();
	}

	ProfileDataOutText(L"ProfileResult.txt");

	ProfileDataPrint();

	return 0;
}