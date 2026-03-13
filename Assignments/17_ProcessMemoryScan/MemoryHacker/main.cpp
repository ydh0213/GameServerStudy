#include <iostream>
#include <vector>
#include <windows.h>

using namespace std;

int main()
{
	DWORD ProcessID;
	int searchValue, newValue;

	cout << "타겟 PID 입력: ";
	cin >> ProcessID;

	cout << "찾을 값 입력 (ex. 1000): ";
	cin >> searchValue;

	cout << "변경할 값 입력 (ex. 9999): ";
	cin >> newValue;

	// 1. 특정 프로세스 핸들 얻음
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
	if (hProcess == nullptr)
	{
		cout << "프로세스 열기 실패! 권한이 부족하거나 PID가 틀렸습니다." << endl;
		return 1;
	}

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	unsigned char* currentAddr = (unsigned char*)systemInfo.lpMinimumApplicationAddress;
	unsigned char* maxAddr = (unsigned char*)systemInfo.lpMaximumApplicationAddress;

	MEMORY_BASIC_INFORMATION memBasicInfo;
	int foundCount = 0;

	cout << "\n메모리 스캔을 시작합니다..." << endl;

	// 5. lpMaximumApplicationAddress 를 벗어났다면 중단
	while (currentAddr < maxAddr)
	{
		// 2. lpMinimumApplicationAddress 메모리 시작주소 부터 VirtualQueryEx 로 정보 얻음
		// return된 size 는 쓰기된 크기 sizeof memBasicInfo 와 같다.
		SIZE_T size = VirtualQueryEx(hProcess, currentAddr, &memBasicInfo, sizeof memBasicInfo);

		if (size == 0)
			break;

		// 3. 얻은 메모리(페이지) 의 속성 개인메모리, 커밋 확인
		if (memBasicInfo.Type == MEM_PRIVATE && memBasicInfo.State == MEM_COMMIT)
		{
			// 3-1. RegionSize 메모리 확보
			vector<unsigned char> buffer(memBasicInfo.RegionSize);
			SIZE_T bytesRead;

			// 3-2. 확보한 메모리로 ReadProcessMemory 로 RegionSize 만큼 읽어들임
			if (ReadProcessMemory(hProcess, memBasicInfo.BaseAddress, buffer.data(), memBasicInfo.RegionSize,
			                      &bytesRead))
			{
				// 3-3. 읽어들인 메모리를 뒤져서 원하는 값 탐색

				for (size_t i = 0; i + sizeof searchValue <= bytesRead; i += sizeof searchValue)
				{
					int* pVal = (int*)(&buffer[i]);

					// 3-4. 원하는 값을 찾았다면, 해당 메모리의 위치를 BaseAddress 기준으로 메모리 계산
					if (*pVal == searchValue)
					{
						unsigned char* targetAddr = (unsigned char*)memBasicInfo.BaseAddress + i;
						cout << "[!] 값 발견! 가상 메모리 주소: " << (void*)targetAddr << endl;

						// 3-5. WriteProcessMemory 로 해당위치 값 변경
						SIZE_T bytesWritten;

						if (WriteProcessMemory(hProcess, targetAddr, &newValue, sizeof newValue, &bytesWritten))
						{
							cout << "    -> 값 변경 성공 (" << searchValue << " -> " << newValue << ")" << endl;
							++foundCount;
						}
						else
							cout << "    -> 값 변경 실패" << endl;
					}
				}
			}
		}

		// 4. 확인 메모리 포인터를 ResionSize 만큼 다음으로 이동
		currentAddr += memBasicInfo.RegionSize;
	}

	cout << "\n스캔 완료. 총 " << foundCount << "개의 메모리 주소를 변조했습니다." << endl;

	CloseHandle(hProcess);

	return 0;
}
