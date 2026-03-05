#pragma once
#include <Windows.h>
#include <cwchar>

// 프로파일링 활성화 토글
#define PROFILE

constexpr int MAX_PROFILES = 100;

struct PROFILE_SAMPLE
{
	long			lFlag;				// 프로파일의 사용 여부 (배열시에만)
	WCHAR			szName[64];			// 프로파일 샘플 이름

	LARGE_INTEGER	lStartTime;			// 프로파일 샘플 실행 시간

	__int64			iTotalTime;			// 전체 사용시간 카운터 Time (출력시 호출회수로 나누어 평균 구함)
	__int64			iMin[2];			// 최소 사용시간 카운터 Time (초단위로 계산하여 저장: [0] 가장 최소 [1] 다음 최소)
	__int64			iMax[2];			// 최대 사용시간 카운터 Time (초단위로 계산하여 저장: [0] 가장 최대 [1] 다음 최대)

	__int64			iCall;				// 누적 호출 횟수
};

void ProfileInit();

// 프로파일링 된 데이터를 모두 초기화
void ProfileReset();

void ProfileBegin(const WCHAR* szName);
void ProfileEnd(const WCHAR* szName);

// 프로파일링 된 데이터를 Text 파일로 출력
void ProfileDataOutText(const WCHAR* szFileName);

// 프로파일링 된 데이터를 콘솔 출력
void ProfileDataPrint();


#ifdef PROFILE
	#define PRO_BEGIN(TagName)	ProfileBegin(TagName)
	#define PRO_END(TagName)	ProfileEnd(TagName)
#else
	#define PRO_BEGIN(TagName)
	#define PRO_END(TagName)
#endif


class Profile
{
public:
	Profile(const WCHAR* tag) : _tag(tag)
	{
		PRO_BEGIN(tag);
	}

	~Profile()
	{
		PRO_END(_tag);
	}

	const WCHAR* _tag;
};