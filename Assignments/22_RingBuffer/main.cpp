#include <iostream>
#include <vector>
#include <deque>
#include <random>

#include "RingBuffer.h"

using namespace std;

static random_device rd;
static mt19937 gen(rd());

// 랜덤 데이터 생성 함수
static void GenerateRandomData(vector<char>& outBuffer, int size)
{
	outBuffer.resize(size);
	uniform_int_distribution<> dis(0, 255);
	for (int i = 0; i < size; ++i)
		outBuffer[i] = static_cast<char>(dis(gen));
}

int main()
{
	// 1. 테스트 설정
	// 일부러 크기를 작게(128 바이트) 잡아서 포인터 회전(Wrap Around)이 자주 일어나게 함
	constexpr int BUFFER_CAPACITY = 1 << 7;
	RingBuffer ringBuffer(BUFFER_CAPACITY);

	// 정답지 (검증용 덱)
	deque<char> referenceBuffer;

	constexpr int TEST_ITERATIONS = 100'000'000;

	cout << "========================================\n";
	cout << "   RingBuffer Stress Test Start\n";
	cout << "========================================\n";

	for (int i = 0; i < TEST_ITERATIONS; ++i)
	{
		// 0: Enqueue (쓰기), 1: Dequeue (읽기)
		const int action = uniform_int_distribution<>(0, 1)(gen);

		// -----------------------------------------------------------
		// ACTION: Enqueue
		// -----------------------------------------------------------
		if (action == 0)
		{
			// 1 ~ 50 바이트 랜덤 크기 생성
			const int writeSize = uniform_int_distribution<>(1, 50)(gen);

			// 넣을 데이터 준비
			vector<char> inputData;
			GenerateRandomData(inputData, writeSize);

			// 링버퍼에 넣기
			const int written = ringBuffer.Enqueue(inputData.data(), writeSize);

			// 정답지(deque)에도 똑같이 넣기 (실제 들어가는 양만큼만)
			if (written > 0)
				referenceBuffer.insert(referenceBuffer.end(), inputData.begin(), inputData.begin() + written);

			// 검증 1: 사이즈가 일치하는가?
			if (ringBuffer.GetUsedSize() != referenceBuffer.size())
			{
				cout << "[ERROR] Size Mismatch after Enqueue!\n";
				cout << "RingBuffer: " << ringBuffer.GetUsedSize() << " / Ref: " << referenceBuffer.size() << endl;
				return -1;
			}
		}
		// -----------------------------------------------------------
		// ACTION: Dequeue
		// -----------------------------------------------------------
		else
		{
			// 1 ~ 50 바이트 랜덤 크기 읽기 시도
			int read_size = uniform_int_distribution<>(1, 50)(gen);

			// 링버퍼에서 읽어올 버퍼
			vector<char> outputData(read_size);

			// 링버퍼에서 읽기 (Peek 테스트도 겸하고 싶으면 여기서 Peek 후 Dequeue 해도 됨)
			const int read = ringBuffer.Peek(outputData.data(), read_size);

			if (read > 0)
			{
				// 정답지(deque)와 비교
				for (int k = 0; k < read; ++k)
				{
					const char refChar = referenceBuffer.front();
					referenceBuffer.pop_front();

					if (outputData[k] != refChar)
					{
						cout << "[ERROR] Data Integrity Failed!\n";
						cout << "Index: " << k << endl;
						cout << "Expected: " << (int)refChar << ", Actual: " << (int)outputData[k] << endl;
						return -1;
					}
				}

				const int removed = ringBuffer.Dequeue(outputData.data(), read_size);

				if (read != removed)
				{
					cout << "[ERROR] Peek한 크기와 Dequeue한 크기가 다릅니다!\n";
					return -1;
				}
			}

			// 검증 2: 사이즈가 일치하는가?
			if (ringBuffer.GetUsedSize() != referenceBuffer.size())
			{
				cout << "[ERROR] Size Mismatch after Dequeue!\n";
				cout << "RingBuffer: " << ringBuffer.GetUsedSize() << " / Ref: " << referenceBuffer.size() << endl;
				return -1;
			}
		}

		// 진행 상황 로그
		constexpr int FREQUENCY = TEST_ITERATIONS / 100;
		if ((i + 1) % FREQUENCY == 0)
			cout << "Iteration " << (i + 1) << " Passed... (Current Size: " << ringBuffer.GetUsedSize() << ")\n";
	}

	// -----------------------------------------------------------
	// Final check: 남은 데이터 모두 빼서 확인
	// -----------------------------------------------------------
	cout << "Drain remaining data...\n";
	while (ringBuffer.GetUsedSize() > 0)
	{
		char buf[10];
		const int read = ringBuffer.Dequeue(buf, 10);
		for (int k = 0; k < read; ++k)
		{
			const char refChar = referenceBuffer.front();
			referenceBuffer.pop_front();
			if (buf[k] != refChar)
			{
				cout << "[ERROR] Final Drain Failed!\n";
				return -1;
			}
		}
	}

	if (referenceBuffer.empty() && ringBuffer.GetUsedSize() == 0)
	{
		cout << "========================================\n";
		cout << "   SUCCESS: All Tests Passed!\n";
		cout << "========================================\n";
	}
	else
		cout << "[ERROR] Buffer not empty after drain\n";

	return 0;
}
